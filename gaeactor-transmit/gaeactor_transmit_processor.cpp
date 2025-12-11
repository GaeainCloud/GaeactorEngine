#include "gaeactor_transmit_processor.h"

#include "h3Index.h"
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>
#include <random>
#include "src/OriginalThread.h"
#include "src/gaeactor_transmit_deploymode_base.h"
#include "src/local/gaeactor_transmit_deploymode_local.h"
#include "src/remote/gaeactor_transmit_deploymode_remote.h"


#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/popo/listener.hpp"

#include "src/gaeactor_transmit_receiver_base.h"
#include "src/gaeactor_transmit_sender_base.h"
#include "src/local/gaeactor_transmit_local_publisher.h"
#include "src/local/gaeactor_transmit_local_subscriber.h"

#include "src/OriginalThread.h"

#include "iceoryx_posh/runtime/posh_runtime.hpp"

#ifdef _MSC_VER
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/popo/subscriber.hpp"
#endif
#include "loghelper.h"

#define THREAD_EXIT_MAX_TIME (3000)


#ifdef USING_WAIT_SET
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#define WAITSET_GROUP_TRANSMIT_ID (76)
#define WAITSET_GROUP_CHANNEL_ID (77)


iox::cxx::optional<iox::popo::WaitSet<iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET>> waitsets[USING_WAIT_SET_NUM];

#endif

#include "src/OriginalDateTime.h"
namespace gaeactortransmit
{
GaeactorTransmitProcessor::GaeactorTransmitProcessor(QObject *parent)
    :m_deployType(E_DEPLOYMODE_TYPE_NULL)
    ,m_pGaeactorTransmitDeployModeBase(nullptr)
    ,m_pRuntime(nullptr)
    ,m_pChannelOperatePublisher(nullptr)
    ,m_pChannelOperateSubscriber(nullptr)
    ,m_pChannelNotifyTimer(nullptr)
{
#ifdef USING_CHANNEL_NOTIFY_TIMEER
    m_pChannelNotifyTimer = new QTimer(this);
    connect(m_pChannelNotifyTimer, &QTimer::timeout, this, &GaeactorTransmitProcessor::timeout_slot);

    m_pChannelNotifyTimer->start(300);
#else

#ifdef _MSC_VER
    m_pChannelNotifyTimer = new stdutils::OriThread(std::bind(&GaeactorTransmitProcessor::notify_channel_deal_thread_func,this,std::placeholders::_1), nullptr, THREAD_PRIORITY_NORMAL);
#else
    m_pChannelNotifyTimer = new stdutils::OriThread(std::bind(&GaeactorTransmitProcessor::notify_channel_deal_thread_func,this,std::placeholders::_1), nullptr, 0);
#endif
    m_pChannelNotifyTimer->start();
#endif

    m_snowflake.setHostId(SNOWFLAKE_TRANSMIT_HOST_ID);
    m_snowflake.setWorkerId(SNOWFLAKE_TRANSMIT_WORK_ID);
}

GaeactorTransmitProcessor::~GaeactorTransmitProcessor()
{
#ifdef USING_CHANNEL_NOTIFY_TIMEER
    if(m_pChannelNotifyTimer && m_pChannelNotifyTimer->isActive())
    {
        m_pChannelNotifyTimer->stop();
        m_pChannelNotifyTimer->deleteLater();
    }
#else
    if (m_pChannelNotifyTimer != nullptr)
    {
        delete m_pChannelNotifyTimer;
        m_pChannelNotifyTimer = nullptr;
    }
#endif


#ifdef USING_WAIT_SET
    //清理DataDealThread
    for(int i = 0; i < USING_WAIT_SET_NUM; i++)
    {
        if (m_hDataDealThread[i] != nullptr)
        {
            delete m_hDataDealThread[i];
            m_hDataDealThread[i] = nullptr;
        }
    }
#endif


    {
        QWriteLocker locker(&m_senderlist_mutex);
        auto itor = m_senderlist.begin();
        while(itor != m_senderlist.end())
        {
            const GaeactorTransmitBase * pBase = std::get<0>(itor->second);
            ChannelObject &channelobj = std::get<1>(itor->second);
            if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
            {
                //LOG_PRINT_STR_EX("publish publishers to other service:"+QString::fromStdString(channelobj.m_service)+" instance:"+QString::fromStdString(channelobj.m_instance)+" event:"+QString::fromStdString(channelobj.m_event)+" total count:"+QString::number(m_senderlist.size()))
                channelobj.m_channel_operate = E_CHANNEL_OPERATE_REMOVE;
                m_pChannelOperatePublisher->publishCopyOf(channelobj).or_else([](auto) {
                    std::cerr << "send failed\n";
                });
                //delete pBase;
                itor = m_senderlist.erase(itor);
                continue;
            }
            itor++;
        }
    }

    auto itor2 = m_receiverlist.begin();
    while(itor2 != m_receiverlist.end())
    {
        const GaeactorTransmitBase * pBase = std::get<0>(itor2->second);
        if(pBase)
        {
            //delete pBase;
            itor2 = m_receiverlist.erase(itor2);
            continue;
        }
        itor2++;
    }

//    if(m_pChannelOperatePublisher)
//    {
//        delete m_pChannelOperatePublisher;
//    }
//    if(m_pChannelOperateSubscriber)
//    {
//        delete m_pChannelOperateSubscriber;
//    }
}

void GaeactorTransmitProcessor::initLocal()
{
    m_pGaeactorTransmitDeployModeBase = std::make_shared<GaeactorTransmitDeployModeLocal>(new GaeactorTransmitDeployModeLocal(this));


    iox::RuntimeName_t name(iox::cxx::TruncateToCapacity, genereteulidstr());
    iox::runtime::PoshRuntime& runtime = iox::runtime::PoshRuntime::initRuntime(name);
    m_pRuntime = &runtime;

    m_pChannelOperatePublisher = new iox::popo::Publisher<ChannelObject>({ declare_channel_service, declare_channel_instance, declare_channel_event });
#ifdef USING_WAIT_SET
    for(int i = 0 ; i< USING_WAIT_SET_NUM; i++)
    {
        waitsets[i].emplace();
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorTransmitProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorTransmitProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                     &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();
    }
#endif
    if(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_RECV)
    {
        m_pChannelOperateSubscriber = new iox::popo::Subscriber<ChannelObject>({ declare_channel_service, declare_channel_instance, declare_channel_event });
        attachSubscriberEvent(m_pChannelOperateSubscriber);
    }


    printMempoolInfo();
}

void GaeactorTransmitProcessor::initRemote()
{
    m_pGaeactorTransmitDeployModeBase = std::make_shared<GaeactorTransmitDeployModeRemote>(this);
}

std::string GaeactorTransmitProcessor::genereteulidstr()
{
    std::mt19937_64 rng(stdutils::OriDateTime::getCurrentUTCTimeStampMSecs());    // 种子，可以选择时间作为seed
    std::uniform_int_distribution<uint64_t> distribution(1, 0xFFFFFFFF);    // 设置范围
    m_ulid = distribution(rng);
    std::string sensorulidstr = QString::number(m_ulid).toStdString();
    return sensorulidstr;
}

void GaeactorTransmitProcessor::onSampleReceivedCallback(iox::popo::Subscriber<ChannelObject> *subscriber, GaeactorTransmitProcessor *self)
{
    subscriber->take().and_then([subscriber, self](auto& sample) {
        auto instanceString = subscriber->getServiceDescription().getInstanceIDString();

        // store the sample in the corresponding cache
        if (instanceString == declare_channel_instance)
        {
            auto  & channelobj = *sample;
            self->dealChannelObject(channelobj);
        }
    });
}

void GaeactorTransmitProcessor::onSampleCallback(iox::popo::UntypedSubscriber *subscriber, GaeactorTransmitLocalSubscriber *self)
{
    self->dealSubscriberInfo(subscriber);
}

void GaeactorTransmitProcessor::dealChannelObject(const ChannelObject &channelobj)
{
    if(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_RECV &&
            !(channelobj.m_channelsrc_ulid == m_ulid))
    {
        //此项相等，不指定该instance 的接收对象，所有均接收
        if(channelobj.m_instance != transmit_channel_instance)
        {
            if(!m_servicenamelist.empty())
            {
                auto itor = std::find_if(m_servicenamelist.begin(),
                                         m_servicenamelist.end(),[&](const std::vector<std::tuple<std::string,std::string>>::value_type& vt){
                                             if(channelobj.m_service == std::get<0>(vt)+"_"+transmit_channel_service)
                                             {
                                                 //此项为空 不过滤任何instance，接收所有的instance
                                                 if(std::get<1>(vt).empty())
                                                 {
                                                     return true;
                                                 }
                                                 //接收指定的instance
                                                 else if(channelobj.m_instance == std::get<1>(vt)+"_"+transmit_channel_instance)
                                                 {
                                                     return true;
                                                 }
                                                 else
                                                 {
                                                     return false;
                                                 }
                                             }
                                             else
                                             {
                                                 return false;
                                             }
                                         });
                if(itor == m_servicenamelist.end())
                {
                    return;
                }
            }
        }


        //std::cout<<"subscriber receive--> service:"<<channelInfo.m_service<<" instance:"<<channelInfo.m_instance<<" event:"<<channelInfo.m_event<<std::endl;


        switch (channelobj.m_channel_operate) {
        case E_CHANNEL_OPERATE_ADD:
        {
            auto itor = std::find_if(m_receiverlist.begin(),
                                     m_receiverlist.end(),[&channelobj](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject>>::value_type& vt){
                                         return vt.first->m_service == channelobj.m_service &&\
                                          vt.first->m_instance == channelobj.m_instance &&\
                                             vt.first->m_event == channelobj.m_event ;
                                     });
            if(itor == m_receiverlist.end())
            {
                GaeactorTransmitReceiver * pReceiver = m_pGaeactorTransmitDeployModeBase.get()->createReceiver();
                pReceiver->setDataCallback(m_preceive_callback);
                pReceiver->setPRuntime(m_pRuntime);

                CHANNEL_INFO *channelInfo = new CHANNEL_INFO;
                channelInfo->m_service = channelobj.m_service;
                channelInfo->m_instance = channelobj.m_instance;
                channelInfo->m_event = channelobj.m_event;


                pReceiver->init(channelInfo);
                m_receiverlist.insert(std::make_pair(channelInfo, std::make_tuple(pReceiver, channelobj)));
                attachUntypedSubscriberEvent(pReceiver);

                LOG_PRINT_STR_EX("append subscribers service:"+QString::fromStdString(channelInfo->m_service)+" instance:"+QString::fromStdString(channelInfo->m_instance)+" event:"+QString::fromStdString(channelInfo->m_event)+" total count:"+QString::number(m_receiverlist.size()))
            }
        }
        break;
        case E_CHANNEL_OPERATE_REMOVE:
        {
            auto itor = std::find_if(m_receiverlist.begin(),
                                     m_receiverlist.end(),[&channelobj](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject>>::value_type& vt){
                                         return vt.first->m_service == channelobj.m_service &&\
                                         vt.first->m_instance == channelobj.m_instance &&\
                                         vt.first->m_event == channelobj.m_event ;
                                     });
            if(itor != m_receiverlist.end())                
            {
                GaeactorTransmitBase * pReceiver = std::get<0>(itor->second);
                if(pReceiver)
                {
                    GaeactorTransmitLocalSubscriber *pGaeactorTransmitLocalSubscriber = dynamic_cast<GaeactorTransmitLocalSubscriber *>(pReceiver);
#ifdef USING_WAIT_SET
                    for(int i = 0 ; i< USING_WAIT_SET_NUM; i++)
                    {
                        waitsets[i]->detachState(*(pGaeactorTransmitLocalSubscriber->pSubscriber()),iox::popo::SubscriberState::HAS_DATA);
                    }
#else
                    pGaeactorTransmitLocalSubscriber->detachListenerEvent();
#endif                    
                    CHANNEL_INFO *channelInfo = pReceiver->channelinfo();
                    LOG_PRINT_STR_EX("remove subscribers service:"+QString::fromStdString(channelInfo->m_service)+" instance:"+QString::fromStdString(channelInfo->m_instance)+" event:"+QString::fromStdString(channelInfo->m_event)+" total count:"+QString::number(m_receiverlist.size()))
                    delete pReceiver;
                }
                m_receiverlist.erase(itor);
            }
        }break;
        default:
            break;
        }
    }
}

void GaeactorTransmitProcessor::createPublisherChannel(std::string &itemNameInfo,CHANNEL_INFO *channelInfo, bool bResue,bool bShared)
{
    QWriteLocker locker(&m_senderlist_mutex);
    if(m_senderlist.size() <  MAX_PUBLISHER_NUM)
    {
        ChannelObject channelobj;
        memset(&channelobj, 0, sizeof(ChannelObject));
        channelobj.m_channelsrc_ulid = m_ulid;

        QString appname = QCoreApplication::applicationName();
        std::string servicename = appname.toStdString();
        servicename+="_"+transmit_channel_service;
        std::string instancename = m_transmit_channel_id.toStdString();
        if(instancename.empty())
        {
            instancename+=transmit_channel_instance;
        }
        else
        {
            instancename+="_"+transmit_channel_instance;
        }
        memcpy(channelobj.m_service, servicename.c_str(), servicename.size());
        memcpy(channelobj.m_instance, instancename.c_str(), instancename.size());

        TYPE_ULID _ulid = m_snowflake.GetId();
        std::string str = QString::number(_ulid).toStdString();
        memcpy(channelobj.m_event, str.c_str(), str.size());

        channelobj.m_channel_operate = E_CHANNEL_OPERATE_ADD;
        channelobj.m_channelsrc_ulid = m_ulid;

        channelInfo->m_service = channelobj.m_service;
        channelInfo->m_instance = channelobj.m_instance;
        channelInfo->m_event = channelobj.m_event;

        m_pChannelOperatePublisher->publishCopyOf(channelobj).or_else([](auto) {
            std::cerr << "send failed\n";
        });

        auto itor = std::find_if(m_senderlist.begin(),
                                 m_senderlist.end(),[&channelInfo](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>>::value_type& vt){
                                     return vt.first->m_service == channelInfo->m_service &&\
                                                                                                vt.first->m_instance == channelInfo->m_instance &&\
                                                  vt.first->m_event == channelInfo->m_event ;
                                 });
        if(itor == m_senderlist.end())
        {
            GaeactorTransmitSender * pSender = m_pGaeactorTransmitDeployModeBase.get()->createSender();
            GaeactorTransmitLocalPublisher * ptargetPublisher = dynamic_cast<GaeactorTransmitLocalPublisher *>(pSender);
            pSender->init(channelInfo);
            itemNameInfo =  ptargetPublisher->getEventItemName(m_snowflake.GetId());
            pSender->setPRuntime(m_pRuntime);
            ptargetPublisher->appendPublisherItem(itemNameInfo);
            m_senderlist.insert(std::make_pair(channelInfo, std::make_tuple(pSender, channelobj, bResue)));


            if(bShared)
            {
                LOG_PRINT_STR_EX("append shared publishers service:"+QString::fromStdString(channelInfo->m_service)+" instance:"+QString::fromStdString(channelInfo->m_instance)+" event:"+QString::fromStdString(channelInfo->m_event)+" total count:"+QString::number(m_senderlist.size()))
                m_shardchannels.push_back(ptargetPublisher);
            }
            else
            {
                if(bResue)
                {
                    LOG_PRINT_STR_EX("append reuse publishers service:"+QString::fromStdString(channelInfo->m_service)+" instance:"+QString::fromStdString(channelInfo->m_instance)+" event:"+QString::fromStdString(channelInfo->m_event)+" total count:"+QString::number(m_senderlist.size()))
                }
                else
                {
                    LOG_PRINT_STR_EX("append Independent publishers service:"+QString::fromStdString(channelInfo->m_service)+" instance:"+QString::fromStdString(channelInfo->m_instance)+" event:"+QString::fromStdString(channelInfo->m_event)+" total count:"+QString::number(m_senderlist.size()))
                }
            }
        }
    }
}

void GaeactorTransmitProcessor::attachSubscriberEvent(iox::popo::Subscriber<ChannelObject> *pChannelOperateSubscriber)
{
#ifdef USING_WAIT_SET

    auto getWaitSet = [&]()->iox::cxx::optional<iox::popo::WaitSet<>> *{

        for(int i = 0; i < USING_WAIT_SET_NUM; i++)
        {
            if(waitsets[i]->size() < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET/2)
            {
                return &waitsets[i];
            }
        }
        return nullptr;
    };

    iox::cxx::optional<iox::popo::WaitSet<>> * ws = getWaitSet();
    if(ws)
    {
        ws->value().attachState(*pChannelOperateSubscriber, iox::popo::SubscriberState::HAS_DATA, WAITSET_GROUP_CHANNEL_ID)
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }

#else
    iox::popo::Listener * pListener = getListener();

    if(pListener)
    {
        pListener->attachEvent(*pChannelOperateSubscriber,
                               iox::popo::SubscriberEvent::DATA_RECEIVED,
                               iox::popo::createNotificationCallback(onSampleReceivedCallback,*this))
            .or_else([](auto) {
                std::cerr << "unable to attach subscriberLeft" << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }
#endif
}


void GaeactorTransmitProcessor::attachUntypedSubscriberEvent(GaeactorTransmitReceiver *pReceiver)
{
#ifdef USING_WAIT_SET
    GaeactorTransmitLocalSubscriber *pGaeactorTransmitLocalSubscriber = dynamic_cast<GaeactorTransmitLocalSubscriber *>(pReceiver);
    auto getWaitSet = [&]()->iox::cxx::optional<iox::popo::WaitSet<>> *{
        for(int i = 0; i < USING_WAIT_SET_NUM; i++)
        {
            if(waitsets[i]->size() < iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET/2)
            {
                return &waitsets[i];
            }
        }
        return nullptr;
    };

    iox::cxx::optional<iox::popo::WaitSet<>> * ws = getWaitSet();
    if(ws)
    {
        ws->value().attachState(*pGaeactorTransmitLocalSubscriber->pSubscriber(), iox::popo::SubscriberState::HAS_DATA, WAITSET_GROUP_TRANSMIT_ID)
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }
#else    
    iox::popo::Listener * pListener = getListener();

    if(pListener)
    {
        GaeactorTransmitLocalSubscriber *pGaeactorTransmitLocalSubscriber = dynamic_cast<GaeactorTransmitLocalSubscriber *>(pReceiver);
        pGaeactorTransmitLocalSubscriber->attachListenerEvent(pListener);
    }

#endif
}


iox::popo::Listener *GaeactorTransmitProcessor::getListener()
{
    iox::popo::Listener * pListener = nullptr;
    if(!m_pSubscriberListenerList.empty())
    {
        auto itor = m_pSubscriberListenerList.cbegin();
        while(itor != m_pSubscriberListenerList.cend())
        {
            iox::popo::Listener * pListenerTmp = *itor;
            if(pListenerTmp && pListenerTmp->size() < iox::MAX_NUMBER_OF_EVENTS_PER_LISTENER)
            {
                pListener = pListenerTmp;
                break;
            }
            itor++;
        }
        if(nullptr == pListener)
        {
            pListener = new iox::popo::Listener();
            m_pSubscriberListenerList.push_back(pListener);
        }
    }
    else
    {
        pListener = new iox::popo::Listener();
        m_pSubscriberListenerList.push_back(pListener);
    }
    return pListener;
};

void GaeactorTransmitProcessor::printMempoolInfo()
{
    const iox::rp::RelativePointer<iox::mepoo::MemoryManager> m_memoryMgr = m_pRuntime->getMiddlewarePublisher({ declare_channel_service, declare_channel_instance, declare_channel_event })->m_chunkSenderData.m_memoryMgr;
    int mempoolnum = m_memoryMgr->getNumberOfMemPools();

    //    m_mempoolconf.addMemPool({ CHUNK_SIZE, NUM_CHUNKS_IN_POOL });
    //    m_memoryMgr->configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);

    for (int index = 0; index < mempoolnum; index++)
    {
        auto memPool = m_memoryMgr->getMemPoolInfo(index);
        std::cout << "  MemPool [ ChunkSize = " << memPool.m_chunkSize
                  << ", ChunkPayloadSize = " << memPool.m_chunkSize - sizeof(iox::mepoo::ChunkHeader)
                  << ", ChunkUsed = " << memPool.m_usedChunks
                  << ", ChunkCount = " << memPool.m_numChunks << " ]"<<std::endl;
    }
}

void GaeactorTransmitProcessor::timeout_slot()
{
    if(!(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_SEND))
    {
        return;
    }
    QReadLocker locker(&m_senderlist_mutex);

    auto itor = m_senderlist.cbegin();
    while(itor != m_senderlist.cend())
    {
        const GaeactorTransmitBase * pBase = std::get<0>(itor->second);
        const ChannelObject &channelobj = std::get<1>(itor->second);
        if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
        {
            m_pChannelOperatePublisher->publishCopyOf(channelobj).or_else([](auto) {
                std::cerr << "send failed\n";
            });
        }
        itor++;
    }
}

void GaeactorTransmitProcessor::data_deal_thread_func(void *pParam)
{
    if (pParam == nullptr)
    {
        return;
    }

    threadParam *pObject = reinterpret_cast<threadParam*>(pParam);
    if(pObject)
    {
#ifdef USING_WAIT_SET
        auto dealFunc =[&](iox::cxx::optional<iox::popo::WaitSet<iox::MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET>> &ws){

            auto notificationVector = ws->wait();
            for (auto& notification : notificationVector)
            {
                if(notification->getNotificationId() == WAITSET_GROUP_CHANNEL_ID)
                {
                    if (notification->doesOriginateFrom(m_pChannelOperateSubscriber))
                    {
                        onSampleReceivedCallback(m_pChannelOperateSubscriber,this);
                    }
                }
                else if (notification->getNotificationId() == WAITSET_GROUP_TRANSMIT_ID)
                {
                    // Consume a sample
                    auto subscriber = notification->getOrigin<iox::popo::UntypedSubscriber>();

                    auto itor = std::find_if(m_receiverlist.begin(),
                                             m_receiverlist.end(),
                                             [&](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject>>::value_type& vl)
                                             {
                                                 GaeactorTransmitBase * pReceiver = std::get<0>(vl.second);
                                                 if(pReceiver)
                                                 {
                                                     GaeactorTransmitLocalSubscriber *pGaeactorTransmitLocalSubscriber = dynamic_cast<GaeactorTransmitLocalSubscriber *>(pReceiver);
                                                     return pGaeactorTransmitLocalSubscriber->pSubscriber() == subscriber;
                                                 }
                                             });
                    if(itor != m_receiverlist.end())
                    {
                        GaeactorTransmitBase * pReceiver = std::get<0>(itor->second);
                        if(pReceiver)
                        {
                            GaeactorTransmitLocalSubscriber *pGaeactorTransmitLocalSubscriber = dynamic_cast<GaeactorTransmitLocalSubscriber *>(pReceiver);
                            pGaeactorTransmitLocalSubscriber->dealSubscriberInfo(subscriber);
                        }
                    }
                }
            }
        };
//        switch (pObject->id) {
//        case 0:
//            dealFunc(waitsets[pObject->id]);
//            break;
//        case 1:
//            dealFunc(waitsets[pObject->id]);
//            break;
//        case 2:
//            dealFunc(waitsets[pObject->id]);
//            break;
//        default:
//            break;
//        }

        dealFunc(waitsets[pObject->id]);
#endif
    }
    return;
}

void GaeactorTransmitProcessor::notify_channel_deal_thread_func(void *pParam)
{
    timeout_slot();
    stdutils::OriDateTime::sleep(300);
}

std::tuple<std::string, CHANNEL_INFO*> GaeactorTransmitProcessor::getReusePublisherChannel()
{
    if(!(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_SEND))
    {
        return std::tuple<std::string, CHANNEL_INFO*>("",nullptr);
    }
    auto getExistReusePublisherChannel = [&]()->GaeactorTransmitLocalPublisher *{
        GaeactorTransmitLocalPublisher * pretPublisher = nullptr;
        if(m_senderlist.empty())
        {
            return pretPublisher;
        }

        auto itor = m_senderlist.cbegin();
        while(itor != m_senderlist.cend())
        {
            const GaeactorTransmitBase * pBase = std::get<0>(itor->second);
            const bool & bReuse = std::get<0>(itor->second);
            if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER && bReuse)
            {
                GaeactorTransmitLocalPublisher * pItemPublisher = dynamic_cast<GaeactorTransmitLocalPublisher *>(const_cast<GaeactorTransmitBase *>(pBase));
                if(pItemPublisher && pItemPublisher->getPublisherCount() < PRE_PUBLISHER_USE_ITEM_NUM)
                {
                    pretPublisher = pItemPublisher;
                    break;
                }
            }
            itor++;
        }
        return pretPublisher;
    };
    std::tuple<std::string, CHANNEL_INFO*> itemChannelInfo;
    if(m_senderlist.size() <=  MAX_PUBLISHER_NUM)
    {
        std::string &itemNameInfo = std::get<0>(itemChannelInfo);
        CHANNEL_INFO* &channelInfo = std::get<1>(itemChannelInfo);

        GaeactorTransmitLocalPublisher * ptargetPublisher = getExistReusePublisherChannel();
        if(ptargetPublisher)
        {
            itemNameInfo =  ptargetPublisher->getEventItemName(m_snowflake.GetId());
            ptargetPublisher->appendPublisherItem(itemNameInfo);
            channelInfo = ptargetPublisher->channelinfo();
            std::cout<<"reusing publishers count:"<<ptargetPublisher->getPublisherCount()<<std::endl;
        }
        else
        {
            channelInfo = new CHANNEL_INFO();
            createPublisherChannel(itemNameInfo, channelInfo, true,false);
        }
    }
    else
    {
        std::cerr<<"publishers count over size :"<<m_senderlist.size()<<std::endl;
    }
    return itemChannelInfo;
}

void GaeactorTransmitProcessor::allocShareChannel(UINT32 iCount)
{
    if(!(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_SEND))
    {
        return;
    }
    for (int var = 0; var < iCount; ++var) {
        std::tuple<std::string, CHANNEL_INFO*> itemChannelInfo;
        if(m_senderlist.size() <  MAX_PUBLISHER_NUM)
        {
            std::string &itemNameInfo = std::get<0>(itemChannelInfo);
            CHANNEL_INFO* &channelInfo = std::get<1>(itemChannelInfo);

            channelInfo = new CHANNEL_INFO();
            createPublisherChannel(itemNameInfo,channelInfo, false,true);

            stdutils::OriDateTime::sleep(1);

            std::cout << "allocShareChannel "<<channelInfo->m_event<<std::endl;
        }
        else
        {
            std::cerr<<"publishers count over size :"<<m_senderlist.size()<<std::endl;
        }
    };

    std::cout << "allocShareChannel succeed "<<std::endl;
}

CHANNEL_INFO* GaeactorTransmitProcessor::applyforShareChannel()
{
    CHANNEL_INFO* ret = nullptr;
    for(int index = 0; index <m_shardchannels.size(); index++)
    {
        GaeactorTransmitLocalPublisher * ptargetPublisher = dynamic_cast<GaeactorTransmitLocalPublisher *>(m_shardchannels.at(index));
#ifdef USING_SINGLE_CHANNEL
        if(ptargetPublisher)
#else
        if(ptargetPublisher && !ptargetPublisher->isPushing() && !ptargetPublisher->applying())
#endif
        {
            ret = ptargetPublisher->channelinfo();
            break;
        }
    }
    return ret;
}

bool GaeactorTransmitProcessor::removePublisherUseItem(const std::tuple<std::string, CHANNEL_INFO*> &itemChannelInfo)
{
    bool bRemove = false;
    CHANNEL_INFO* pchanelinfo = std::get<1>(itemChannelInfo);
    if(nullptr == pchanelinfo)
    {
        return bRemove;
    }
    auto itor = std::find_if(m_senderlist.begin(),
                             m_senderlist.end(),[&pchanelinfo](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>>::value_type& vt){
                                 return vt.first->m_service == pchanelinfo->m_service &&\
                                        vt.first->m_instance == pchanelinfo->m_instance &&\
                                        vt.first->m_event == pchanelinfo->m_event ;
                             });
    if(itor != m_senderlist.end())
    {
        const GaeactorTransmitBase * pBase = std::get<0>(itor->second);
        if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
        {
            GaeactorTransmitLocalPublisher * pItemPublisher = dynamic_cast<GaeactorTransmitLocalPublisher *>(const_cast<GaeactorTransmitBase *>(pBase));
            if(pItemPublisher && pItemPublisher->getPublisherCount() < PRE_PUBLISHER_USE_ITEM_NUM)
            {
                bRemove = pItemPublisher->removePbulisherItem(std::get<0>(itemChannelInfo));
            }
        }
    }
    return bRemove;
}

void GaeactorTransmitProcessor::setDataCallback(receive_callback func)
{
    m_preceive_callback = std::move(func);
}

bool GaeactorTransmitProcessor::transmitData(const CHANNEL_INFO *channelinfo, const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen)
{
    if(nullptr == channelinfo)
    {
        return false;
    }
    auto itor = std::find_if(m_senderlist.begin(),
                             m_senderlist.end(),[&channelinfo](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>>::value_type& vt){
                                 return vt.first->m_service == channelinfo->m_service &&\
                                                                                            vt.first->m_instance == channelinfo->m_instance &&\
                                              vt.first->m_event == channelinfo->m_event ;
                             });
    if(itor != m_senderlist.end())
    {
        GaeactorTransmitBase * pBase = std::get<0>(itor->second);
        if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
        {
            GaeactorTransmitSender * pSender = dynamic_cast<GaeactorTransmitSender *>(pBase);
            if(pSender)
            {
                pSender->transmitData(channelTransmitDataType, pData,iLen);
            }
        }
    }
    return false;
}

void *GaeactorTransmitProcessor::loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen)
{
    if(nullptr == channelinfo)
    {
        return nullptr;
    }
    void * _usrpayload = nullptr;
    auto itor = std::find_if(m_senderlist.begin(),
                             m_senderlist.end(),[&channelinfo](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>>::value_type& vt){
                                 return vt.first->m_service == channelinfo->m_service &&\
                                                                                            vt.first->m_instance == channelinfo->m_instance &&\
                                              vt.first->m_event == channelinfo->m_event ;
                             });
    if(itor != m_senderlist.end())
    {
        GaeactorTransmitBase * pBase = std::get<0>(itor->second);
        if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
        {
            GaeactorTransmitSender * pSender = dynamic_cast<GaeactorTransmitSender *>(pBase);
            if(pSender)
            {
                _usrpayload = pSender->loanTransmitBuffer(iLen);
            }
        }
    }
    return _usrpayload;
}

void GaeactorTransmitProcessor::publish(const CHANNEL_INFO *channelinfo)
{
    if(nullptr == channelinfo)
    {
        return;
    }
    auto itor = std::find_if(m_senderlist.begin(),
                             m_senderlist.end(),[&channelinfo](const std::unordered_map<CHANNEL_INFO*, std::tuple<GaeactorTransmitBase*, ChannelObject, bool>>::value_type& vt){
                                 return vt.first->m_service == channelinfo->m_service &&\
                                                                                            vt.first->m_instance == channelinfo->m_instance &&\
                                              vt.first->m_event == channelinfo->m_event ;
                             });
    if(itor != m_senderlist.end())
    {
        GaeactorTransmitBase * pBase = std::get<0>(itor->second);
        if(pBase && pBase->transmitType() == E_TRANSMIT_TYPE_SENDER)
        {
            GaeactorTransmitSender * pSender = dynamic_cast<GaeactorTransmitSender *>(pBase);
            if(pSender)
            {
                pSender->publish();
            }
        }
    }
}

void GaeactorTransmitProcessor::initDeployType(const E_DEPLOYMODE_TYPE &deployType, const std::vector<std::tuple<std::string,std::string>> &servicename)
{
    m_deployType = deployType;
    m_servicenamelist = servicename;
    switch (deployType)
    {
    case E_DEPLOYMODE_TYPE_LOCAL_SEND:
    case E_DEPLOYMODE_TYPE_LOCAL_RECV:
    case E_DEPLOYMODE_TYPE_LOCAL_RECV_SEND:
    {
        initLocal();
    }break;
    case E_DEPLOYMODE_TYPE_REMOTE:
    {
        initRemote();
    }break;
    default:
        break;
    }
}

QString GaeactorTransmitProcessor::transmit_channel_id() const
{
    return m_transmit_channel_id;
}

void GaeactorTransmitProcessor::set_transmit_channel_id(const QString &new_transmit_channel_id)
{
    m_transmit_channel_id = new_transmit_channel_id;
}

std::tuple<std::string, CHANNEL_INFO*> GaeactorTransmitProcessor::requireIndependentPublisherChannel()
{
    if(!(m_deployType & E_DEPLOYMODE_TYPE_LOCAL_SEND))
    {
        return std::tuple<std::string, CHANNEL_INFO*>("",nullptr);
    }
    std::tuple<std::string, CHANNEL_INFO*> itemChannelInfo;
    if(m_senderlist.size() <  MAX_PUBLISHER_NUM)
    {
        std::string &itemNameInfo = std::get<0>(itemChannelInfo);
        CHANNEL_INFO *&channelInfo = std::get<1>(itemChannelInfo);

        channelInfo = new CHANNEL_INFO();
        createPublisherChannel(itemNameInfo,channelInfo, false,false);
    }
    else
    {
        std::cerr<<"publishers count over size :"<<m_senderlist.size()<<std::endl;
    }
    return itemChannelInfo;
}
}
