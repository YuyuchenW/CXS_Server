#include "fiber.hpp"
#include "scheduler.hpp"
namespace CXS {
// 全局协程id计数器
static std::atomic<uint64_t> s_fiber_id(0);
// 用于统计当前的协程数
static std::atomic<uint64_t> s_fiber_count(0);
// 使用system进行日志输出
static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");
// 当前协程
// 初始化时，指向线程主协程
static thread_local Fiber *t_fiber = nullptr;

/* 指向线程的主协程
初始化时，指向线程主协程
当子协程resume时，主协程让出执行权，并保存上下文到t_threadFiber的ucontext_t中
同时激活子协程的ucontext_t的上下文。
子协程yield时，子协程让出执行权，从t_threadFiber获得主协程上下文恢复运行。*/
static thread_local Fiber::ptr t_threadFiber = nullptr;
// 约定协程栈的大小1MB
static ConfigVar<uint32_t>::ptr g_fiber_stack_size =
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");
class MallocStackAllocator {
public:
    static void *Alloc(size_t size) {
        return malloc(size);
    }
    static void Dealloc(void *vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

// 协程构造函数
// 初始化协程状态为 EXEC（执行中）
// 设置当前线程的当前协程为 this
// 使用 getcontext 初始化 m_ctx
// 增加活跃协程计数
Fiber::Fiber() {
    m_state = EXEC;
    // 设置当前协程
    SetThis(this);
    // 获取当前协程的上下文信息保存到m_ctx中
    if (getcontext(&m_ctx)) {
        CXS_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;

    CXS_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) :
    m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    // 若给了初始化值则用给定值，若没有则用约定值
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();
    // 获得协程运行指针
    m_stack = StackAllocator::Alloc(m_stacksize);
    // 保存当前协程上下文信息到m_ctx中
    if (getcontext(&m_ctx)) {
        CXS_ASSERT2(false, "getcontext");
    }

    // uc_link为空，执行完当前context之后退出程序。
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    // 指明该context入口函数
    if (!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }
    CXS_LOG_DEBUG(g_logger) << "Fiber::Fiber(p)   id: " << m_id;
}
Fiber::~Fiber() {
    CXS_LOG_DEBUG(g_logger) << "~Fiber::Fiber  id:" << m_id;
    --s_fiber_count;
    if (m_stack) {
        // 不在准备和运行状态
        CXS_ASSERT(m_state == TERM || m_state == EXECEP || m_state == INIT);
        // 释放运行栈
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        // 主协程的释放要保证没有任务并且当前正在运行
        CXS_ASSERT(!m_cb);
        CXS_ASSERT(m_state == EXEC);
        Fiber *cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

void Fiber::reset(std::function<void()> cb) {
    // 主协程不分配栈
    CXS_ASSERT(m_stack);
    // 当前协程不在准备和运行态
    CXS_ASSERT(m_state == TERM || m_state == INIT || m_state == EXECEP);
    m_cb = cb;
    if (getcontext(&m_ctx)) {
        CXS_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}


// 从调度器的主协程切换到当前协程
void Fiber::swapIn() {
    SetThis(this);
    CXS_ASSERT(m_state != EXEC);
    m_state = EXEC;
    if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        CXS_ASSERT2(false, "swapIn_context");
    }
}

// 从协程主协程切换到当前协程
void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        CXS_ASSERT2(false, "swapcontext");
    }
}

// 从当前协程切换到主协程
void Fiber::back() {
    SetThis(t_threadFiber.get());
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        CXS_ASSERT2(false, "swapcontext");
    }
}

// 从当前协程切换到调度器主协程
void Fiber::swapOut() {

    SetThis(Scheduler::GetMainFiber());
    if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        perror("swapcontext failed");
        printf("Error: %s\n", strerror(errno));
        CXS_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber *f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    Fiber::ptr main_fiber(new Fiber);
    CXS_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}
void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    CXS_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception &ex) {
        cur->m_state = EXECEP;
        CXS_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                << " fiber_id=" << cur->getId()
                                << std::endl
                                << CXS::BacktraceToString();
    } catch (...) {
        cur->m_state = EXECEP;
        CXS_LOG_ERROR(g_logger) << "Fiber Except"
                                << " fiber_id=" << cur->getId()
                                << std::endl
                                << CXS::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    CXS_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    CXS_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception &ex) {
        cur->m_state = EXECEP;
        CXS_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                                << " fiber_id=" << cur->getId()
                                << std::endl
                                << CXS::BacktraceToString();
    } catch (...) {
        cur->m_state = EXECEP;
        CXS_LOG_ERROR(g_logger) << "Fiber Except"
                                << " fiber_id=" << cur->getId()
                                << std::endl
                                << CXS::BacktraceToString();
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    CXS_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}
} // namespace CXS