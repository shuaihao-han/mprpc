#include "mprpcapplication.h"
#include <iostream>
#include <unistd.h>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp()
{
    std::cout << "Format: command -i config_file" << std::endl;
}

void MprpcApplication::Init(int argc, char** argv) {
    if (argc < 3)
    {
        ShowArgsHelp();
        exit(EXIT_FAILURE); // EXIT_FAILURE 是一个宏，表示程序异常退出
    }

    std::string config_file;  // 配置文件
    // 解析命令行参数
    int c = 0;
    while ((c = getopt(argc, argv, "i:")) != -1)  // getopt函数解析命令行参数
    {
        switch (c)
        {
        case 'i':
            config_file = optarg; // optarg是一个全局变量，存储当前选项的参数
            break;
        case '?':  // '?'表示没有找到对应的选项
        std::cout << "invalid option: " << (char)c<< std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':  // ':'表示选项缺少参数
            std::cout << "need config_file " << std::endl;
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        }
    } 
    // 开始加载配置文件 rpcserver_ip=  rpcserver_port=  zookeeper_ip=  zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());
    // 将m_config的全部内容输出以进行调试

    std::cout << "rpcserver_ip: " << m_config.Load("rpcserverip") << std::endl;
    std::cout << "rpcserver_port: " << m_config.Load("rpcserverport") << std::endl;
    std::cout << "zookeeper_ip: " << m_config.Load("zookeeperip") << std::endl;
    std::cout << "zookeeper_port: " << m_config.Load("zookeeperport") << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance() {
    static MprpcApplication instance;  // 静态局部变量，保证单例
    return instance;
}

// 获取配置文件对象
MprpcConfig &MprpcApplication::GetConfig()
{
 return m_config; // 返回配置文件对象
}