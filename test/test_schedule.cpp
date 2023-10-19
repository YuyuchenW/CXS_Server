#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/scheduler.hpp"

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
    static int s_count = 5;

void test_fiber()
{

    CXS_LOG_INFO(g_logger) << "test in fiber";
    CXS_LOG_INFO(g_logger) << "s_count = " + std::to_string(s_count);


    while (--s_count >= 0)
    {
        CXS::Scheduler::GetThis()->schedule(&test_fiber,CXS::GetThreadId());
    }
}

int main(int argc, char const *argv[])
{
    CXS_LOG_INFO(g_logger) << "main";

    CXS::Scheduler sc(3, false, "test");

    sc.start();
    sc.schedule(&test_fiber);

    sc.stop();
    CXS_LOG_INFO(g_logger) << "over";

    return 0;
}
