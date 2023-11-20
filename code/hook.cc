#include "fiber.hpp"
#include "iomanager.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <dlfcn.h>
#include <memory>
#include <sys/types.h>
#include <utility>
#include "hook.h"
#include "fd_manager.h"
#include "timer.h"
namespace CXS {

static thread_local bool t_hook_enable = false;
#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(getsockopt)   \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

struct _HookIniter {
    _HookIniter() {
        hook_init();
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
};

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
};
struct tiemr_info {
    int cancelled = 0;
};

template <typename OriginFun, typename... Args>
static size_t do_io(int fd, OriginFun fun, const char *hook_fun_name, uint32_t event, int timeout_so, size_t buflen, Args &&...args) {
    if (!CXS::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }
    CXS::FdCtx::ptr ctx = CXS::Fdmgr::GetInstance()->get(fd);
    if (!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    if (!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);

    std::shared_ptr<tiemr_info> tinfo(new tiemr_info);

    ssize_t  n =fun(fd, std::forward<Args>(args)...);
    while (n == -1 && errno == EINTR) {
        n =fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) {
        CXS::IOManager* iom = CXS::IOManager::GetThis();
        CXS::Timer::ptr timer;
        std::weak_ptr<tiemr_info> winfo(tinfo);
        if(to != (uint64_t)-1)
        {
            timer = iom->addConditionTImer(to, std::function<void ()> cb, std::weak_ptr<void> weal_cond)
        }
    }
}   
} // namespace CXS

extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!CXS::t_hook_enable) {
        return sleep_f(seconds);
    }

    CXS::Fiber::ptr fiber = CXS::Fiber::GetThis();
    CXS::IOManager *iom = CXS::IOManager::GetThis();
    iom->addTimer(seconds * 1000, [iom, fiber]() { iom->schedule(fiber); });
    CXS::Fiber::YieldToHold();

    return 0;
}

int usleep(useconds_t usec) {
    if (!CXS::t_hook_enable) {
        return usleep_f(usec);
    }

    CXS::Fiber::ptr fiber = CXS::Fiber::GetThis();
    CXS::IOManager *iom = CXS::IOManager::GetThis();
    iom->addTimer(usec / 1000, [iom, fiber]() { iom->schedule(fiber); });
    CXS::Fiber::YieldToHold();

    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!CXS::t_hook_enable) {
        return nanosleep_f(req, rem);
    }

    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000000;

    CXS::Fiber::ptr fiber = CXS::Fiber::GetThis();
    CXS::IOManager *iom = CXS::IOManager::GetThis();
    iom->addTimer(timeout_ms, [iom, fiber]() { iom->schedule(fiber); });
    CXS::Fiber::YieldToHold();
    return 0;
}
}