#ifndef GAEACTOR_COMM_PUBLISH_PROTOBUF_H
#define GAEACTOR_COMM_PUBLISH_PROTOBUF_H

#include "../gaeactor_comm_base.h"

#include <ecal/ecal.h>

#include <ecal/msg/protobuf/publisher.h>
namespace gaeactorcomm {
template <typename T>
class GaeactorCommPublishProtobuf:public GaeactorCommBase
{
public:
    explicit GaeactorCommPublishProtobuf(const COMM_CHANNEL_INFO &_channel_info)
        : GaeactorCommBase(_channel_info)
    {
        m_pProtobufPublisher= new eCAL::protobuf::CPublisher<T>(_channel_info.m_topic.c_str());
#ifdef USING_ZERO_COPY
        m_pProtobufPublisher->ShmEnableZeroCopy(true);
#endif
    }
    virtual ~GaeactorCommPublishProtobuf()
    {
        if(m_pProtobufPublisher)
        {
            delete m_pProtobufPublisher;
        }
    }

    bool sendProtobufData(const T &pData)
    {
        m_pProtobufPublisher->Send(pData);
        return true;
    }
private:
    eCAL::protobuf::CPublisher<T> *m_pProtobufPublisher;
};
}
#endif // GAEACTOR_COMM_PUBLISH_PROTOBUF_H
