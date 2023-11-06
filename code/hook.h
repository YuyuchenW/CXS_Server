#ifndef __CXS_HOOK_H__
#define __CXS_HOOK_H__
#include <unistd.h>
namespace CXS
{
    bool is_hook_enable();
    void set_hook_enable(bool flag);
}
extern "C"
{
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;

    typedef unsigned int (*sleep_fun)(useconds_t usec);
    extern usleep usleep_f;


}

#endif