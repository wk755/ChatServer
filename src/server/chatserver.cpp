#include "chatserver.hpp"
#include "json.hpp"
#include <functional>
#include "chatservice.hpp"
#include <muduo/base/Logging.h>
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
/*
构造函数初始化列表调用其它函数或方法
*/
ChatServer::ChatServer(EventLoop *loop,               // 事件循环
            const InetAddress &listenAddr, // IP+Port
            const string &nameArg)
        : _server(loop, listenAddr, nameArg), _loop(loop)
        {
             // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量 1个I/O线程   3个worker线程
        _server.setThreadNum(4);
        }
 void ChatServer::start()
 {
    _server.start();
 }

 //上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    //客户端断开链接
    if(!conn->connected()){
        ChatService::instance() ->clientCloseException(conn);
        conn->shutdown();
    }
}

//上报读写事件相关信息的回调函数
 void ChatServer::onMessage(const TcpConnectionPtr& conn,
                    Buffer * buffer,
                    Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化
    json js = json::parse(buf);
    //通过js["msgid"]获取=》业务handler =》conn js time
    auto msgHandler = ChatService::instance() -> getHandler(js["msgid"].get<int>());
    //回调消息对应绑定好的事件处理器来执行相应的业务处理
    msgHandler(conn, js, time);
}
/*
事件触发和参数传递流程
1. 注册回调函数
cpp
复制代码
_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
通过 std::bind，将类的成员函数 绑定为回调函数。
std::bind(&ChatServer::onConnection, this, _1):
将 onConnection 绑定为 _server 的连接事件回调。
当 muduo 监听到新的连接或断开事件时，会调用 onConnection。
conn 参数会被 muduo 传入。
std::bind(&ChatServer::onMessage, this, _1, _2, _3):
将 onMessage 绑定为 _server 的消息事件回调。
当 muduo 检测到客户端有数据发送过来时，会调用 onMessage。
conn, buffer, time 都是由 muduo 框架在事件发生时生成并传入的。

2. 回调函数的参数传递
这些参数（如 conn, buffer, time）的传递由 muduo 的事件循环管理。当以下事件发生时，muduo 会自动调用注册的回调函数，并传入相应的参数：
连接事件 (onConnection):
conn 参数表示当前的客户端连接，是由 muduo 框架在监听到连接事件时生成并传入。
当连接建立或断开时，onConnection 会被调用，conn 包含连接的所有信息。
读写事件 (onMessage):
conn: 当前的客户端连接。
buffer: 包含从客户端接收到的数据，由 muduo 管理。
time: 记录接收到数据的时间戳，是 muduo 生成的时间对象。

3. 回调函数触发示例
假设客户端发送了一条消息（如通过 Telnet），以下是触发顺序：
连接建立时:
客户端连接到服务器时，muduo 触发 onConnection。
muduo 将当前连接 TcpConnectionPtr 类型的对象传入 onConnection 函数中。
收到消息时:
客户端发送数据后，muduo 检测到该事件，触发 onMessage。
muduo 将以下参数传入 onMessage:
conn: 表示当前连接。
buffer: 保存从客户端接收到的数据。
time: 接收到数据的时间戳。
具体参数传入的过程
onConnection 参数传递

void ChatServer::onConnection(const TcpConnectionPtr& conn) {
    // 客户端断开连接
    if (!conn->connected()) {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
    }
}
TcpConnectionPtr& conn:
当客户端连接或断开时，muduo 生成一个智能指针 TcpConnectionPtr，表示客户端的连接对象，并将其传入 onConnection。
通过 conn->connected() 判断连接状态。
onMessage 参数传递

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer *buffer, Timestamp time) {
    string buf = buffer->retrieveAllAsString();  // 获取客户端发来的数据
    json js = json::parse(buf);                 // 将数据反序列化为 JSON
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);                 // 执行具体业务处理
}
conn:
和 onConnection 一样，表示客户端连接。
由 muduo 传入。
Buffer* buffer:
表示当前从客户端接收的数据缓冲区。
muduo 会将收到的数据存储到一个 Buffer 对象中，并传递给回调函数。
Timestamp time:
表示接收到该数据的时间。
muduo 在事件触发时生成并传递。
调用流程示例
假设客户端使用 Telnet 发送消息：

建立连接:

客户端通过 telnet 127.0.0.1 6000 连接到服务器。
muduo 触发 onConnection，传入 conn 参数。
如果是新连接，conn->connected() 为 true。
如果是断开连接，conn->connected() 为 false。
发送消息:

客户端发送数据 {"msgid":1,"id":2,"password":"123456"}。
muduo 触发 onMessage，传入以下参数：
conn: 表示客户端连接。
buffer: 包含上述字符串数据。
time: 表示当前时间。
处理消息:

服务器调用 onMessage:
从 buffer 中读取数据并反序列化为 JSON。
根据 msgid 获取业务处理函数。
调用获取的回调函数，执行具体的业务逻辑。
总结
参数是由 muduo 网络库 根据具体事件生成并传入回调函数的。
onConnection 和 onMessage 的触发条件分别是 连接事件 和 消息事件。
参数传递由 muduo 内部自动管理，开发者只需关注如何在回调函数中处理这些参数即可。
 */