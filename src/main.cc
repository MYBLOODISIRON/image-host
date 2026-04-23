#include <iostream>
#include "HttpServer.h"
#include "MyReactor.h"
#include "db_pool.h"
#include "config_file_reader.h"
#include "cache_pool.h"
#include "upload.h"


int main(int argc, char* argv[])
{

    char* config_path = nullptr;    // 配置文件路径，命令行或者默认路径
    if(argc > 1)
    {
        config_path = argv[1];
    }
    else
    {
        config_path = (char*) "image_host.config";
    }
    std::cout << config_path << std::endl;
    CConfigFileReader config_file {config_path};
    char* dfs_path_client = config_file.GetConfigName("dfs_path_client");
    char* storage_web_server_ip = config_file.GetConfigName("storage_web_server_ip");
    char* storage_web_server_port = config_file.GetConfigName("storage_web_server_port");
    apiUploadInit(dfs_path_client, storage_web_server_ip, storage_web_server_port, "", "");

    DBManager::setConfPath(config_path);    // 连接池配置文件路径
    DBManager* db_manager = DBManager::getInstance();
    if(! db_manager)
    {
        LOG_ERROR("database connection pool init failed.");
        return -1;
    }


    CacheManager::SetConfPath(config_path);
    CacheManager* cache_manager = CacheManager::getInstance();
    if(! cache_manager)
    {
        LOG_ERROR("cache manager init failed.");
        return -1;
    }


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