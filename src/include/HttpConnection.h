#pragma once
#include <memory>
#include "MyReactor.h"
#include "http_parser_wrapper.h"

class HttpConnection: enable_shared_from_this<HttpConnection>
{
    private:
        TcpConnectionPtr    m_tcp_conn;
        uint32_t            m_uuid {0};
        CHttpParserWrapper  m_http_parser;

    public:
        HttpConnection  (const TcpConnectionPtr& conn);
        virtual ~HttpConnection ();
        void    onRead          (Buffer* buf);

    private:
        int     handleRegistRequest (std::string& url, std::string& post_data);
        int     handleLoginRequest  (std::string& url, std::string& post_data);
        int     handleMd5Request    (std::string& url, std::string& post_data);
        int     handleUpLoadRequest (std::string& url, std::string& post_data);
        int     handleMyFilesRequest(std::string& url, std::string& post_data);
        int     handleSharePictureRequest(std::string& url, std::string& post_data);
};
using HttpConnPtr = std::shared_ptr<HttpConnection>;