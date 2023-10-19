
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <string>
#include <memory>

#include "macro.h"
#include "log.h"
#include "iomanager.h"
namespace CXS
{
    static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

    // epoll use
    // epoll_create创建epoll实例
    // epoll_ctl
    // epoll_wait
    IOManager::IOManager(size_t threads, bool use_caller, const std::string &name) : Scheduler(threads, use_caller, name)
    {
        m_epfd = epoll_create(10);
        CXS_ASSERT(m_epfd > 0);
        int rt = pipe(m_tickleFds);
        CXS_ASSERT(rt);
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.fd = m_tickleFds[0];
        rt = fcntl(m_tickleFds[0], F_SETFD, O_NONBLOCK);
        CXS_ASSERT(rt);

        contextResize(32);

        start();
    }
    IOManager::~IOManager()
    {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (m_fdContexts[i])
            {
                delete m_fdContexts[i];
            }
        }
    }
    IOManager::FdContext::EventContext &IOManager::FdContext::getContext(IOManager::Event event)
    {
        switch (event)
        {
        case IOManager::READ:
            return read;
        case IOManager::WRITE:
            return write;
        default:
            CXS_ASSERT2(false, "getContext");
        }
    }

    void IOManager::FdContext::resetContext(EventContext &ctx)
    {
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }

    void IOManager::FdContext::triggerEvent(Event event)
    {
        CXS_ASSERT(events & event);
        events = (Event)(events & ~event);
        EventContext &ctx = getContext(event);
        if (ctx.cb)
        {
            ctx.scheduler->schedule(&ctx.cb);
        }
        else
        {
            ctx.scheduler->schedule(&ctx.fiber);
        }
        ctx.scheduler = nullptr;
        return;
    }

    void IOManager::contextResize(size_t size)
    {
        m_fdContexts.resize(size);

        for (size_t i = 0; i < m_fdContexts.size(); ++i)
        {
            if (m_fdContexts[i] == nullptr)
            {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = i;
            }
        }
    }
    // 1 success|| 0 retry ||-1 error
    int IOManager::addEvent(int fd, Event event, std::function<void()> cb = nullptr)
    {
        FdContext *fd_ctx = nullptr;
        RWMutexType::ReadLock lock(m_mutex);
        if (m_fdContexts.size() > fd)
        {
            fd_ctx = m_fdContexts[fd];
            lock.unlock();
        }
        else
        {
            lock.unlock();
            RWMutexType::WriteLock lock2(m_mutex);
            contextResize(fd * 1.5);
            fd_ctx = m_fdContexts[fd];

            FdContext::MutexType::Lock lock2(fd_ctx->mutex);

            if (fd_ctx->events & event)
            {
                CXS_LOG_ERROR(g_logger) << "addEvent assert fd =" << fd
                                        << " event = " << event
                                        << " fd_ctx.event = " << fd_ctx->events;
                CXS_ASSERT(!(fd_ctx->events & event));
            }

            int op = fd_ctx->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
            epoll_event epevent;
            epevent.events = EPOLLET | fd_ctx->events | event;
            epevent.data.ptr = fd_ctx;

            int rt = epoll_ctl(m_epfd, op, fd, &epevent);

            if (rt)
            {
                CXS_LOG_ERROR(g_logger) << "epoll_clt (" << m_epfd << ","
                                        << op << "," << fd << "," << epevent.events << "),"
                                        << rt << " ( " << errno << ") (" << strerror(errno) << ")";
                return 1;
            }

            ++m_pendingEventCount;
            fd_ctx->events = (Event)(fd_ctx->events | event);
            FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
            CXS_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);
            event_ctx.scheduler = Scheduler::GetThis();
            if (cb)
            {
                event_ctx.cb.swap(cb);
            }
            else
            {
                event_ctx.fiber = Fiber::GetThis();
                CXS_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
            }
            return 0;
        }
    }
    bool IOManager::delEvent(int fd, Event event)
    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if (!(fd_ctx->events & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if (rt)
        {
            CXS_LOG_ERROR(g_logger) << "epoll_clt (" << m_epfd << ","
                                    << op << "," << fd << "," << epevent.events << "),"
                                    << rt << " ( " << errno << ") (" << strerror(errno) << ")";
            return false;
        }
        fd_ctx->events = new_events;
        FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);
        --m_pendingEventCount;
        return true;
    }
    bool IOManager::cancelEvent(int fd, Event event)
    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if (!(fd_ctx->events & event))
        {
            return false;
        }

        Event new_events = (Event)(fd_ctx->events & ~event);
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET | new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if (rt)
        {
            CXS_LOG_ERROR(g_logger) << "epoll_clt (" << m_epfd << ","
                                    << op << "," << fd << "," << epevent.events << "),"
                                    << rt << " ( " << errno << ") (" << strerror(errno) << ")";
            return false;
        }
        fd_ctx->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }

    bool IOManager::cancelAll(int fd)
    {
        RWMutexType::ReadLock lock(m_mutex);
        if (m_fdContexts.size() <= fd)
        {
            return false;
        }
        FdContext *fd_ctx = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->mutex);
        if (!fd_ctx->events)
        {
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epfd, op, fd, &epevent);
        if (rt)
        {
            CXS_LOG_ERROR(g_logger) << "epoll_clt (" << m_epfd << ","
                                    << op << "," << fd << "," << epevent.events << "),"
                                    << rt << " ( " << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        if (fd_ctx->events & READ)
        {
            fd_ctx->triggerEvent(READ);
            --m_pendingEventCount;
        }
        if (fd_ctx->events & WRITE)
        {
            fd_ctx->triggerEvent(WRITE);
            --m_pendingEventCount;
        }
        CXS_ASSERT(fd_ctx->events == 0);
        return true;
    }

    IOManager *IOManager::GetThis()
    {
        return dynamic_cast<IOManager *>(Scheduler::GetThis());
    }

    void IOManager::tickle()
    {
        if (hasIdleThreads())
        {
            return;
        }
        int rt = write(m_tickleFds[1], "T", 1);
        CXS_ASSERT(rt == 1);
    }
    bool IOManager::stopping()
    {
        return Scheduler::stopping() && m_pendingEventCount == 0;
    }

    void IOManager::idle()
    {
        epoll_event *events = new epoll_event[64]();
        std::shared_ptr<epoll_event> shared_events(events, [events](epoll_event *ptr)
                                                   { delete[] events; });
        while (true)
        {
            if (stopping())
            {
                CXS_LOG_INFO(g_logger) << "idle stopping exit";
                break;
            }
            int rt = 0;
            do
            {
                static const int MAX_TIMEOUT = 5000;
                rt = epoll_wait(m_epfd, events, 64, MAX_TIMEOUT);

                if (rt < 0 && errno == EINTR)
                {
                }
                else
                {
                    break;
                }
            } while (true);
            for (int i = 0; i < rt; ++i)
            {
                epoll_event &event = events[i];
                if (event.data.fd == m_tickleFds[0])
                {
                    uint8_t dummy;
                    while (read(m_tickleFds[0], &dummy, 1) == 1)
                    {
                        continue;
                    }
                    FdContext *fd_ctx = event.data.ptr;

                    FdContext::MutexType::Lock lock(fd_ctx->mutex);
                    if(event.events & (EPOLLERR | EPOLLOUT))
                    {
                        event.events |= EPOLLERR | EPOLLOUT ;
                    }
                    int real_events = NONE;
                    if(event.events & EPOLLIN)
                    {
                        real_events |= READ;
                    }
                    if(event.events & EPOLLOUT)
                    {
                        real_events |= WRITE;
                    }

                    if(fd_ctx->events & real_events == NONE)
                    {
                        continue;
                    }

                    int left_events = (fd_ctx->events & ~real_events);
                    int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                    event.events = EPOLLET | left_events;
                    
                }
            }
        }
    }
}