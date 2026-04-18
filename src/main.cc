#include <iostream>
#include "HttpServer.h"
#include "MyReactor.h"

int main()
{
    uint16_t port = 8081;
    const char * ip = "0.0.0.0";
    int32_t num_threads = 4;
    EventLoop loop;
    InetAddress addr {port, ip};

    HttpServer server {&loop, addr, "HttpServer", num_threads};

    server.start();
    loop.loop();

    return 0;
}