#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>
#include <jsoncpp/json/json.h>
#include "common.h"
#include "MyReactor.h"
#include "db_pool.h"
#include "cache_pool.h"


int apiUploadInit(const std::string& dfs_path_client, const std::string& storage_web_server_ip, const std::string& storage_web_server_port, const std::string& shorturl_server_address, const std::string& access_token)
{
    s_dfs_path_client       =   dfs_path_client;
    s_storage_web_server_ip =   storage_web_server_ip;
    s_storage_web_server_port = storage_web_server_port;
    s_shorturl_server_address   =   shorturl_server_address;
    s_shorturl_server_access_token = access_token;

    return 0;
}


bool uploadFileToFastDfs(const std::string& file_path, std::string& fileid)
{
    bool ret {true};

    if(s_dfs_path_client.empty())
    {
        LOG_ERROR("s_dfs_path_client is empty");
        return false;
    }


    pid_t pid;  
    int pfd [2];

    if(pipe(pfd) < 0)   // 创建管道传递fileid
    {
        LOG_ERROR("pipe error");
        ret = false;
        goto END;
    }
    pid = fork();
    if(pid < 0)
    {
        LOG_ERROR("fork failed");
        ret = false;
        goto END;
    }
    else if(pid == 0)   // 子进程
    {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);    // 标准输出重定向到写管道
        execlp("fdfs_upload_file", "fdfs_upload_file", s_dfs_path_client.c_str(), file_path, nullptr);

        LOG_ERROR("execlp fdfs_uploac_file failed");
        close(pfd[1]);
    }
    else
    {
        close(pfd[1]);
        char buf [TEMP_BUF_MAX_LEN];
        read(pfd[0], buf, TEMP_BUF_MAX_LEN);

        TrimSpace(buf);
        if(strlen(buf) == 0)
        {
            LOG_ERROR("upload failed");
            ret = false;
            goto END;
        }
        LOG_INFO("fileid: %s", buf);

        wait(nullptr);
        close(pfd[0]);

        fileid = buf;
    }


    END:
        return ret;
}


bool getFullUrlByFileId(std::string& fileid, std::string& fdfs_file_url)
{
    if(s_storage_web_server_ip.empty())
    {
        LOG_ERROR("s_storage_web_server_ip is empty.");
        return false;
    }

    char *p {nullptr};
    char *q {nullptr};
    char* k {nullptr};

    char fdfs_file_stat_buf [TEMP_BUF_MAX_LEN];
    char fdfs_file_host_name    [HOST_NAME_LEN];

    pid_t pid;
    int pfd [2];
    if(pipe(pfd) < 0)
    {
        LOG_ERROR("pipe failed");
        return false;
    }
    pid = fork();
    if(pid < 0)
    {
        LOG_ERROR("fork failed");
        return false;
    }
    else if (pid == 0)
    {
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        char buf [TEMP_BUF_MAX_LEN];
        execlp("fdfs_file_info", "fdfs_file_info", s_dfs_path_client.c_str(), buf, nullptr);
        fileid = buf;

        LOG_ERROR("execlp fdfs_file_info error");
        close(pfd[1]);
    }
    else
    {
        close(pfd[1]);
        read(pfd[0], fdfs_file_stat_buf, TEMP_BUF_MAX_LEN);
        wait(nullptr);
        close(pfd[0]);

        //拼接上传文件的完整url地址--->http://host_name/group1/M00/00/00/D12313123232312.png
        p = strstr(fdfs_file_stat_buf, "source ip address: ");
        q = p + strlen("source ip address: ");
        k = strstr(q, "\n");

        strncpy(fdfs_file_host_name, q, k - q);
        fdfs_file_host_name[k - q] = '\0';


        LOG_INFO("host_name: %s, fdfs_file_host_name: %s", s_storage_web_server_ip.c_str(), fdfs_file_host_name);

        fdfs_file_url = "http://" + s_storage_web_server_ip + "/" + fileid;

        return true;
    }
}

bool storeFileInfo(DBConn *db_conn, CacheConn *cache_conn, char *user, char *filename, char *md5, long size, char *fileid, const char *fdfs_file_url)
{
    time_t now;
    char create_time[TIME_STRING_LEN];
    char suffix     [SUFFIX_LEN];
    char sql_cmd    [SQL_MAX_LEN];

    GetFileSuffix(filename, suffix);
    uint64_t sql_len = snprintf(sql_cmd, SQL_MAX_LEN, "insert into file_info(md5, file_id, url, size, type, count) values('%s', '%s', '%s', '%ld', '%s', %d);", md5, fileid, fdfs_file_url, size, suffix, 1);
    if(! db_conn->executeUpdate(sql_cmd, sql_len, false))
    {
        LOG_ERROR("execute update failed");
        return false;
    }

    now = time(nullptr);
    strftime(create_time, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S", localtime(&now));
    sql_len = snprintf(sql_cmd, SQL_MAX_LEN, "insert into user_file_list(user, md5, create_time, file_name, shared_status, pv) values ('%s', '%s', '%s', '%s',  %d, %d);", user, md5, create_time, filename, 0, 0);
    if(! db_conn->executeUpdate(sql_cmd, sql_len, false))
    {
        LOG_ERROR("execute update failed: %s", sql_cmd);
        return false;
    }

    return true;
}



int apiUpload(std::string& post_data, std::string& str_json)
{
    LOG_INFO("post data: %s", post_data.c_str());

    char suffix     [SUFFIX_LEN];
    std::string     fileid;
    std::string     fdfs_file_url;
    char boundary       [TEMP_BUF_MAX_LEN];

    char file_name      [128];
    char file_content_type  [128];
    char file_path          [128];
    char new_file_path      [128];

    char file_md5           [128];
    char file_size          [32];
    char user               [32];
    long fileszl = 0;


    char *begin = (char*)post_data.c_str();
    char *p1, *p2;
    Json::Value value;

    DBManager *db_manager { DBManager::getInstance() };
    DBConn *dbconn { db_manager->getConn("image_host_master") };
    AUTO_REL_DB_CONN(db_manager, dbconn);


    int ret {0};
    p1 = strstr(begin, "\r\n");
    if(p1 == nullptr)
    {
        LOG_ERROR("no boundary");
        ret = -1;
        goto END;
    }

    // 拷贝分界线
    strncpy(boundary, begin, p1 - begin);
    boundary[p1 - begin] = '\0';
    LOG_INFO("boundary is: %s", boundary);

    begin = p1 + 2;
    p2 = strstr(begin, "name=\"file_name\"");   //找到file_name字段
    if(p2 == nullptr)
    {
        LOG_ERROR("no file name");
        ret = -1;
        goto END;
    }
    p2 = strstr(begin, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(file_name, begin, p2 - begin);
    LOG_INFO("file_name: %s", file_name);


     // 查找文件类型file_content_type
    begin = p2 + 2;
    p2 = strstr(begin, "name=\"file_content_type\""); //
    if (!p2) {
        LOG_ERROR("wrong no file_content_type!");
        ret = -1;
        goto END;
    }
    p2 = strstr(p2, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(file_content_type, begin, p2 - begin);
    LOG_INFO("file_content_type: %s",  file_content_type);

 // 查找文件file_path
    begin = p2 + 2;
    p2 = strstr(begin, "name=\"file_path\""); //
    if (!p2) {
        LOG_ERROR("wrong no file_path!");
        ret = -1;
        goto END;
    }
    p2 = strstr(p2, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(file_path, begin, p2 - begin);
    LOG_INFO("file_path: %s",  file_path);

    // 查找文件file_md5
    begin = p2 + 2;
    p2 = strstr(begin, "name=\"file_md5\""); //
    if (!p2) {
        LOG_ERROR("wrong no file_md5!");
        ret = -1;
        goto END;
    }
    p2 = strstr(p2, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(file_md5, begin, p2 - begin);
    LOG_INFO("file_md5: %s", file_md5);

    // 查找文件file_size
    begin = p2 + 2;
    p2 = strstr(begin, "name=\"file_size\""); //
    if (!p2) {
        LOG_ERROR("wrong no file_size!");
        ret = -1;
        goto END;
    }
    p2 = strstr(p2, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(file_size, begin, p2 - begin);
    LOG_INFO("file_size: %s.", file_size);
    fileszl = strtol(file_size, NULL, 10); //字符串转long

    // 查找user
    begin = p2 + 2;
    p2 = strstr(begin, "name=\"user\""); //
    if (!p2) {
        LOG_ERROR("wrong no user!");
        ret = -1;
        goto END;
    }
    p2 = strstr(p2, "\r\n");
    p2 += 4;
    begin = p2;
    p2 = strstr(begin, "\r\n");
    strncpy(user, begin, p2 - begin);
    LOG_INFO("user:%s", user);


    //从文件名获取文件后缀
    //把临时文件 改成带后缀名文件
    // 获取文件名后缀
    GetFileSuffix(file_name, suffix); //  20230720-2.txt -> txt  mp4, jpg, png
    strcat(new_file_path, file_path); // /root/tmp/1/0045118901
    strcat(new_file_path, ".");  // /root/tmp/1/0045118901.
    strcat(new_file_path, suffix); // /root/tmp/1/0045118901.txt

    // 重命名 修改文件名  fastdfs 他需要带后缀的文件
    ret = rename(file_path, new_file_path);
    if(ret < 0)
    {
        LOG_ERROR("rename %s to %s failed", file_path, new_file_path);
        ret = -1;
        goto END;
    }

    // 存入fastdfs并得到文件的file_id
    LOG_INFO("uploadFileToFastDfs, file name: %s, new_file_path: %s.", file_name, new_file_path);
    if(uploadFileToFastDfs(new_file_path, fileid) == false)
    {
        LOG_ERROR("uploadFileToFastDfs failed. unlinking %s.", new_file_path);
        ret = unlink(new_file_path);
        if(ret != 0)
        {
            LOG_ERROR("unlink %s failed.", new_file_path);
        }
        ret = -1;
        goto END;
    }

    //删除本地临时存放的上传文件
    LOG_INFO("unlink: %s.", new_file_path);
    ret = unlink(new_file_path);
    if(ret != 0)
    {
        LOG_ERROR("unlink %s failed.", new_file_path);
    }


    // 获取完整的http url
    if(getFullUrlByFileId(fileid, fdfs_file_url) == false)
    {
        LOG_ERROR("getFullUrlByFileId failed.");
        ret = -1;
        goto END;
    }

    // 将该文件的FastDFS相关信息存入mysql中 
    if(storeFileInfo(dbconn, nullptr, user, file_name, file_md5, fileszl, (char*)fileid.c_str(), fdfs_file_url.c_str()) == false)
    {
        LOG_ERROR("storeFileInfo failed.");
        ret = -1;
        goto END;
    }

    ret = 0;
    value["code"] = 0;
    str_json = value.toStyledString();
    return ret;


    END:
        value["code"] = 1;
        str_json = value.toStyledString();
        return ret;
}