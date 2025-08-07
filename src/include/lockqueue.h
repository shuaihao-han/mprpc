#pragma once
#include <mutex>
#include <queue>
#include <mutex>  // pthread_mutex_t
#include <condition_variable>  // pthread_condition_t

// 异步写日志的日志队列
// 使用模板来实现
// 模板代码的实现只能放在头文件中， 而不能放到.cpp文件中
template<typename T>
class LockQueue
{
public:
    // 向队列中添加数据
    // 多个worker都会写日志queue
    void Push(const T &data)
    {
        // 使用 std::lock_guard 来上锁，保证线程安全
        std::lock_guard<std::mutex> lock(m_mutex);  // 上锁
        m_queue.push(data);  // 将数据放入队列
        // m_queue 不为空 通知等待的后台磁盘写入线程
        m_cond.notify_one();  // 通知一个等待的线程
    }

    // 从队列中弹出数据
    // 一个线程读日志queue, 
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 如果队列为空，则线程进入wait状态, 等待数据的到来
            m_cond.wait(lock);  // 等待条件变量通知
        }

        T data = m_queue.front();  // 获取队列头部数据
        m_queue.pop();  // 弹出队列头部数据
        return data;  // 返回数据
    }
private:
    std::queue<T> m_queue;  // 队列
    std::mutex m_mutex;  // 互斥锁, 用于互斥访问实现线程安全的队列
    std::condition_variable m_cond; // 条件变量, 用于实现线程间通信
};
