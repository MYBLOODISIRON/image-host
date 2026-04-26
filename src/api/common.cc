#include <string>
#include <memory>
#include <string.h>
#include "common.h"
#include "cache_pool.h"

std::string s_dfs_path_client;
std::string s_storage_web_server_ip;
std::string s_storage_web_server_port;
std::string s_shorturl_server_address;
std::string s_shorturl_server_access_token;






int TrimSpace(char* inbuf)
{
    int i {0};
    int j{(int) strlen(inbuf) - 1};
    char *str = inbuf;

    int count {0};
    if(str == nullptr)
    {
        return -1;
    }

    while( isspace(str[i]) && str[i] != '\0')
    {
        i++;
    }

    while( j > i && isspace(str[j]))
    {
        j --;
    }

    count = j - i + 1;
    strncpy(inbuf, str + i, count);
    inbuf[count] = '\0';
    return 0;
}



//通过文件名file_name， 得到文件后缀字符串, 保存在suffix
//如果非法文件后缀,返回"null"
int GetFileSuffix(const char *file_name, char *suffix) {
    const char *p = file_name;
    int len = 0;
    const char *q = NULL;
    const char *k = NULL;

    if (p == NULL) {
        return -1;
    }

    q = p;

    // mike.doc.png
    //              ↑

    while (*q != '\0') {
        q++;
    }

    k = q;
    while (*k != '.' && k != p) {
        k--;
    }

    if (*k == '.') {
        k++;
        len = q - k;

        if (len != 0) {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        } else {
            strncpy(suffix, "null", 5);
        }
    } else {
        strncpy(suffix, "null", 5);
    }

    return 0;
}


/**
 * @brief  解析url query 类似 abc=123&bbb=456 字符串
 *          传入一个key,得到相应的value
 * @returns
 *          0 成功, -1 失败
 */
int QueryParseKeyValue(const char *query, const char *key, char *value,
                       int *value_len_p) {
    char *temp = NULL;
    char *end = NULL;
    int value_len = 0;

    //找到是否有key
    temp = (char *)strstr(query, key);
    if (temp == NULL) {
        return -1;
    }

    temp += strlen(key); //=
    temp++;              // value

    // get value
    end = temp;

    while ('\0' != *end && '#' != *end && '&' != *end) {
        end++;
    }

    value_len = end - temp;

    strncpy(value, temp, value_len);
    value[value_len] = '\0';

    if (value_len_p != NULL) {
        *value_len_p = value_len;
    }

    return 0;
}



//验证登陆token，成功返回0，失败-1
int VerifyToken(std::string &user_name, std::string &token) {
    int ret = 0;
    CacheManager *cache_manager = CacheManager::getInstance();
    CacheConn *cache_conn = cache_manager->GetCacheConn("token");
    AUTO_REL_CACHECONN(cache_manager, cache_conn);

    if (cache_conn) {
        std::string temp_user_name  = cache_conn->Get(token);    //校验token和用户名的关系
        if (temp_user_name == user_name) {
            ret = 0;
        } else {
            ret = -1;
        }
    } else {
        ret = -1;
    }

    return ret;
}


std::string RandomString(const int len)
{
    std::string str;
    char c;
    int index;
    for(index = 0; index < len; index ++)
    {
        c = 'a' + rand() % 26;
        str.push_back(c);
    }

    return str;
}


int getSharedPictureCountByUsername(DBConn* connection, std::string& user_name, int& count)
{
    count = 0;
    int ret {0};
    std::string sql {FormatString("selece count(*) from share_picture_list where user = '%s';", user_name.c_str())};
    
    ResultSet *result = connection->executeQuery(sql.c_str(), sql.length());
    if(result && result->next())
    {
        count = result->getInt("count(*)");
        ret = 0;
        delete result;
    }
    else if(! result)
    {
        ret = -1;
    }
    else
    {
        ret = 0;
        delete result;
    }

    return ret;
}