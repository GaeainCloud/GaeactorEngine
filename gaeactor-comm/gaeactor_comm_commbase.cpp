#include "gaeactor_comm_commbase.h"
#include "src/OriginalThread.h"
#include "src/OriginalDateTime.h"
#include "EventLoop.hpp"

#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
#include <winsock2.h>
#else /* UNIX */
#endif

namespace gaeactorcomm
{
GaeactorCommCommBase::GaeactorCommCommBase()
    :m_pRunningThread(nullptr),
    m_loop(nullptr)
{
    m_bLoopRnning.store(false);
    m_loop = new EventLoop();
    m_pRunningThread = new stdutils::OriThread(std::bind(&GaeactorCommCommBase::thread_callback_Loop, this, std::placeholders::_1), this);
    m_pRunningThread->start();
}



GaeactorCommCommBase::~GaeactorCommCommBase()
{
    exit();
    if (m_pRunningThread)
    {
        delete m_pRunningThread;
        m_pRunningThread = nullptr;
    }
    delete m_loop;
}


void GaeactorCommCommBase::start()
{
    m_bLoopRnning.store(true);
}

void GaeactorCommCommBase::stop()
{
    m_bLoopRnning.store(false);
    quit();
}

void GaeactorCommCommBase::initaddr(const std::string &ip, const UINT16 &port)
{
    m_socketaddr = SocketAddr(ip,port);
}

void GaeactorCommCommBase::exit()
{
    stop();
}

void GaeactorCommCommBase::quit()
{
    m_loop->stop();
}


void GaeactorCommCommBase::thread_callback_Loop(void* param)
{
    if (m_bLoopRnning.load())
    {
//        uv_run(m_loop, UV_RUN_NOWAIT);
        std::cout << "uv event looping " << m_socketaddr.toStr()<<"\n";
        m_loop->run();
    }
    else
    {
        stdutils::OriDateTime::sleep(1);
    }
}

}
