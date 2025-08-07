#include "rpcprovider.h"
#include "rpcheader.pb.h"

/*
    service_name ==> service描述
        service* 记录服务对象
        method_name ==> method方法对象
*/
// 这里是框架提供给外部使用的可以发布rpc服务的函数接口, 记录了要发布的rpc服务的服务对象信息和服务对象的方法信息
void RpcProvider::NotifyService(google::protobuf::Service *service) {

    // 1. 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *serviceDescriptor = service->GetDescriptor();
    std::string serviceName = serviceDescriptor->name(); // 获取服务的描述符对象，获取服务名

    // 2. 获取服务对象的方法数量
    int methodCount = serviceDescriptor->method_count();

    // 3. 遍历所有方法，获取方法名和方法描述符
    ServiceInfo serviceInfo;
    serviceInfo.m_service = service;  // 保存服务对象
    
    // 将服务信息打印到控制台和日志中
    std::cout << "service name: " << serviceName << std::endl;
    LOG_INFO("service name: %s", serviceName.c_str());

    for (int i = 0; i < methodCount; ++i) {
        const google::protobuf::MethodDescriptor *methodDescriptor = serviceDescriptor->method(i);
        std::string methodName = methodDescriptor->name(); // 获取方法名
        serviceInfo.m_methodMap[methodName] = methodDescriptor; // 保存方法名和方法描述符的映射关系

        std::cout << "method name: " << methodName << std::endl; // 输出方法名
        LOG_INFO("method name: %s", methodName.c_str()); // 将方法名写入日志
    }

    // 4. 将服务信息保存到服务映射表中
    m_serviceMap[serviceName] = serviceInfo; // 保存服务名和服务信息的映射关系
}

// 启动rpc服务，开始提供rpc方法的远程调用服务
void RpcProvider::Run() {
    // muduo库的编程范式
    
    // 根据InetAddress对象所需要的参数进行参数的获取
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");  // 获取配置项中的rpcserverip
    // 获取配置项中的rpcserverport并转换为无符号短整型， atoi需要的是char*类型所以需要使用c_str()转换
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());

    muduo::net::InetAddress addr(ip, port); // 绑定ip和端口号

    // 创建TcpServer对象： server
    muduo::net::TcpServer server(&m_eventLoop, addr, "RpcProvider");

    // 设置线程数量 设置为4 --> 一个是I/O线程(用于处理网络连接事件), 3个是工作线程(用于处理读写事件)
    server.setThreadNum(4);

    // 设置连接回调函数
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));

    // 设置消息回调函数， 已建立连接事件的读写回调
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // ****************** 服务注册 ******************
    // 将当前rpc节点上要发布的服务全部注册到zookeeper上， 让rpc_client可以从ZK上发现服务
    ZKClient zkClient; // 创建ZKClient对象
    zkClient.Start(); // 启动连接ZKserver
    // service_name 为永久性节点   method_name 为临时性节点
    for(auto &sp : m_serviceMap)
    {
        // service_name
        std::string service_path = "/" + sp.first; // 服务名作为znode节点的路径
        zkClient.Create(service_path.c_str(), nullptr, 0, 0); // 创建永久性节点
        for(auto &mp : sp.second.m_methodMap)
        {
            // method_name
            std::string method_path = service_path + "/" + mp.first; // 方法名作为znode节点的路径
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port); // 拼接ip和端口号
            // ZOO_EPHEMERAL: 表示Znode是一个临时性节点
            zkClient.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL); // 创建临时性节点
        }
    }

    std::cout << "RpcProvider start service at ip: " << ip << " port:" << port << std::endl;

    // 启动网络服务
    server.start();
    // 开启事件循环
    m_eventLoop.loop();
}

// 实现连接回调函数  处理连接/断开事件 
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        // 和 rpc client的连接断开了
        std::cout << "connection disconnected!" << std::endl;
        conn->shutdown(); // 关闭连接
    }
}

/*
    在框架内部，RpcProvider和RpcConsumer协商好之间的通信数据格式
    头部数据的格式：4字节的service_name + 4字节的method_name + 4字节的args_size + args
    定义proto的message类型，进行数据头的序列化和反序列化
*/
// 实现消息回调函数  核心：收到请求 → 反序列化 → 查找方法 → 调用本地函数 → 序列化返回
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn, // 消息回调函数
                         muduo::net::Buffer *buffer,
                         muduo::Timestamp time)
{
    // 获取网络上(muduo)接收的远程rpc调用请求的字符流数据
    std::string recvBuf = buffer->retrieveAllAsString(); 
    
    // 从字符流中读取前4个字节的内容
    uint32_t headerSize = 0;
    recvBuf.copy((char*)&headerSize, 4, 0); // 获取头部数据的大小

    // 根据headersize读取数据头的原始字符流，反序列化数据，得到rpc请求的详细信息
    std::string rpc_header_str = recvBuf.substr(4, headerSize); // 获取数据头的字符流
    // 反序列化数据头
    mprpc::RpcHeader rpc_header;
    std::string serviceName;
    std::string methodName;
    uint32_t args_size;
    if(rpc_header.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        serviceName = rpc_header.service_name(); // 获取服务名
        methodName = rpc_header.method_name(); // 获取方法名
        args_size = rpc_header.args_size(); // 获取参数大小
    }
    else
    {
        // 数据头反序列化失败
        std::cout << "rpc_head_str:" << rpc_header_str << " ParseFromString failed!" << std::endl;
        return; // 直接返回
    }

    // 解析rpc方法参数的字符流数据
    std::string args_str = recvBuf.substr(4 + headerSize, args_size); // 获取参数的字符流

    // 打印调试信息
    std::cout << "===============================" << std::endl;
    std::cout << "header_size: " << headerSize << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << serviceName << std::endl;
    std::cout << "method_name: " << methodName << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "===============================" << std::endl;

    // 获取 service 对象和 method 对象
    auto it = m_serviceMap.find(serviceName);
    if(it == m_serviceMap.end())
    {
        // 没有找到对应的服务
        std::cout << "service not found: " << serviceName << std::endl;
        return; // 直接返回
    }

    auto methodIt = it->second.m_methodMap.find(methodName);
    if(methodIt == it->second.m_methodMap.end())
    {
        // 没有找到对应的方法
        std::cout << serviceName << ": " << methodName << "is not exist!" << std::endl;
        return; // 直接返回
    }
    google::protobuf::Service *service = it->second.m_service; // 获取服务对象
    const google::protobuf::MethodDescriptor *method = methodIt->second; // 获取方法描述符

    // 生成rpc方法调用的请求request和响应response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New(); // 创建请求对象
    google::protobuf::Message *response = service->GetResponsePrototype(method).New(); // 创建响应对象

    if(!request->ParseFromString(args_str))
    {
        // 反序列化参数字符流到请求对象中失败
        std::cout << "ParseFromString failed! args_str: " << args_str << std::endl;
    } 

    // 给下面的method方法的调用，绑定一个Clousure的回调函数
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr &, const google::protobuf::Message *>
        (this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上根据解析出的rpc请求信息，调用当前rpc节点上发布的rpc服务方法
    // new UserService().Login(controller, request, response, done); // 调用本地方法
    service->CallMethod(method, nullptr, request, response, done); // 调用本地方法
}

void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, 
                        const google::protobuf::Message *response)
{
    std::string response_str; // 用于存储序列化的响应数据
    if(response->SerializeToString(&response_str))
    {
        // 序列化成功
        conn->send(response_str); // 通过网络(TCPconntection conn)把rpc方法的响应发送给rpc的调用方    }else
    }else{
        // 序列化失败
        std::cout << "Serialize response failed!" << std::endl;
    }
    conn->shutdown();  // 模拟 http 的短链接服务，发送完数据后关闭连接
}