#ifndef GAEACTOR_COMM_INTERFACE_H
#define GAEACTOR_COMM_INTERFACE_H

#include "gaeactor_comm_global.h"
#include <QObject>
#include "gaeactor_comm_define.h"
#include "gaeactor_comm_processor.h"
//#define USING_GAEACTOR_COMM

namespace gaeactorcomm {
class GaeactorCommProcessor;
class GAEACTOR_COMM_EXPORT GaeactorComm : public QObject
{
    Q_OBJECT
public:
    static GaeactorComm & getInstance();
    virtual ~GaeactorComm();

    void init(const char* servicename);

    std::string comm_servicename() const;

    COMM_CHANNEL_INFO allocPublishTopicChannel(const char* topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType);
    COMM_CHANNEL_INFO allocSubscribeTopicChannel(const char* topicname, E_COMM_CHANNEL_DATE_TYPE eCommChannelDataType);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    COMM_CHANNEL_INFO allocStructPublishTopicChannel(const char* topicname,const char* structTypeName)
    {
        return m_pGaeactorCommProcessor->allocStructPublishTopicChannel<T>(topicname,structTypeName);
    }

    template <typename T>
    COMM_CHANNEL_INFO allocStructSubscribeTopicChannel(const char* topicname)
    {
        return m_pGaeactorCommProcessor->allocStructSubscribeTopicChannel<T>(topicname);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template <typename T>
    COMM_CHANNEL_INFO allocProtobufPublishTopicChannel(const char* topicname)
    {
        return m_pGaeactorCommProcessor->allocProtobufPublishTopicChannel<T>(topicname);
    }

    template <typename T>
    COMM_CHANNEL_INFO allocProtobufSubscribeTopicChannel(const char* topicname)
    {
        return m_pGaeactorCommProcessor->allocProtobufSubscribeTopicChannel<T>(topicname);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool removeTopicChannel(const COMM_CHANNEL_INFO & _channel_info);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool sendData(const COMM_CHANNEL_INFO& channelinfo, const BYTE *pData, UINT32 iLen);
    bool sendData(const COMM_CHANNEL_INFO& channelinfo, const std::string &pData);

    template <typename T>
    bool sendStructData(const COMM_CHANNEL_INFO& channelinfo,  const T &pData)
    {
        return m_pGaeactorCommProcessor->sendStructData<T>(channelinfo, pData);
    }

    template <typename T>
    bool sendProtobufData(const COMM_CHANNEL_INFO& channelinfo,  const T &pData)
    {
        return m_pGaeactorCommProcessor->sendProtobufData<T>(channelinfo, pData);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void set_binary_data_callback(const COMM_CHANNEL_INFO& channelinfo,comm_binary_receive_callback func);
    void set_string_data_callback(const COMM_CHANNEL_INFO& channelinfo,comm_string_receive_callback func);

    // 接受函数对象的函数
    template<typename T>
    void set_struct_data_callback(const COMM_CHANNEL_INFO& channelinfo, std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
    {
        return m_pGaeactorCommProcessor->set_struct_data_callback<T>(channelinfo, callback);
    }

    template<typename T>
    void set_protobuf_data_callback(const COMM_CHANNEL_INFO& channelinfo, std::function<void (const COMM_CHANNEL_INFO &, const T*)> callback)
    {
        return m_pGaeactorCommProcessor->set_protobuf_data_callback<T>(channelinfo, callback);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

private:
    explicit GaeactorComm(QObject *parent = nullptr);

private:
    GaeactorCommProcessor * m_pGaeactorCommProcessor;

};
}
#endif // GAEACTOR_COMM_INTERFACE_H
