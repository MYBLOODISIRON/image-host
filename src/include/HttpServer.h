#pragma once
#include <atomic>
#include <map>
#include "HttpConnection.h"
#include "MyReactor.h"

class HttpServer
{
    public:
        static std::map<uint32_t, HttpConnPtr>  sm_httpMap;

    private:
        TcpServer   m_tcpServer;
        EventLoop*  m_loop  {nullptr};
        std::atomic<uint32_t>   m_conn_uuid_generator {0};

    public:
        HttpServer  (EventLoop* loop, const InetAddress& addr, const std::string& name, int num_event_loop);
        ~HttpServer () = default;
        void    start   ();

    private:
        void    onConnection    (const TcpConnectionPtr& conn);
        void    onMessage       (const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);
        void    onWriteComplete (const TcpConnectionPtr& conn);
};