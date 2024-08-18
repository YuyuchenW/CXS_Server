#include "http_server.h"
#include "http.h"
#include "http_session.h"

namespace CXS {
namespace http {

static CXS::Logger::ptr g_logger = CXS_LOG_NAME("system");

HttpServer::HttpServer(bool keepalive,
                       CXS::IOManager *worker,
                       CXS::IOManager *accept_woker) :
    TCPServer(worker, accept_woker),
    m_isKeepAlive(keepalive) {
}
void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if (req == nullptr) {
            CXS_LOG_WARN(g_logger) << "recvRequest failed errno = " << errno
                                   << ", errmsg = " << strerror(errno) << "client " << *client;
            break;
        }
        HttpResponse::ptr rsp(new HttpResponse(req->getVersion(), req->isClose() || !m_isKeepAlive));
        rsp->setBody("Hello, world!");
        CXS_LOG_INFO(g_logger) << "recvRequest: " << std::endl
                               << *req;
        session->sendResponse(rsp);
    } while (m_isKeepAlive);
    session->close();
}
}
} // namespace CXS::http