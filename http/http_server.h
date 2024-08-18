#ifndef __CXS_HTTP_HTTTP_SERVER__
#define __CXS_HTTP_HTTTP_SERVER__
#include "http_session.h"
#include "../code/tcp_server.h"
#include <memory>
namespace CXS {
namespace http {
class HttpServer : public TCPServer {
public:
    typedef std::shared_ptr<TCPServer> ptr;
    HttpServer(bool keepalive = false,
               CXS::IOManager *worker = CXS::IOManager::GetThis(),
               CXS::IOManager *accept_woker = CXS::IOManager::GetThis());

protected:
    virtual void handleClient(Socket::ptr client) override;

private:
    bool m_isKeepAlive;
};
}
} // namespace CXS::http
#endif