/*
作为服务器，负责监听客户端连接、接收消息、并通过回调机制将任务分发出去。
*/

#ifndef CHATSERVER_H
#define CHATSERVER_H

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>
using namespace std;
using namespace muduo;
using namespace muduo::net;

//聊天服务器的主类
class ChatServer
{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop *loop,               // 事件循环
            const InetAddress &listenAddr, // IP+Port
            const string &nameArg);

    // 启动服务
    void start();

private:
    //上报链接相关信息的回调函数
    void onConnection(const TcpConnectionPtr&);

    //上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr&,
                    Buffer *,
                    Timestamp);

    TcpServer _server; //组合的muduo库，实现服务器功能的对象
    EventLoop* _loop;  //指向时间循环对象的指针
};





#endif