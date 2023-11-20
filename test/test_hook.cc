#include "../code/hook.h"
#include "../code/iomanager.h"
#include "../code/log.h"

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void test_sleep()
{
    CXS::IOManager iom(1);
    iom.schedule([]()
                 {
        sleep(2);
        CXS_LOG_INFO(g_logger) << "sleep 2 "; });

iom.schedule([]()
                 {
        sleep(3);
        CXS_LOG_INFO(g_logger) << "sleep 3 "; });
    CXS_LOG_INFO(g_logger) <<"test_sleep";
}

int main(int argc, char const *argv[])
{
    test_sleep();
    return 0;
}
