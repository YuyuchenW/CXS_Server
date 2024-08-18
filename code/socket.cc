#include "socket.h"
#include "address.h"
#include "fd_manager.h"
#include <asm-generic/socket.h>
#include <bits/types/struct_iovec.h>
#include <bits/types/struct_timeval.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "iomanager.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

namespace CXS {
Socket::Socket(int family, int type, int protocol) :
    m_sock(-1), m_family(family),
    m_type(type), m_protocol(protocol), m_isConnected(false){};

Socket::~Socket() {
    close();
};

Socket::ptr Socket::CreateTCP(CXS::Address::ptr addr) {
    Socket::ptr sock(new Socket(addr->getFamily(), Socket::TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUDP(CXS::Address::ptr addr) {
    Socket::ptr sock(new Socket(addr->getFamily(), Socket::UDP, 0));
    return sock;
}
Socket::ptr Socket::CreateTCPSocket() {
    Socket::ptr sock(new Socket(Socket::IPv4, Socket::TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket() {
    Socket::ptr sock(new Socket(Socket::IPv4, Socket::UDP, 0));
    return sock;
}

Socket::ptr Socket::CreateTCPSocket6() {
    Socket::ptr sock(new Socket(Socket::IPv6, Socket::TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUDPSocket6() {
    Socket::ptr sock(new Socket(Socket::IPv6, Socket::UDP, 0));
    return sock;
}
Socket::ptr Socket::CreateUinxTCPSocket() {
    Socket::ptr sock(new Socket(Socket::Unix, Socket::TCP, 0));
    return sock;
}
Socket::ptr Socket::CreateUinxUDPSocket() {
    Socket::ptr sock(new Socket(Socket::Unix, Socket::UDP, 0));
    return sock;
}

void Socket::setSendTimeOut(int64_t v) {
    struct timeval tv {
        int(v / 1000), int(v % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
};

int64_t Socket::getSendTimeOut() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
};

bool Socket::getOption(int level, int optname, void *result, size_t *optlen) {
    int rt = getsockopt(m_sock, level, optname, result, (socklen_t *)optlen);
    if (rt) {
        CXS_LOG_ERROR(g_logger) << "getOption sock = " << m_sock << " level = " << level
                                << "option = " << optname << " failed, errno = " << errno << "error str = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int optname, const void *optval, size_t optlen) {
    int rt = setsockopt(m_sock, level, optname, optval, (socklen_t)optlen);
    if (rt) {
        CXS_LOG_ERROR(g_logger) << "setOption sock = " << m_sock << " level = " << level
                                << "option = " << optname << " failed, errno = " << errno << "error str = " << strerror(errno);
        return false;
    }
    return true;
}
Socket::ptr Socket::accept() {
    Socket::ptr sock(new Socket(m_family, m_type, m_protocol));
    int newSock = ::accept(m_sock, nullptr, nullptr);
    if (newSock == -1) {
        CXS_LOG_ERROR(g_logger) << " accept (" << m_sock << ") failed, errno = " << errno << " error str = " << strerror(errno);
        return nullptr;
    }
    if (sock->init(newSock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    if (ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

void Socket::initSock() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if (m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, &val);
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if (CXS_UNLIKLY(m_sock)) {
        initSock();
    } else {
        CXS_LOG_ERROR(g_logger) << "new socket failed, errno = " << errno << " error str = " << strerror(errno);
    }
    // initSock();
}

bool Socket::bind(const Address::ptr addr) {
    if (!isValid()) {
        newSock();
        if (CXS_UNLIKLY(!isValid())) {
            return false;
        }
    }
    if (CXS_UNLIKLY(addr->getFamily() != m_family)) {
        CXS_LOG_ERROR(g_logger) << "bind sock.family = " << m_family
                                << " addr.family = " << addr->getFamily() << " not match"
                                << " addr = " << addr->toString();
        return false;
    }

    if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        CXS_LOG_ERROR(g_logger) << "bind sock = " << m_sock << " failed, errno = " << errno
                                << " error str = " << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::connect(const Address::ptr addr, int64_t timeout_ms) {
    if (!isValid()) {
        newSock();
        if (CXS_UNLIKLY(!isValid())) {
            return false;
        }
    }
    if (CXS_UNLIKLY(addr->getFamily() != m_family)) {
        CXS_LOG_ERROR(g_logger) << "connect sock.family = " << m_family
                                << " addr.family = " << addr->getFamily() << " not match"
                                << " addr = " << addr->toString();
        return false;
    }

    if (timeout_ms == -1) {
        if (::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            CXS_LOG_ERROR(g_logger) << "connect sock = " << m_sock << " connect (" << addr->toString()
                                    << ") failed, errno = " << errno << " error str = " << strerror(errno);
            close();
            return false;
        }
    } else {
        if (::connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            CXS_LOG_ERROR(g_logger) << "connect sock = " << m_sock << " connect (" << addr->toString()
                                    << ") timeout = " << timeout_ms << "failed, errno = " << errno << " error str = " << strerror(errno);
            close();
            return false;
        }
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}
bool Socket::listen(int backlog) {
    if (!isValid()) {
        CXS_LOG_ERROR(g_logger) << "listen sock = " << m_sock << " failed, errno = " << errno;
        return false;
    }
    if (::listen(m_sock, backlog)) {
        CXS_LOG_ERROR(g_logger) << "listen sock = " << m_sock << " failed, errno = " << errno << " error str = " << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if (!m_isConnected && m_sock == -1) {
        return true;
    }
    m_isConnected = false;
    if (m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
    }
    return false;
}

int Socket::send(const void *buffer, size_t length, int flags) {
    if (isConnected()) {
        return ::send(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::send(const iovec *buffer, size_t length, int flags) {
    if (isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (struct iovec *)buffer;
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags) {
    if (isConnected()) {
        return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec *buffer, size_t length, const Address::ptr to, int flags) {
    if (isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (struct iovec *)buffer;
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void *buffer, size_t length, int flags) {
    if (isConnected()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}
int Socket::recv(iovec *buffer, size_t length, int flags) {
    if (isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (struct iovec *)buffer;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}
int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags) {
    if (isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
    }
    return -1;
}
int Socket::recvFrom(iovec *buffer, size_t length, Address::ptr from, int flags) {
    if (isConnected()) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (struct iovec *)buffer;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}
int64_t Socket::getRecvTimeOut() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock);
    if (ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeOut(int64_t v) {
    struct timeval tv {
        int(v / 1000), int(v % 1000 * 1000)
    };
    setOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}
Address::ptr Socket::getRemoteAddress() {
    if (m_remoteAddr) {
        return m_remoteAddr;
    }
    Address::ptr result;
    switch (m_family) {
    case AF_INET:
        result.reset(new IPv4Address());
        break;
    case AF_INET6:
        result.reset(new IPv6Address());
        break;
    case AF_UNIX:
        result.reset(new UnixAddress());
        break;
    default:
        result.reset(new UnknownAddress(m_family));
        break;
    }

    socklen_t addrlen = result->getAddrLen();
    if (getpeername(m_sock, result->getAddr(), &addrlen)) {
        CXS_LOG_ERROR(g_logger) << "getpeername error sock = " << m_sock
                                << ", errno = " << errno << ", str error = " << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr unixAddr = std::dynamic_pointer_cast<UnixAddress>(result);
        unixAddr->setAddrLen(addrlen);
    }
    m_remoteAddr = result;
    return m_remoteAddr;
}
Address::ptr Socket::getLocalAddress() {
    if (m_localAddr) {
        return m_localAddr;
    }
    Address::ptr result;
    switch (m_family) {
    case AF_INET:
        result.reset(new IPv4Address());
        break;
    case AF_INET6:
        result.reset(new IPv6Address());
        break;
    case AF_UNIX:
        result.reset(new UnixAddress());
        break;
    default:
        result.reset(new UnknownAddress(m_family));
        break;
    }

    socklen_t addrlen = result->getAddrLen();
    if (getsockname(m_sock, result->getAddr(), &addrlen)) {
        CXS_LOG_ERROR(g_logger) << "getsockname error sock = " << m_sock
                                << ", errno = " << errno << ", str error = " << strerror(errno);
        return Address::ptr(new UnknownAddress(m_family));
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr unixAddr = std::dynamic_pointer_cast<UnixAddress>(result);
        unixAddr->setAddrLen(addrlen);
    }
    m_localAddr = result;
    return m_localAddr;
}
bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    size_t len = sizeof(error);
    if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    return error;
}

std::ostream &Socket::dump(std::ostream &os) const {
    os << "[sock = " << m_sock << " isConnected = " << isConnected() << " faimly = " << m_family
       << " type = " << m_type << " prorocol = " << m_protocol;
    if (m_localAddr) {
        os << " local = " << m_localAddr->toString();
    }
    if (m_remoteAddr) {
        os << " remote = " << m_remoteAddr->toString();
    }
    os << "]";
    return os;
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sock, CXS::IOManager::READ);
}
bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, CXS::IOManager::WRITE);
}
bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, CXS::IOManager::READ);
}
bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}

std::ostream &operator<<(std::ostream &os, const Socket &socket) {
    return socket.dump(os);
}
} // namespace CXS