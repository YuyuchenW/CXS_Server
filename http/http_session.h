#ifndef __CXS_HTTP_SESSION__
#define __CXS_HTTP_SESSION__
#include "../code/socket_stream.h"
#include "http.h"
#include <memory>
namespace CXS {
namespace http {
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);
    int sendResponse(HttpResponse::ptr response);
    HttpRequest::ptr recvRequest();
};
}
} // namespace CXS::http
#endif