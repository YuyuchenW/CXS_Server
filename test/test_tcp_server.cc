#include "../code/iomanager.h"
#include "../code/tcp_server.h"
#include "../code/log.h"
#include <unistd.h>
#include <vector>

CXS::Logger::ptr g_logger = CXS_LOG_ROOT();
void run() {
    auto addr = CXS::IPAddress::LookupAny("0.0.0.0:8033");
    auto addr2 = CXS::UnixAddress::ptr(new CXS::UnixAddress("/tmp/unix_addr"));
    CXS_LOG_INFO(g_logger) << *addr << "--" << *addr2;
    std::vector<CXS::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);
    std::vector<CXS::Address::ptr> fails;
    CXS::TCPServer::ptr tcp_server(new CXS::TCPServer);
    while (!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }

    tcp_server->start();
}

int main(int argc, char *argv[]) {
    CXS::IOManager iom(2);
    iom.schedule(run);
    return 0;
}