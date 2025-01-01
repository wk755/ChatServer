# ChatServer  

本项目开发了一套基于 **C++** 和 **muduo** 的集群聊天系统，支持高并发通信和跨服务器消息同步，并在 **Nginx TCP 负载均衡** 环境下稳定运行。系统采用 **Redis** 进行跨服务器消息分发，**MySQL** 负责数据持久化，通信采用 **JSON** 序列化。  

## 编译过程  
```bash
cd build
rm -rf *
cmake ..
make

- 编程语言：C++  
- 框架/库：muduo、JSON  
- 数据库：Redis、MySQL  
- 构建工具：CMake、Shell 脚本  
- 负载均衡：Nginx TCP  
- 开发环境：VSCode 远程 Linux 开发  

