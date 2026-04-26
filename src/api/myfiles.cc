#include <jsoncpp/json/json.h>
#include <string.h>
#include "myfiles.h"
#include "common.h"
#include "MyReactor.h"
#include "db_pool.h"


int decodeCountJson(std::string& str_json, std::string& user_name, std::string& token)
{
    bool res;
    Json::Value root;
    Json::Reader jsonReader;

    res = jsonReader.parse(str_json, root);
    if(res == false)
    {
        LOG_ERROR("parse count json failed.");
        return -1;
    }

    user_name = root["user"].asString();

    if(root["token"].isNull())
    {
        LOG_ERROR("token is null.");
        return -1;
    }

    token = root["token"].asString();

    return 0;
}

int encodeCountJson(int ret, int total, std::string& str_json)
{
    Json::Value root;
    root["code"] = ret;
    if(ret == 0)
    {
        root["total"] = total;
    }

    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}


int handleUserFileCount(std::string& user_name, int& count)
{
    DBManager *db_manager = DBManager::getInstance();
    DBConn* connection = db_manager->getConn("image_host_slave");
    AUTO_REL_DB_CONN(db_manager, connection);

    std::string sql { FormatString("selece count(*) from user_file_list where user = '%s';", user_name.c_str()) };
    ResultSet* result = connection->executeQuery(sql.c_str(), sql.length());
    if(result && result->next())
    {
        count = result->getInt("count(*)");
        LOG_INFO("count is %d", count);
        delete result;
        return 0;
    }
    else
    {
        LOG_ERROR("get count failed.");
        return -1;
    }
    
}


int decodeFilesListJson(std::string& str_json, std::string& user_name, std::string& token, int& start, int& count)
{
    Json::Value root;
    Json::Reader reader;
    bool res { reader.parse(str_json, root) };
    if(res != true)
    {
        LOG_ERROR("decode file list json failed.");
        return -1;
    }


    if(root["user"].isNull())
    {
        LOG_ERROR("user is null");
        return -1;
    }
    user_name = root["user"].asString();

    if(root["token"].isNull())
    {
        LOG_ERROR("token is null.");
        return -1;
    }
    token = root["token"].asString();

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

int encodeGetFileListFailedJson(std::string &str_json) {
    Json::Value root;
    root["code"] = 1;
    
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}


int getUserFileList(const std::string cmd, std::string& user_name, int& start, int& count, std::string& str_json)
{
    LOG_INFO("get user file list.");

    int total_count {0};
    if(0 !=  handleUserFileCount(user_name, total_count))
    {
        LOG_ERROR("get user file list count failed.");
        Json::Value root;
        root["code"] = 1;
        Json::FastWriter writer;
        str_json = writer.write(root);
        return -1;
    }

    if(total_count == 0)
    {
        Json::Value root;
        root["code"] = 0;
        root["count"] = 0;
        root["total"] = 0;
        Json::FastWriter writer;
        str_json = writer.write(root);
        return 0;
    }

    std::string sql { FormatString("select user_file_list.*, file_info.size, file_info.type from file_info, user_file_list where user = '%s' and file_info.md5 = user_file_list.md5 limit %d, %d;", user_name.c_str(), start, count) };
    LOG_INFO("execute sql %s", sql.c_str());

    DBManager* db_manager = DBManager::getInstance();
    DBConn* connection = db_manager->getConn("image_host_slave");
    ResultSet* result = connection->executeQuery(sql.c_str(), sql.length());
    if(result)
    {
        Json::Value root;
        Json::Value files;
        root["code"] = 0;
        root["total"] = total_count;
        int file_index {0};

        while(result->next())
        {
            Json::Value file;
            file["user"] = result->getString("user");
            file["md5"] = result->getString("md5");
            file["create_time"] = result->getString("create_time");
            file["file_name"] = result->getString("file_name");
            file["share_status"] = result->getInt("shared_status");
            file["pv"] = result->getInt("pv");
            file["url"] = result->getString("url");
            file["size"] = result->getInt("size");
            file["type"] = result->getString("type");

            files[file_index] = file;
            file_index++;
        }

        root["files"] = files;
        root["count"] = file_index;
        Json::FastWriter writer;
        str_json = writer.write(root);
        LOG_INFO("str_json %s", str_json.c_str());
        delete result;
        return 0;
    }
    else
    {
        LOG_ERROR("execute query failed.");
        Json::Value root;
        root["code"] = 1;
        Json::FastWriter writer;
        str_json = writer.write(root);
        return -1;
    }
}


int apiMyfiles(std::string& url, std::string& post_data, std::string& str_json)
{
    char cmd [20];
    QueryParseKeyValue(url.c_str(), "cmd", cmd, nullptr);  // 解析url参数


    std::string user_name;
    std::string token;

    if(strcmp(cmd, "count") == 0)   // 获取文件个数， api/myfiles&cmd=count
    {

        if(decodeCountJson(post_data, user_name, token) != 0)
        {
            encodeCountJson(1, 0, str_json);
            LOG_ERROR("decodeCountJson failed.");
            return -1;
        }

        if(0 != VerifyToken(user_name, token))
        {
            encodeCountJson(1, 0, str_json);
            LOG_ERROR("VerifyToken failed.");
            return -1;
        }

        int total_count {0};
        if(handleUserFileCount(user_name, total_count) != 0)
        {
            encodeCountJson(1, 0, str_json);
        }
        else
        {
            encodeCountJson(0, total_count, str_json);
        }

        return 0;

    }
    else if(strcmp(cmd, "normal") == 0) // 获取文件列表， api/myfiles&cmd=normal
    {

        int start, count;

        if(decodeFilesListJson(post_data, user_name, token, start, count) != 0)
        {
            encodeGetFileListFailedJson(str_json);
            LOG_ERROR("decode file list failed.");
            return -1;
        }

        if(VerifyToken(user_name, token) != 0)
        {
            encodeGetFileListFailedJson(str_json);
            LOG_ERROR("verify token failed.");
            return -1;
        }

        getUserFileList(cmd, user_name, start, count, str_json);
        return 0;
    }
    else
    {
        encodeGetFileListFailedJson(str_json);
        LOG_ERROR("invalid command.");
        return -1;
    }
}


