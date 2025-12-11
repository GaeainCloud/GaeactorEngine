#include <QCoreApplication>
#include <QThreadPool>
#include <QRunnable>
#include <QObject>
#include <QMutex>
#include <iostream>
#include <atomic>

//禁止拷贝构造、右值拷贝和赋值
#define CAOS_NO_ALLOWED_COPY(CType)                                     \
    CType(const CType &) = delete;                                      \
    CType(const CType&&) = delete;                                      \
    const CType& operator=(const CType &) = delete;                     \

//线程任务回调（考虑参数可能会过多，使用时可添加结构体声明）
typedef std::function<void (const QByteArray&)> deal_data_func_callback;


//任务模板
template<typename T>
class ThreadTaskProcessor: public QRunnable
{
public:
    ThreadTaskProcessor(int id,
                        deal_data_func_callback _pCallbackfunc,
                        T&& param1 = T())
        :m_id(id)
        ,m_pCallbackfunc(std::move(_pCallbackfunc))
        ,m_param1(std::move(param1))
    {
        //m_processorNum++;
    }
    virtual ~ThreadTaskProcessor(){}
    virtual void run() override
    {
        if(m_pCallbackfunc)
        {
            m_pCallbackfunc(m_param1);
        }
    }
private:
    int m_id;
    deal_data_func_callback m_pCallbackfunc;
    T m_param1;
};


//线程任务回调（考虑参数可能会过多，使用时可添加结构体声明）
typedef std::function<void (const QVector<QByteArray>&)> deal_data_array_func_callback;

//任务模板
template<typename T>
class ThreadTaskArrayProcessor: public QRunnable
{
public:
    ThreadTaskArrayProcessor(int id,
                        deal_data_array_func_callback _pCallbackfunc,
                        T&& param1 = T())
        :m_id(id)
        ,m_pCallbackfunc(std::move(_pCallbackfunc))
        ,m_param1(std::move(param1))
    {
        //m_processorNum++;
    }
    virtual ~ThreadTaskArrayProcessor(){}
    virtual void run() override
    {
        if(m_pCallbackfunc)
        {
            m_pCallbackfunc(m_param1);
        }
    }
private:
    int m_id;
    deal_data_array_func_callback m_pCallbackfunc;
    T m_param1;
};

class ThreadPoolTaskManager:public QObject
{
    Q_OBJECT
public:
    static ThreadPoolTaskManager& getInstance();

public:
    //添加线程任务
    template<typename T>
    void appendProcessor(deal_data_func_callback _pCallbackfunc,
                         T&& param1 = T())
    {
        ThreadTaskProcessor<T> *pThreadTaskProcessor = new ThreadTaskProcessor<T>(0, _pCallbackfunc, std::move(param1));
        pThreadTaskProcessor->setAutoDelete(true);
#if 0  //采用全局线程池
        QThreadPool::globalInstance()->start(pThreadTaskProcessor);
#else
        m_threadPool->start(pThreadTaskProcessor,QThread::TimeCriticalPriority);
#endif
    }

    //添加线程任务
    template<typename T>
    void appendArrayProcessor(deal_data_array_func_callback _pCallbackfunc,
                         T&& param1 = T())
    {
        ThreadTaskArrayProcessor<T> *pThreadTaskProcessor = new ThreadTaskArrayProcessor<T>(0, _pCallbackfunc,std::move(param1));
        pThreadTaskProcessor->setAutoDelete(true);
#if 0  //采用全局线程池
        QThreadPool::globalInstance()->start(pThreadTaskProcessor);
#else
        m_threadPool->start(pThreadTaskProcessor,QThread::TimeCriticalPriority);
#endif
    }

    //获取线程池中活跃线程数
    int getActiveThreadCount() const;

private:
    ThreadPoolTaskManager(QObject* parent = nullptr);
    virtual ~ThreadPoolTaskManager();
    CAOS_NO_ALLOWED_COPY(ThreadPoolTaskManager)

private:
    QThreadPool* m_threadPool;
    std::atomic<unsigned int> m_runningTaskNum;
};


