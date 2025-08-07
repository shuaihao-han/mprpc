#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include "mprpcapplication.h"
#include "logger.h"
#include "zookeeperutil.h"
#include <unordered_map>
#include <string>
#include <google/protobuf/descriptor.h>


// 框架提供的专门用来进行服务发布rpc服务器的网络对象类
class RpcProvider {
public:
    // 框架给外部提供的接口，可以发布rpc方法的函数接口
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务，开始提供rpc方法的远程调用服务
    void Run();

private:
    // // 组合TcpServer ----- 不写成成员了, 只有 run 会访问
    // std::unique_ptr<muduo::net::TcpServer> m_pTcpServer; // 智能指针
    // 组合EventLoop对象
    muduo::net::EventLoop m_eventLoop;   // 事件循环

    struct ServiceInfo {
        google::protobuf::Service *m_service;  // 服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap;  // 方法名和方法描述符的映射
    };

    std::unordered_map<std::string, ServiceInfo> m_serviceMap;  // 服务名和服务对象的映射关系

    void OnConnection(const muduo::net::TcpConnectionPtr &conn);   // 连接回调函数

    void OnMessage(const muduo::net::TcpConnectionPtr &conn,   // 消息回调函数
                muduo::net::Buffer *buffer,
                muduo::Timestamp time);
    // Closure的回调操作,用于序列化 rpc 的 响应 和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, 
                         const google::protobuf::Message *response);
};