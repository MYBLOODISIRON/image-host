#include <jsoncpp/json/json.h>
#include <string.h>
#include "sharepicture.h"
#include "MyReactor.h"
#include "common.h"
#include "db_pool.h"


int decodeSharePictureJson(std::string& str_json, std::string& user_name, std::string& token, std::string& md5, std::string& filename)
{
    Json::Value root;
    Json::Reader reader;
    if(reader.parse(str_json, root) == false)
    {
        LOG_ERROR("parse share picture json failed.");
        return -1;
    }

    if(root["tokeen"].isNull())
    {
        LOG_ERROR("token is null");
        return -1;
    }
    token = root["token"].asString();

    if(root["user"].isNull())
    {
        LOG_ERROR("user is null.");
        return -1;
    }
    user_name = root["user"].asString();

    if(root["md5"].isNull())
    {
        LOG_ERROR("md5 is null.");
        return -1;
    }
    md5 = root["md5"].asString();

    if (root["filename"].isNull()) 
    {
        LOG_ERROR("file name is null.");
        return -1;
    }
    filename = root["filename"].asString();

    return 0;

}


int encodeSharePictureJson(int ret, std::string& urlmd5, std::string& str_json)
{
    Json::Value root;
    root["code"] = ret;
    if(HTTP_RESP_OK == ret)
    {
        root["urlmd5"] = urlmd5;
    }
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}


int handleSharePicture(const char* user, const char* filemd5, const char* file_name, std::string& str_json)
{
    DBManager* db_manager = DBManager::getInstance();
    DBConn* connection = db_manager->getConn("image_host_slave");
    AUTO_REL_DB_CONN(db_manager, connection);

    std::string urlmd5;
    urlmd5 = RandomString(32);

    char create_time [TIME_STRING_LEN];
    time_t now { time(nullptr) };
    strftime(create_time, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::string key {""};
    std::string sql { FormatString("insert into share_picture_list(user, filemd5, file_name, urlmd5, `key`, pv, create_time) value('%s', '%s', '%s', '%s', '%s', %d, '%s');", user, filemd5, file_name, urlmd5.c_str(), key.c_str(), 0, create_time) };
    LOG_INFO("execute sql %s", sql.c_str());

    int retval {0};
    if(! connection->executeUpdate(sql.c_str(), sql.length()))
    {
        LOG_ERROR("execute sql failed.");
        retval = -1;
    }
    else
    {
        retval = 0;
    }

    if(retval == 0)
    {
        encodeSharePictureJson(HTTP_RESP_OK, urlmd5, str_json);
    }
    else
    {
        encodeSharePictureJson(HTTP_RESP_FAIL, urlmd5, str_json);
    }

    return retval;
}



int decodeBrowsePictureJson(std::string&str_json, std::string& urlmd5)
{
    Json::Value root;
    Json::Reader reader;
    if(false == reader.parse(str_json, root))
    {
        LOG_ERROR("parse browse pictire failed.");
        return -1;
    }

    if(root[urlmd5].isNull())
    {
        LOG_ERROR("urlmd5 is null.");
        return -1;
    }

    urlmd5 = root["urlmd5"].asString();
    return 0;
}


int encodeBrowsePictureJson(int ret, int pv, std::string& url, std::string user, std::string& time, std::string& str_json)
{
    Json::Value root;
    root["code"] = ret;
    if(ret == 0)
    {
        root["pv"] = pv;
        root["url"] = url;
        root["user"] = user;
        root["time"] = time;
    }

    Json::FastWriter writer;
    str_json = writer.write(root);

    return 0;
}



int handleBrowsePicture(const char* urlmd5, std::string& str_json)
{

    int retval {0};

    DBManager* db_manager = DBManager::getInstance();
    DBConn* connection = db_manager->getConn("image_host_master");
    AUTO_REL_DB_CONN(db_manager, connection);

    std::string sql { FormatString("selece user, filemd5, file_name, pv, create_time from share_picture_list where urlmd5 = '%s';", urlmd5) };
    LOG_INFO("execute sql: %s.", sql.c_str());

    std::string     picture_url;
    std::string     file_name;
    std::string     user;
    std::string     filemd5;
    std::string     create_time;
    int pv {0};

    ResultSet* result = connection->executeQuery(sql.c_str(), sql.length());
    if(result && result->next())
    {
        user = result->getString("user");
        filemd5 = result->getString("urlmd5");
        file_name = result->getString("file_name");
        pv = result->getInt("pv");
        create_time = result->getString("create_time");
        delete result;
    }
    else
    {
        if(result)
        {
            delete result;
        }
        retval = -1;
        goto END;
    }


    // 根据MD5查询对应的url
    sql = FormatString("select url from file_info where md5 = '%s';", filemd5.c_str());
    LOG_INFO("execute sql: %s.", sql.c_str());
    result = connection->executeQuery(sql.c_str(), sql.length());

    if(result && result->next())
    {
        picture_url = result->getString("url");
        delete result;
    }
    else
    {
        if(result)
        {
            delete result;
        }
        retval = -1;
        goto END;
    }


    // 更新访问计数pv
    pv += 1;
    sql = FormatString("update share_picture_list set pv = %d where urlmd5 = '%s';", pv, urlmd5);
    LOG_INFO("execute sql: %s.", sql.c_str());
    if(! connection->executeUpdate(sql.c_str(), sql.length()))
    {
        LOG_ERROR("execute sql:%s failed.", sql.c_str());
        retval = -1;
        goto END;
    }
    retval = 0;
    
END:
    if(retval == 0)
    {
        encodeBrowsePictureJson(HTTP_RESP_OK, pv, picture_url, user, create_time, str_json);
    }
    else
    {
        encodeBrowsePictureJson(HTTP_RESP_FAIL, pv, picture_url, user, create_time, str_json);
    }
    return retval;
}


int decodePictureListJson(std::string& str_json, std::string& user_name, std::string& token, int& start, int& count)
{
    Json::Value root;
    Json::Reader reader;
    if(reader.parse(str_json, root) == false)
    {
        LOG_ERROR("parse picture list failed.");
        return -1;
    }


    if(root["token"].isNull())
    {
        LOG_ERROR("token is null.");
        return -1;
    }
    token = root["token"].asString();

    if(root["user"].isNull())
    {
        LOG_ERROR("user is null.");
        return -1;
    }
    user_name = root["user"].asString();

    if(root["start"].isNull())
    {
        LOG_ERROR("start is null.");
        return -1;
    }
    start = root["start"].asInt();

    if(root["count"].isNull())
    {
        LOG_ERROR("count is null.");
        return -1;
    }
    count = root["count"].asInt();

    return 0;
}


int getSharedPictureCount(DBConn* connection, std::string& user_name, int& count)
{
    if(getSharedPictureCountByUsername(connection, user_name, count) < 0)
    {
        LOG_ERROR("get shared picture count failed.");
        return -1;
    }

    return 0;
}


void handleGetSharePictureList(const char* user, int start, int count, std::string& str_json)
{
    int ret {0};
    Json::Value root;
    int total {0};
    std::string user_temp = user;

    DBManager* db_manager = DBManager::getInstance();
    DBConn* connection = db_manager->getConn("image_host_slave");
    AUTO_REL_DB_CONN(db_manager, connection);
    std::string sql {FormatString(
        "select share_picture_list.user, share_picture_list.filemd5, share_picture_list.file_name, share_pciture_list.urlmd5, share_picture_list.pv, share_picture_list.create_time, file_info.size"
        "from file_info, share_picture_list where share_picture_list.user = '%s' and file_info.md5 = share_picture_list.filemd5 limit %d, %d;",
        user, start, count
    )};
    int file_count {0};
    ResultSet* result = nullptr;
    ret = getSharedPictureCount(connection, user_temp, total);
    if(ret != 0)
    {
        LOG_ERROR("get share picture list failed.");
        ret = -1;
        goto END;
    }

    
    LOG_INFO("execute sql: %s.", sql.c_str());


    result = connection->executeQuery(sql.c_str(), sql.length());
    if(result)
    {
        Json::Value files;
        while(result->next())
        {
            Json::Value file;
            file["user"]    =   result->getString("user");
            file["filemd5"] =   result->getString("filemd5");
            file["file_name"]   =   result->getString("file_name");
            file["urlmd5"]  =   result->getString("urlmd5");
            file["pv"]  =   result->getInt("pv");
            file["create_time"] =   result->getString("create_time");
            file["size"]    =   result->getInt("size");
            files[file_count] = file;
            file_count ++;
        }
        if(file_count > 0)
        {
            root["files"] = files;
        }
        ret = 0;
        delete result;
    }
    else
    {
        ret = -1;
    }


    END:
    if(ret != 0)
    {
        Json::Value root;
        root["code"] = 1;
    }
    else
    {
        root["code"] = 0;
        root["count"] = file_count;
        root["total"] = total;
    }
    str_json = root.toStyledString();

    return;
}


int apiSharePicture(std::string& url, std::string& post_data, std::string& str_json)
{
    char cmd [20];  // url携带的命令
    QueryParseKeyValue(url.c_str(), "cmd", cmd, nullptr);
    LOG_INFO("command is: %s.", cmd);

    int retval {0};
    std::string     user_name;
    std::string     md5;
    std::string     urlmd5;
    std::string     filename;
    std::string     token;

    if(strcmp(cmd, "share") == 0)   // 分享文件
    {
        retval = decodeSharePictureJson(post_data, user_name, token, md5, filename);
        if(retval == 0)
        {
            handleSharePicture(user_name.c_str(), md5.c_str(), filename.c_str(), str_json);
        }
        else
        {
            encodeSharePictureJson(HTTP_RESP_FAIL, urlmd5, str_json);
        }
    }
    else if(strcmp(cmd, "browse") == 0) // 浏览文件
    {
        retval = decodeBrowsePictureJson(post_data, urlmd5);

        if(retval == 0)
        {
            handleBrowsePicture(urlmd5.c_str(), str_json);
        }
        else
        {
            encodeSharePictureJson(HTTP_RESP_FAIL, urlmd5, str_json);
        }
    }
    else if(strcmp(cmd, "normal") == 0)
    {
        int start {0};
        int count {0};

        retval = decodePictureListJson(post_data, user_name, token, start, count);

        if(retval == 0)
        {
            handleGetSharePictureList(user_name.c_str(), start, count, str_json);
        }
        else
        {
            encodeSharePictureJson(HTTP_RESP_FAIL, urlmd5, str_json);
        }
    }
    else
    {
        LOG_ERROR("invalid command.");
    }

    return retval;
}