#include <string>
#include <iostream>
#include "HttpConnection.h"
#include "MyReactor.h"
#include "url.h"
#include "api.h"



#define HTTP_RESPONSE_JSON_MAX 4096
#define HTTP_RESPONSE_JSON                                                     \
    "HTTP/1.1 200 OK\r\n"                                                      \
    "Connection:close\r\n"                                                     \
    "Content-Length:%d\r\n"                                                    \
    "Content-Type:application/json;charset=utf-8\r\n\r\n%s"

#define HTTP_RESPONSE_HTML                                                    \
    "HTTP/1.1 200 OK\r\n"                                                      \
    "Connection:close\r\n"                                                     \
    "Content-Length:%d\r\n"                                                    \
    "Content-Type:text/html;charset=utf-8\r\n\r\n%s"


#define HTTP_RESPONSE_BAD_REQ                                                     \
    "HTTP/1.1 400 Bad\r\n"                                                      \
    "Connection:close\r\n"                                                     \
    "Content-Length:%d\r\n"                                                    \
    "Content-Type:application/json;charset=utf-8\r\n\r\n%s"


HttpConnection::HttpConnection(const TcpConnectionPtr& conn)
:   m_tcp_conn {conn}
{
    m_uuid = std::any_cast<uint32_t>( conn->getContext());
    LOG_INFO("construct HttpConnection:%u.\n", m_uuid);
}


HttpConnection::~HttpConnection()
{
    LOG_INFO("destruct HttpConnection:%u.\n", m_uuid);
}


void HttpConnection::onRead(Buffer* buf)
{
    const char* in_buffer { buf->peek() };
    int32_t len { buf->readableBytes() };

    m_http_parser.ParseHttpContent(in_buffer, len);
    if(m_http_parser.IsReadAll())
    {
        std::string url {m_http_parser.GetUrlString()};
        std::string content {m_http_parser.GetBodyContentString()};

        url::urlid id { url::get_urlid(url) };
        switch(id)
        {
            case url::URLID_REG:{
                handleRegistRequest(url, content);
                break;
            }
            case url::URLID_LOGIN:{
                handleLoginRequest(url, content);
                break;
            }
            default:{
                char resp_content [256];
                std::string str_json = "{\"code\": 1}";
                uint32_t len_json {str_json.size()};

                #define HTTP_RESPONSE_REQ                                                     \
                "HTTP/1.1 404 OK\r\n"                                                      \
                "Connection:close\r\n"                                                     \
                "Content-Length:%d\r\n"                                                    \
                "Content-Type:application/json;charset=utf-8\r\n\r\n%s"

                snprintf(resp_content, 256, HTTP_RESPONSE_REQ, len_json, str_json.c_str());
                m_tcp_conn->send(resp_content);
            }
        }
    }
}


int HttpConnection::handleRegistRequest(std::string& url, std::string& post_data)
{
    std::string resp_json;
    int ret = apiRegistUser(post_data, resp_json);
    
    char http_body [HTTP_RESPONSE_JSON_MAX];
    uint32_t ulen { resp_json.length() };
    snprintf(http_body, HTTP_RESPONSE_JSON_MAX, HTTP_RESPONSE_JSON, ulen, resp_json.c_str());

    m_tcp_conn->send(http_body);
    return 0;
}


int HttpConnection::handleLoginRequest(std::string& url, std::string& post_data)
{
    std::string resp_json;
    int ret = apiUserLogin(post_data, resp_json);

    char resp_body [HTTP_RESPONSE_JSON_MAX];
    uint32_t ulen { resp_json.length() };
    snprintf(resp_body, HTTP_RESPONSE_JSON_MAX, HTTP_RESPONSE_JSON, ulen, resp_json.c_str());
    m_tcp_conn->send(resp_body);

    return 0;
}