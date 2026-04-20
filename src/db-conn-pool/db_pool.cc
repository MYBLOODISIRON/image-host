#include "db_pool.h"
#define MIN_DB_CONN_CNT 1
#define MAX_DB_CONN_FAIL_NUM 10

DBPool::DBPool(const std::string& pool_name, const std::string& ip, uint16_t port, const std::string& user_name, const std::string& pwd, const std::string& db_name, int max_conn_cnt)
:   m_pool_name {pool_name},
    m_ip        {ip},
    m_port      {port},
    m_user_name {user_name},
    m_pwd       {pwd},
    m_db_name   {db_name},
    m_max_conn_cnt  {max_conn_cnt},
    m_cur_conn_cnt  {MIN_DB_CONN_CNT}
{

}


DBPool::~DBPool()
{
    std::lock_guard<std::mutex> lock {m_mutex};

    m_abort_request = true;
    m_cond_var.notify_all();

    for(auto iter = m_free_list.begin(); iter != m_free_list.end(); iter ++)
    {
        DBConn* conn = *iter;
        delete conn;
    }

    m_free_list.clear();
}


int DBPool::init()
{
    for(int i {0}; i < m_cur_conn_cnt; i ++)
    {
        DBConn* conn = new DBConn {this};
        int retval = conn->init();
        if(retval)
        {
            delete conn;
            return retval;
        }
        m_free_list.push_back(conn);
    }

    return 0;
}


DBConn* DBPool::getConnection(const int timeout_ms)
{
    std::unique_lock<std::mutex> lock {m_mutex};

    if(m_abort_request)
    {
        return nullptr;
    }

    if(m_free_list.empty()) // free_list为空
    {
        if(m_cur_conn_cnt  >= m_max_conn_cnt)    // 连接池连接达到最大连接数，等待其他释放
        {
            if(timeout_ms <= 0)
            {
                m_cond_var.wait(lock, [this] { return (!m_free_list.empty()) || m_abort_request; });
            }
            else
            {
                m_cond_var.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return (! m_free_list.empty()) || m_abort_request; });

            }
            if(m_free_list.empty())
            {
                return nullptr;
            }
        }
        else
        {
            DBConn* connection = new DBConn {this};
            int retval = connection->init();
            if(retval)
            {
                delete connection;
                return nullptr;
            }
            else
            {
                m_free_list.push_back(connection);
                m_cur_conn_cnt ++;
            }
        }
    }

    DBConn* connection = m_free_list.front();
    m_free_list.pop_front();
    return connection;
}


void DBPool::releaseConnection(DBConn *connection)
{
    std::lock_guard<std::mutex> lock {m_mutex};

    auto iter = m_free_list.begin();
    for(; iter != m_free_list.end(); iter ++)
    {
        if(*iter == connection)
        {
            break;
        }
    }

    if(iter == m_free_list.end())
    {
        m_free_list.push_back(connection);
        m_cond_var.notify_one();
    }
}

