#include "thread_manager.h"
#include <memory>
#include <thread>
using namespace std;
//Thread
void Thread::start(ThreadPool *owner, thread t)
{
    this->owner = owner;
    this->t = move(t);
    this->state = 0;
}
void Thread::send_interrupt()
{
    state |= RECEIVED_STOP_SIGNAL;
}
void Thread::join()
{
    t.join();
    owner->remove_thread(this);
}
void Thread::force_join()
{
    send_interrupt();
    join();
}
//ThreadPool
template<class Func, class... Args> shared_ptr<Thread> ThreadPool::create(Func &&func, Args &&...args)
{
    shared_ptr<Thread> t = make_shared<Thread>();
    t->start(this, thread(func, t, args...));
    threads.push_back(t);
    return t;
}
void ThreadPool::exit()
{
    for(auto &i: threads)
        i->send_interrupt();
    for(auto &i: threads)
        i->join();
}
void ThreadPool::remove_thread(Thread *t)
{
    for(auto i=threads.begin(); i!=threads.end(); i++)
    {
        if(t == i->get())
        {
            i = threads.erase(i);
            return;
        }
    }
}
