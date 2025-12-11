#ifndef GAEACTOR_COMM_SUBSCRIBE_PROTOBUF_H
#define GAEACTOR_COMM_SUBSCRIBE_PROTOBUF_H

#include "../gaeactor_comm_base.h"

#include <ecal/msg/protobuf/subscriber.h>

namespace gaeactorcomm {
template <typename T>
class GaeactorCommSubscribeProtobuf:public GaeactorCommBase
{
public:
    explicit GaeactorCommSubscribeProtobuf(const COMM_CHANNEL_INFO &_channel_info)
        : GaeactorCommBase(_channel_info)
        , m_comm_proto_recive_callback(nullptr)
        , m_pDataBufferProcessor(nullptr)
    {
        m_pDataBufferProcessor = new gaeactorcomm::DataBufferProcessor<T>();
        m_pDataBufferProcessor->setDeal_runtimedata_func_callback(std::bind(&GaeactorCommSubscribeProtobuf::deal_msg_callback,this, std::placeholders::_1));

        m_pSubscriber.Create(_channel_info.m_topic.c_str());

        m_pSubscriber.AddReceiveCallback(std::bind(&GaeactorCommSubscribeProtobuf::msg_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }
    virtual ~GaeactorCommSubscribeProtobuf()
    {
        if(m_pDataBufferProcessor)
        {
            delete m_pDataBufferProcessor;
        }
    }

    void set_protobuf_data_callback(std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
    {
        m_comm_proto_recive_callback = std::move(callback);
    }
private:

    void msg_callback(const char* topic_name_, const T& pData, const long long time_, const long long clock_)
    {
        if(m_pDataBufferProcessor)
        {
            T recive_data = pData;
            m_pDataBufferProcessor->appendData(std::move(recive_data));
        }
    }


    void deal_msg_callback(const T &pData)
    {
        if(m_comm_proto_recive_callback)
        {
            m_comm_proto_recive_callback(m_channel_info, &pData);
        }
    }
private:
    eCAL::protobuf::CSubscriber<T> m_pSubscriber;
    std::function<void (const COMM_CHANNEL_INFO &, const T*)> m_comm_proto_recive_callback;

    gaeactorcomm::DataBufferProcessor<T> *m_pDataBufferProcessor;
};
}
#endif // GAEACTOR_COMM_PUBLISH_TEXT_H
