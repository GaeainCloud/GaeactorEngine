#ifndef GAEACTOR_TRANSMIT_PROCESSOR_H
#define GAEACTOR_TRANSMIT_PROCESSOR_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QReadWriteLock>

#include "src/local_topic_data.h"
#include "snowflake.h"

namespace stdutils {
class OriThread;
}

namespace iox {
namespace runtime {
class PoshRuntime;
}

namespace mepoo
{
/// @brief Helper struct to use as default template parameter when no user-header is used
struct NoUserHeader;
}

namespace popo {
class Listener;
class UntypedSubscriber;
#ifdef _MSC_VER
template <typename T, typename H = mepoo::NoUserHeader>
class Publisher;

template <typename T, typename H = mepoo::NoUserHeader>
class Subscriber;
#endif
}
}

#ifndef _MSC_VER
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#endif

#define USING_WAIT_SET

#define USING_WAIT_SET_NUM (3)


#define USING_CHANNEL_NOTIFY_TIMEER

#ifdef USING_CHANNEL_NOTIFY_TIMEER
class QTimer;
#else
namespace stdutils {
class OriThread;
}
#endif

namespace gaeactortransmit
{
class GaeactorTransmitDeployModeBase;
class GaeactorTransmitBase;
class GaeactorTransmitWaitset;
class GaeactorTransmitReceiver;
class GaeactorTransmitLocalSubscriber;
class GaeactorTransmitProcessor : public QObject
{
    Q_OBJECT
public:    
    explicit GaeactorTransmitProcessor(QObject *parent = nullptr);
    virtual ~GaeactorTransmitProcessor();
    //servicename 第二项为空，则不过滤任何instance，接收所有的instance
    void initDeployType(const E_DEPLOYMODE_TYPE& deployType, const std::vector<std::tuple<std::string,std::string>> &servicename);
    
    QString transmit_channel_id() const;
    //为空，则不指定该instance 的接收对象，所有均接收
    void set_transmit_channel_id(const QString &new_transmit_channel_id);

    std::tuple<std::string, CHANNEL_INFO*> requireIndependentPublisherChannel();

    std::tuple<std::string, CHANNEL_INFO*> getReusePublisherChannel();

    void allocShareChannel(UINT32 iCount);
    CHANNEL_INFO* applyforShareChannel();

    bool removePublisherUseItem(const std::tuple<std::string, CHANNEL_INFO*> & itemChannelInfo);
    void setDataCallback(receive_callback func);
    bool transmitData(const CHANNEL_INFO *channelinfo,const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen);

    void* loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen);
    void publish(const CHANNEL_INFO *channelinfo);
    void printMempoolInfo();
private:
    void initLocal();
    void initRemote();
    std::string genereteulidstr();
private:
    static void onSampleReceivedCallback(iox::popo::Subscriber<ChannelObject>* subscriber,GaeactorTransmitProcessor* self);

    static void onSampleCallback(iox::popo::UntypedSubscriber* subscriber,GaeactorTransmitLocalSubscriber* self);

private:
    void dealChannelObject(const ChannelObject & channelobj);
    void createPublisherChannel(std::string &itemNameInfo, CHANNEL_INFO *channelInfo, bool bResue, bool bShared);

    void attachSubscriberEvent(iox::popo::Subscriber<ChannelObject>* pChannelOperateSubscriber);
    void attachUntypedSubscriberEvent(GaeactorTransmitReceiver * pReceiver);

    iox::popo::Listener * getListener();
private slots:
    void timeout_slot();
private:
    E_DEPLOYMODE_TYPE m_deployType;
    std::vector<std::tuple<std::string,std::string>> m_servicenamelist;

    std::shared_ptr<GaeactorTransmitDeployModeBase> m_pGaeactorTransmitDeployModeBase;

    iox::runtime::PoshRuntime* m_pRuntime;

    iox::popo::Publisher<ChannelObject>* m_pChannelOperatePublisher;

    QList<iox::popo::Listener*> m_pSubscriberListenerList;

    iox::popo::Subscriber<ChannelObject>* m_pChannelOperateSubscriber;

    TYPE_ULID m_ulid;


    std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject>> m_receiverlist;

    QReadWriteLock  m_senderlist_mutex;
    std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>> m_senderlist;


    std::vector<GaeactorTransmitBase*> m_shardchannels;

    receive_callback m_preceive_callback;

#ifdef USING_CHANNEL_NOTIFY_TIMEER
    QTimer * m_pChannelNotifyTimer;
#else
    stdutils::OriThread *m_pChannelNotifyTimer;
#endif

private:
    void data_deal_thread_func(void *pParam);
    void notify_channel_deal_thread_func(void *pParam);
private:

    struct threadParam
    {
        int id;
    }m_hDataDealThreadParam[USING_WAIT_SET_NUM];
    stdutils::OriThread* m_hDataDealThread[USING_WAIT_SET_NUM];
    int currentevent;

    Snowflake m_snowflake;

    QString m_transmit_channel_id;
};
}
#endif // GAEACTOR_TRANSMIT_PROCESSOR_H
