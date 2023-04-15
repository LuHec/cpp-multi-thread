#pragma once 

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

template<class T>
class SyncQueue
{
public:
    SyncQueue(size_t q_size) : _max_size(q_size){}
    SyncQueue(const SyncQueue & rhs) = delete;
    ~SyncQueue() {}

    void put(T& data)
    {
        std::unique_lock u_mtx(_mtx);
        // 如果队列满了，就唤醒消费者变量，并进行等待
        while(is_full())
        {
            _con_c.notify_all();
            _con_p.wait(u_mtx);
        }
        // 不满了，推入数据
        _queue.push(data);

        // 有数据，直接唤醒消费者变量
        _con_c.notify_one();
    }

    void get(T& data)
    {
        std::unique_lock u_mtx(_mtx);
        while(is_empty())
        {
            // 当队列没有数据，一直等待
            _con_c.wait(u_mtx, [this] {return !is_empty();});    
        }

        data = _queue.front();
        _queue.pop();
        // 取出数据后唤醒prod线程
        _con_p.notify_one();
    }

    bool full()
    {
        // 公有函数，获取数据时要加锁
        std::lock_guard<std::mutex> locker(_mtx);
        return _queue.size() == _max_size;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> locker(_mtx);
        return _queue.empty();
    }


private:
    std::queue<T> _queue;
    std::mutex _mtx;
    std::condition_variable _con_p;
    std::condition_variable _con_c;
    size_t _max_size;

    bool is_full()
    {
        return _queue.size() == _max_size;
    }

    bool is_empty()
    {
        return _queue.empty();
    }
};

