#include "db_pool.h"

AutoRelConn::AutoRelConn(DBManager *manager, DBConn *connection)
:   m_manager   {manager},
    m_conn      {connection}
{
    
}

AutoRelConn::~AutoRelConn()
{
    if(m_manager && m_conn)
    {
        m_manager->releaseConn(m_conn);
    }
}