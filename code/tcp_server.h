#ifndef __CXS_TCP_SERVER__
#define __CXS_TCP_SERVER__

#include "address.h"
#include "iomanager.h"
#include "socket.h"
#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "noncopyable.h"

namespace CXS {
class TCPServer : public std::enable_shared_from_this<TCPServer>, Noncopyable {
public:
    typedef std::shared_ptr<TCPServer> ptr;
    TCPServer(CXS::IOManager *work = CXS::IOManager::GetThis(),
              CXS::IOManager *acceptWorker = CXS::IOManager::GetThis());
    virtual ~TCPServer();
    virtual bool bind(CXS::Address::ptr addr);
    virtual bool bind(const std::vector<Address::ptr> &addrs,
                      std::vector<Address::ptr> &fails);
    virtual bool start();
    virtual void stop();

    uint64_t getReadTimeout() const {
        return m_readTimeout;
    };

    std::string getName() const {
        return m_name;
    }

    void setTimeout(uint64_t timeout) {
        m_readTimeout = timeout;
    }
    void setName(const std::string &name) {
        m_name = name;
    }

    bool isStop() const {
        return m_isStop;
    }

protected:
    virtual void handleClient(Socket::ptr client);
    virtual void startAccept(Socket::ptr sock);

private:
    std::vector<Socket::ptr> m_socks;
    IOManager *m_work;
    IOManager *m_acceptWork;
    uint64_t m_readTimeout;
    std::string m_name;
    bool m_isStop;
};
} // namespace CXS

#endif