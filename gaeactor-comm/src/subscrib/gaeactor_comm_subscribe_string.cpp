#include "gaeactor_comm_subscribe_string.h"

#include <ecal/msg/string/subscriber.h>
namespace gaeactorcomm
{
GaeactorCommSubscribeString::GaeactorCommSubscribeString(const COMM_CHANNEL_INFO &_channel_info)
    : GaeactorCommBase(_channel_info)
    , m_pSubscriber(nullptr)
    , m_pDataBufferProcessor(nullptr)
{
    m_pDataBufferProcessor = new gaeactorcomm::DataBufferProcessor<std::string>();
    m_pDataBufferProcessor->setDeal_runtimedata_func_callback(std::bind(&GaeactorCommSubscribeString::deal_msg_callback,this, std::placeholders::_1));

    m_pSubscriber = new eCAL::string::CSubscriber<std::string>(_channel_info.m_topic.c_str());
    m_pSubscriber->AddReceiveCallback(std::bind(&GaeactorCommSubscribeString::msg_callback,this, std::placeholders::_2));
}

GaeactorCommSubscribeString::~GaeactorCommSubscribeString()
{
    if(m_pDataBufferProcessor)
    {
        delete m_pDataBufferProcessor;
    }
}

void GaeactorCommSubscribeString::msg_callback(const std::string &pData)
{
    if(m_pDataBufferProcessor)
    {
        std::string recive_data = pData;
        m_pDataBufferProcessor->appendData(std::move(recive_data));
    }
}

void GaeactorCommSubscribeString::deal_msg_callback(const std::string &pData)
{
    if(m_comm_string_receive_callback)
    {
        m_comm_string_receive_callback(m_channel_info, pData);
    }
}

} // namespace gaeactorcomm
