#pragma once
#include <condition_variable>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <mysql/mysql.h>
#include <string>
#include <stdint.h>
#include <mysql/mysql.h>
class DBPool;


#define MAX_ESCAPE_STRING_LEN 1024

class ResultSet
{
    private:
        std::map<std::string, int>  m_key_map;  // 结果集字段详细信息
        MYSQL_ROW                   m_row;
        MYSQL_RES                   *m_result;
        
    public:
        ResultSet   (MYSQL_RES *res);
        virtual     ~ResultSet  ();

        bool    next    ();
        char*   getString   (const char* key);
        int     getInt      (const char* key);
    
    private:
        int getIndex    (const char* key); // 获取字段名的索引
};




class PrepareStatement
{
    private:
        MYSQL_STMT  *m_stmt     {nullptr};
        MYSQL_BIND  *m_param_bind   {nullptr};
        uint32_t    m_param_count   {0};

    public:
        PrepareStatement    () = default;
        ~PrepareStatement   ();

        bool    init    (MYSQL* mysql, std::string& sql);

        void    setParam    (uint32_t index, int value);
        void    setParam    (uint32_t index, uint32_t value);
        void    setParam    (uint32_t index, const std::string& value);

        bool    executeUpdate   ();
        uint32_t getInsertId    ();
};


class DBConn
{
    private:
        int     m_row_num   {0};
        DBPool  *m_pool     {nullptr};
        MYSQL   *m_conn     {nullptr};
        char    m_escape_string [MAX_ESCAPE_STRING_LEN + 1];

    public:

        DBConn  (DBPool *pool);
        virtual ~DBConn ();

        int init    ();
        bool    executeCreate   (const char* sql_query, uint64_t sql_len);    // 创建表
        bool    executeDrop     (const char* sql_query, uint64_t sql_len);    // 删除表
        ResultSet*  executeQuery(const char* sql_query, uint64_t sql_len);    // 查询

        bool    executePassQuery(const char* sql_query, uint64_t sql_len);
        bool    executeUpdate    (const char* sql_query, uint64_t sql_len, bool if_care = true);
        uint32_t getInsertId    ();

        bool    startTransaction    ();
        bool    commit              ();
        bool    rollback            ();

        std::string     poolName    ();
        MYSQL*          mysql       ();
        int             rowNum      ();
};


class DBPool
{
    private:
        std::string     m_pool_name;
        std::string     m_ip;
        uint16_t        m_port;
        std::string     m_user_name;
        std::string     m_pwd;
        std::string     m_db_name;

        int     m_cur_conn_cnt; // 当前连接数
        int     m_max_conn_cnt; // 最大连接数

        std::list<DBConn*>  m_free_list;
        std::list<DBConn*>  m_used_list;
        std::mutex          m_mutex;
        std::condition_variable     m_cond_var;
        bool    m_abort_request {false};

    public:

        DBPool  () = default;
        DBPool  (const std::string& pool_name, const std::string& ip, uint16_t port, const std::string& user_name, const std::string& pwd, const std::string& db_name, int max_conn_cnt);
        virtual ~DBPool     ();

        int     init    ();     // 创建最小连接数
        DBConn* getConnection  (const int timeout_ms = 0);
        void    releaseConnection   (DBConn* connection);

        const std::string&  getPoolName ();
        const std::string&  getIp       ();
        uint16_t            getPort     ();
        const std::string&  getUserName ();
        const std::string&  getPwd      ();
        const std::string&  getDBName   ();

};


class DBManager
{
    private:
        static DBManager*               sm_db_manager;
        static std::string              sm_config_path;
        std::map<std::string, DBPool*>  m_pool_map;

    public:
        static void         setConfPath (const std::string& conf_path);
        static DBManager*   getInstance ();

        virtual ~DBManager  () = default;
        int     init        ();
        DBConn* getConn     (const std::string& pool_name);
        void    releaseConn (DBConn* connection);

    private:
        DBManager   () = default;   // 单例
};


class AutoRelConn
{
    private:
        DBManager   *m_manager  {nullptr};
        DBConn      *m_conn     {nullptr};

    public:
        AutoRelConn (DBManager *manager, DBConn* conn);
        ~AutoRelConn();
};


#define AUTO_REL_DB_CONN(manager, connection) AutoRelConn auto_release {manager, connection}    // 栈上变量自动释放连接