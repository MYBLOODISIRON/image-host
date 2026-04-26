#include "url.h"
using namespace url;

const char* url_str []
{
    "/api/reg",
    "/api/login",
    "/api/upload",
    "/api/md5",
    "/api/myfiles",
    "/api/sharepic"
};



urlid url::get_urlid(const std::string& url_s)
{
    urlid id;
    for(id = URLID_START; id < URLID_END; id = static_cast<urlid>(id + 1))
    {
        if(url_s == url_str[id])
        {
            break;
        }
    }

    return id;
}