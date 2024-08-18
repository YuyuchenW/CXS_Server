#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include "thread.h"
#include "iomanager.h"
#include "singleton.h"
namespace CXS {
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    typedef std::shared_ptr<FdCtx> ptr;
    FdCtx(int fd);
    ~FdCtx();
    bool init();
    bool isInit() const {
        return m_isInit;
    }
    bool isSocket() const {
        return m_isSocket;
    }

    bool close();

    void setUserNonblock(bool v) {
        m_userNonblock = v;
    }
    bool getUserNonblock() const {
        return m_userNonblock;
    }

    void setSysNonblock(bool v) {
        m_sysNonbool = v;
    }
    bool getSysNonblock() const {
        return m_sysNonbool;
    }

    void setTimeout(int type, uint64_t v);
    uint64_t getTimeout(int type);
    bool isClose() const {
        return m_isClose;
    }

private:
    //是否初始化
    bool m_isInit : 1;
    //是否socket
    bool m_isSocket : 1;
    //是否hook非阻塞
    bool m_sysNonbool : 1;
    //是否关闭
    bool m_isClose : 1;
    //用户是否设置非阻塞
    bool m_userNonblock : 1;
    // 读写超时
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
    // 文件句柄
    int m_fd;
};

class FdManager {
public:
    typedef RWMutex RWMutexType;
    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);
    void del(int ft);

private:
    RWMutexType m_mutex;
    // 文件句柄集合
    std::vector<FdCtx::ptr> m_datas;
};

typedef Singleton<FdManager> FdMgr;
}; // namespace CXS
#endif