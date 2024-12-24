 #include "offlinemessagemodel.hpp"
 #include "db.h"
 
 
//存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg){
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

//删除用户的离线消息
void OfflineMsgModel::remove(int userid){
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid =%d", userid);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid =%d", userid);
    vector<string> vec;
    MySQL mysql;
    if(mysql.connect()) //连接到数据库
    {
        MYSQL_RES *res = mysql.query(sql);//调用 mysql.query() 方法，执行 SQL 查询语句，查询 offlinemessage 表中属于指定 userid 的所有消息。
        //返回值 res 是一个指向 MYSQL_RES 类型的指针，表示查询结果集。
        if(res != nullptr)
        {
            //把userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row ;
            /*
            定义一个变量 row，类型是 MYSQL_ROW，用于存储结果集中的一行数据。
            MYSQL_ROW 本质上是一个指针数组，每个元素表示查询结果中某一列的值。
             */
            
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            }

            /*
            使用 mysql_fetch_row() 获取查询结果中的每一行,返回的 row 是指向当前行数据的指针数组。
            每调用一次 mysql_fetch_row()，游标移动到下一行。如果没有更多行，则返回 nullptr。
            */
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}