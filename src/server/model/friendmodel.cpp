#include "friendmodel.hpp"
#include "db.h"

void FriendModel::insert(int userid, int friendid){
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values(%d, %d)", userid,friendid);
    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}


vector<User> FriendModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid=%d", userid);
    vector<User> vec;
    MySQL mysql;
    if(mysql.connect()) //连接到数据库
    {
        MYSQL_RES *res = mysql.query(sql);//调用 mysql.query() 方法，执行 SQL 查询语句，
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
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
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