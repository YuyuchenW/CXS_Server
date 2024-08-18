#ifndef __CXS_SOCKET_STREAM__
#define __CXS_SOCKET_STREAM__
#include "stream.h"
#include "socket.h"
#include <memory>

namespace CXS {
class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;
    SocketStream(Socket::ptr sock, bool owner = true);
    ~SocketStream();
    virtual int read(void *buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;
    virtual int write(const void *buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;
    virtual void close() override;

    Socket::ptr getSocket() const {
        return m_sock;
    }
    bool isConnected() const;

protected:
    Socket::ptr m_sock;
    bool m_owner;
};

} // namespace CXS

#endif