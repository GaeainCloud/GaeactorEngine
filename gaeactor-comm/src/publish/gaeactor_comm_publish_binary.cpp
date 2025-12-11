#include "gaeactor_comm_publish_binary.h"
#include <ecal/ecal_publisher.h>
namespace gaeactorcomm
{
GaeactorCommPublishBinary::GaeactorCommPublishBinary(const COMM_CHANNEL_INFO &_channel_info)
    :GaeactorCommBase(_channel_info)
{
    m_pPublisher = new eCAL::CPublisher(_channel_info.m_topic.c_str());
#ifdef USING_ZERO_COPY
    m_pPublisher->ShmEnableZeroCopy(true);
#endif
}

GaeactorCommPublishBinary::~GaeactorCommPublishBinary()
{
}

bool GaeactorCommPublishBinary::sendData(const BYTE *pData, UINT32 iLen)
{
    size_t isendlen = m_pPublisher->Send(pData,iLen);
    return isendlen == iLen ? true : false;
}


} // namespace gaeactorcomm
