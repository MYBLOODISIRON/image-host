#pragma once
#include <string>
#define SQL_MAX_LEN (512) // sql语句长度


#define FILE_NAME_LEN (256)    //文件名字长度
#define TEMP_BUF_MAX_LEN (512) //临时缓冲区大小
#define FILE_URL_LEN (512)     //文件所存放storage的host_name长度
#define HOST_NAME_LEN (30)     //主机ip地址长度
#define USER_NAME_LEN (128)    //用户名字长度
#define TOKEN_LEN (128)        //登陆token长度
#define MD5_LEN (256)          //文件md5长度
#define PWD_LEN (256)          //密码长度
#define TIME_STRING_LEN (25)   //时间戳长度
#define SUFFIX_LEN (8)         //后缀名长度
#define PIC_NAME_LEN (10)      //图片资源名字长度
#define PIC_URL_LEN (256)      //图片资源url名字长度

#define HTTP_RESP_OK 0
#define HTTP_RESP_FAIL 1           //
#define HTTP_RESP_USER_EXIST 2     // 用户存在
#define HTTP_RESP_DEALFILE_EXIST 3 // 别人已经分享此文件
#define HTTP_RESP_TOKEN_ERR 4      //  token验证失败
#define HTTP_RESP_FILE_EXIST 5     //个人已经存储了该文件




//redis key相关定义
#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_SERVER_PORT "6379"



extern std::string s_dfs_path_client;
extern std::string s_storage_web_server_ip;
extern std::string s_storage_web_server_port;
extern std::string s_shorturl_server_address;
extern std::string s_shorturl_server_access_token;

template <typename... Args>
std::string FormatString(const std::string& format, Args... args)
{
    auto size { snprintf(nullptr, 0, format.c_str(), args...) + 1 };
    std::unique_ptr<char[]> buf {new char [size] };

    snprintf(buf.get(), size, format.c_str(), args...);

    return std::string(buf.get(), buf.get() + size - 1);
};


int TrimSpace(char *inbuf);

int GetFileSuffix(const char *file_name, char *suffix);

int QueryParseKeyValue(const char *query, const char *key, char *value, int *value_len_p);


//验证登陆token，成功返回0，失败-1
int VerifyToken(std::string &user_name, std::string &token);