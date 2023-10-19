#ifndef __CXS_MACRO_H__
#define __CXS_MACRO_H__
#include <string.h>
#include <assert.h>

#define CXS_ASSERT(x)                                                            \
    if (!(x))                                                                    \
    {                                                                            \
        CXS_LOG_ERROR(CXS_LOG_ROOT()) << "ASSERTION : " #x                       \
                                      << "\n backtrace: \n"                      \
                                      << CXS::BacktraceToString(100, 2, "    "); \
        assert(x);                                                               \
    }

#define CXS_ASSERT2(x, w)                                                        \
    if (!(x))                                                                    \
    {                                                                            \
        CXS_LOG_ERROR(CXS_LOG_ROOT()) << "ASSERTION : " #x                       \
                                      << "\n"                                    \
                                      << w                                       \
                                      << "\n backtrace: \n"                      \
                                      << CXS::BacktraceToString(100, 2, "    "); \
        assert(x);                                                               \
    }



#endif