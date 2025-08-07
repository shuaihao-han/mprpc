#include "zookeeperutil.h"

// 全局的回调函数, ZKServer 给 ZKClient的通知
void global_watcher(zhandle_t * zh, int type, 
                int state, const char* path, void* watcherCtx)
{
    if(type == ZOO_SESSION_EVENT) // 回调的消息类型是和会话相关的消息类型
    {
        // ZKServer 和 ZKClient 连接成功
        if(state == ZOO_CONNECTED_STATE) 
        {
            sem_t* sem = (sem_t*)zoo_get_context(zh); // 获取zookeeper的上下文
            sem_post(sem);  // 释放信号量，通知等待的线程
        }
        else if(state == ZOO_EXPIRED_SESSION_STATE) // 会话过期
        {
            std::cout << "zookeeper session expired!" << std::endl;
        }
    }
}

ZKClient::ZKClient()
{

}

ZKClient::~ZKClient()
{
    if(m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源
    }
}

// 启动连接ZKserver
void ZKClient::Start()
{
    // ********** 根据配置文件获取zookeeper的ip和端口号 **********
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string hostport = host + ":" + port; // 拼接ip和端口号

    /*
    zookeeper_mt: 多线程版本的zookeeper客户端库
    zookeeper的API客户端提供了三个线程：
        API线程：专门负责和ZKserver进行通信，收发数据
        I/O线程：专门负责网络I/O事件
        Watcher线程：专门负责回调函数的调用
    */
    // zookeeper_init：zookeeper的初始化api
    m_zhandle = zookeeper_init(hostport.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if(m_zhandle == nullptr)
    {
        // 连接失败
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }
    sem_t sem;
    sem_init(&sem, 0, 0); // 初始化信号量
    zoo_set_context(m_zhandle, &sem); // 设置zookeeper的上下文为信号量

    sem_wait(&sem); // 等待信号量
    std::cout << "zookeeper_init successfully!" << std::endl;
}

// 在ZKserver上根据指定的path创建znode节点
void ZKClient::Create(const char* path, const char* data, int datalen, int state)
{
    // state = 0: 临时节点
    // state = ZOO_EPHEMERAL: 临时节点
    // state = ZOO_PERSISTENT: 永久节点
    char path_buffer[128] = {0};
    int bufferlen = sizeof(path_buffer);
    int flag;
    // 先判断path表示的znode节点是否存在，如果存在就不再重复创建了
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if(ZNONODE == flag)   // path表示的znode节点不存在
    {
        // 创建znode节点
        flag = zoo_create(m_zhandle, path, data, datalen, 
                &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if(flag == ZOK)
        {
            std::cout << "znode create success! path: " << path << std::endl;
        }
        else
        {
            std::cout << "flag: " << flag << std::endl;
            std::cout << "znode create error! path: " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

// 根据参数指定的zonde节点路径，获取znode节点的值 (rpc服务方法的ip地址和端口号)
std::string ZKClient::GetData(const char* path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    // 获取 znode 节点的值
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if(flag != ZOK)
    {
        std::cout << "znode get error! path: " << path << std::endl;
        return "";
    }
    else
    {
        return buffer; // 返回znode节点的值
    }
}