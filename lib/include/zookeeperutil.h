#pragma once

#include <semaphore.h>  // 信号量
#include <zookeeper/zookeeper.h>
#include <string>
#include <iostream>
#include "mprpcapplication.h"

// ZK客户端类的实现主要就在三点
// 1. 启动连接ZKserver ：Start
// 2. 在ZKserver上根据指定的path创建znode节点 ：Create
// 3. 根据参数指定的zonde节点路径，获取znode节点的值 : GetData
// 作为ZK server的client， ZKClient主要就是将数据组织成ZKserver需要的形式
// 然后利用ZK提供的API来进行完成操作即可
class ZKClient
{
public:
    ZKClient();
    ~ZKClient();

    // 启动连接ZKserver
    void Start();

    // 在ZKserver上根据指定的path创建znode节点
    void Create(const char* path, const char* data, int datalen, int state = 0);
    // 根据参数指定的zonde节点路径，获取znode节点的值
    std::string GetData(const char* path);
private:
    // zk的客户端句柄, 用来操作zookeeper server
    zhandle_t* m_zhandle;
};