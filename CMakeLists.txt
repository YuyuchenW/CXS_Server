cmake_minimum_required(VERSION 2.8)
PROJECT(CXS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#include (cmake/utils.cmake)
SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3  -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

SET(LIB_SRC
    code/log.cc
    code/util.cc
    code/config.cc
    code/thread.cc
    code/fiber.cc
    code/scheduler.cc
    code/iomanager.cc
    code/timer.cpp

)


add_library(CXS SHARED ${LIB_SRC})

set(
    LIB_LIB
    CXS
    pthread
    yaml-cpp
)
#force_redefine_file_macro_for_sources(cxs) #__FILE__
add_executable(test test/test.cc)
add_dependencies(test CXS)
target_link_libraries(test CXS  ${LIB_LIB})

add_executable(testConfig test/test_config.cc)
add_dependencies(testConfig CXS)
target_link_libraries(testConfig CXS ${LIB_LIB})

add_executable(testThread test/test_thread.cc)
add_dependencies(testThread CXS)
target_link_libraries(testThread CXS ${LIB_LIB})

add_executable(test_util test/test_util.cc)
add_dependencies(test_util CXS)
target_link_libraries(test_util CXS ${LIB_LIB})

add_executable(test_fiber test/test_fiber.cc)
add_dependencies(test_fiber CXS)
target_link_libraries(test_fiber CXS ${LIB_LIB})

add_executable(test_schedule test/test_schedule.cpp)
add_dependencies(test_schedule CXS)
target_link_libraries(test_schedule CXS ${LIB_LIB})

add_executable(test_iomanager test/test_iomanager.cc)
add_dependencies(test_iomanager CXS)
target_link_libraries(test_iomanager CXS ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)