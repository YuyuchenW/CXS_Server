#ifndef __CXS_FIBER_H__
#define __CXS_FIBER_H__

#include <functional>
#include <memory>
#include <ucontext.h>
#include "thread.h"
#include "log.h"

#include "config.hpp"
#include "macro.h"
#include <atomic>


namespace CXS
{
    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    friend class Schdeduler;
    public:
        typedef std::shared_ptr<Fiber> ptr;

        enum State
        {
            INIT,
            HOLD,
            EXEC,
            TERM,
            READY,
            EXECEP
        };

    private:
        Fiber();

    public:
        Fiber(std::function<void()> cb, size_t stacksize = 0 , bool use_caller = false);
        ~Fiber();
        // 重置协程函数，并重置状态
        void reset(std::function<void()> cb);
        // 切换到当前协程执行
        void swapIn();
        void call();
        void back();
        // 切换到后台执行
        void swapOut();
        // 获取线程状态
        const State& getState() const {return m_state;}
        void setState(State state){ m_state = state;}
        uint64_t getId() const { return m_id;}
    public:
        
        // 设置当前协程
        static void SetThis(Fiber *f);
        // 返回当前协程
        static Fiber::ptr GetThis();
        // 协程切换到后台，并且设置位ready
        static void YieldToReady();
        // 协程切换到后台，并且设置位hold
        static void YieldToHold();
        // 总协程数
        static uint64_t TotalFibers();

        static void MainFunc();
        static void CallerMainFunc();
        // 获取协程id
        static uint64_t GetFiberId();

    private:
        uint64_t m_id = 0;
        uint32_t m_stacksize = 0;

        State m_state = INIT;

        ucontext_t m_ctx;
        void *m_stack = nullptr;

        std::function<void()> m_cb;
    };
} // namespace CXS

#endif