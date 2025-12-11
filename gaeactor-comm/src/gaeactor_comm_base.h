#ifndef GAEACTOR_COMM_BASE_H
#define GAEACTOR_COMM_BASE_H

#include "gaeactor_comm_global.h"
#include "gaeactor_comm_define.h"
#include <atomic>
#include <queue>
#include "./src/OriginalMutex.h"
#include "./src/OriginalThread.h"

#define USING_ZERO_COPY

namespace eCAL {
class CPublisher;
class CSubscriber;
}

namespace stdutils {
class OriThread;
}

namespace gaeactorcomm {


template <class T>
class DataBufferProcessor
{
public:
    typedef std::function<void (const T &pdata)> deal_runtimedata_func_callback;

    DataBufferProcessor()
        :m_hDataDealThread(nullptr)
        ,m_deal_runtimedata_func_callback(nullptr)
    {
#ifdef _MSC_VER
        m_hDataDealThread = new stdutils::OriThread(std::bind(&DataBufferProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                    nullptr,\
                                                    THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread = new stdutils::OriThread(std::bind(&DataBufferProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                    nullptr,\
                                                    99);
#endif
        m_hDataDealThread->start();
    }

    ~DataBufferProcessor()
    {
        m_dealfullCond.wakeAll();
        //清理DataDealThread

        if (m_hDataDealThread != nullptr)
        {
            delete m_hDataDealThread;
            m_hDataDealThread = nullptr;
        }
    }
    void dealWake()
    {
        m_dealfullCond.wakeAll();
    }

    void appendData(T && eventdata)
    {
        stdutils::OriMutexLocker locker(&m_data_queue_mutex);
        m_data_queue.push(std::move(eventdata));
        dealWake();
    }

    void setDeal_runtimedata_func_callback(deal_runtimedata_func_callback newDeal_runtimedata_func_callback)
    {
        m_deal_runtimedata_func_callback = std::move(newDeal_runtimedata_func_callback);
    }

private:
    void data_deal_thread_func(void *pParam)
    {
        if(is_queue_empty())
        {
            m_dealfullCond.wait(&m_dealmutex);
        }
        deal_data();
    }

    void deal_data()
    {
        std::queue<T> data_queue_high_priority;
        get_high_priority(data_queue_high_priority);
        while (!data_queue_high_priority.empty())
        {
            const T& data = data_queue_high_priority.front();
            if(m_deal_runtimedata_func_callback)
            {
                m_deal_runtimedata_func_callback(data);
            }
            data_queue_high_priority.pop();
        }
    }

    bool is_queue_empty()
    {
        bool bEmpty = true;
        {
            stdutils::OriMutexLocker locker(&m_data_queue_mutex);
            bEmpty &= m_data_queue.empty();
        }
        return bEmpty;
    }

    void get_high_priority(std::queue<T> &dataqueue)
    {
        stdutils::OriMutexLocker locker(&m_data_queue_mutex);
        dataqueue = std::move(m_data_queue);
    }

private:
    stdutils::OriThread* m_hDataDealThread;
    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;

    stdutils::OriMutexLock m_data_queue_mutex;
    std::queue<T> m_data_queue;

    deal_runtimedata_func_callback m_deal_runtimedata_func_callback;
};

class GAEACTOR_COMM_EXPORT GaeactorCommBase
{
public:
    explicit GaeactorCommBase(const COMM_CHANNEL_INFO &_channel_info);
    virtual ~GaeactorCommBase();
    virtual bool sendData(const BYTE *pData, UINT32 iLen);
    virtual bool sendData(const std::string &pData);

//    virtual void deal_process();

    void set_binary_data_callback(comm_binary_receive_callback func);
    void set_string_data_callback(comm_string_receive_callback func);
//    bool isrunning() const;
//    void setBrunning(bool newBrunning);

//private:
//    void deal_thread_func(void *pParam);
protected:
    COMM_CHANNEL_INFO m_channel_info;
    comm_binary_receive_callback m_comm_binary_receive_callback;
    comm_string_receive_callback m_comm_string_receive_callback;
protected:
    eCAL::CPublisher *m_pPublisher;
    eCAL::CSubscriber *m_pSubscriber;


//    stdutils::OriThread* m_hDataDealThread;

//    std::atomic_bool m_brunning;
};
}
#endif // GAEACTOR_COMM_BASE_H
