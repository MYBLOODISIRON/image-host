#include <string>
#include <memory>
#include <string.h>
#include "common.h"

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