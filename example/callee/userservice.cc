#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
/*
UserService 原来是一个本地服务提供了两个进程内的本地方法 Login 和 GetFriendList
UserService 继承于 fixbug::UserServiceRpc 核心在于重写 UserServiceRpc 的用于本地方法调用的虚函数
*/
class UserService : public fixbug::UserServiceRpc  // 使用在Rpc服务发布端
{
public:
    // 登录方法
    bool Login(const std::string &username, const std::string &password)
    {
        std::cout << "do Login service" << std::endl;
        std::cout << "User " << username << " login in with password: " << password << std::endl;
        return true;  // 假设登录成功
    }

    bool Register(const uint32_t id, std::string name, std::string password)
    {
        std::cout << "do Register service" << std::endl;
        std::cout << "User " << name << " registered with id: " << id << " and password: " << password << std::endl;
        return true;  // 假设注册成功
    }
    /*
    重写基类UserServiceRpc的虚函数，下面的方法都是 rpc框架 直接调用的
    1. caller  ===>  Login(LoginRequest) ==> muduo  ==>  Callee
    2. callee  ===>  Login(LoginResponse) ==> 交给下面重写的Login方法
    */
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done) override
    {
        // 框架给业务上报了请求参数LoginRequest， 应用获取相应的数据做本地业务
        std::string username = request->username();
        std::string password = request->password();
        // 调用本地业务方法
        bool success = Login(username, password);
        // 设置响应结果, 包括错误码和错误信息
        fixbug::ResultCode *result = response->mutable_result();
        result->set_errcode(0);  // 假设没有错误
        result->set_errmsg("");
        response->set_success(success);
        // 通知框架，业务处理完毕，执行回调函数
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                const ::fixbug::RegisterRequest* request,
                ::fixbug::RegisterResponse* response,
                ::google::protobuf::Closure* done) override 
    {
        // 框架给业务上报了注册的请求参数RegisterRequest， 应用获取相应的数据做本地业务
        uint32_t id = request->id();
        std::string name = request->username();
        std::string password = request->password(); 
        // 调用本地的业务方法
        bool success = Register(id, name, password);
        // 设置响应结果, 包括错误码和错误信息
        fixbug::ResultCode *result = response->mutable_result();
        result->set_errcode(0);  // 假设没有错误
        result->set_errmsg("");
        response->set_success(success);
        // 通知框架，业务处理完毕，执行回调函数
        done->Run();
    }
};


int main(int argc, char **argv)
{
    // 1. 初始化框架 provider -i config.conf
    MprpcApplication::Init(argc, argv);
    
    // 2. 发布服务, provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;  // 创建一个RpcProvider对象
    provider.NotifyService(new UserService());  // 将UserService注册到框架中
    
    // 3. 启动一个rpc服务发布节点, Run以后 进程进入阻塞状态，等待远程的rpc调用
    provider.Run();
    
    return 0;
}