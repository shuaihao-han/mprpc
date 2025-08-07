#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"
#include "mprpcchannel.h"

int main(int argc, char** argv)
{
    // 整个程序启动以后，想要使用mprpc框架来享受rpc服务调用
    // 一定需要先调用框架的初始化函数(只初始化一次)
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法 login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel()); // RpcChannel 需要框架提供的channel
    // Rpc方法的请求参数
    fixbug::GetFriendListRequest request;
    request.set_user_id(5312);
    // Rpc方法的响应参数
    fixbug::GetFriendListResponse response;

    // 定义控制块对象
    MprpcController controller;

    // 发起rpc方法调用
    // stub.Login() -> RpcChannel::CallMethod()
    stub.GetFriendList(&controller, &request, &response, nullptr);   // RpcChannel -> RpcChannel::CallMethod()

    // Rpc 调用完成，读取响应结果response
    if(controller.Failed())
    {
        // 打印rpc调用过程中的错误信息
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if(response.result().errcode() != 0)
        {
            std::cout << "rpc GetFriendList error: " << response.result().errmsg() << std::endl;
            return 0;
        }else{
            std::cout << "Rpc GetFriendList success, the freiend list is: " << std::endl;
            // 读取返回的好友列表
            for(int i = 0; i < response.friend_list_size(); ++i)
            {
                std::cout << "friend: " << response.friend_list(i) << std::endl;
            }
        }
    }

    return 0;
}