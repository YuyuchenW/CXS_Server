#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/fiber.hpp"

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
void run_in_fiber()
{
    CXS_LOG_INFO(g_logger) << "run_in_fiber begin";
    CXS_LOG_INFO(g_logger) << "run_in_fiber begin";
    CXS::Fiber::YieldToHold();
    CXS_LOG_INFO(g_logger) << "run_in_fiber end";
    // return;
    //  CXS::Fiber::YieldToHold();
}

void test_fiber()
{

    CXS::Fiber::GetThis();
    CXS_LOG_INFO(g_logger) << " --- test start --- ";
    CXS::Fiber::ptr fiber(new CXS::Fiber(run_in_fiber));
    fiber->swapIn();
    CXS_LOG_INFO(g_logger) << " main after swap In ";
    fiber->swapIn();
    CXS_LOG_INFO(g_logger) << " --- test end --- ";
}

int main(int argc, char const *argv[])
{
    CXS::Thread::SetName("Main");
    std::vector<CXS::Thread::ptr> thrs;
    for (int i = 0; i < 30; ++i)
    {
        thrs.push_back(
            CXS::Thread::ptr(new CXS::Thread(&test_fiber, "thread_name_" + std::to_string(i))));
    }
    for(auto i : thrs)
    {
        i->join();
    }
    return 0;
}
