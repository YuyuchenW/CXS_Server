#include "fd_manager.h"
#include "hook.h"
#include "thread.h"
#include <asm-generic/socket.h>
namespace CXS {
FdCtx::FdCtx(int fd) :
    m_isInit(false),
    m_isSocket(false),
    m_sysNonbool(false),
    m_isClose(false),
    m_userNonblock(false),
    m_recvTimeout(-1),
    m_sendTimeout(-1),
    m_fd(fd) {
    init();
}
FdCtx::~FdCtx() {
}
bool FdCtx::init() {
    if (m_isInit) {
        return true;
    }
    m_recvTimeout = -1;
    m_sendTimeout = -1;
    //  文件状态
    struct stat fd_stat;

    // return 0 : 成功取出
    // return -1 : 取出失败，关闭了
    if (-1 == fstat(m_fd, &fd_stat)) {
        m_isInit = false;
        m_isSocket = false;
    } else {
        m_isInit = true;
        // 判断文件是否为socket
        m_isSocket = S_ISSOCK(fd_stat.st_mode);
    }

    // 是socket则设置为非阻塞模式
    if (m_isSocket) {
        // 获取文件描述符状态
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        // 确保非阻塞模式
        if (!(flags & O_NONBLOCK)) {
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
        }
        m_sysNonbool = true;
    } else {
        m_sysNonbool = false;
    }
    m_userNonblock = false;
    m_isClose = false;

    return m_isInit;
}

//设置超时事件
void FdCtx::setTimeout(int type, uint64_t v) {
    if (type == SO_RCVTIMEO) {
        m_recvTimeout = v;
        return;
    }
    m_sendTimeout = v;
    return;
}

//获取超时事件
uint64_t FdCtx::getTimeout(int type) {
    if (type == SO_RCVTIMEO) {
        return m_recvTimeout;
    }
    return m_sendTimeout;
}

FdManager::FdManager() {
    m_datas.resize(64);
}

// 获取/创建文件句柄类FdCtx
FdCtx::ptr FdManager::get(int fd, bool auto_create) {
    RWMutexType::ReadLock lock(m_mutex);
    // 文件句柄不存在，且不允许创建则返回空
    if ((int)m_datas.size() <= fd) {
        if (auto_create == false) {
            return nullptr;
        }
    } else {
        if (m_datas[fd] || !auto_create) {
            return m_datas[fd];
        }
    }

    lock.unlock();
    // 文件句柄不存在，且允许创建则创建
    RWMutexType::WriteLock lock2(m_mutex);
    FdCtx::ptr ctx(new FdCtx(fd));
    if (fd >= (int)m_datas.size()) {
        // 扩容
        m_datas.resize(fd * 1.5);
    }
    m_datas[fd] = ctx;
    return ctx;
}

void FdManager::del(int fd) {
    RWMutexType::WriteLock lock(m_mutex);
    if ((int)m_datas.size() <= fd) {
        return;
    }
    m_datas[fd].reset();
}
} // namespace CXS