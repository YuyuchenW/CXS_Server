#include "config.hpp"
#include "fiber.hpp"
#include "iomanager.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstdarg>
#include <cstddef>
#include <dlfcn.h>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>
#include "hook.h"
#include "fd_manager.h"
#include "scheduler.hpp"
#include "timer.h"
#include "log.h"
#include "util.h"
#include <sys/ioctl.h>
CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

namespace CXS {

static CXS::ConfigVar<int>::ptr g_tcp_connect_timeout =
    CXS::Config::Lookup("tcp.connet.timeout", 5000, "tcp connect timeout");
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

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();

        g_tcp_connect_timeout->addListener([](const int &old_vale, const int &new_value) {
            CXS_LOG_INFO(g_logger) << "change tcp.connect.timeout from " << old_vale << " to " << new_value;
            s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() {
    return t_hook_enable;
};

void set_hook_enable(bool flag) {
    t_hook_enable = flag;
};
struct timer_info {
    int cancelled = 0;
};

/*
 * 	fd 			 	文件描述符
 * 	fun				原始函数
 *	hook_fun_name	hook的函数名称
 *	event			事件
 *	timeout_so		超时时间类型
 *	args			可变参数
 *
 * 	例如：return do_io(fd, read_f, "read", sylar::IOManager::READ, SO_RCVTIMEO, buf, count);
 */
template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&...args) {
    // 如果不hook，则直接调用原始函数
    if (!CXS::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }
    // 获取fd对应的Fdctx
    CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(fd);
    // 没有文件
    if (!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }
    // 文件已经关闭
    if (ctx->isClose()) {
        // 坏文件描述符
        errno = EBADF;
        return -1;
    }

    // 不是socket或用户设置非阻塞
    if (!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }
    // 获取超时时间
    uint64_t to = ctx->getTimeout(timeout_so);
    // 设置超时条件
    std::shared_ptr<timer_info> tinfo(new timer_info);
retry:
    // 先执行fun 读数据或写数据 若函数返回值有效就直接返回
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    // CXS_LOG_DEBUG(g_logger) << "do_io <" << hook_fun_name << ">"
    //                         << " n = " << n;
    // 若中断则重试
    while (n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    // 若为阻塞状态
    if (n == -1 && errno == EAGAIN) {
        // 重置EAGIN(errno = 11)，此处已处理，不在向上返回该错误
        errno = 0;
        // 获得当前IO调度器
        CXS::IOManager *iom = CXS::IOManager::GetThis();
        // 定时器
        CXS::Timer::ptr timer;
        // tinfo的弱指针，可以判断tinfo是否已经销毁
        std::weak_ptr<timer_info> winfo(tinfo);

        if (to != (uint64_t)-1) {
            /*	添加条件定时器
             *	to时间消息还没来就触发callback */
            timer = iom->addConditionTImer(
                to, [winfo, fd, iom, event]() {
                    auto t = winfo.lock();
                    /* tinfo失效 || 设了错误
                     * 定时器失效了 */
                    if (!t || t->cancelled) {
                        return;
                    }
                    // 没错误的话设置为超时而失败
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, (CXS::IOManager::Event)(event));
                },
                winfo);
        }
        int c = 0;
        uint64_t now = 0;
        int rt = iom->addEvent(fd, (CXS::IOManager::Event)(event));
        if (rt == -1) {
            CXS_LOG_ERROR(g_logger) << hook_fun_name << "addEvent (" << fd << "," << event << ") retry c = " << c << "used = " << (CXS::GetCurrentUS() - now);
            if (timer) {
                timer->cancel();
            }
            return -1;
        } else {
            CXS::Fiber::YieldToHold();
            if (timer) {
                timer->cancel();
            }
            if (tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
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
    // iom->addTimer(seconds * 1000, [iom, fiber]() { iom->schedule(fiber); });
    iom->addTimer(seconds * 1000, std::bind((void(CXS::Scheduler::*)(CXS::Fiber::ptr, int thread)) & CXS::IOManager::schedule, iom, fiber, -1));
    CXS::Fiber::YieldToHold();

    return 0;
}

int usleep(useconds_t usec) {
    if (!CXS::t_hook_enable) {
        return usleep_f(usec);
    }

    CXS::Fiber::ptr fiber = CXS::Fiber::GetThis();
    CXS::IOManager *iom = CXS::IOManager::GetThis();
    // iom->addTimer(usec / 1000, [iom, fiber]() { iom->schedule(fiber); });
    iom->addTimer(usec / 1000, std::bind((void(CXS::Scheduler::*)(CXS::Fiber::ptr, int thread)) & CXS::IOManager::schedule, iom, fiber, -1));

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
    // iom->addTimer(timeout_ms, [iom, fiber]() { iom->schedule(fiber); });
    iom->addTimer(timeout_ms, std::bind((void(CXS::Scheduler::*)(CXS::Fiber::ptr, int thread)) & CXS::IOManager::schedule, iom, fiber, -1));
    CXS::Fiber::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if (!CXS::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        return fd;
    }
    CXS::FdMgr::GetInstance()->get(fd, true);
    return fd;
}

int accept(int s, struct sockaddr *addr, socklen_t *addr_len) {
    int fd = do_io(s, accept_f, "accept", CXS::IOManager::READ, SO_RCVTIMEO, addr, addr_len);
    if (fd >= 0) {
        CXS::FdMgr::GetInstance()->get(fd, true);
    }
    return fd;
}

int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms) {
    if (!CXS::t_hook_enable) {
        return connect_f(sockfd, addr, addrlen);
    }
    CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(sockfd);
    if (!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }
    if (!ctx->isSocket()) {
        return connect_f(sockfd, addr, addrlen);
    }
    if (ctx->getUserNonblock()) {
        return connect_f(sockfd, addr, addrlen);
    }

    int n = connect_f(sockfd, addr, addrlen);

    if (n == 0) {
        return 0;
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    CXS::IOManager *iom = CXS::IOManager::GetThis();
    CXS::Timer::ptr timer;

    std::shared_ptr<CXS::timer_info> tinfo(new CXS::timer_info);
    std::weak_ptr<CXS::timer_info> winfo(tinfo);

    if (timeout_ms != (uint64_t)-1) {
        timer = iom->addConditionTImer(
            timeout_ms, [winfo, sockfd, iom]() {
                auto t = winfo.lock();
                if (!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(sockfd, CXS::IOManager::WRITE);
            },
            winfo);
    }

    int rt = iom->addEvent(sockfd, CXS::IOManager::WRITE);
    if (rt == 0) {
        CXS::Fiber::YieldToHold();
        if (timer) {
            timer->cancel();
        }
        if (tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if (timer) {
            timer->cancel();
        }
        CXS_LOG_ERROR(g_logger) << "connect addEvent (" << sockfd << ") failed, errno = " << strerror(errno);
    }
    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if (!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, CXS::s_connect_timeout);
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", CXS::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", CXS::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", CXS::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", CXS::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", CXS::IOManager::READ, SO_RCVTIMEO, msg, flags);
}
ssize_t write(int fd, const void *buf, size_t n) {
    return do_io(fd, write_f, "write", CXS::IOManager::WRITE, SO_SNDTIMEO, buf, n);
}
ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", CXS::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}
ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return do_io(sockfd, send_f, "send", CXS::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);
}
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    return do_io(sockfd, sendto_f, "sendto", CXS::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags, dest_addr, addrlen);
}
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    return do_io(sockfd, sendmsg_f, "sendmsg", CXS::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if (!CXS::t_hook_enable) {
        return close_f(fd);
    }
    CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(fd);
    if (ctx) {
        auto iom = CXS::IOManager::GetThis();
        if (iom) {
            iom->cancelAll(fd);
        }
        CXS::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */) {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
    case F_DUPFD:

    case F_DUPFD_CLOEXEC:
    case F_SETFD:
    // 设置文件描述符标志
    case F_SETFL: {
        int arg = va_arg(va, int);
        va_end(va);
        CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(fd);
        if (!ctx || ctx->isClose() || !ctx->isSocket()) {
            return fcntl_f(fd, cmd, arg);
        }
        ctx->setUserNonblock(arg & O_NONBLOCK);
        if (ctx->getSysNonblock()) {
            arg |= O_NONBLOCK;
        } else {
            arg &= ~O_NONBLOCK;
        }
        return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFL: {
        va_end(va);
        int arg = fcntl(fd, cmd);
        CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(fd);
        if (ctx && !ctx->isClose() && ctx->isSocket()) {
            return arg;
        }

        if (ctx->getUserNonblock()) {
            return arg | O_NONBLOCK;
        } else {
            return arg & ~O_NONBLOCK;
        }
    }
    case F_SETOWN:
    case F_SETSIG:
    case F_SETLEASE:
    case F_NOTIFY:
    case F_SETPIPE_SZ: {
        int arg = va_arg(va, int);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    } break;
    case F_GETFD:

    case F_GETOWN:
    case F_GETSIG:
    case F_GETLEASE:
    case F_GETPIPE_SZ: {
        va_end(va);
        return fcntl(fd, cmd);
    } break;
    case F_SETLK:
    case F_SETLKW:
    case F_GETLK: {
        struct flock *arg = va_arg(va, struct flock *);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    }

    break;
    case F_GETOWN_EX:
    case F_SETOWN_EX: {
        struct f_owner_ex *arg = va_arg(va, struct f_owner_ex *);
        va_end(va);
        return fcntl_f(fd, cmd, arg);
    }
    default:
        va_end(va);
        return fcntl(fd, cmd);
        break;
    }
    va_end(va);
    return fcntl(fd, cmd);
}
int ioctl(int fd, unsigned long request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void *);
    va_end(va);

    if (FIONBIO == request) {
        bool user_nonblock = !!*(int *)arg;
        CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(fd);
        if (!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl(fd, request, arg);
        }
    }
    return ioctl_f(fd, request, arg);
}
int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}
int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen) {
    if (!CXS::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }

    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            CXS::FdCtx::ptr ctx = CXS::FdMgr::GetInstance()->get(sockfd);
            if (ctx) {
                const timeval *v = (const timeval *)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}
