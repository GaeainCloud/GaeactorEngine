#ifndef GAEACTOR_COMM_SUBSCRIBE_STRING_H
#define GAEACTOR_COMM_SUBSCRIBE_STRING_H

#include "../gaeactor_comm_base.h"
namespace eCAL {
namespace string {

template <typename T = std::string>
    class CSubscriber;
}
}

namespace gaeactorcomm {

class GaeactorCommSubscribeString:public GaeactorCommBase
{
public:
    explicit GaeactorCommSubscribeString(const COMM_CHANNEL_INFO &_channel_info);
    virtual ~GaeactorCommSubscribeString();
private:
    void msg_callback(const std::string &pData);
    void deal_msg_callback(const std::string &pData);

private:
    eCAL::string::CSubscriber<std::string> *m_pSubscriber;

    gaeactorcomm::DataBufferProcessor<std::string> *m_pDataBufferProcessor;
};
}
#endif // GAEACTOR_COMM_PUBLISH_TEXT_H
