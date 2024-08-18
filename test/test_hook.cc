#include "../code/hook.h"
#include "../code/iomanager.h"
#include "../code/log.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void test_sleep() {
    CXS::IOManager iom(1);
    iom.schedule([]() {
        sleep(2);
        CXS_LOG_INFO(g_logger) << "sleep 2 "; });

    iom.schedule([]() {
        sleep(3);
        CXS_LOG_INFO(g_logger) << "sleep 3 "; });
    CXS_LOG_INFO(g_logger) << "test_sleep";
}

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    const char data[] = "GET / HTTP/1.0\r\n\r\n";

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "183.2.172.42", &addr.sin_addr.s_addr);
    CXS_LOG_DEBUG(g_logger) << "start connect ";
    int rt = connect(sock, (const sockaddr *)&addr, sizeof(addr));

    if (rt ) {
        CXS_LOG_ERROR(g_logger);
        return;
    }
    if (send(sock, data, sizeof(data), 0) < 0) {
        std::cerr << "send failed" << std::endl;
        close(sock);
        return;
    }
    std::string buff;

    buff.resize(8192);
    rt = recv(sock, &buff[0], buff.size(), 0);
    CXS_LOG_DEBUG(g_logger) << " recv rt = " << rt;
    if (rt <= 0) {
        return;
    }
    buff.resize(rt);
    CXS_LOG_DEBUG(g_logger) << buff;
}

int main(int argc, char const *argv[]) {
    // test_sleep();

    CXS::IOManager iom;
    iom.schedule(test_sock);
    return 0;
}
