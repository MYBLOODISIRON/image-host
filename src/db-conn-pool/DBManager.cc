#include "db_pool.h"
#include "config_file_reader.h"

DBManager* DBManager::getInstance()
{
    if(sm_db_manager == nullptr)
    {
        sm_db_manager = new DBManager;
        if(sm_db_manager->init())
        {
            delete sm_db_manager;
            sm_db_manager = nullptr;
        }
    }
    return sm_db_manager;
}


void DBManager::setConfPath(const std::string& conf_path)
{
    sm_config_path = conf_path;
}

int DBManager::init()
{
    CConfigFileReader config_file {sm_config_path.c_str()};

    char* db_instances { config_file.GetConfigName("DBInstances") };
    if(! db_instances)
    {
        return -1;
    }

    char host[64];
    char port[64];
    char dbname[64];
    char username[64];
    char password[64];
    char maxconncnt[64];
    CStrExplode instances_name(db_instances, ',');

    for (uint32_t i = 0; i < instances_name.GetItemCnt(); i++) {
        char *pool_name = instances_name.GetItem(i);
        snprintf(host, 64, "%s_host", pool_name);
        snprintf(port, 64, "%s_port", pool_name);
        snprintf(dbname, 64, "%s_dbname", pool_name);
        snprintf(username, 64, "%s_username", pool_name);
        snprintf(password, 64, "%s_password", pool_name);
        snprintf(maxconncnt, 64, "%s_maxconncnt", pool_name);

        char *db_host = config_file.GetConfigName(host);
        char *str_db_port = config_file.GetConfigName(port);
        char *db_dbname = config_file.GetConfigName(dbname);
        char *db_username = config_file.GetConfigName(username);
        char *db_password = config_file.GetConfigName(password);
        char *str_maxconncnt = config_file.GetConfigName(maxconncnt);


        if (!db_host || !str_db_port || !db_dbname || !db_username ||
            !db_password || !str_maxconncnt) {
            return 2;
        }

        int db_port = atoi(str_db_port);
        int db_maxconncnt = atoi(str_maxconncnt);
        DBPool *pDBPool = new DBPool(pool_name, db_host, db_port, db_username,
                                       db_password, db_dbname, db_maxconncnt);
        if (pDBPool->init()) {
            return 3;
        }
        m_pool_map.insert(make_pair(pool_name, pDBPool));
    }

    return 0;
}


DBConn* DBManager::getConn(const std::string& pool_name)
{
    auto iter = m_pool_map.find(pool_name);
    if(iter != m_pool_map.end())
    {
        return iter->second->getConnection();
    }
    else
    {
        return nullptr;
    }
}


void DBManager::releaseConn(DBConn* connection)
{
    if(! connection)
    {
        return ;
    }

    auto iter = m_pool_map.find(connection->poolName());
    if(iter != m_pool_map.end())
    {
        iter->second->releaseConnection(connection);
    }
}