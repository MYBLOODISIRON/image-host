#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include <uuid/uuid.h>
#include "MyReactor.h"
#include "db_pool.h"
#include "common.h"
#include "cache_pool.h"


std::string generateUUID()
{
    uuid_t uuid;
    uuid_generate_time_safe(uuid);

    char str_uuid [40];
    uuid_unparse(uuid, str_uuid);

    return std::string {str_uuid};
}


int decodeLoginJson(const std::string& str_json, std::string& user_name, std::string& pwd)
{
    // decode recive json data
    // 传入str_json，解析值由user_name和pwd带出
    // return value: 0 on success, -1 on error.
    bool res;
    Json::Value root;
    Json::Reader jsonReader;

    res = jsonReader.parse(str_json, root);

    if(! res)
    {
        LOG_ERROR("login: parse json failed.\n");
        return -1;
    }

    if(root["user"].isNull())
    {
        LOG_ERROR("user is null.\n");
        return -1;
    }
    user_name = root["user"].asString();

    if(root["pwd"].isNull())
    {
        LOG_ERROR("pwd is null.\n");
        return -1;
    }
    pwd = root["pwd"].asString();

    return 0;

}


int encodeLoginJson(int code, const std::string& token, std::string& str_json)
{
    // encode send json data
    // 传入code和token，编码值由str_json带出
    // return value: 0 on success.
    Json::Value root;
    root["code"] = code;
    if(code == 0)
    {
        root["token"] = token;  // 返回正常时写入token
    }

    Json::FastWriter writer;
    str_json = writer.write(root);

    return 0;
}


bool verifyUserPassword(std::string& uname, std::string& pwd)
{
    bool ret {false};

    DBManager *db_manager { DBManager::getInstance() };
    DBConn *db_connection { db_manager->getConn("image_host_slave") };
    AUTO_REL_DB_CONN(db_manager, db_connection);

    std::string sql = FormatString("select password from user_info where user_name = '%s';", uname.c_str());
    ResultSet *result { db_connection->executeQuery(sql.c_str(), sql.length()) };
    if(result != nullptr && result->next())
    {
        std::string password = result->getString("password");
        LOG_INFO("pwd in db: %s, pwd from user: %s.", password.c_str(), pwd.c_str());
        if(password == pwd)
        {
            ret = true;
        }
        else
        {
            ret = false;
        }   
    }
    else    // 用户不存在
    {
        ret = false;
    }
    delete result;
    return ret;
}


bool setToken(std::string& uname, std::string& token)
{
    bool retval {true};
    CacheManager *cache_manager = CacheManager::getInstance();
    CacheConn *cache_connection = cache_manager->GetCacheConn("token");
    AUTO_REL_CACHECONN(cache_manager, cache_connection);

    token = generateUUID();
    if(cache_connection != nullptr)
    {
        cache_connection->SetEx(token, 86400, uname);   // token - uname键值对，24小时有效
    }
    else
    {
        retval = false;
    }

    return retval;
}


int apiUserLogin(const std::string& post_data, std::string& resp_json)
{
    std::string uname;
    std::string pwd;
    std::string token;

    if(post_data.empty())
    {
        encodeLoginJson(1, token, resp_json);
        return -1;
    }

    if(decodeLoginJson(post_data, uname, pwd))
    {
        LOG_INFO("decode regist json data failed.\n");
        encodeLoginJson(1, token, resp_json);
        return -1;
    }

    if(verifyUserPassword(uname, pwd))
    {
        LOG_ERROR("verify user password failed.\n");
        encodeLoginJson(1, token, resp_json);
        return -1;
    }

    // 生成token

    if(setToken(uname, token))
    {
        LOG_ERROR("set token failed.\n");
        encodeLoginJson(0, token, resp_json);
        return -1;
    }

    encodeLoginJson(0, token, resp_json);
    return 0;
}