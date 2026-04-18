#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>
#include "MyReactor.h"

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


int verifyUserPassword(std::string& uname, std::string& token)
{

}


int setToken(std::string& uname, std::string& token)
{

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