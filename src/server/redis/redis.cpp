/*
发布消息和订阅消息分别用独立的上下文，客户端发布消息给redis某个channel，而redis每个channel都与服务器进行了订阅，实时监听着消息，
（为了避免线程堵塞，开辟一个新的线程）所以redis将消息传递给服务器
*/


#include "redis.hpp"
#include <iostream>
using namespace std;

Redis::Redis()
    : _publish_context(nullptr), _subcribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }

    if (_subcribe_context != nullptr)
    {
        redisFree(_subcribe_context);
    }
}

bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    /*功能：连接 Redis 服务器，用于发布消息。
调用 redisConnect 函数，参数为 Redis 服务器的地址和端口（127.0.0.1:6379）。
返回一个 redisContext 指针，表示与 Redis 的连接上下文。
逻辑：
如果返回值为 nullptr，表示连接失败。
输出错误信息并返回 false，结束函数执行。*/ 
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subcribe_context = redisConnect("127.0.0.1", 6379);
    /*
redisConnect 是 hiredis 提供的一个函数，用于与 Redis 服务器建立一个 TCP 连接，并返回一个上下文对象（redisContext）。
每次调用 redisConnect，都会创建一个新的连接，且每个连接的上下文对象独立存在，彼此互不干扰。
    */
    if (nullptr == _subcribe_context)
    {
        cerr << "connect redis failed!" << endl;
        return false;
    }

    // 在单独的线程中，监听通道上的事件，有消息给业务层进行上报
    thread t([&]() {
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis-server success!" << endl;

    return true;
}

// 向redis指定的通道channel发布消息
bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish command failed!" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

// 向redis指定的通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    /*
redisAppendCommand 是 Redis 客户端库中的一个函数，用于将命令添加到 Redis 请求队列中（Redis 上下文的输出缓冲区）。
this->_subcribe_context表示一个 Redis 连接上下文对象（redisContext 类型），通过它与 Redis 服务器通信。
这里使用的是类的成员变量 _subcribe_context，说明这是一个封装了 Redis 功能的类。
"SUBSCRIBE %d"Redis 的 SUBSCRIBE 命令，用于订阅一个频道。这里的 %d 是一个占位符，表示该频道的编号将由后续的参数 channel 替换。
channel表示频道的 ID 或编号。%d 说明它是一个整数类型。
REDIS_ERR 是一个常量，用于表示 Redis 操作失败。通常在 Redis 客户端库（如 hiredis）中定义。
    */
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    // redisGetReply,不写防止阻塞，统一放到独立线程池中，不造成竞争，下面独立线程抢占响应，导致这边上下文堵塞，线程无法工作

    return true;
}

// 向redis指定的通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subcribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subcribe_context, &done))
        {
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 在独立线程中接收订阅通道中的消息
void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subcribe_context, (void **)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str) , reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

void Redis::init_notify_handler(function<void(int,string)> fn)
{
    this->_notify_message_handler = fn;
}