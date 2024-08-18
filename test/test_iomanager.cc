#include "../code/thread.h"
#include "../code/singleton.h"
#include "../code/log.h"
#include "../code/config.hpp"
#include "../code/scheduler.hpp"
#include "../code/iomanager.h"
#include <fcntl.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
int sock = 0;

void test_fiber()
{
    CXS_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    // sleep(3);

    // close(sock);
    // CXS::IOManager::GetThis()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22);
    inet_pton(AF_INET, "14.119.104.189", &addr.sin_addr.s_addr);

    if (!connect(sock, (const sockaddr *)&addr, sizeof(addr)))
    {
    }
    else if (errno == EINPROGRESS)
    {
        CXS_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        CXS::IOManager::GetThis()->addEvent(sock, CXS::IOManager::READ, []()
                                            { CXS_LOG_INFO(g_logger) << "read callback"; });
        CXS::IOManager::GetThis()->addEvent(sock, CXS::IOManager::WRITE, []()
                                            {
            CXS_LOG_INFO(g_logger) << "write callback";
            //close(sock);
            CXS::IOManager::GetThis()->cancelEvent(sock, CXS::IOManager::READ);
            close(sock); });
    }
    else
    {
        CXS_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
    }
}
static int i = 0;
CXS::Timer::ptr s_timer;
void test_timer()
{
    CXS::IOManager iom(5);

    s_timer = iom.addTimer(
        1000, []()
        { CXS_LOG_INFO(g_logger) << "hello world!";
        if(++i == 5)
        {
            s_timer->reset((uint64_t)2000,true);
        } },
        true);
}

void test1()
{
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    CXS::IOManager iom(2);
    iom.schedule(&test_fiber);
}

int main(int argc, char **argv)
{
    test1();
    return 0;
}