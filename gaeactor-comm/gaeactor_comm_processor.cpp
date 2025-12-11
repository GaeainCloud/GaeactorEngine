#include "gaeactor_comm_processor.h"
#include "ecal/ecal.h"

#include "src/OriginalThread.h"

//#include "loghelper.h"

#include "src/OriginalDateTime.h"
#include "src/publish/gaeactor_comm_publish_string.h"
#include "src/publish/gaeactor_comm_publish_binary.h"
#include "src/subscrib/gaeactor_comm_subscribe_string.h"
#include "src/subscrib/gaeactor_comm_subscribe_binary.h"



namespace gaeactorcomm
{
GaeactorCommProcessor::GaeactorCommProcessor(QObject *parent)
{

}

GaeactorCommProcessor::~GaeactorCommProcessor()
{
    destory();
}

void GaeactorCommProcessor::init(const char *servicename)
{
    m_servicename = servicename;
    // 初始化 ECAL 库
    eCAL::Initialize(0, nullptr, servicename);
    std::cout<<"GetLoadedEcalIniPath ecal ini file is "<<eCAL::Config::GetLoadedEcalIniPath()<<"\n";
}

void GaeactorCommProcessor::destory()
{
    // 关闭 ECAL 库
    eCAL::Finalize();
}

std::string GaeactorCommProcessor::comm_servicename() const
{
    return m_servicename;
}


COMM_CHANNEL_INFO GaeactorCommProcessor::allocPublishTopicChannel(const char *topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType)
{
    COMM_CHANNEL_INFO _channel_info(m_servicename,
                                    std::string(topicname),
                                    eCommChannelDataType,
                                    E_COMM_CHANNEL_TYPE_PUBLISHER);

    return allocTopicChannel(_channel_info);
}

COMM_CHANNEL_INFO GaeactorCommProcessor::allocSubscribeTopicChannel(const char *topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType)
{
    COMM_CHANNEL_INFO _channel_info(m_servicename,
                                    std::string(topicname),
                                    eCommChannelDataType,
                                    E_COMM_CHANNEL_TYPE_SUBSCRIBER);

    return allocTopicChannel(_channel_info);
}

bool GaeactorCommProcessor::removeTopicChannel(const COMM_CHANNEL_INFO &_channel_info)
{
    auto itor = m_channels.find(_channel_info);
    if(itor != m_channels.end())
    {
        GaeactorCommBase* pGaeactorCommBase = itor->second;
        if(pGaeactorCommBase)
        {
            delete pGaeactorCommBase;
        }
        m_channels.erase(_channel_info);
    }
    return true;
}


bool GaeactorCommProcessor::sendData(const COMM_CHANNEL_INFO &channelinfo, const BYTE *pData, UINT32 iLen)
{
    if(channelinfo.m_eCommChannelType != E_COMM_CHANNEL_TYPE_PUBLISHER)
    {
        return false;
    }
    GaeactorCommBase * pGaeactorCommBase = getGaeactorCommBase(channelinfo);
    if(nullptr == pGaeactorCommBase)
    {
        return false;
    }
    return pGaeactorCommBase->sendData(pData, iLen);
}

bool GaeactorCommProcessor::sendData(const COMM_CHANNEL_INFO &channelinfo, const std::string &pData)
{
    if(channelinfo.m_eCommChannelType != E_COMM_CHANNEL_TYPE_PUBLISHER)
    {
        return false;
    }

    GaeactorCommBase * pGaeactorCommBase = getGaeactorCommBase(channelinfo);
    if(nullptr == pGaeactorCommBase)
    {
        return false;
    }
    return pGaeactorCommBase->sendData(pData);
}

void GaeactorCommProcessor::set_binary_data_callback(const COMM_CHANNEL_INFO& channelinfo, comm_binary_receive_callback func)
{
    if(channelinfo.m_eCommChannelType != E_COMM_CHANNEL_TYPE_SUBSCRIBER)
    {
        return;
    }

    GaeactorCommBase * pGaeactorCommBase = getGaeactorCommBase(channelinfo);
    if(nullptr == pGaeactorCommBase)
    {
        return;
    }
    return pGaeactorCommBase->set_binary_data_callback(func);
}

void GaeactorCommProcessor::set_string_data_callback(const COMM_CHANNEL_INFO& channelinfo, comm_string_receive_callback func)
{
    if(channelinfo.m_eCommChannelType != E_COMM_CHANNEL_TYPE_SUBSCRIBER)
    {
        return;
    }

    GaeactorCommBase * pGaeactorCommBase = getGaeactorCommBase(channelinfo);
    if(nullptr == pGaeactorCommBase)
    {
        return;
    }
    return pGaeactorCommBase->set_string_data_callback(func);
}

COMM_CHANNEL_INFO GaeactorCommProcessor::allocTopicChannel(const COMM_CHANNEL_INFO &_channel_info)
{
    if(m_channels.find(_channel_info) == m_channels.end())
    {
        GaeactorCommBase* pGaeactorCommBase = nullptr;

        switch(_channel_info.m_eCommChannelType)
        {
        case E_COMM_CHANNEL_TYPE_PUBLISHER:
        {
            switch (_channel_info.m_eCommChannelDateType)
            {
            case E_COMM_CHANNEL_DATE_TYPE_STRING:
            {
                pGaeactorCommBase = new GaeactorCommPublishString(_channel_info);
            }
            break;
            case E_COMM_CHANNEL_DATE_TYPE_BINARY:
            {
                pGaeactorCommBase = new GaeactorCommPublishBinary(_channel_info);
            }
            break;
            default:
                break;
            }
        }
        break;
        case E_COMM_CHANNEL_TYPE_SUBSCRIBER:
        {
            switch (_channel_info.m_eCommChannelDateType)
            {
            case E_COMM_CHANNEL_DATE_TYPE_STRING:
            {
                pGaeactorCommBase = new GaeactorCommSubscribeString(_channel_info);
            }
            break;
            case E_COMM_CHANNEL_DATE_TYPE_BINARY:
            {
                pGaeactorCommBase = new GaeactorCommSubscribeBinary(_channel_info);
            }
            break;
            default:
                break;
            }
        }
        break;
        default:
            break;
        }

        if(pGaeactorCommBase)
        {
            m_channels.insert(std::make_pair(_channel_info, pGaeactorCommBase));
        }

    }
    return _channel_info;
}

GaeactorCommBase *GaeactorCommProcessor::getGaeactorCommBase(const COMM_CHANNEL_INFO &_channel_info)
{
    if(m_channels.find(_channel_info) != m_channels.end())
    {
        return m_channels.at(_channel_info);
    }
    return nullptr;
}
}


