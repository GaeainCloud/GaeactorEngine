#include "gaeactor_comm_interface.h"
#include <QDebug>
namespace gaeactorcomm
{
GaeactorComm::GaeactorComm(QObject *parent)
    :QObject(parent)
{
    m_pGaeactorCommProcessor = new GaeactorCommProcessor(this);
}

GaeactorComm &GaeactorComm::getInstance()
{
    static GaeactorComm _gaeactorcomm;
    return _gaeactorcomm;
}

GaeactorComm::~GaeactorComm()
{
    if(m_pGaeactorCommProcessor)
    {
        m_pGaeactorCommProcessor->deleteLater();
    }
}

void GaeactorComm::init(const char *servicename)
{
    m_pGaeactorCommProcessor->init(servicename);
}

std::string GaeactorComm::comm_servicename() const
{
    return m_pGaeactorCommProcessor->comm_servicename();
}

COMM_CHANNEL_INFO GaeactorComm::allocPublishTopicChannel(const char *topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType)
{
    return m_pGaeactorCommProcessor->allocPublishTopicChannel(topicname, eCommChannelDataType);
}


COMM_CHANNEL_INFO GaeactorComm::allocSubscribeTopicChannel(const char *topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType)
{
    return m_pGaeactorCommProcessor->allocSubscribeTopicChannel(topicname, eCommChannelDataType);
}

bool GaeactorComm::removeTopicChannel(const COMM_CHANNEL_INFO &topicname)
{
    return m_pGaeactorCommProcessor->removeTopicChannel(topicname);
}

bool GaeactorComm::sendData(const COMM_CHANNEL_INFO &channelinfo, const BYTE *pData, UINT32 iLen)
{
    return m_pGaeactorCommProcessor->sendData(channelinfo,pData,iLen);
}

bool GaeactorComm::sendData(const COMM_CHANNEL_INFO &channelinfo, const std::string &pData)
{
    return m_pGaeactorCommProcessor->sendData(channelinfo,pData);
}

void GaeactorComm::set_binary_data_callback(const COMM_CHANNEL_INFO& channelinfo, comm_binary_receive_callback func)
{
    return m_pGaeactorCommProcessor->set_binary_data_callback(channelinfo, func);
}

void GaeactorComm::set_string_data_callback(const COMM_CHANNEL_INFO &channelinfo, comm_string_receive_callback func)
{
    return m_pGaeactorCommProcessor->set_string_data_callback(channelinfo, func);
}

}
