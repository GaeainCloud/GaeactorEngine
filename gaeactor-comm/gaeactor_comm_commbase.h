#ifndef GAEACTOR_COMM_COMMBASE_H
#define GAEACTOR_COMM_COMMBASE_H

#include <uv.h>
#include <string>
#include "gaeactor_comm_global.h"
#include "gaeactor_comm_socketaddr.h"
namespace stdutils
{
class OriThread;
};
namespace gaeactorcomm {

class EventLoop;
class GAEACTOR_COMM_EXPORT GaeactorCommCommBase
{
public:

    GaeactorCommCommBase();
    virtual ~GaeactorCommCommBase();
    void start();
    virtual void stop();
protected:
    void initaddr(const std::string &ip, const uint16_t &port);

private:
    void exit();
    void quit();
    void thread_callback_Loop(void* param);

private:
    stdutils::OriThread *m_pRunningThread;

    std::atomic<bool> m_bLoopRnning;
protected:
    EventLoop* m_loop;
    SocketAddr m_socketaddr;
};
}
#endif // GAEACTOR_COMM_COMMBASE_H
