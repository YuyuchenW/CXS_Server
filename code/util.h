#ifndef __CXS_UTIL_H__
#define __CXS_UTIL_H__


#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>

namespace CXS
{
    pid_t GetThreadId();
    uint32_t GetFiberId();
    
    void Backtrace(std::vector<std::string>& bt, int size, int skip =1);
    std::string BacktraceToString(int size =64, int skip = 2, const std::string& prefix = "");

    //time
    uint64_t GetCurrentMS();
    uint64_t GetCurrentUS();
}

#endif