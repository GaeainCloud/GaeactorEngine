#include "gaeactor_comm_publish_string.h"
#include "./src/OriginalDateTime.h"

#include <ecal/msg/string/publisher.h>
namespace gaeactorcomm
{
GaeactorCommPublishString::GaeactorCommPublishString(const COMM_CHANNEL_INFO &_channel_info)
    : GaeactorCommBase(_channel_info)
{
    m_pPublisher = new eCAL::string::CPublisher<std::string>(_channel_info.m_topic.c_str());
#ifdef USING_ZERO_COPY
    m_pPublisher->ShmEnableZeroCopy(true);
#endif
}

GaeactorCommPublishString::~GaeactorCommPublishString()
{
}

bool GaeactorCommPublishString::sendData(const std::string &pData)
{
    size_t isendlen = m_pPublisher->Send(pData);
    return isendlen == pData.size() ? true : false;
}

//void GaeactorCommPublishString::deal_process()
//{
//    // Publish messages
//    while (isrunning())
//    {
//        eCAL::CPublisher pub("topic_name");
//        pub.Send("Hello, eCAL!");

//        // Sleep for some time
//        stdutils::OriDateTime::sleep(10);
//    }
//}

} // namespace gaeactorcomm
