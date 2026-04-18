#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "MyReactor.h"


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
    return 0;
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