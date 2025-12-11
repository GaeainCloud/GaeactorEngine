#ifndef GAEACTOR_COMM_PROCESSOR_H
#define GAEACTOR_COMM_PROCESSOR_H

#include <QObject>
#include "gaeactor_comm_define.h"
#include <QHash>
#include <QList>
#include <QReadWriteLock>

#include "src/gaeactor_comm_base.h"
#include "src/publish/gaeactor_comm_publish_struct.h"
#include "src/subscrib/gaeactor_comm_subscribe_struct.h"

#include "src/publish/gaeactor_comm_publish_protobuf.h"
#include "src/subscrib/gaeactor_comm_subscribe_protobuf.h"

namespace gaeactorcomm
{
class GAEACTOR_COMM_EXPORT GaeactorCommProcessor : public QObject
{
    Q_OBJECT
public:    
    explicit GaeactorCommProcessor(QObject *parent = nullptr);
    virtual ~GaeactorCommProcessor();
    void init(const char* servicename);
    void destory();

    std::string comm_servicename() const;

    COMM_CHANNEL_INFO allocPublishTopicChannel(const char* topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType);
    COMM_CHANNEL_INFO allocSubscribeTopicChannel(const char* topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    COMM_CHANNEL_INFO allocStructPublishTopicChannel(const char* topicname,const char* structTypeName)
    {
        COMM_CHANNEL_INFO _channel_info(m_servicename,
                                        std::string(topicname),
                                        E_COMM_CHANNEL_DATE_TYPE_STRUCT,
                                        E_COMM_CHANNEL_TYPE_PUBLISHER);

        if(m_channels.find(_channel_info) == m_channels.end())
        {
            GaeactorCommBase* pGaeactorCommBase = nullptr;
            pGaeactorCommBase = new GaeactorCommPublishStruct<T>(_channel_info, structTypeName);
            m_channels.insert(std::make_pair(_channel_info, pGaeactorCommBase));
        }
        return _channel_info;
    }
    template <typename T>
    COMM_CHANNEL_INFO allocStructSubscribeTopicChannel(const char* topicname)
    {
        COMM_CHANNEL_INFO _channel_info(m_servicename,
                                        std::string(topicname),
                                        E_COMM_CHANNEL_DATE_TYPE_STRUCT,
                                        E_COMM_CHANNEL_TYPE_SUBSCRIBER);
        if(m_channels.find(_channel_info) == m_channels.end())
        {
            GaeactorCommBase* pGaeactorCommBase = nullptr;
            pGaeactorCommBase = new GaeactorCommSubscribeStruct<T>(_channel_info);
            m_channels.insert(std::make_pair(_channel_info, pGaeactorCommBase));
        }
        return _channel_info;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    COMM_CHANNEL_INFO allocProtobufPublishTopicChannel(const char* topicname)
    {
        COMM_CHANNEL_INFO _channel_info(m_servicename,
                                        std::string(topicname),
                                        E_COMM_CHANNEL_DATE_TYPE_PROTOBUF,
                                        E_COMM_CHANNEL_TYPE_PUBLISHER);

        if(m_channels.find(_channel_info) == m_channels.end())
        {
            GaeactorCommBase* pGaeactorCommBase = nullptr;
            pGaeactorCommBase = new GaeactorCommPublishProtobuf<T>(_channel_info);
            m_channels.insert(std::make_pair(_channel_info, pGaeactorCommBase));
        }
        return _channel_info;
    }

    template <typename T>
    COMM_CHANNEL_INFO allocProtobufSubscribeTopicChannel(const char* topicname)
    {
        COMM_CHANNEL_INFO _channel_info(m_servicename,
                                        std::string(topicname),
                                        E_COMM_CHANNEL_DATE_TYPE_PROTOBUF,
                                        E_COMM_CHANNEL_TYPE_SUBSCRIBER);
        if(m_channels.find(_channel_info) == m_channels.end())
        {
            GaeactorCommBase* pGaeactorCommBase = nullptr;
            pGaeactorCommBase = new GaeactorCommSubscribeProtobuf<T>(_channel_info);
            m_channels.insert(std::make_pair(_channel_info, pGaeactorCommBase));
        }
        return _channel_info;
    }

    bool removeTopicChannel(const COMM_CHANNEL_INFO & _channel_info);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    bool sendData(const COMM_CHANNEL_INFO& channelinfo, const BYTE *pData, UINT32 iLen);
    bool sendData(const COMM_CHANNEL_INFO& channelinfo, const std::string &pData);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool sendStructData(const COMM_CHANNEL_INFO& channelinfo,  const T &pData)
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
        GaeactorCommPublishStruct<T> *pGaeactorCommPublishStruct = dynamic_cast<GaeactorCommPublishStruct<T> *>(pGaeactorCommBase);
        if(nullptr == pGaeactorCommPublishStruct)
        {
            return false;
        }

        return pGaeactorCommPublishStruct->sendStructData(pData);
    }

    template <typename T>
    bool sendProtobufData(const COMM_CHANNEL_INFO& channelinfo,  const T &pData)
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
        GaeactorCommPublishProtobuf<T> *pGaeactorCommPublishProtobuf = dynamic_cast<GaeactorCommPublishProtobuf<T> *>(pGaeactorCommBase);
        if(nullptr == pGaeactorCommPublishProtobuf)
        {
            return false;
        }
        return pGaeactorCommPublishProtobuf->sendProtobufData(pData);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    void set_binary_data_callback(const COMM_CHANNEL_INFO& channelinfo, comm_binary_receive_callback func);
    void set_string_data_callback(const COMM_CHANNEL_INFO& channelinfo, comm_string_receive_callback func);
    template<typename T>
    void set_struct_data_callback(const COMM_CHANNEL_INFO& channelinfo, std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
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
        GaeactorCommSubscribeStruct<T> *pGaeactorCommPublishStruct = dynamic_cast<GaeactorCommSubscribeStruct<T> *>(pGaeactorCommBase);
        return pGaeactorCommPublishStruct->set_struct_data_callback(callback);
    }

    template<typename T>
    void set_protobuf_data_callback(const COMM_CHANNEL_INFO& channelinfo, std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
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
        GaeactorCommSubscribeProtobuf<T> *pGaeactorCommSubscribeStruct = dynamic_cast<GaeactorCommSubscribeProtobuf<T> *>(pGaeactorCommBase);
        return pGaeactorCommSubscribeStruct->set_protobuf_data_callback(callback);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    COMM_CHANNEL_INFO allocTopicChannel(const COMM_CHANNEL_INFO &_channel_info);

    GaeactorCommBase* getGaeactorCommBase(const COMM_CHANNEL_INFO &_channel_info);
private:
    std::string m_servicename;

    std::unordered_map<COMM_CHANNEL_INFO, GaeactorCommBase*> m_channels;
};
}
#endif // GAEACTOR_COMM_PROCESSOR_H
