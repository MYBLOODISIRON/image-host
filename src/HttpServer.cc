#include <memory>
#include "HttpServer.h"
#include "HttpConnection.h"
#include "MyReactor.h"
std::map<uint32_t, HttpConnPtr> HttpServer::sm_httpMap;

HttpServer::HttpServer(EventLoop* loop, const InetAddress& addr, const std::string& name, int num_event_loops)
:   m_loop {loop}, m_tcpServer {loop, addr, name}
{
    m_tcpServer.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    m_tcpServer.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_tcpServer.setWriteCompleteCallback(std::bind(&HttpServer::onWriteComplete, this, std::placeholders::_1));
    m_tcpServer.setThreadNum(num_event_loops);
}


void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    if(conn->connected())
    {
        LOG_INFO("HttpServer::onConnection: new connection.\n");
        uint32_t uuid { m_conn_uuid_generator++ };
        conn->setContext(uuid);

        sm_httpMap[uuid] = std::make_shared<HttpConnection>(conn);
    }
    else
    {
        LOG_INFO("HttpServer::onConnection: dis connected.\n");
    }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    LOG_INFO("HttpServer::onMessage.\n");
    uint32_t uuid { std::any_cast<uint32_t>(conn->getContext())};

    HttpConnPtr& http_conn {sm_httpMap[uuid]};
    http_conn->onRead(buf);
}

void HttpServer::onWriteComplete(const TcpConnectionPtr& conn)
{
    LOG_INFO("HttpServer::onWriteComplete.\n");
}


void HttpServer::start()
{
    m_tcpServer.start();
}