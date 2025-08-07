#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc, char** argv)
{
    // 整个程序启动以后，想要使用mprpc框架来享受rpc服务调用
    // 一定需要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法 login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel()); // RpcChannel 需要框架提供的channel
    // Rpc方法的请求参数
    fixbug::LoginRequest request;
    request.set_username("zhang san");
    request.set_password("123456");
    // Rpc方法的响应参数
    fixbug::LoginResponse response;

    // 发起rpc方法调用
    // stub.Login() -> RpcChannel::CallMethod()
    stub.Login(nullptr, &request, &response, nullptr);   // RpcChannel -> RpcChannel::CallMethod()

    // Rpc 调用完成，读取响应结果response
    if(response.result().errcode() == 0)
    {
        std::cout << "Rpc Login success, response: " << response.success() << std::endl;
    }
    else
    {
        std::cout << "Rpc Login failed, errormsg: " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法 register
    fixbug::RegisterRequest register_request;
    register_request.set_id(1001);
    register_request.set_username("lisi");
    register_request.set_password("123456");
    fixbug::RegisterResponse register_response;
    // 以同步的方式发起 rpc方法调用,等待返回结果
    // stub.Register() -> RpcChannel::CallMethod()
    stub.Register(nullptr, &register_request, &register_response, nullptr); // RpcChannel -> RpcChannel::CallMethod()
    
    // Rpc 调用完成，读取响应结果response
    if(register_response.result().errcode() == 0)
    {
        std::cout << "Rpc Register success, response: " << register_response.success() << std::endl;
    }
    else
    {
        std::cout << "Rpc Register failed, errormsg: " << register_response.result().errmsg() << std::endl;
    }

    return 0;
}