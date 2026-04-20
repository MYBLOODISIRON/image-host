#include <string.h>
#include "db_pool.h"

PrepareStatement::~PrepareStatement()
{
    if(m_stmt)
    {
        mysql_stmt_close(m_stmt);
        m_stmt = nullptr;
    }

    if(m_param_bind)
    {
        delete[] m_param_bind;
        m_param_bind = nullptr;
    }
}


bool PrepareStatement::init(MYSQL* mysql, std::string& sql)
{
    mysql_ping(mysql);  // mysql丢失时，重连数据库

    m_stmt = mysql_stmt_init(mysql);
    if(! m_stmt)
    {
        return false;
    }

    if(mysql_stmt_prepare(m_stmt, sql.c_str(), sql.size()))
    {
        return false;
    }

    m_param_count  = mysql_stmt_param_count(m_stmt);
    if(m_param_count > 0)
    {
        m_param_bind = new MYSQL_BIND [m_param_count];
        if(m_param_bind == nullptr)
        {
            return false;
        }

        memset(m_param_bind, 0, sizeof(MYSQL_BIND) * m_param_count);
    }

    return true;
}


void PrepareStatement::setParam(uint32_t index, int value)
{
    if(index >= m_param_count)
    {
        return;
    }

    m_param_bind [index].buffer_type = MYSQL_TYPE_LONG;
    m_param_bind [index].buffer = &value;
}


void PrepareStatement::setParam(uint32_t index, uint32_t value)
{
    if(index >= m_param_count)
    {
        return;
    }

    m_param_bind [index].buffer_type = MYSQL_TYPE_LONG;
    m_param_bind [index].buffer = &value;
}


void PrepareStatement::setParam(uint32_t index, const std::string& value)
{
    if(index >= m_param_count)
    {
        return;
    }
    m_param_bind [index].buffer_type = MYSQL_TYPE_STRING;
    m_param_bind [index].buffer = (char*)value.c_str();
    m_param_bind [index].buffer_length = value.length();
}


bool PrepareStatement::executeUpdate()
{
    if(! m_stmt)
    {
        return false;
    }

    if(mysql_stmt_bind_param(m_stmt, m_param_bind))
    {
        return false;
    }

    if(mysql_stmt_execute(m_stmt))
    {
        return false;
    }

    if(mysql_stmt_affected_rows(m_stmt) == 0)
    {
        return false;
    }

    return true;
}


uint32_t PrepareStatement::getInsertId()
{
    return mysql_stmt_insert_id(m_stmt);
}

