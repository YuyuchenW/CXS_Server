#include "tcp_server.h"
#include "address.h"
#include "iomanager.h"
#include "config.hpp"
#include "log.h"
#include "socket.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace CXS {

CXS::ConfigVar<uint64_t>::ptr g_tcp_server_readTimeout = CXS::Config::Lookup("tcp_server.read_timeout",
                                                                             (uint64_t)(120000),
                                                                             "tcp server read time out");

static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

TCPServer::TCPServer(CXS::IOManager *work,
                     CXS::IOManager *acceptWork) :
    m_work(work),
    m_acceptWork(acceptWork),
    m_readTimeout(g_tcp_server_readTimeout->getValue()),
    m_name("CXS/1.0.0"),
    m_isStop(true) {
}

TCPServer::~TCPServer() {
    for (auto &i : m_socks) {
        i->close();
    }
    m_socks.clear();
}
bool TCPServer::bind(CXS::Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    std::vector<Address::ptr> fails;
    addrs.push_back(addr);
    return bind(addrs, fails);
}
bool TCPServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails) {
    for (auto &addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if (!sock->bind(addr)) {
            CXS_LOG_ERROR(g_logger) << "bind fail errno :"
                                    << errno << " errstr = " << strerror(errno)
                                    << " addr = [" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        if (!sock->listen()) {
            CXS_LOG_ERROR(g_logger) << "listen fail errno :" << errno
                                    << " errstr = " << strerror(errno)
                                    << " addr = [" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if (!fails.empty()) {
        m_socks.clear();
        return false;
    }

    for (auto &i : m_socks) {
        CXS_LOG_INFO(g_logger) << "server bind success : " << *i;
    }
    return true;
}

void TCPServer::handleClient(Socket::ptr client) {
    CXS_LOG_INFO(g_logger) << "handle client : " << *client;
}
void TCPServer::startAccept(Socket::ptr sock) {
    while (!m_isStop) {
        Socket::ptr client = sock->accept();
        if (client) {
            client->setRecvTimeOut((int64_t)m_readTimeout);
            m_work->schedule(std::bind(&TCPServer::handleClient, shared_from_this(), client));
        } else {
            CXS_LOG_ERROR(g_logger) << "accept fail errno :"
                                    << errno << " errstr = " << strerror(errno);
        }
    }
}

bool TCPServer::start() {
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for (auto &sock : m_socks) {
        m_acceptWork->schedule(std::bind(&TCPServer::startAccept, shared_from_this(), sock));
    }
    return true;
}

void TCPServer::stop() {
    m_isStop = true;
    m_acceptWork->schedule([this]() {
        for (auto &sock : m_socks) {
            sock->cancelAll();
            sock->close();
        }
        m_socks.clear();
    });
}
} // namespace CXS