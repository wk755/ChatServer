/*
作为业务处理层，负责处理具体的功能（如登录、注册、客户端异常退出等）。
*/
#ifndef CHATSERVICE_H
#define CHATSERVIC

#include "redis.hpp"
#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "json.hpp"
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;

//表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>; 
//std::function 封装了一个（返回） void 且接收三个参数的函数，MsgHandler是别名
//因为 MsgHandler 是一个函数类型的别名，它可以被用作函数返回值类型或者函数参数类型。
//所以，您可以像下面这样使用它来声明一个函数：MsgHandler getHandler(int msgid);
/*
在 muduo 网络库中，TcpConnectionPtr 是一个智能指针，指向一个 TcpConnection 对象，
表示客户端与服务器之间的连接。这个智能指针是 std::shared_ptr<TcpConnection> 类型，
负责管理 TcpConnection 对象的生命周期。
 */

//单例模式，聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登录业务
    void login(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //处理注册业务
    void reg(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //创建群组业务
    void createGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //加入群组业务
    void addGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //群聊天业务
    void GroupChat(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    //处理注销业务
    void loginout(const TcpConnectionPtr& conn, json &js, Timestamp time);
    //服务器异常，业务重置方法
    void reset();
    //获取消息对应处理器
    MsgHandler getHandler(int msgid);
    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);
 
private:
    ChatService();

    //存储消息id和其对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    //定义互斥锁， 保证_userconnmap的安全
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //redis操作对象
    Redis _redis;
};

#endif