#include <iostream>
#include <vector>
#include <chrono>
#include <atomic>
#include <mutex>
#include <thread>
#include "SyncQueue.h"

using namespace std;
atomic_int g_count = 0;
mutex g_mtx;

template<typename T>
void producer(SyncQueue<T>* syncqueue, const int data_nums)
{
    int i = 0;
    while (i++ < data_nums)
    {
        syncqueue->put(i);
    }    
}

template<typename T>
void consumer(SyncQueue<T>* syncqueue, const int data_nums)
{
    int i = 0;
    while(i ++ < data_nums)
    {
        lock_guard<mutex> locker(g_mtx);
        T data;
        syncqueue->get(data);
        this_thread::sleep_for(chrono::milliseconds(1));
        cout << "thread: " << this_thread::get_id() << " get->" << data << endl;
    }
}

int main()
{
    SyncQueue<int> syncqueue(5);

    const int th_nums = 3;
    const int data_nums = 20;

    // 启动生产者线程,每个线程生产有限的数据
    vector<thread> th_prods;
    for(int i = 0; i < th_nums; i ++)
    {
        th_prods.emplace_back(producer<int>, &syncqueue, data_nums);
    }

    // 启动消费者线程,每个线程都要消费数据
    vector<thread> th_cons;
    for(int i = 0; i < th_nums; i ++)
    {
        th_cons.emplace_back(consumer<int>, &syncqueue, data_nums);
    }

    for(int i = 0; i < th_nums; i ++)
    {
        th_prods[i].join();
        th_cons[i].join();
    }

    cout << g_count << endl;
}
