#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/fiber.hpp"

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
void run_in_fiber() {
    CXS_LOG_INFO(g_logger) << "run_in_fiber begin";
    CXS::Fiber::YieldToHold();
    CXS_LOG_INFO(g_logger) << "run_in_fiber end";
    CXS::Fiber::YieldToHold();
}

void test_fiber() {
    CXS_LOG_INFO(g_logger) << "main begin -1";
    {
        CXS::Fiber::GetThis();
        CXS_LOG_INFO(g_logger) << "main begin";
        CXS::Fiber::ptr fiber(new CXS::Fiber(run_in_fiber));
        fiber->call();
        CXS_LOG_INFO(g_logger) << "main after swapIn";
        fiber->call();
        CXS_LOG_INFO(g_logger) << "main after end";
        fiber->call();
    }
    CXS_LOG_INFO(g_logger) << "main after end2";
}

int main(int argc, char const *argv[]) {
    CXS::Thread::SetName("main");

    std::vector<CXS::Thread::ptr> thrs;
    for(int i = 0; i < 3; ++i) {
        thrs.push_back(CXS::Thread::ptr(
                    new CXS::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}
