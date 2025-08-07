#pragma once   // 防止头文件被重复包含
#include "mprpcconfig.h"
// mprpc 框架的基础类  单例模式 负责框架的一些初始化操作
class MprpcApplication {
public:
    static void Init(int argc, char** argv);
    // 定义唯一的一个实例
    static MprpcApplication& GetInstance();
    static MprpcConfig &GetConfig();
private:
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;  // 禁止拷贝构造
    MprpcApplication(MprpcApplication&&) = delete;  // 禁止移动构造
    static MprpcConfig m_config;  // 框架配置对象
    // 获取配置文件对象
};