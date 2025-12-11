#include "gaeactor_comm_subscribe_binary.h"
#include <ecal/ecal_subscriber.h>
#include <iostream>
namespace gaeactorcomm
{
GaeactorCommSubscribeBinary::GaeactorCommSubscribeBinary(const COMM_CHANNEL_INFO &_channel_info)
    :GaeactorCommBase(_channel_info)
    , m_pDataBufferProcessor(nullptr)
{
    m_pDataBufferProcessor = new gaeactorcomm::DataBufferProcessor<BINARYARRY>();
    m_pDataBufferProcessor->setDeal_runtimedata_func_callback(std::bind(&GaeactorCommSubscribeBinary::deal_msg_callback,this, std::placeholders::_1));

    m_pSubscriber = new eCAL::CSubscriber(_channel_info.m_topic.c_str());

    m_pSubscriber->AddReceiveCallback(std::bind(&GaeactorCommSubscribeBinary::msg_callback,this, std::placeholders::_1, std::placeholders::_2));
}

GaeactorCommSubscribeBinary::~GaeactorCommSubscribeBinary()
{
    if(m_pDataBufferProcessor)
    {
        delete m_pDataBufferProcessor;
    }
}

void GaeactorCommSubscribeBinary::msg_callback(const char * topic_name_, const eCAL::SReceiveCallbackData *data_)
{
    if(m_pDataBufferProcessor)
    {
        BINARYARRY recive_data;
        recive_data.resize(data_->size);
        memcpy(recive_data.data(), data_->buf, data_->size);
        m_pDataBufferProcessor->appendData(std::move(recive_data));
    }
}

void GaeactorCommSubscribeBinary::deal_msg_callback(const BINARYARRY &pData)
{
    if(m_comm_binary_receive_callback)
    {
        m_comm_binary_receive_callback(m_channel_info, reinterpret_cast<const BYTE*>(pData.data()), pData.size());
    }
}


} // namespace gaeactorcomm
