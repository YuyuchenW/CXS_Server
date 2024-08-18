#include "../code/socket.h"
#include "../code/log.h"
#include "../code/iomanager.h"
#include <string>
static CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

void test_socket() {
    CXS::IPAddress::ptr addr = CXS::Address::LookupAnyIPAddress("www.baidu.com");
    if (addr) {
        CXS_LOG_INFO(g_logger) << "addr: " << addr->toString();

    } else {
        CXS_LOG_INFO(g_logger) << "get addr faild";
    }

    CXS::Socket::ptr sock = CXS::Socket::CreateTCP(addr);
    addr->setPort(80);
    if (!sock->connect(addr)) {
        CXS_LOG_INFO(g_logger) << "connect faild";
    } else {
        CXS_LOG_INFO(g_logger) << "connect " << addr->toString() << "success";
    }

    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int ret = sock->send(buff, sizeof(buff));
    if (ret <= 0) {
        CXS_LOG_ERROR(g_logger) << "send faild";
        return;
    }
    std::string buffs;
    buffs.resize(40960);
    ret = sock->recv(&buffs[0], buffs.size());
    if (ret <= 0) {
        CXS_LOG_ERROR(g_logger) << "recv faild";
        return;
    }
    CXS_LOG_INFO(g_logger) << "recv: " << buffs;
    sock->close();
    CXS_LOG_INFO(g_logger) << "close";
    return;
}
int main(int argc, char **argv) {
    CXS::IOManager iom;
    iom.schedule(&test_socket);
    return 0;
}