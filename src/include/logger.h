#pragma once

#include <string>
#include <time.h>
#include <thread>
#include <iostream>
#include "lockqueue.h"

// 一般日志系统会设置不同的日志级别
// 对于我们的rpc框架，先简单的设置为普通信息和错误信息
enum LogLevel
{
    INFO,   // 普通信息
    ERROR,  // 错误信息
};

// Mprpc 框架提供的日志系统
class Logger
{
public:
    // 获取日志的单例
    static Logger& GetInstance();
    // 设置日志级别
    void SetLogLevel(LogLevel level); // 设置日志级别
    // 写日志
    void Log(std::string msg);
private:
    int m_logLevel;  // 日志级别
    // 实例化为string类型的日志队列
    LockQueue<std::string> m_lockQueue; // 日志队列
    // 将Logger设计为单例模式，禁用拷贝以及移动构造，保证全局唯一
    Logger();
    Logger(const Logger&) = delete;  // 禁止拷贝构造
    Logger(Logger&&) = delete;  // 禁止移动构造
};


// 定义宏, 支持可变参数的日志打印
#define LOG_INFO(logmsgformat, ...) \
    do { \
        Logger & logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char logmsg[1024] = {0}; \
        snprintf(logmsg, sizeof(logmsg), logmsgformat, ##__VA_ARGS__); \
        logger.Log(logmsg); \
    } while(0);

#define LOG_ERR(logmsgformat, ...) \
    do { \
        Logger & logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char logmsg[1024] = {0}; \
        snprintf(logmsg, sizeof(logmsg), logmsgformat, ##__VA_ARGS__); \
        logger.Log(logmsg); \
    } while(0);