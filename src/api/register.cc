#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <time.h>
#include "MyReactor.h"
#include "db_pool.h"
#include "common.h"


int decodeRegistJson(const std::string& str_json, std::string& uname, std::string& nick_name, std::string& pwd, std::string& phone, std::string& email)
{
    // 传入str_json，其余传出
    // return value: 0 on success, other on error

    Json::Value root;
    Json::Reader reader;
    bool res { reader.parse(str_json, root) };

    if(! res)
    {
        LOG_ERROR("parse regist json data failed.\n");
        return -1;
    }

    if(root["userName"].isNull())
    {
        LOG_ERROR("user name is null.\n");
        return -1;
    }
    uname = root["userName"].asString();

    if(root["nickName"].isNull())
    {
        LOG_ERROR("nick name is null.\n");
        return -1;
    }
    nick_name = root["nickName"].asString();

    if(root["firstPwd"].isNull()){
        LOG_ERROR("first password is null.\n");
        return -1;
    }
    pwd = root["firstPwd"].asString(); 

    if(! root["phone"].isNull())    // 非必要
    {
        phone = root["phone"].asString();
    }

    if(! root["email"].isNull())    // 非必要
    {
        email = root["email"].asString();
    }

    return 0;
}


int encodeRegistJson(int code, std::string& str_json)
{
    Json::Value root;
    root["code"] = code;

    Json::FastWriter writer;
    str_json = writer.write(root);

    return 0;
}


int registUser(const std::string& uname, const std::string& nick_name, const std::string& pwd, const std::string& phone, const std::string& email)
{
    // return value: =0 success, =2 user has exist, =1 exception.
    int ret {0};
    uint32_t user_id {0};
    
    DBManager* db_manager = DBManager::getInstance();
    DBConn* db_connection = db_manager->getConn("image_host_master");
    AUTO_REL_DB_CONN(db_manager, db_connection);    // 栈上对象自动释放
    if(db_connection == nullptr)
    {
        LOG_ERROR("get db connection failed.");
        return 1;
    }
    char tmp [256];
    snprintf(tmp, 256, "select id from user_info where user_name = '%s';", uname.c_str());

    std::string sql { tmp }; // 查找用户是否已经存在
    ResultSet* result_set = db_connection->executeQuery(sql.c_str(), sql.length());
    if(result_set != nullptr && result_set->next())
    {   // 用户已存在
        ret = 2;
    }
    else
    {   // 用户不存在

        time_t regist_time;
        regist_time = time(nullptr);
        char regist_time_fstr[TIME_STRING_LEN];
        strftime(regist_time_fstr, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S", localtime(&regist_time));

        sql =   "insert into user_info"
                "(`usr_name`, `nick_name`, `password`, `phone`, `email`, `create_time`)"
                "values(?,?,?,?,?,?);";
        
        LOG_INFO("execute sql: %s", sql.c_str());
        PrepareStatement* stmt = new PrepareStatement {};
        if(stmt->init(db_connection->mysql(), sql))
        {
            uint32_t index {0};
            stmt->setParam(index ++, uname);
            stmt->setParam(index ++, nick_name);
            stmt->setParam(index ++, pwd);
            stmt->setParam(index ++, phone);
            stmt->setParam(index ++, email);
            stmt->setParam(index ++, regist_time_fstr);
            if(stmt->executeUpdate())
            {
                ret = 0;
                user_id = db_connection->getInsertId();
                LOG_INFO("insert table user_info success. user_id: %u, user_name: %s.", user_id, uname.c_str());
            }
            else
            {
                LOG_ERROR("insert table user_info failed.");
                ret = 1;
            }
        }
        delete stmt;
    }

    return ret;
}


int apiRegistUser(const std::string& post_data, std::string& resp_json)
{
    if(post_data.empty())
    {
        LOG_ERROR("post data is empty.\n");
        encodeRegistJson(1, resp_json);
        return -1;
    }

    std::string uname, nick_name, pwd, phone, email;
    int ret = decodeRegistJson(post_data, uname, nick_name, pwd, phone, email);
    if(ret != 0)
    {
        LOG_ERROR("decode regist json data failed.\n");
        encodeRegistJson(1, resp_json);
        return -1;
    }

    ret = registUser(uname, nick_name, pwd, phone, email);
    if(ret != 0)
    {

    }

    encodeRegistJson(0, resp_json);
    return 0;
}