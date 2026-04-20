#include "db_pool.h"


ResultSet::ResultSet(MYSQL_RES *res)
:   m_result    {res}
{
    int num_fields { mysql_num_fields(m_result) };
    MYSQL_FIELD *field { mysql_fetch_fields(m_result) };

    for(int i {0}; i < num_fields; i ++)
    {
        m_key_map.insert(std::make_pair(field[i].name, i));
    }
}


ResultSet::~ResultSet()
{
    if(m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}


bool ResultSet::next()
{
    m_row = mysql_fetch_row(m_result);
    if(m_row)
    {
        return true;
    }
    else
    {
        return false;
    }
}


int ResultSet::getIndex(const char* key)
{
    auto iter { m_key_map.find(key) };
    if(iter != m_key_map.end())
    {
        return iter->second;
    }
    else
    {
        return -1;
    }
}


char* ResultSet::getString(const char* key)
{
    int idx = getIndex(key);
    if(idx != -1)
    {
        return m_row[idx];
    }
    else
    {
        return nullptr;
    }
}


int ResultSet::getInt(const char* key)
{
    int idx { getIndex(key) };
    if(idx != -1)
    {
        return atoi(m_row[idx]);
    }
    else
    {
        return -1;
    }
}