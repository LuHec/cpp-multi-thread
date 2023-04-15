// #include <iostream>
// #include <vector>
// #include <chrono>
// #include <atomic>
// #include <mutex>
// #include <thread>
// #include "SyncQueue.h"

// using namespace std;
// atomic_int g_count = 0;
// mutex g_mtx;

// template<typename T>
// void producer(SyncQueue<T>* syncqueue, const int data_nums)
// {
//     int i = 0;
//     while (i++ < data_nums)
//     {
//         syncqueue->put(i);
//     }    
// }

// template<typename T>
// void consumer(SyncQueue<T>* syncqueue, const int data_nums)
// {
//     int i = 0;
//     while(i ++ < data_nums)
//     {
//         lock_guard<mutex> locker(g_mtx);
//         T data;
//         syncqueue->get(data);
//         this_thread::sleep_for(chrono::milliseconds(1));
//         cout << "thread: " << this_thread::get_id() << " get->" << data << endl;
//     }
// }

// int main()
// {
//     SyncQueue<int> syncqueue(5);

//     const int th_nums = 3;
//     const int data_nums = 20;

//     // 启动生产者线程,每个线程生产有限的数据
//     vector<thread> th_prods;
//     for(int i = 0; i < th_nums; i ++)
//     {
//         th_prods.emplace_back(producer<int>, &syncqueue, data_nums);
//     }

//     // 启动消费者线程,每个线程都要消费数据
//     vector<thread> th_cons;
//     for(int i = 0; i < th_nums; i ++)
//     {
//         th_cons.emplace_back(consumer<int>, &syncqueue, data_nums);
//     }

//     for(int i = 0; i < th_nums; i ++)
//     {
//         th_prods[i].join();
//         th_cons[i].join();
//     }

//     cout << g_count << endl;
// }

#include <iostream>
#include <string>
#include <thread>                      // 线程类头文件。
#include <mutex>                      // 互斥锁类的头文件。
#include <deque>                      // deque容器的头文件。
#include <queue>                      // queue容器的头文件。
#include <condition_variable>  // 条件变量的头文件。
using namespace std;
class AA
{
    mutex m_mutex;                                    // 互斥锁。
    condition_variable m_cond;                  // 条件变量。
    queue<string, deque<string>> m_q;   // 缓存队列，底层容器用deque。
public:
    void incache(int num)     // 生产数据，num指定数据的个数。
    {
        lock_guard<mutex> lock(m_mutex);   // 申请加锁。
        for (int ii=0 ; ii<num ; ii++)
        {
            static int bh = 1;           // 超女编号。
            string message = to_string(bh++) + "号超女";    // 拼接出一个数据。
            m_q.push(message);     // 把生产出来的数据入队。
        }
        //m_cond.notify_one();     // 唤醒一个被当前条件变量阻塞的线程。
        m_cond.notify_all();          // 唤醒全部被当前条件变量阻塞的线程。
    }
    
    void outcache()   {    // 消费者线程任务函数。
        while (true)   {
            // 把互斥锁转换成unique_lock<mutex>，并申请加锁。
            unique_lock<mutex> lock(m_mutex);

            // 条件变量虚假唤醒：消费者线程被唤醒后，缓存队列中没有数据。
            //while (m_q.empty())    // 如果队列空，进入循环，否则直接处理数据。必须用循环，不能用if
            //    m_cond.wait(lock);  // 1）把互斥锁解开；2）阻塞，等待被唤醒；3）给互斥锁加锁。
            m_cond.wait(lock, [this] { return !m_q.empty(); });

            // 数据元素出队。
            string message = m_q.front();  m_q.pop();
            cout << "线程：" << this_thread::get_id() << "，" << message << endl;
            lock.unlock();      // 手工解锁。

            // 处理出队的数据（把数据消费掉）。
            this_thread::sleep_for(chrono::milliseconds(1));   // 假设处理数据需要1毫秒。
        }
    }
};

int main()
{
    AA aa;
  
    thread t1(&AA::outcache, &aa);     // 创建消费者线程t1。
    thread t2(&AA::outcache, &aa);     // 创建消费者线程t2。
    thread t3(&AA::outcache, &aa);     // 创建消费者线程t3。

    this_thread::sleep_for(chrono::seconds(2));    // 休眠2秒。
    aa.incache(2);      // 生产2个数据。

    this_thread::sleep_for(chrono::seconds(3));    // 休眠3秒。
    aa.incache(5);      // 生产5个数据。

    t1.join();   // 回收子线程的资源。
    t2.join();
    t3.join(); 
}