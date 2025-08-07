#include "mprpcchannel.h"

/*
rpc方法提供者和rpc方法调用者之间的通信协议
header_size + service_name  method_name args_size + args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                google::protobuf::RpcController* controller,
                const google::protobuf::Message* request,
                google::protobuf::Message* response,
                google::protobuf::Closure* done)
{
    // ************* 进行数据的序列化 ****************
    const google::protobuf::ServiceDescriptor* service = method->service();
    std::string serviceName = service->name(); // 获取服务名
    std::string methodName = method->name(); // 获取方法名

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str))
    {
        // 序列化成功,需要获取序列化后的字符串长度
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("request SerializeToString failed!");
        return ;
    }

    // 定义rpc请求的header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(serviceName); // 设置服务名
    rpcHeader.set_method_name(methodName); // 设置方法名
    rpcHeader.set_args_size(args_size); // 设置参数的序列化字符串长度

    uint32_t header_size; // 获取header的序列化长度
    std::string rpc_header_str;
    // 将rpcHeader序列化成字符串
    if(rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }else{
        controller->SetFailed("Rpcheader serialize error!");
        return ;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char*)&header_size, sizeof(uint32_t))); // 前4个字节存储header的长度:head_size
    send_rpc_str += rpc_header_str; // header的序列化字符串
    send_rpc_str += args_str; // 参数的序列化字符串

    // 打印调试信息 
    std::cout << "================================" << std::endl;
    std::cout << "header size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service name: " << serviceName << std::endl;
    std::cout << "method name: " << methodName << std::endl;
    std::cout << "args size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "================================" << std::endl;

    // ************* 发起网络连接，并发送已经序列化的数据,最后处理响应 ***************
    // 使用 tcp 编程(rpc方法的调用方，不需要高并发，因此没有必要使用muduo库)，完成rpc方法的远程调用
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd < 0)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "socket create failed, errno:%d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    // 获取rpc服务提供者的ip和端口号
    // std::string server_ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // uint16_t server_port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    // *********** 修改为去ZK服务注册中心查找服务方法的ip和端口号 *********
    ZKClient zkcli;
    zkcli.Start();   // 启动连接ZKserver
    // 获取服务名对应的znode节点路径
    std::string method_path = "/" + serviceName + "/" + methodName; // /service_name/method_name
    // 根据Znode节点路径查找服务方法的ip和端口号
    std::string host_data = zkcli.GetData(method_path.c_str()); // 获取znode节点的值
    if(host_data == "")
    {
        controller->SetFailed("GetData from zookeeper failed, method_path: " + method_path);
        return;
    }
    int idx = host_data.find(":"); // 查找ip和端口号的分隔符
    if(idx == -1)
    {
        controller->SetFailed("GetData from zookeeper failed, method_path: " + method_path + ", host_data: " + host_data);
        return ;
    }
    std::string server_ip = host_data.substr(0, idx);
    uint16_t server_port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str()); // 获取端口号

    // *********** 向rpc远程服务提供方发起TCP连接 ***********
    // 绑定 ip地址 和 端口号
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    // 调用connect发起 tcp 连接
    if (connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "connect failed, errno: %d", errno);
        close(clientfd);
        controller->SetFailed(errtxt);
        return ;
    }
    // tcp连接建立成功后，send 发送已经经过序列化的rpc请求
    if(send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0) < 0)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "send failed, errno: %d", errno);
        close(clientfd);
        controller->SetFailed(errtxt);
        return ;
    }

    // recv 接收rpc请求的响应值，并反序列化成response 供调用者使用
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if((recv_size = recv(clientfd, recv_buf, sizeof(recv_buf), 0)) < 0)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "recv failed, errno: %d", errno);
        close(clientfd);
        controller->SetFailed(errtxt);
        return ;
    }
    // 将接收的数据先转换成字符串，再反序列化
    // bug：出现问题， recv_buf中遇到\0会被截断 --> 反序列化失败
    // std::string response_str(recv_buf, 0, recv_size);
    if(!response->ParseFromArray(recv_buf, recv_size))
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "recv failed, errno: %s", recv_buf);
        close(clientfd);
        controller->SetFailed(errtxt);
        return ;
    }
    close(clientfd);
}