#include "scheduler.hpp"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace CXS
{
    static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");
    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
        : m_name(name)
    {

        CXS_ASSERT(threads > 0);
        // use_caller 为true 表示主线程也参加协程调度
        if (use_caller)
        {

            CXS::Fiber::GetThis();
            --threads;
            // 将调度器指针设置为当前调度器对象
            CXS_ASSERT(GetThis() == nullptr);
            t_scheduler = this;
            // 创建一个主协程，执行的函数是调度器的run
            m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));

            CXS::Thread::SetName(m_name);
            // 将当前协程指针设置为主协程对象
            t_fiber = m_rootFiber.get();
            // 获取当前线程的id，用于标识主线程
            m_rootThread = CXS::GetThreadId();

            m_threadIds.push_back(m_rootThread);
        }
        else
        {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }
    Scheduler::~Scheduler()
    {
        CXS_ASSERT(m_stopping);
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }
    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
    }

    void Scheduler::start()
    {
        MutexType::Lock lock(m_mutex);
        if (!m_stopping)
        {
            return;
        }
        m_stopping = false;
        CXS_LOG_INFO(g_logger) << "start";
        CXS_ASSERT(m_threads.empty())
        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
        // CXS_LOG_INFO(g_logger) << "end";
        lock.unlock();
        // if (m_rootFiber != nullptr)
        // {
        //     m_rootFiber->call();
        //     CXS_LOG_INFO(g_logger) << "call";
        // }
    }
    void Scheduler::stop()
    {
        m_autoStop = true;
        if (m_rootFiber && m_threadCount == 0 &&
            (m_rootFiber->getState() == Fiber::TERM || m_rootFiber->getState() == Fiber::INIT))
        {
            CXS_LOG_INFO(g_logger) << this << "stopped";
            m_stopping = true;

            if (stopping())
            {
                return;
            }
        }

        if (m_rootThread != -1)
        {
            CXS_ASSERT(GetThis() == this);
        }
        else
        {
            CXS_ASSERT(GetThis() != this);
        }
        m_stopping = true;
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }
        if (m_rootFiber)
        {
            if (!stopping())
            {
                m_rootFiber->call();
            }
        }
        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }

        for (auto &i : thrs)
        {
            i->join();
        }
    }

    void Scheduler::setThis()
    {
        t_scheduler = this;
    }

    void Scheduler::run()
    {
        CXS_LOG_DEBUG(g_logger) << m_name << " run";
        // 将调度器指针设为当前调度器对象
        set_hook_enable(true);

        setThis();
        // 如果当前线程不是主线程。将当前协程设置为主协程
        if (CXS::GetThreadId() != m_rootThread)
        {
            t_fiber = Fiber::GetThis().get();
        }
        // 创建一个空闲协程用于处理空闲状态
        Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));

        // 创建一个协程对象，用于存放需要执行的协程
        Fiber::ptr cb_fiber;

        // 声明一个结构体 FiberAndThread，用于存放协程和线程信息
        FiberAndThread ft;
        while (true)
        {
            // 情况结构体
            ft.reset();
            // 用于标记是否需要唤醒其他线程
            bool tickle_me = false;
            // 用于标记当前是否有协程在执行
            bool is_active = false;
            {

                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end())
                {
                    // 如果协程的线程信息不匹配当前线程，则将该协程留在队列中并继续查找
                    if (it->thread != -1 && it->thread != CXS::GetThreadId())
                    {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    CXS_ASSERT(it->fiber || it->cb);
                    // 如果协程有效且处于执行状态，则继续查找
                    if (it->fiber && it->fiber->getState() == Fiber::EXEC)
                    {
                        ++it;
                        continue;
                    }
                    // 获取一个有效的协程或回调
                    ft = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    is_active = true;
                    break;
                }
                tickle_me |= it != m_fibers.end();
            }
            // 如果需要唤醒其他线程，就执行唤醒操作
            if (tickle_me)
            {
                tickle();
            }

            if (ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXECEP))
            {
                // 切换到要执行的协程
                ft.fiber->swapIn();
                --m_activeThreadCount;

                if (ft.fiber->getState() == Fiber::READY)
                {
                    // 如果协程处于就绪状态，重新调度该协程
                    schedule(ft.fiber);
                }
                else if (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXECEP)
                {
                    // 如果协程不处于终止或异常状态，将其状态设置为 HOLD
                    ft.fiber->setState(Fiber::HOLD);
                }
                ft.reset();
            }
            else if (ft.cb)
            {
                if (cb_fiber)
                {
                    // 如果有回调协程正在运行，重置其回调函数
                    cb_fiber->reset(ft.cb);
                }
                else
                {
                    // 否则创建一个回调协程并设置回调函数
                    cb_fiber.reset(new Fiber(ft.cb));
                }
                ft.reset();
                // 切换到回调协程执行
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if (cb_fiber->getState() == Fiber::READY)
                {
                    // 如果回调协程处于就绪，重新调度它
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                else if (cb_fiber->getState() == Fiber::EXECEP || cb_fiber->getState() == Fiber::TERM)
                {
                    // 如果回调协程处于异常或终止状态，重置回调函数
                    cb_fiber->reset(nullptr);
                }
                else
                { // if(cb_fiber->getState() != Fiber::TERM) {
                    // 如果回调协程不处于终止状态，将其状态设置为 HOLD
                    cb_fiber->setState(Fiber::HOLD);
                    cb_fiber.reset();
                }
            }
            else
            {
                if (is_active)
                {
                    --m_activeThreadCount;
                    continue;
                }
                if (idle_fiber->getState() == Fiber::TERM)
                {
                    // 如果空闲协程处于终止状态，跳出循环
                    CXS_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }

                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXECEP)
                {
                    // 如果空闲协程不处于终止或异常状态，将其状态设置为 HOLD
                    idle_fiber->setState(Fiber::HOLD);
                }
            }
        }
    }

    void Scheduler::tickle()
    {
        CXS_LOG_INFO(g_logger) << "tickle";
    }

    bool Scheduler::stopping()
    {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }
    void Scheduler::idle()
    {
        CXS_LOG_INFO(g_logger) << "idle";
        while (!stopping())
        {
            CXS::Fiber::YieldToHold();
        }
    }

}
