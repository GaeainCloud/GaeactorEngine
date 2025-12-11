#ifndef GAEACTOR_COMM_SUBSCRIBE_STRUCT_H
#define GAEACTOR_COMM_SUBSCRIBE_STRUCT_H

#include "../gaeactor_comm_base.h"

#include <ecal/ecal_subscriber.h>

namespace gaeactorcomm {
template <typename T>
class GaeactorCommSubscribeStruct:public GaeactorCommBase
{
public:
    explicit GaeactorCommSubscribeStruct(const COMM_CHANNEL_INFO &_channel_info)
        : GaeactorCommBase(_channel_info)
        , m_comm_struct_recive_callback(nullptr)
    {
        m_pDataBufferProcessor = new gaeactorcomm::DataBufferProcessor<T>();
        m_pDataBufferProcessor->setDeal_runtimedata_func_callback(std::bind(&GaeactorCommSubscribeStruct::deal_msg_callback,this, std::placeholders::_1));

        m_pSubscriber.Create(_channel_info.m_topic.c_str());

        m_pSubscriber.AddReceiveCallback(std::bind(&GaeactorCommSubscribeStruct::msg_callback,this, std::placeholders::_1, std::placeholders::_2));
    }
    virtual ~GaeactorCommSubscribeStruct()
    {
        if(m_pDataBufferProcessor)
        {
            delete m_pDataBufferProcessor;
        }
    }
    void set_struct_data_callback(std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
    {
        m_comm_struct_recive_callback = std::move(callback);
    };
    void msg_callback(const char * topic_name_, const eCAL::SReceiveCallbackData *data_)
    {
        if (data_->size < 1) return;

        if(m_pDataBufferProcessor && sizeof(T) ==  data_->size)
        {
            T recive_data;
            memcpy(&recive_data, data_->buf, data_->size);
            m_pDataBufferProcessor->appendData(std::move(recive_data));
        }
    }
    void deal_msg_callback(const T &pData)
    {
        if(m_comm_struct_recive_callback)
        {
            m_comm_struct_recive_callback(m_channel_info, &pData);
        }
    }
private:
    eCAL::CSubscriber m_pSubscriber;
    std::function<void (const COMM_CHANNEL_INFO &, const T*)> m_comm_struct_recive_callback;
    gaeactorcomm::DataBufferProcessor<T> *m_pDataBufferProcessor;
};
}
#endif // GAEACTOR_COMM_PUBLISH_TEXT_H
