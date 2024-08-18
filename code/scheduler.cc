#include "scheduler.hpp"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace CXS {
static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");
// 当前协程调度器
static thread_local Scheduler *t_scheduler = nullptr;
// 线程主协程
static thread_local Fiber *t_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name) :
    m_name(name) {
    CXS_ASSERT(threads > 0);
    // use_caller 为true 表示协程调度线程也参加协程调度
    if (use_caller) {
        CXS::Fiber::GetThis();
        --threads;
        // 将调度器指针设置为当前调度器对象
        CXS_ASSERT(GetThis() == nullptr);
        t_scheduler = this;
        // 将此fiber设置为 use_caller，协程则会与 Fiber::CallerMainFunc() 绑定
        // 非静态成员函数需要传递this指针作为第一个参数，用 std::bind()进行绑定
        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));

        CXS::Thread::SetName(m_name);
        // 设置当前线程的主协程为m_rootFiber
        // 这里的m_rootFiber是该线程的主协程（执行run任务的协程），只有默认构造出来的fiber才是主协程
        t_fiber = m_rootFiber.get();
        // 获取当前线程的id，用于标识主线程
        m_rootThread = CXS::GetThreadId();

        m_threadIds.push_back(m_rootThread);
    } else {
        m_rootThread = -1;
    }
    m_threadCount = threads;
}
Scheduler::~Scheduler() {
    CXS_ASSERT(m_stopping);
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler *Scheduler::GetThis() {
    return t_scheduler;
}
Fiber *Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    CXS_LOG_INFO(g_logger) << "start";
    MutexType::Lock lock(m_mutex);
    if (!m_stopping) {
        return;
    }
    m_stopping = false;
    CXS_ASSERT(m_threads.empty())
    m_threads.resize(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
        m_threadIds.push_back(m_threads[i]->getId());
    }

    lock.unlock();
    // if (m_rootFiber != nullptr)
    // {
    //     m_rootFiber->call();
    //     CXS_LOG_INFO(g_logger) << "call";
    // }
}
void Scheduler::stop() {
    m_autoStop = true;
    // 使用use_caller,并且只有一个线程，并且主协程的状态为结束或者初始化
    if (m_rootFiber
        && m_threadCount == 0
        && (m_rootFiber->getState() == Fiber::TERM
            || m_rootFiber->getState() == Fiber::INIT)) {
        CXS_LOG_INFO(g_logger) << this->m_name << " sheduler stopped";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    // use_caller线程

    if (m_rootThread != -1) {
        // 当前调度器和t_secheduler相同
        CXS_ASSERT(GetThis() == this);
    } else {
        // 非use_caller，此时的t_secheduler应该为nullptr
        CXS_ASSERT(GetThis() != this);
    }
    m_stopping = true;
    for (size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }
    // 使用use_caller，只要没达到停止条件，调度器主协程交出执行权，执行run
    if (m_rootFiber) {
        if (!stopping()) {
            m_rootFiber->call();
        }
    }
    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for (auto &i : thrs) {
        i->join();
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::run() {
    CXS_LOG_DEBUG(g_logger) << m_name << " run";
    // hook
    set_hook_enable(true);
    // 设置当前调度器
    setThis();
    // 非user_caller线程，设置主协程为线程主协程
    if (CXS::GetThreadId() != m_rootThread) {
        t_fiber = Fiber::GetThis().get();
    }
    // 定义idle_fiber，当任务队列中的任务执行完之后，执行idle()
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));

    // 创建一个协程对象，用于存放需要执行的协程
    Fiber::ptr cb_fiber;

    // 声明一个结构体 FiberAndThread，用于存放协程和线程信息
    FiberAndThread ft;
    while (true) {
        ft.reset();
        // 用于标记是否需要唤醒其他线程
        bool tickle_me = false;
        // 用于标记当前是否有协程在执行
        bool is_active = false;
        {
            // 从任务队列中拿fiber和cb
            MutexType::Lock lock(m_mutex);
            auto it = m_fibers.begin();
            while (it != m_fibers.end()) {
                // 如果协程的线程信息不匹配当前线程，则将该协程留在队列中并继续查找
                if (it->thread != -1 && it->thread != CXS::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }

                CXS_ASSERT(it->fiber || it->cb);
                // 如果协程有效且处于执行状态，则继续查找
                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }
                // 获取一个有效的协程或回调
                ft = *it;
                m_fibers.erase(it);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end();
        }
        // 如果需要唤醒其他线程，就执行唤醒操作
        if (tickle_me) {
            tickle();
        }
        // 如果任务是fiber，并且任务处于可执行状态
        if (ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXECEP)) {
            // 切换到要执行的协程
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if (ft.fiber->getState() == Fiber::READY) {
                // 如果协程处于就绪状态，重新调度该协程
                schedule(ft.fiber);
            } else if (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXECEP) {
                // 如果协程不处于终止或异常状态，将其状态设置为 HOLD
                ft.fiber->setState(Fiber::HOLD);
            }
            ft.reset();
        } else if (ft.cb) {
            // cb_fiber存在，重置该fiber
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                // cb_fiber不存在，new新的fiber
                cb_fiber.reset(new Fiber(ft.cb));
            }
            // 重置数据ft
            ft.reset();
            // 切换到回调协程执行
            cb_fiber->swapIn();
            --m_activeThreadCount;
            // 若cb_fiber状态为READY
            if (cb_fiber->getState() == Fiber::READY) {
                // 重新放入任务队列中
                schedule(cb_fiber);
                // 释放智能指针
                cb_fiber.reset();
            }
            // cb_fiber异常或结束，就重置状态，可以再次使用该cb_fiber
            else if (cb_fiber->getState() == Fiber::EXECEP || cb_fiber->getState() == Fiber::TERM) {
                // 设置状态为HOLD，此任务后面还会通过ft.fiber被拉起
                cb_fiber->reset(nullptr);
            } else {
                // 如果回调协程不处于终止状态，将其状态设置为 HOLD
                cb_fiber->setState(Fiber::HOLD);
                // 释放该智能指针，调用下一个任务时要重新new一个新的cb_fiber
                cb_fiber.reset();
            }
        } else {
            if (is_active) {
                --m_activeThreadCount;
                continue;
            }
            if (idle_fiber->getState() == Fiber::TERM) {
                // 如果空闲协程处于终止状态，跳出循环
                CXS_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXECEP) {
                // 如果空闲协程不处于终止或异常状态，将其状态设置为 HOLD
                idle_fiber->setState(Fiber::HOLD);
            }
        }
    }
}

void Scheduler::tickle() {
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);
    return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}
void Scheduler::idle() {
    CXS_LOG_INFO(g_logger) << "idle";
    while (!stopping()) {
        CXS::Fiber::YieldToHold();
    }
}

} // namespace CXS
