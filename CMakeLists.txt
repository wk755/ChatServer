cmake_minimum_required(VERSION 3.0) #要求cmake最低的版本
project(main) #定义当前工程的名字

#g++ -o server（可执行文件） -g muduo_server.cpp xxx.cpp（源文件） -l/usr/include(头文件) -L/usr/lib（库文件） -lmuduo_net-lmuduo_base-lpthread（所要链接库）

#配置编译选项
Set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

#配置最终得可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)


#加载子目录 
add_subdirectory(src)