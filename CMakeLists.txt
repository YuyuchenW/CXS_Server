cmake_minimum_required(VERSION 2.8)
PROJECT(CXS)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#include (cmake/utils.cmake)
SET(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3  -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-unused-variable")


#clanged
set(CMAKE_EXPORT_COMPILECOMMANDS ON)

SET(LIB_SRC
    code/log.cc
    code/util.cc
    code/config.cc
    code/thread.cc
    code/fiber.cc
    code/scheduler.cc
    code/iomanager.cc
    code/timer.cpp
    code/hook.cc
    code/fd_manager.cc
    code/address.cc
    code/socket.cc
    code/bytearray.cc
    http/http.cc
    http/http11_paeser.rl.cc
    http/httpclient_paeser.rl.cc
    http/http_parser.cc
    code/tcp_server.cc
    code/stream.cc
    code/socket_stream.cc
    http/http_session.cc
    http/http_server.cc
)


add_library(CXS SHARED ${LIB_SRC})

set(
    LIB_LIB
    CXS
    pthread
    yaml-cpp
    dl
)
find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
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

add_executable(test_hook test/test_hook.cc)
add_dependencies(test_hook CXS)
target_link_libraries(test_hook CXS ${LIB_LIB})

add_executable(test_address test/test_address.cc)
add_dependencies(test_address CXS)
target_link_libraries(test_address CXS ${LIB_LIB})

add_executable(test_socket test/test_socket.cc)
add_dependencies(test_socket CXS)
target_link_libraries(test_socket CXS ${LIB_LIB})

add_executable(test_bytearrays test/test_bytearrays.cc)
add_dependencies(test_bytearrays CXS)
target_link_libraries(test_bytearrays CXS ${LIB_LIB})

add_executable(test_http test/test_http.cc)
add_dependencies(test_http CXS)
target_link_libraries(test_http CXS ${LIB_LIB})

add_executable(test_http_parser test/test_http_parser.cc)
add_dependencies(test_http_parser CXS)
target_link_libraries(test_http_parser CXS ${LIB_LIB})

add_executable(test_tcp_server test/test_tcp_server.cc)
add_dependencies(test_tcp_server CXS)
target_link_libraries(test_tcp_server CXS ${LIB_LIB})

add_executable(echo_server examples/echo_server.cc)
add_dependencies(echo_server CXS)
target_link_libraries(echo_server CXS ${LIB_LIB})

add_executable(test_http_server test/test_http_server.cc)
add_dependencies(test_http_server CXS)
target_link_libraries(test_http_server CXS ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)