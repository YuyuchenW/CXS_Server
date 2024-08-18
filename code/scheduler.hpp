#ifndef __CXS_SCHEDULER_H__
#define __CXS_SCHEDULER_H__

#include <memory>
#include "thread.h"
#include "fiber.hpp"
#include <list>
#include <vector>
#include <functional>

namespace CXS {
class Scheduler {
public:
    typedef CXS::Mutex MutexType;
    typedef std::shared_ptr<Scheduler> ptr;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "");
    ~Scheduler();
    const std::string &getName() const {
        return m_name;
    }

    static Scheduler *GetThis();
    static Fiber *GetMainFiber();

    void start();
    void stop();
    // 调度协程
    template <class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            // 将任务加入到队列中，若任务队列中已经有任务了，则tickle（）
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }

        if (need_tickle) {
            tickle();
        }
    };

    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if (need_tickle) {
            tickle();
        }
    };

protected:
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();
    void setThis();

    bool hasIdleThreads() {
        return m_idleThreadCount > 0;
    }

private:
    template <class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    };

    struct FiberAndThread {
        //协程
        Fiber::ptr fiber;
        //协程执行的函数
        std::function<void()> cb;
        // 线程id 协程在哪个线程上
        int thread;
        // 确定协程在哪个线程上跑
        FiberAndThread(Fiber::ptr f, int thr) :
            fiber(f), thread(thr){};
        FiberAndThread(Fiber::ptr *f, int thr) :
            thread(thr) {
            // 通过swap将传入的 fiber 置空，使其引用计数-1
            fiber.swap(*f);
        }

        // 确定回调在哪个线程上跑
        FiberAndThread(std::function<void()> f, int thr) :
            cb(f), thread(thr){};
        // 通过swap将传入的 cb 置空，使其引用计数-1
        FiberAndThread(std::function<void()> *f, int thr) :
            thread(thr) {
            cb.swap(*f);
        };
        // 默认构造
        FiberAndThread() :
            thread(-1){};

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    MutexType m_mutex;
    // 线程池
    std::vector<Thread::ptr> m_threads;
    // 待执行的协程队列
    std::list<FiberAndThread> m_fibers;
    // 协程调度器名称
    std::string m_name;
    // use_caller为true时有效，调度协程
    Fiber::ptr m_rootFiber;
    int m_idleThreadCount = 0;

protected:
    // 协程下的线程id数组
    std::vector<int> m_threadIds;
    // 线程数量
    std::atomic<size_t> m_threadCount = {0};
    // 工作线程数量
    std::atomic<size_t> m_activeThreadCount = {0};
    // 是否正在停止
    bool m_stopping = true;
    // 是否自动停止
    bool m_autoStop = false;
    //主线程id
    int m_rootThread = 0;
};
} // namespace CXS

#endif