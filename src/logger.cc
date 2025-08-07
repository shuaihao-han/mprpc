#include "logger.h"

// 获取日志的单例
Logger& Logger::GetInstance()
{
    static Logger instance; // 局部静态变量
    return instance;
}
// 设置日志级别
void Logger::SetLogLevel(LogLevel level) // 设置日志级别
{
    m_logLevel = level;
}
// 写日志, 把日志信息写入到lockqueue缓冲区中
// 真正写入磁盘的操作会在专门的线程(构造函数中启动的线程)中完成
void Logger::Log(std::string msg)
{
    m_lockQueue.Push(msg);  // 将日志信息放入到日志队列中
}

Logger::Logger()
{
    // 启动专门的写日志线程(磁盘I/O)
    std::thread writeLogTask([&](){
        for(;;)
        {
            // 获取当天的日期，然后取日志信息，写入相应的日志文件当中
            time_t now = time(nullptr);   // 获取当前时间
            struct tm* localTime = localtime(&now); // 将时间转换为本地时间

            char filename[128] = {0};  // 日志文件名
            sprintf(filename, "%04d-%02d-%02d-log.txt", 
                    localTime->tm_year + 1900, 
                    localTime->tm_mon + 1, 
                    localTime->tm_mday); // 格式化日志文件名
            
            // 打开日志文件, 以追加的方式打开
            FILE* fp = fopen(filename, "a+"); 
            if(fp == nullptr)
            {
                // 日志文件打开失败
                std::cout << "File name: " << filename << "open error!" << std::endl;
                exit(EXIT_FAILURE);
            }
            // 从日志队列中获取日志信息
            std::string msg= m_lockQueue.Pop();

            // 为日志添加详细的时间信息(时分秒信息)
            char timeStr[128] = {0}; // 时间字符串
            sprintf(timeStr, "%d:%d:%d ==> [%s] ", localTime->tm_hour, 
                    localTime->tm_min, 
                    localTime->tm_sec, 
                    (m_logLevel == INFO ? "info" : "error")); // 格式化时间字符串
            msg.insert(0, timeStr); // 在日志信息前面添加时间信息

            // 写入到日志文件中
            fputs(msg.c_str(), fp); // 写入日志信息
            fputs("\n", fp); // 换行
            fclose(fp); // 关闭文件
        }
    });
    // 设置分离线程，守护线程
    writeLogTask.detach(); // 分离线程   
}
