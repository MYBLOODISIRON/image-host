#pragma once
#include <string>
namespace url
{
    enum urlid
    {
        URLID_START = 0,

        URLID_REG = URLID_START,
        URLID_LOGIN,
        URLID_UPLOAD,
        URLID_MD5,

        URLID_END
    };


    urlid get_urlid(const std::string& url_s);
}