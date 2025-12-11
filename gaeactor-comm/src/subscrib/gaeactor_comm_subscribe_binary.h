#ifndef GAEACTOR_COMM_SUBSCRIBE_BINARY_H
#define GAEACTOR_COMM_SUBSCRIBE_BINARY_H

#include "../gaeactor_comm_base.h"
#include <vector>
namespace eCAL {
class CSubscriber;
struct SReceiveCallbackData;
}

namespace gaeactorcomm {

typedef std::vector<BYTE> BINARYARRY;

class GaeactorCommSubscribeBinary:public GaeactorCommBase
{
public:
    explicit GaeactorCommSubscribeBinary(const COMM_CHANNEL_INFO &_channel_info);
    virtual ~GaeactorCommSubscribeBinary();

private:
    void msg_callback(const char* /*topic_name_*/, const struct eCAL::SReceiveCallbackData* data_);
    void deal_msg_callback(const BINARYARRY &pData);
private:
    eCAL::CSubscriber *m_pSubscriber;

    gaeactorcomm::DataBufferProcessor<BINARYARRY> *m_pDataBufferProcessor;
};
}
#endif // GAEACTOR_COMM_SUBSCRIBE_BINARY_H
