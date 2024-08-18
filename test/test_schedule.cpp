#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/scheduler.hpp"
#include "../code/hook.h"
CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    CXS_LOG_INFO(g_logger) << "---test in fiber---" << s_count;
    CXS::set_hook_enable(false);
    sleep(1);
    while (--s_count >= 0) {
        CXS::Scheduler::GetThis()->schedule(&test_fiber, CXS::GetThreadId());
    }
}

int main(int argc, char const *argv[]) {
    CXS_LOG_INFO(g_logger) << "main";
    CXS::Scheduler sc(2, false, "work");
    sc.start();
    sc.schedule(&test_fiber);
    sc.stop();
    CXS_LOG_INFO(g_logger) << "over";
    return 0;
}
