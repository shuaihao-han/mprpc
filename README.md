# mprpc
基于Zookeeper 和 protobuf 实现的分布式网络通信框架-mprpc

# Packages requirement
Muduo Zookeeper boost protobuf

# Compile
./autobuild.sh

example 文件夹中提供了服务发布方 Callee 和 服务调用方 Caller 的代码示例

# usage
eg. 
1. 运行bin目录下的 ./friend_provider -i test.conf 启动服务
2. 运行bin目录下的 ./friendconsumer -i test.conf 调用服务方提供的服务方法
