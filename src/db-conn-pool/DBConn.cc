#include <string.h>
#include "db_pool.h"


DBConn::DBConn(DBPool *pool)
:   m_pool {pool}
{

}

DBConn::~DBConn()
{
    if(m_conn)
    {
        mysql_close(m_conn);
    }
}


int DBConn::init()
{
    m_conn = mysql_init(nullptr);
    if(! m_conn)
    {
        return -1;
    }

    mysql_options(m_conn, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    if(! mysql_real_connect(m_conn, m_pool->getIp().c_str(), m_pool->getUserName().c_str(), m_pool->getPwd().c_str(), m_pool->getDBName().c_str(), m_pool->getPort(), nullptr, 0))
    {
        return -1;
    }

    return 0;
}

std::string DBConn::poolName()
{
    return m_pool->getPoolName();
}


bool DBConn::executeCreate(const char* sql_query, uint64_t sql_len)
{
    mysql_ping(m_conn);

    if(mysql_real_query(m_conn, sql_query, sql_len))
    {
        return false;
    }

    return true;
}


bool DBConn::executePassQuery(const char* sql_query, uint64_t sql_len)
{
    mysql_ping(m_conn);

    if(mysql_real_query(m_conn, sql_query, sql_len))
    {
        return false;
    }

    return true;
}


bool DBConn::executeDrop(const char* sql_query, uint64_t sql_len)
{
    mysql_ping(m_conn);
    if(mysql_real_query(m_conn, sql_query, sql_len))
    {
        return false;
    }

    return true;
}


ResultSet* DBConn::executeQuery(const char* sql_query, uint64_t sql_len)
{
    mysql_ping(m_conn);

    m_row_num = 0;
    if(mysql_real_query(m_conn, sql_query, sql_len))
    {
        return nullptr;
    }

    MYSQL_RES* result = mysql_store_result(m_conn);
    if(! result)
    {
        return nullptr;
    }

    m_row_num = mysql_num_rows(result);

    return new ResultSet {result};
}


bool DBConn::executeUpdate(const char* sql_query, uint64_t sql_len, bool if_care)
{
    mysql_ping(m_conn);

    if(mysql_real_query(m_conn, sql_query, sql_len))
    {
        return false;
    }

    if(mysql_affected_rows(m_conn) > 0)
    {
        return true;
    }
    else if(if_care)
    {
        return false;
    }
    else
    {
        return true;
    }
}


bool DBConn::startTransaction()
{
    mysql_ping(m_conn);
    if(mysql_real_query(m_conn, "start transaction\n", 17))
    {
        return false;
    }

    return true;
}

bool DBConn::rollback()
{
    mysql_ping(m_conn);

    if(mysql_real_query(m_conn, "rollback\n", 8))
    {
        return false;
    }

    return true;
}


bool DBConn::commit()
{
    mysql_ping(m_conn);

    if(mysql_real_query(m_conn, "commit\n", 6))
    {
        return false;
    }

    return true;
}


uint32_t DBConn::getInsertId()
{
    return (uint32_t) mysql_insert_id(m_conn);
}


MYSQL* DBConn::mysql()
{
    return m_conn;
}

int DBConn::rowNum()
{
    return m_row_num;
}