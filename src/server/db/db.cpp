#include <muduo/base/Logging.h>
#include <db.h>
using namespace std;


// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";
// 初始化数据库连接
MySQL::MySQL(){
    _conn = mysql_init(nullptr);
}

 // 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
       mysql_close(_conn);
}
    // 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
            password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else{
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}
// 更新操作
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
              << sql << "更新失败!";
        return false;
    }
    return true;
}
// 查询操作
MYSQL_RES* MySQL::query(string sql)
{
    /*
mysql_query：发送 SQL 查询语句到数据库。
SQL 查询通过 conn 发送，确保绑定到当前的数据库连接。
sql.c_str() 将 C++ 的 std::string 转换为 C 风格字符串。
mysql_use_result：从数据库服务器提取查询结果。
mysql_fetch_row：逐行读取结果数据。
    */
    if (mysql_query(_conn, sql.c_str())) 
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}
//连接操作
MYSQL* MySQL::getConnection(){
    return _conn;
}