#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include <vector>
CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
uint64_t count = 0;
struct testData
{
    int a = 0;
    int b = 0;
};

testData data;
// CXS::RWMutex m_mutex;
CXS::Mutex mutex;
void fun1()
{
    CXS_LOG_INFO(CXS_LOG_ROOT()) << "name: " << CXS::Thread::GetName()
                                 << " this.name:" << CXS::Thread::GetThis()->getName()
                                 << " id:" << CXS::GetThreadId()
                                 << "this.id" << CXS::Thread::GetThis()->getId();
    // CXS::RWMutex::WriteLock lock(m_mutex);
    CXS::Mutex::Lock lock(mutex);

    data.a = 100;
    sleep(1);
    data.b = 200;
    CXS_LOG_INFO(g_logger) << "data.a = " << data.a;
    CXS_LOG_INFO(g_logger) << "data.b = " << data.b;
}

void fun2()
{
    CXS_LOG_INFO(CXS_LOG_ROOT()) << "name: " << CXS::Thread::GetName()
                                 << " this.name:" << CXS::Thread::GetThis()->getName()
                                 << " id:" << CXS::GetThreadId()
                                 << "this.id" << CXS::Thread::GetThis()->getId();
    // CXS::RWMutex::WriteLock lock(m_mutex);
    CXS::Mutex::Lock lock(mutex);

    data.a = 300;
    data.b = 400;
    CXS_LOG_INFO(g_logger) << "data.a = " << data.a;
    CXS_LOG_INFO(g_logger) << "data.b = " << data.b;
}

void fun3()
{
    while (true)
    {
        CXS_LOG_INFO(g_logger) << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    }
}

void fun4()
{
    while (true)
    {
        CXS_LOG_INFO(g_logger) << "======================================================";
    }
}
int main(int argc, char const *argv[])
{
    CXS_LOG_INFO(g_logger) << "thread test begin";
    YAML::Node root = YAML::LoadFile("/cxs/bin/conf/log1.yml");
    CXS::Config::LoadFromYaml(root);
    std::vector<CXS::Thread::ptr> thrs;
    for (size_t i = 0; i < 1; i++)
    {
        CXS::Thread::ptr thr(new CXS::Thread(&fun1, "name_num_" + std::to_string(i)));
        CXS::Thread::ptr thr1(new CXS::Thread(&fun2, "name_num_" + std::to_string(i)));

        thrs.push_back(thr);
        thrs.push_back(thr1);
    }
    for (size_t i = 0; i < 2; i++)
    {
        thrs[i]->join();
    }
    CXS_LOG_INFO(g_logger) << "thread test end";

    return 0;
}
