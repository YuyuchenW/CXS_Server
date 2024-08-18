#include "thread.h"
#include "log.h"
#include "util.h"
namespace CXS
{
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOW";
    static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

    Semaphore::Semaphore(uint32_t count)
    {
        //Initialize semaphore object SEM to VALUE. If PSHARED then share it with other processes.
        if(sem_init(&m_semaphore,0,count))
        {
            throw std::logic_error("sem_init error");
        }
    }
    Semaphore::~Semaphore()
    {
        sem_destroy(&m_semaphore);
    }
    void Semaphore::wait()
    {
        if(sem_wait(&m_semaphore))
        {
            throw std::logic_error("sem_wait error");
        }
    }
    void Semaphore::notify()
    {
        if(sem_post(&m_semaphore))
        {
            throw std::logic_error("sem_post error");
        }
    }

    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    void Thread::SetName(const std::string &name)
    {
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    const std::string &Thread::GetName()
    {
        return t_thread_name;
    }

    Thread::Thread(std::function<void()> cb, const std::string &name)
        : m_cb(cb), m_name(name)
    {
        if (name.empty())
        {
            m_name = "UNKNOW";
        }
        //创建线程
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (rt)
        {
            CXS_LOG_ERROR(g_logger) << "pthread_create thread fail, rt = " << rt
                                    << " name = " << name;
            throw std::logic_error("pthread_create erro");
        }
        m_semaphore.wait();
    }

    Thread::~Thread()
    {
        if (m_thread)
        {
            pthread_detach(m_thread);
        }
    }

    void Thread::join()
    {
        if (m_thread)
        {
            // 清理线程使用的资源
            int rt = pthread_join(m_thread, nullptr);
            if (rt)
            {
                CXS_LOG_ERROR(g_logger) << "pthread_join thread fail, rt = " << rt
                                        << " name = " << m_name;
                throw std::logic_error("pthread_join erro");
            }
            m_thread = 0;
        }
    }

    void *Thread::run(void *arg)
    {
        // 拿到新创建的Thread对象
    Thread* thread = (Thread*)arg;
    // 更新当前线程
    t_thread = thread;
    t_thread_name = thread->m_name;
    // 设置当前线程的id
    // 只有进了run方法才是新线程在执行，创建时是由主线程完成的，threadId为主线程的
    thread->m_id = CXS::GetThreadId();
    // 设置线程名称
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    // pthread_creat时返回引用 防止函数有智能指针,
    std::function<void()> cb;
    cb.swap(thread->m_cb);

    // 在出构造函数之前，确保线程先跑起来，保证能够初始化id
    thread->m_semaphore.notify();

    cb();
    return 0;
    }
} // namespace CXS
