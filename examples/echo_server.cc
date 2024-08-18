#include "../code/tcp_server.h"
#include "../code/log.h"
#include "../code/iomanager.h"
#include "../code/bytearray.h"
#include "../code/address.h"
#include <bits/types/struct_iovec.h>
#include <iostream>
#include <vector>

static CXS::Logger::ptr g_logger = CXS_LOG_ROOT();

class EchoServer : public CXS::TCPServer {
public:
    typedef std::shared_ptr<EchoServer> Ptr;
    EchoServer(int type);
    void handleClient(CXS::Socket::ptr client);

private:
    int m_type = 0;
};

EchoServer::EchoServer(int type) :
    m_type(type) {
}

void EchoServer::handleClient(CXS::Socket::ptr client) {
    CXS_LOG_INFO(g_logger) << "handing client " << *client;
    CXS::ByteArray::ptr ba(new CXS::ByteArray());
    while (true) {
        ba->clear();
        std::vector<iovec> iovs;
        ba->getWriteBuffers(iovs, 1024);

        int ret = client->recv(&iovs[0], iovs.size());

        if (ret == 0) {
            CXS_LOG_INFO(g_logger) << "client closed " << *client;
            break;
        }
        if (ret < 0) {
            CXS_LOG_ERROR(g_logger) << "recv error: " << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + ret);
        ba->setPosition(0);
        if (m_type == 1) {
            std::cout << ba->toString();
        } else {
            std::cout << ba->toHexString();
        }
        std::cout.flush();
    }
}

int type = 1;
void run() {
    EchoServer::Ptr server(new EchoServer(type));
    auto addr = CXS::Address::LookupAny("172.26.162.152:8020");
    while (!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        CXS_LOG_INFO(g_logger) << "used as [" << argv[0] << " -t]  or [" << argv[0] << " -b]";
        return 0;
    }
    if (!strcmp(argv[1], "-b")) {
        type = 2;
    }
    CXS::IOManager iom(2);
    iom.schedule(run);

    return 0;
}