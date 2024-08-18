#include "address.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "endian.h"
#include "log.h"
#include <netdb.h>
#include <ifaddrs.h>
#include <utility>
#include <vector>
namespace CXS {

static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");
template <class T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}
int Address::getFamily() const {
    return getAddr()->sa_family;
}

template <class T>
static uint32_t CountBytes(T value) {
    uint32_t result = 0;
    for (; value; ++result) {
        value &= value - 1;
    }
    return result;
}

std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

Address::ptr Address::Create(const sockaddr *address, socklen_t addrlen) {
    if (address == nullptr) {
        return nullptr;
    }
    Address::ptr result;
    switch (address->sa_family) {
    case AF_INET:
        result.reset(new IPv4Address(*(const sockaddr_in *)address));
        break;
    case AF_INET6:
        result.reset(new IPv6Address(*(const sockaddr_in6 *)address));
        break;
    default:
        result.reset(new UnknownAddress(*address));
        break;
    }
    return result;
}
bool Address::Lookup(std::vector<Address::ptr> &result, const std::string &host, int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_addr = nullptr;
    hints.ai_addrlen = 0;
    hints.ai_canonname = nullptr;
    hints.ai_family = family;
    hints.ai_flags = 0;
    hints.ai_next = nullptr;
    hints.ai_protocol = protocol;
    hints.ai_socktype = type;
    std::string node;
    const char *service = nullptr;
    //  host = www.baidu.com:http
    //  检查 ipv6address service  [address]:
    if (!host.empty() && host[0] == '[') {
        // 查找第一个 ] 的位置，返回该指针
        const char *endipv6 = (const char *)memchr(host.c_str(), ']', host.size() - 1);
        //找到 ]
        if (endipv6) {
            // 是否为 ：
            if (*(endipv6 + 1) == ':') {
                // endipv6 后两个字节为端口号
                service = endipv6 + 2;
            }
            // 地址为[]里的内容
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }
    // IPv4 ip:port
    if (node.empty()) {
        // 查找第一个 : 的位置，返回该指针
        service = (const char *)memchr(host.c_str(), ':', host.size());
        //找到 :
        if (service) {
            // 后面没有:了
            if (!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                // 拿到地址
                node = host.substr(0, service - host.c_str());
                // 拿到端口号
                ++service;
            }
        }
    }

    // 如果没有设置端口号，就将host赋给他
    if (node.empty()) {
        node = host;
    }
    // 获得地址链表
    CXS_LOG_DEBUG(g_logger) << " 端口号 = " << service;
    CXS_LOG_DEBUG(g_logger) << " 地址 = " << node;
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if (error) {
        CXS_LOG_ERROR(g_logger) << "Address::Lookup: getaddrinfo failed host = " << host
                                << ", family = " << family << ", type = " << type << " error = " << error
                                << ", gai_strerror = " << gai_strerror(error);
        return false;
    }
    // 遍历地址链表
    next = results;
    while (next) {
        result.push_back(Create(next->ai_addr, next->ai_addrlen));
        next = next->ai_next;
    }

    // 释放地址链表
    freeaddrinfo(results);
    return true;
}

bool Address::GetInterfaceAddress(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result, int family) {
    struct ifaddrs *next, *results;
    if (getifaddrs(&results) != 0) {
        CXS_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress: getifaddrs failed"
                                << "err = " << strerror(errno);
        return false;
    }
    try {
        // 遍历地址链表
        for (next = results; next; next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0;
            // 地址族确定 且 该地址族与解析处来的不同
            if (family != AF_UNSPEC && next->ifa_addr->sa_family != family) {
                continue;
            }
            switch (next->ifa_addr->sa_family) {
            case AF_INET: {
                // 创建ipv4地址
                addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                //掩码地址
                uint32_t netmask = ((sockaddr_in *)next->ifa_netmask)->sin_addr.s_addr;
                // 掩码长度
                prefix_len = CountBytes(netmask);
            } break;
            case AF_INET6: {
                // 创建ipv6地址
                addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                //掩码地址
                in6_addr &netmask = ((sockaddr_in6 *)next->ifa_netmask)->sin6_addr;
                prefix_len = 0;
                // 前缀长度，16字节
                for (int i = 0; i < 16; ++i) {
                    prefix_len += CountBytes(netmask.__in6_u.__u6_addr8[i]);
                }
            } break;
            default:
                break;
            }
            // 插入到容器中
            if (addr) {
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        CXS_LOG_ERROR(g_logger) << "Address::GetInterfaceAddress failed";
        freeifaddrs(results);
        return false;
    }
    // 释放地址链表
    freeifaddrs(results);
    return true;
}

bool Address::GetInterfaceAddress(std::vector<std::pair<Address::ptr, uint32_t>> &result, const std::string &iface, int family) {
    if (iface.empty() || iface == "*") {
        // 创建监听任意IP地址的连接请求的ipv4
        if (family == AF_INET || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        }
        // 创建监听任意IP地址的连接请求的ipv6
        if (family == AF_INET6 || family == AF_UNSPEC) {
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        }
        return true;
    }

    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
    //获取失败
    if (!GetInterfaceAddress(results, family)) {
        return false;
    }

    // 返回一个pair，first为第一个等于iface的迭代器，second为最后一个等于iface的下一个元素的迭代器
    auto its = results.equal_range(iface);
    for (; its.first != its.second; ++its.first) {
        result.push_back(its.first->second);
    }
    return true;
}

Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if (Lookup(result, host, family, type, protocol)) {
        for (auto &i : result) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if (v) {
                return v;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string &host, int family, int type, int protocol) {
    std::vector<Address::ptr> result;
    if (Lookup(result, host, type, protocol)) {
        for (auto &i : result) {
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if (v) {
                return v;
            }
        }
    }
    return nullptr;
}

bool Address::operator<(const Address &other) const {
    socklen_t minlen = std::min(getAddrLen(), other.getAddrLen());
    int result = memcmp(getAddr(), other.getAddr(), minlen);
    if (result < 0) {
        return true;
    } else if (result > 0) {
        return false;
    }
    return getAddrLen() < other.getAddrLen();
}

bool Address::operator==(const Address &rhs) const {
    return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}
bool Address::operator!=(const Address &rhs) const {
    return !(*this == rhs);
}

IPAddress::ptr IPAddress::Create(const char *address, uint16_t port) {
    addrinfo hints, *results;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    // hints.ai_flags = AI_NUMERICHOST;
    int err = getaddrinfo(address, "http", &hints, &results);
    if (err) {
        CXS_LOG_ERROR(g_logger) << "IPAddress::Create: getaddrinfo failed address = " << address
                                << ", port = " << port << "err = " << strerror(errno);
        return nullptr;
    }
    try {
        IPAddress::ptr rt = std::dynamic_pointer_cast<IPAddress>(Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if (rt) {
            rt->setPort(port);
        }
        freeaddrinfo(results);
        return rt;
    } catch (...) {
        freeaddrinfo(results);
        return nullptr;
    }
}
IPv4Address::IPv4Address(uint32_t ip, uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = byteswapOnLittleEndian(ip);
}
IPv4Address::IPv4Address(const sockaddr_in &address) {
    m_addr = address;
}

IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port) {
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapOnLittleEndian(port);
    // 将一个IP地址的字符串表示转换为网络字节序的二进制形式
    int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if (result <= 0) {
        CXS_LOG_ERROR(g_logger) << "IPv4Address::Create: inet_pton failed address = " << address << ", port = " << port
                                << "rt = " << result << "err = " << strerror(errno);
        return nullptr;
    }
    return rt;
}

const sockaddr *IPv4Address::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IPv4Address::getAddr() {
    return (sockaddr *)&m_addr;
}
socklen_t IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}
std::ostream &IPv4Address::insert(std::ostream &os) const {
    uint32_t addr = byteswapOnLittleEndian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << byteswapOnLittleEndian(m_addr.sin_port);
    return os;
}
IPAddress::ptr IPv4Address::broadCastAddress(uint32_t prefix_len) const {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) const {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr &= byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(baddr));
}
IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) const {
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~byteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin_port);
}
void IPv4Address::setPort(uint16_t port) {
    m_addr.sin_port = byteswapOnLittleEndian(port);
    return;
}

IPv6Address::ptr IPv6Address::Create(char *address, uint16_t port) {
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapOnLittleEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr.__in6_u.__u6_addr8);
    if (result <= 0) {
        CXS_LOG_ERROR(g_logger) << "IPv6Address::Create: inet_pton failed address = " << address << ", port = " << port
                                << "rt = " << result << "err = " << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv6Address::IPv6Address() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6 &address) {
    m_addr = address;
}
IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.__in6_u.__u6_addr8, address, 16);
}
const sockaddr *IPv6Address::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *IPv6Address::getAddr() {
    return (sockaddr *)&m_addr;
}
socklen_t IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream &IPv6Address::insert(std::ostream &os) const {
    os << "[";
    // 两个字节为一块
    uint16_t *addr = (uint16_t *)m_addr.sin6_addr.__in6_u.__u6_addr8;
    bool used_zeros = false;
    for (int i = 0; i < 8; ++i) {
        // 将连续0块省略
        if (addr[i] == 0 && !used_zeros) {
            continue;
        }
        // 上一块为0，多输出个:
        if (i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            // 省略过了，后面不能再省略连续0块了
            used_zeros = true;
        }
        // 每个块后都要输出:
        if (i) {
            os << ":";
        }
        // 按十六进制输出
        os << std::hex << (int)byteswapOnLittleEndian(addr[i]) << std::dec;
    }

    // 若最后一块为0省略
    if (!used_zeros && addr[7] == 0) {
        os << "::";
    }

    os << "]: " << byteswapOnLittleEndian(m_addr.sin6_port);
    return os;
}

uint32_t IPv6Address::getPort() const {
    return byteswapOnLittleEndian(m_addr.sin6_port);
}
void IPv6Address::setPort(uint16_t port) {
    m_addr.sin6_port = byteswapOnLittleEndian(port);
    return;
}

IPAddress::ptr IPv6Address::broadCastAddress(uint32_t prefix_len) const {
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] |= ~CreateMask<uint8_t>(prefix_len % 8);
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xFF;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}
IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) const {
    sockaddr_in6 naddr(m_addr);
    /*	找到前缀长度结尾在第几个字节，在该字节在哪个位置。
     *	将该字节前剩余位置全部置为0	*/
    naddr.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] &= CreateMask<uint8_t>(prefix_len % 8);
    // 将后面其余字节置为0
    for (int i = prefix_len / 8 + 1; i < 16; ++i) {
        naddr.sin6_addr.__in6_u.__u6_addr8[i] = 0x00;
    }
    return IPv6Address::ptr(new IPv6Address(naddr));
}
IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) const {
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin6_family = AF_INET6;
    subnet.sin6_addr.__in6_u.__u6_addr8[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
    for (uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.__in6_u.__u6_addr8[i] = 0xFF;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}
// Unix域套接字路径名的最大长度。-1是减去'\0'
static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un *)0)->sun_path) - 1;
UnixAddress::UnixAddress() {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_addr_len = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string &path) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    // 加上'\0'的长度
    m_addr_len = path.size() + 1;
    if (!path.empty() && path[0] == '\0') {
        --m_addr_len;
    }
    if (m_addr_len > sizeof(m_addr.sun_path)) {
        throw std::runtime_error("path is too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_addr_len);
    m_addr_len += offsetof(sockaddr_un, sun_path);
}

const sockaddr *UnixAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *UnixAddress::getAddr() {
    return (sockaddr *)&m_addr;
}
socklen_t UnixAddress::getAddrLen() const {
    return m_addr_len;
}

void UnixAddress::setAddrLen(uint32_t v) {
    m_addr_len = v;
}

std::ostream &UnixAddress::insert(std::ostream &os) const {
    if (m_addr_len > offsetof(sockaddr_un, sun_path)
        && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1, m_addr_len - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

UnknownAddress::UnknownAddress(int family) {
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr &addr) {
    m_addr = addr;
}

const sockaddr *UnknownAddress::getAddr() const {
    return (sockaddr *)&m_addr;
}

sockaddr *UnknownAddress::getAddr() {
    return (sockaddr *)&m_addr;
}

socklen_t UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}
std::ostream &UnknownAddress::insert(std::ostream &os) const {
    os << "[Unknown address family: " << m_addr.sa_family
       << "]";
    return os;
}

std::ostream &operator<<(std::ostream &os, const Address &addr) {
    return addr.insert(os);
}
} // namespace CXS
  // namespace CXS
