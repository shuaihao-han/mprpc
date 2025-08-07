#include <iostream>
#include <string>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"
#include <vector>
#include <string>

class FriendService : public fixbug::FriendServiceRpc  // 使用在Rpc服务发布端
{
public:
    std::vector<std::string> GetFriendList(uint32_t user_id)
    {
        std::cout << "do GetFriendList service!, Userid: " << user_id << std::endl;
        // 模拟返回好友列表
        std::vector<std::string> friends;
        friends.push_back("Gao yang");
        friends.push_back("Liu shuo");
        friends.push_back("Wnag yu");
        return friends;
    }

    void GetFriendList(google::protobuf::RpcController* controller,
                    const ::fixbug::GetFriendListRequest* request,
                    ::fixbug::GetFriendListResponse* response,
                    ::google::protobuf::Closure* done) override
    {
        // 获取框架上报的请求数据
        uint32_t user_id = request->user_id();
        // 调用本地业务
        std::vector<std::string> friends = GetFriendList(user_id);
        // 填充响应数据
        for (const auto& friend_name : friends)
        {
            response->add_friend_list(friend_name);
        }
        fixbug::ResultCode* result = response->mutable_result();
        result->set_errcode(0); // 假设没有错误
        result->set_errmsg("");
        // 执行回调操作
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message!");
    LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 1. 初始化框架 provider -i config.conf
    MprpcApplication::Init(argc, argv);
    
    // 2. 发布服务, provider是一个rpc网络服务对象，把FriendService对象发布到rpc节点上
    RpcProvider provider;  // 创建一个RpcProvider对象
    provider.NotifyService(new FriendService());  // 将FriendService注册到框架中
    
    // 3. 启动一个rpc服务发布节点, Run以后 进程进入阻塞状态，等待远程的rpc调用
    provider.Run();
    
    return 0;
}