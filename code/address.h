#ifndef __CXS_ADDRESSES_H__
#define __CXS_ADDRESSES_H__

#include <cstdint>
#include <memory>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <utility>
#include <vector>
#include <map>
namespace CXS {
class IPAddress;

class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    static Address::ptr Create(const sockaddr *address, socklen_t addrlen);
    static bool Lookup(std::vector<Address::ptr> &result, const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
    static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);
    static bool GetInterfaceAddress(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family = AF_INET);
    static bool GetInterfaceAddress(std::vector<std::pair<Address::ptr, uint32_t>> &result, const std::string &iface, int family = AF_INET);
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host, int family = AF_INET,
                                                         int type = 0, int protocol = 0);
    virtual ~Address() {
    }
    int getFamily() const;
    virtual const sockaddr *getAddr() const = 0;
    virtual sockaddr *getAddr() = 0;
    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString() const;

    bool operator<(const Address &rhs) const;
    bool operator==(const Address &rhs) const;
    bool operator!=(const Address &rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    static IPAddress::ptr Create(const char *address, uint16_t port = 0);
    virtual IPAddress::ptr broadCastAddress(uint32_t prefix_len) const = 0;
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) const = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) const = 0;

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint16_t port) = 0;
};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    IPv4Address(uint32_t ip = INADDR_ANY, uint16_t port = 0);
    IPv4Address(const sockaddr_in &address);
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;

    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;
    IPAddress::ptr broadCastAddress(uint32_t prefix_len) const override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) const override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) const override;

    static IPv4Address::ptr Create(const char *address, uint16_t port = 0);

    uint32_t getPort() const override;
    void setPort(uint16_t port) override;

private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;
    IPv6Address();
    static IPv6Address::ptr Create(char *ip, uint16_t port = 0);
    IPv6Address(const sockaddr_in6 &address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;
    IPAddress::ptr broadCastAddress(uint32_t prefix_len) const override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) const override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) const override;

    uint32_t getPort() const override;
    void setPort(uint16_t port) override;

private:
    sockaddr_in6 m_addr;
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string &path);
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    void setAddrLen(uint32_t v);
    std::ostream &insert(std::ostream &os) const override;

private:
    sockaddr_un m_addr;
    socklen_t m_addr_len;
};

class UnknownAddress : public Address {
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr &addr);
    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    socklen_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

private:
    sockaddr m_addr;
    socklen_t m_addr_len;
};

std::ostream &operator<<(std::ostream &os, const Address &addr);
} // namespace CXS

#endif