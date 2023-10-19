#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/macro.h"
#include "../code/util.h"
#include <assert.h>

CXS::Logger::ptr g_logger  = CXS_LOG_ROOT();

void test_assert()
{
    CXS_LOG_ERROR(g_logger) << CXS::BacktraceToString(10);
   // CXS_ASSERT(false);
    CXS_ASSERT2(0==1, "hello cxs"); 
    //assert(0);

}
int main(int argc, char const *argv[])
{
    test_assert();
    return 0;
}
