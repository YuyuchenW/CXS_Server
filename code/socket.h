#ifndef __CXS_SOCKET_H__
#define __CXS_SOCKET_H__
#include <bits/types/struct_iovec.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ostream>
#include <sys/socket.h>
#include "address.h"
#include "noncopyable.h"
namespace CXS {

class Socket : public std::enable_shared_from_this<Socket>, public Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM,
    };
    enum Family {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        Unix = AF_UNIX,
    };
    static Socket::ptr CreateTCP(CXS::Address::ptr addr);
    static Socket::ptr CreateUDP(CXS::Address::ptr addr);

    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();

    static Socket::ptr CreateUinxTCPSocket();
    static Socket::ptr CreateUinxUDPSocket();

    Socket(int family, int type, int protocol = 0);
    ~Socket();

    int64_t getSendTimeOut();
    void setSendTimeOut(int64_t time);

    int64_t getRecvTimeOut();
    void setRecvTimeOut(int64_t time);

    bool getOption(int level, int optname, void *result, size_t *optlen);
    template <class T>
    bool getOption(int level, int optname, T &result) {
        size_t length = sizeof(T);
        return getOption(level, optname, &result, &length);
    }
    bool setOption(int level, int optname, const void *optval, size_t optlen);
    template <class T>
    bool setOption(int level, int optname, const T &optval) {
        return setOption(level, optname, &optval, sizeof(T));
    }

    Socket::ptr accept();
    bool bind(const Address::ptr addr);
    bool connect(const Address::ptr addr, int64_t timeout_ms = -1);
    bool listen(int backlog = 1024);
    bool close();
    int send(const void *buffer, size_t length, int flags = 0);
    int send(const iovec *buffer, size_t length, int flags = 0);
    int sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec *buffer, size_t length, const Address::ptr to, int flags = 0);

    int recv(void *buffer, size_t length, int flags = 0);
    int recv(iovec *buffer, size_t length, int flags = 0);
    int recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);
    int recvFrom(iovec *buffer, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily() const {
        return m_family;
    };
    int getProtocol() const {
        return m_protocol;
    };
    int getType() const {
        return m_type;
    };

    bool isConnected() const {
        return m_isConnected;
    };
    bool isValid() const;
    int getError();

    std::ostream &dump(std::ostream &os) const;
    int getSocket() const {
        return m_sock;
    };

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();

private:
    void initSock();
    void newSock();
    bool init(int sock);

private:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;

    Address::ptr m_remoteAddr;
    Address::ptr m_localAddr;
};

std::ostream &operator<<(std::ostream &os, const Socket &sock);
} // namespace CXS

#endif