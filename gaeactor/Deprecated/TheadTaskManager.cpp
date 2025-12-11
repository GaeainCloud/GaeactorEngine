#include "TheadTaskManager.h"
#include "head_define.h"


ThreadPoolTaskManager& ThreadPoolTaskManager::getInstance()
{
    static ThreadPoolTaskManager threadPoolTaskManager;
    return threadPoolTaskManager;
}
ThreadPoolTaskManager::ThreadPoolTaskManager(QObject* parent)
    :QObject(parent)
{
    m_threadPool = new QThreadPool(this);
#if 0
    m_threadPool->setMaxThreadCount((std::thread::hardware_concurrency()>1)?std::thread::hardware_concurrency() : 1);
#else
    //线程池中最大工作线程数量设置为4
    m_threadPool->setMaxThreadCount(THREADPOOL_THREAD_NUM*2);
#endif
}

ThreadPoolTaskManager::~ThreadPoolTaskManager()
{
    if(m_threadPool)
    {
        m_threadPool->clear();
        m_threadPool->waitForDone();
        delete m_threadPool;
        m_threadPool = nullptr;
    }
}

int ThreadPoolTaskManager::getActiveThreadCount() const
{
    return m_threadPool->activeThreadCount();
}



