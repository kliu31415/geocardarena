#pragma once
#include <thread>
#include <memory>
#include <vector>
#include <future>
#include <mutex>
#include <atomic>
class ThreadPool;
class Thread
{
    std::thread t;
    ThreadPool *owner;
public:
    constexpr static uint8_t RECEIVED_STOP_SIGNAL = 1<<0, STOPPED = 1<<1;
    std::atomic<uint8_t> state;
    Thread() = default;
    void start(ThreadPool *owner, std::thread t);
    void send_interrupt(); //doesn't actually attempt to join the thread
    void join(); //normal join
    void force_join(); //interrupts and joins
};
class ThreadPool
{
    std::vector<std::shared_ptr<Thread> > threads;
public:
    template<class Func, class... Args> std::shared_ptr<Thread> create(Func&&, Args&&...);
    void exit();
    void remove_thread(Thread *t);
} thread_pool;
