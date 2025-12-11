#include "gaeactortransmitmanager.h"
#include <QTimer>
#include "gaeactor_agent_cores_interface.h"
#include "gaeactor_agent_sensors_interface.h"
#include "gaeactor_interactions_interface.h"
#include "gaeactor_auditions_interface.h"
#include "gaeactor_event_engine_interface.h"
#include <iostream>

#include "src/OriginalDateTime.h"
#include "loghelper.h"
#include "easy/profiler.h"
#include "runningmodeconfig.h"


#include "gaeactor_comm_interface.h"
#include "./proto/protoc/AgentPositionInfo.pb.h"

#include "gaeactor_transmit_interface.h"

#define THREAD_EXIT_MAX_TIME (3000)

#define DEAL_DIRECT

#define MAX_BUFFER_LEN (10*1024*1024*10)

//iceoryx 单次传输最大字节数，视单次产生的时间量而定 50904个事件 占用字节数为：4683168   4573.41kb
#if 1
//采用8m最大传输量 ，能分配的 内存池块数 270000个，
#define MAX_PRE_ICEORYX_SNED_BUFFER ((8*1024-1)*1024)
//针对事件的数据的缓存，需要16ms 的线程休眠，已试缓存数据积累至8M，再将将缓存派发一次，减缓iceoryx 数据订阅的频率
#define BUFFER_THREAD_TRANSFER_INTERVAL (5)
#else
//采用4m最大传输量 ，能分配的 内存池块数 550000个
#define MAX_PRE_ICEORYX_SNED_BUFFER ((4*1024-1)*1024)
//针对事件的数据的缓存，需要5ms 的线程休眠将缓存派发一次，否则缓存超过4m将多次派发
#define BUFFER_THREAD_TRANSFER_INTERVAL (5)
#endif

#define MAX_SEND_BUFFER_SIZE (MAX_PRE_ICEORYX_SNED_BUFFER)

#define DATA_DEAL_MODE_QUEUE

//#define DATA_DEAL_INTERVAL (60)
#define DATA_DEAL_INTERVAL (20)

//#define SEND_ADD_REMOVE_EVENTS_3

GaeactorTransmitManager &GaeactorTransmitManager::getInstance(QString transmit_channel_title)
{
    static GaeactorTransmitManager gaeactormanager(transmit_channel_title);
    return gaeactormanager;
}

GaeactorTransmitManager::~GaeactorTransmitManager()
{
#ifndef DATA_DEAL_MODE_QUEUE
    for(int i = 0; i < USING_THREAD_NUM; i++)
    {
        if (m_hDataDealThread[i] != nullptr)
        {
            delete m_hDataDealThread[i];
            m_hDataDealThread[i] = nullptr;
        }
    }
#endif

    if(m_pGaeactorAgentCores)
    {
        m_pGaeactorAgentCores->deleteLater();
    }

    if(m_pGaeactorAgentSensors)
    {
        m_pGaeactorAgentSensors->deleteLater();
    }

    if(m_pPosDataCircularBufferThread)
    {
        delete m_pPosDataCircularBufferThread;
    }

    if(m_pWaveDataCircularBufferThread)
    {
        delete m_pWaveDataCircularBufferThread;
    }


    if(m_pInteractionsDataCircularBufferThread)
    {
        delete m_pInteractionsDataCircularBufferThread;
    }


    if(m_pEventDataCircularBufferThread)
    {
        delete m_pEventDataCircularBufferThread;
    }

    if(m_pSensorUpdateDataCircularBufferThread)
    {
        delete m_pSensorUpdateDataCircularBufferThread;
    }
}


void GaeactorTransmitManager::displayPosCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const transdata_entityposinfo& posinfo, E_DISPLAY_MODE eDdisplayMode)
{
    if(runningmode::RunningModeConfig::getInstance().get_USING_TRANS_MODE_POSDATA())
    {
        EASY_FUNCTION(profiler::colors::Navy)
        transentityhexidxpostdata positiondata;
        memset(&positiondata,0,sizeof(transentityhexidxpostdata));
        positiondata.uildsrc = uildsrc;
        positiondata.uilddst = uilddst;
        positiondata.eDdisplayMode = eDdisplayMode;
        positiondata.entityinfo = posinfo;

#if 0

        this->trans_type_receive_callback(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY, (const BYTE *)(&data), sizeof(transentityhexidxpostdata));

        //    if(m_pPosDataCircularBufferThread)
        //    {
        //        m_pPosDataCircularBufferThread->pushbackData( (const BYTE *)&data, sizeof(transentityhexidxpostdata),0);
        //        m_pPosDataCircularBufferThread->wake();
        //    }
#else

        ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentPositionInfo_array;

        *_AgentPositionInfo_array.add_transentityhexidxpostdata() = gaeactortransmit::GaeactorTransmit::SerializePositionData(positiondata);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        E_CHANNEL_TRANSMITDATA_TYPE datatype = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY;
        uint32_t irecvlen = _AgentPositionInfo_array.ByteSizeLong();
        std::vector<BYTE> buffer;
        buffer.resize(sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t) + irecvlen);
        memcpy(buffer.data(), &datatype, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(buffer.data() + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &irecvlen, sizeof(uint32_t));
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        BYTE * data = (BYTE *)(buffer.data()+sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t));
        _AgentPositionInfo_array.SerializeToArray(data, irecvlen);

        size_t userPayloadSize = buffer.size();
        void * usrpayload = (void *)buffer.data();
        trans_receive_callback(usrpayload, userPayloadSize);
#endif
    }
}

void GaeactorTransmitManager::displayHexidxCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode)
{
    if(runningmode::RunningModeConfig::getInstance().get_USING_TRANS_MODE_SENSORDATA())
    {
        dealHexidxCallback(uildsrc, uilddst, hexidxslist,polygonlist,sensorinfo, eDdisplayMode);
    }
}

void GaeactorTransmitManager::displayEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE&echowaveinfo, const HEXIDX_HGT_ARRAY &hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave)
{
    dealEchoWaveHexidxCallback(uildval, echowaveinfo, hexidxslist, geolatlnglist, bEchoWave);
}

void GaeactorTransmitManager::displayInteractionsHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO &sensorinfo, E_DISPLAY_MODE eDdisplayMode)
{
    dealHexidxCallback(uildsrc, uilddst, hexidxslist, polygonlist,sensorinfo, eDdisplayMode);
}
void GaeactorTransmitManager::displayInteractionsEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist,const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave)
{
   dealEchoWaveHexidxCallback(uildval, echowaveinfo, hexidxslist, geolatlnglist, bEchoWave);
}

void GaeactorTransmitManager::displayInteractionsListEchoWaveHexidxCallback(const std::list<std::tuple<TYPE_ULID, EVENT_TUPLE, HEXIDX_HGT_ARRAY, QVector<LAT_LNG>, bool> > &result)
{
   for(auto itor = result.begin(); itor != result.end(); itor++)
   {
        dealEchoWaveHexidxCallback(std::get<0>(*itor),\
                                   std::get<1>(*itor),\
                                   std::get<2>(*itor),\
                                   std::get<3>(*itor),\
                                   std::get<4>(*itor));
   }
   //       if(m_pInteractionsDataCircularBufferThread)
   //       {
   //           m_pInteractionsDataCircularBufferThread->popfrontDataToDeal();
   //       }
}

void GaeactorTransmitManager::displayAuditionsHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST &polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode)
{
    dealHexidxCallback(uildsrc, uilddst, hexidxslist, polygonlist,sensorinfo, eDdisplayMode);
}
void GaeactorTransmitManager::displayAuditionsEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist,const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave)
{
    dealEchoWaveHexidxCallback(uildval, echowaveinfo, hexidxslist, geolatlnglist, bEchoWave);
}

void GaeactorTransmitManager::dealEventUpdateCallback(IDENTIFI_EVENT_INFO &eventlist)
{
    transmitinentifieventinfo(eventlist);
}

void GaeactorTransmitManager::dealSensorUpdateCallback(const TYPE_ULID &ulidsrc, E_EVENT_MODE type)
{
#if 0
    size_t isize = sizeof(trans_sensor_update_info);

    std::tuple<std::string, CHANNEL_INFO> publisherchannel = std::make_tuple("",m_pGaeactorTransmit->applyforShareChannel());
    void * usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(std::get<1>(publisherchannel), isize);

    if(usrpayload)
    {
       E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE;
       BYTE *pDstData = static_cast<BYTE*>(usrpayload);
       memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
       memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
       trans_sensor_update_info *pData = reinterpret_cast<trans_sensor_update_info *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
       memset(pData,0,isize);

       pData->src = ulidsrc;
       pData->mode = type;

       static int32_t cc = 0;
       cc++;
       std::cout << "sensor count " << cc << std::endl;

       m_pGaeactorTransmit->publish(std::get<1>(publisherchannel));
    }
#else
    //EASY_FUNCTION(profiler::colors::Navy)
    trans_sensor_update_info data;
    memset(&data,0,sizeof(trans_sensor_update_info));
    data.src = ulidsrc;
    data.mode = type;
    if(m_pSensorUpdateDataCircularBufferThread)
    {
       m_pSensorUpdateDataCircularBufferThread->pushbackData( (const BYTE *)&data, sizeof(trans_sensor_update_info),0);
       m_pSensorUpdateDataCircularBufferThread->wake();
    }
#endif
}

void GaeactorTransmitManager::transmitinentifieventinfo(IDENTIFI_EVENT_INFO &eventinfo)
{
    const EVENTS_HASHMAP& addeventinfolist = std::get<0>(eventinfo);
    const EVENTS_HASHMAP& removeeventinfolist = std::get<1>(eventinfo);
    const EVENTS_HASHMAP& updateeventinfolist = std::get<2>(eventinfo);
    EASY_FUNCTION(profiler::colors::Navy)
    auto pushEventInfoToCacheBuf=[&](EVENTS_HASHMAP& eventinfolisttmp,E_EVENT_MODE eventType)
    {
        auto eventitor = eventinfolisttmp.begin();
        while(eventitor != eventinfolisttmp.end())
        {
            transeventlistdatasimple eventdata;
            eventdata.eventType = eventType;
            eventdata.event.eventType = eventType;
            eventdata.event.eventifo = eventitor->second;
            if(m_pEventDataCircularBufferThread)
            {                
//                m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                switch (eventType) {
                case E_EVENT_MODE_ADD:
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //add event send 3 times
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#ifdef SEND_ADD_REMOVE_EVENTS
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#endif
                }break;
                case E_EVENT_MODE_UPDATE:
                {
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                }break;
                case E_EVENT_MODE_REMOVE:
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //remove event send 3 times
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#ifdef SEND_ADD_REMOVE_EVENTS
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#endif
                }break;
                default:
                    break;
                }
            }
            eventitor++;
        }
    };

#define SEND_EVENT_SINGLE
    auto pushEventInfoNow=[&](const EVENTS_HASHMAP& eventinfolisttmp,E_EVENT_MODE eventType)
    {
        auto eventitor = eventinfolisttmp.begin();
        while(eventitor != eventinfolisttmp.end())
        {
            int struct_size = sizeof(transeventlistdatasimple);

            std::vector<BYTE> buffer;
            size_t userPayloadSize = struct_size + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            buffer.resize(userPayloadSize);

            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY;
            BYTE *pDstData = static_cast<BYTE*>(buffer.data());
            memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
            memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &struct_size, sizeof(UINT32));
            transeventlistdatasimple *pEventdata = reinterpret_cast<transeventlistdatasimple *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));

            memset(pEventdata,0,struct_size);

            pEventdata->eventType = eventType;
            pEventdata->event.eventType = eventType;
            pEventdata->event.eventifo = eventitor->second;


#ifdef SEND_EVENT_SINGLE
            switch (eventType) {
            case E_EVENT_MODE_ADD:
            case E_EVENT_MODE_UPDATE:
            case E_EVENT_MODE_REMOVE:
            {
#ifdef USING_GAEACTOR_TRANSMIT
                std::tuple<std::string, CHANNEL_INFO*> publisherchannel = std::make_tuple("",m_pGaeactorTransmit->applyforShareChannel());
                if(std::get<1>(publisherchannel) && (std::get<1>(publisherchannel)->m_instance.empty() || std::get<1>(publisherchannel)->m_event.empty() || std::get<1>(publisherchannel)->m_service.empty()))
                {
                    std::cout<<" get sharechannel failed"<<std::endl;
                }
                else
                {
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    trans_receive_callback(pDstData, userPayloadSize);
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                    m_pGaeactorTransmit->transmitData(std::get<1>(publisherchannel), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY, (const BYTE *)(pEventdata), struct_size);
                }
#else
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                trans_receive_callback(pDstData, userPayloadSize);
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
            }break;
            default:
                break;
            }

#else

            if(m_pEventDataCircularBufferThread)
            {
                //                m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                switch (eventType) {
                case E_EVENT_MODE_ADD:
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //add event send 3 times
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#ifdef SEND_ADD_REMOVE_EVENTS
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#endif
                }break;
                case E_EVENT_MODE_REMOVE:
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    //remove event send 3 times
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#ifdef SEND_ADD_REMOVE_EVENTS
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
                    m_pEventDataCircularBufferThread->pushbackData((const BYTE *)&eventdata, sizeof(transeventlistdatasimple),0);
#endif
                }break;
                default:
                    break;
                }
            }
#endif
            eventitor++;
        }
#ifndef SEND_EVENT_SINGLE
        if(m_pEventDataCircularBufferThread)
        {
            m_pEventDataCircularBufferThread->popfrontDataToDeal();
        }
#endif
    };

#if 0
    pushEventInfoNow(addeventinfolist, E_EVENT_MODE_ADD);
    pushEventInfoNow(removeeventinfolist, E_EVENT_MODE_REMOVE);
    pushEventInfoToCacheBuf(updateeventinfolist, E_EVENT_MODE_UPDATE);
    if(m_pEventDataCircularBufferThread)
    {
        m_pEventDataCircularBufferThread->wake();
    }
#else

    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
       pushEventInfoNow(addeventinfolist, E_EVENT_MODE_ADD);
       if(runningmode::RunningModeConfig::getInstance().get_USING_DEAL_UPDATE_EVENT())
       {
          pushEventInfoNow(updateeventinfolist, E_EVENT_MODE_UPDATE);
       }
       pushEventInfoNow(removeeventinfolist, E_EVENT_MODE_REMOVE);
    }
    else
    {

       if(addeventinfolist.empty() &&
           removeeventinfolist.empty() &&
           updateeventinfolist.empty())
       {
            int struct_size = sizeof(transeventlistdatasimple);

            std::vector<BYTE> buffer;
            size_t userPayloadSize = struct_size + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            buffer.resize(userPayloadSize);

            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY;
            BYTE *pDstData = static_cast<BYTE*>(buffer.data());
            memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
            memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &struct_size, sizeof(UINT32));
            transeventlistdatasimple *pEventdata = reinterpret_cast<transeventlistdatasimple *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));

            memset(pEventdata,0,struct_size);

            pEventdata->eventType = E_EVENT_MODE_NULL;
            pEventdata->event.eventType = E_EVENT_MODE_NULL;

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            trans_receive_callback(pDstData, userPayloadSize);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       }
       else
       {
            uint32_t ieventcount = addeventinfolist.size();
            if(runningmode::RunningModeConfig::getInstance().get_USING_DEAL_UPDATE_EVENT())
            {
                ieventcount += updateeventinfolist.size();
            }
            ieventcount += removeeventinfolist.size();

            // std::stringstream ss;
            // ss<<" event_count "<<ieventcount<<" add "<< addeventinfolist.size() <<" upt "<< updateeventinfolist.size() <<" rem "<< removeeventinfolist.size() <<"\n";
            // TRACE_LOG_PRINT_EX2(ss);
#if 0
       int struct_size = sizeof(transeventlistdata) + sizeof(transeventdata) * ieventcount;


       std::vector<BYTE> buffer;
       size_t userPayloadSize = struct_size + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
       buffer.resize(userPayloadSize);


       E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT;
       BYTE *pDstData = static_cast<BYTE*>(buffer.data());
       memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
       memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &struct_size, sizeof(UINT32));
       transeventlistdata *pEventdata = reinterpret_cast<transeventlistdata *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));

       memset(pEventdata,0,struct_size);

       pEventdata->eventType = E_EVENT_MODE_NULL;
       pEventdata->eventcount = ieventcount;

       transeventdata *ptranseventdata = reinterpret_cast<transeventdata *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + sizeof(E_EVENT_MODE) + sizeof(UINT32));
       {
            auto addeventitor = addeventinfolist.begin();
            while(addeventitor != addeventinfolist.end())
            {
                ptranseventdata->eventType = E_EVENT_MODE_ADD;
                ptranseventdata->eventifo = addeventitor->second;
                ptranseventdata++;
                addeventitor++;
            }
       }
       if(runningmode::RunningModeConfig::getInstance().get_USING_DEAL_UPDATE_EVENT())
       {
            auto updateeventitor = updateeventinfolist.begin();
            while(updateeventitor != updateeventinfolist.end())
            {
                ptranseventdata->eventType = E_EVENT_MODE_UPDATE;
                ptranseventdata->eventifo = updateeventitor->second;
                ptranseventdata++;
                updateeventitor++;
            }
       }
       {
            auto removeeventitor = removeeventinfolist.begin();
            while(removeeventitor != removeeventinfolist.end())
            {
                ptranseventdata->eventType = E_EVENT_MODE_REMOVE;
                ptranseventdata->eventifo = removeeventitor->second;
                ptranseventdata++;
                removeeventitor++;
            }
       }
       ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       trans_receive_callback(pDstData, userPayloadSize);
       ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
            int struct_size = sizeof(transeventlistdatasimple) * ieventcount;

            std::vector<BYTE> buffer;
            size_t userPayloadSize = struct_size + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            buffer.resize(userPayloadSize);

            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY;
            BYTE *pDstData = static_cast<BYTE*>(buffer.data());
            memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
            memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &struct_size, sizeof(UINT32));
            transeventlistdatasimple *pEventdata = reinterpret_cast<transeventlistdatasimple *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));

            memset(pEventdata,0,struct_size);

            {
                auto addeventitor = addeventinfolist.begin();
                while(addeventitor != addeventinfolist.end())
                {
                    pEventdata->eventType = E_EVENT_MODE_ADD;
                    pEventdata->event.eventType = E_EVENT_MODE_ADD;
                    pEventdata->event.eventifo = addeventitor->second;

                    pEventdata++;
                    addeventitor++;
                }
            }
            if(runningmode::RunningModeConfig::getInstance().get_USING_DEAL_UPDATE_EVENT())
            {
                auto updateeventitor = updateeventinfolist.begin();
                while(updateeventitor != updateeventinfolist.end())
                {
                    pEventdata->eventType = E_EVENT_MODE_UPDATE;
                    pEventdata->event.eventType = E_EVENT_MODE_UPDATE;
                    pEventdata->event.eventifo = updateeventitor->second;

                    pEventdata++;
                    updateeventitor++;
                }
            }
            {
                auto removeeventitor = removeeventinfolist.begin();
                while(removeeventitor != removeeventinfolist.end())
                {
                    pEventdata->eventType = E_EVENT_MODE_REMOVE;
                    pEventdata->event.eventType = E_EVENT_MODE_REMOVE;
                    pEventdata->event.eventifo = removeeventitor->second;

                    pEventdata++;
                    removeeventitor++;
                }
            }

            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            trans_receive_callback(pDstData, userPayloadSize);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
       }
    }
#endif

}

void GaeactorTransmitManager::data_receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pdata, const UINT32 &ilen, const BYTE *pOrignaldata, const UINT32 &iOrignallen)
{
    //EASY_FUNCTION(profiler::colors::Green)
    switch (channelTransmitDataType) {
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS:
    {
        transformHexPosData(pdata,ilen);
    }
        break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS_ARRAY:
    {
        transformHexPosArrayData(pdata,ilen);
    }
    break;

    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT:
    {
        transformHexPosattData(pdata,ilen);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT_ARRAY:
    {
        transformHexPosattArrayData(pdata,ilen);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR_ARRAY:
    {
        transformHexSensorArrayData(pdata,ilen);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR:
    {
        transformHexSensorData(pdata,ilen);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_PATH:
    {
        transformGetSensorPathData(pdata,ilen);
    }
    break;
    default:
        break;
    }
}

void GaeactorTransmitManager::dealHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO &sensorinfo, E_DISPLAY_MODE eDdisplayMode)
{
    if(runningmode::RunningModeConfig::getInstance().get_USING_TRANS_MODE_SENSORDATA())
    {
        EASY_FUNCTION(profiler::colors::Navy)
#if 0
        size_t ibufsize = sizeof(UINT16) + sizeof(H3INDEX) * hexidxslist.size() + sizeof(UINT16) + sizeof(TYPE_POLYGON_PT) * polygonlist.size();
        size_t isize = sizeof(transentityhexidxdata) + ibufsize;

        size_t userPayloadSize = isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
        CHANNEL_INFO* publisherchannel = applyforShareChannel();
        void * usrpayload = loanTransmitBuffer(publisherchannel, userPayloadSize);

        if(usrpayload)
        {
            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX;
            BYTE *pDstData = static_cast<BYTE*>(usrpayload);
            memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
            memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
            transentityhexidxdata *pData = reinterpret_cast<transentityhexidxdata *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
            memset(pData,0,isize);

            pData->uildsrc = uildsrc;
            pData->uilddst = uilddst;
            pData->eDdisplayMode = eDdisplayMode;
            pData->sensorinfo = sensorinfo;
            pData->buffersize = ibufsize;

            if(ibufsize > 0)
            {
                BYTE *p_byte_buffer = (BYTE*)(pData->buffer);
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                transentityhexidxlistdata *ptransdata_payload_hexidx = TRANSMIT_GET_HEXIDX_STRUCT_PTR(p_byte_buffer);
                ptransdata_payload_hexidx->flexarrycount = hexidxslist.size();
                if(!hexidxslist.empty())
                {
                    memcpy(ptransdata_payload_hexidx->hexidxlist, hexidxslist.data(), sizeof(transdata_param_seq_hexidx) * hexidxslist.size());
                }
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                transentitypolygonlistdata *ptransdata_payload_polygon = TRANSMIT_GET_POLYGON_STRUCT_PTR(p_byte_buffer);
                ptransdata_payload_polygon->flexarrycount = polygonlist.size();
                if(!hexidxslist.empty())
                {
                    memcpy(ptransdata_payload_polygon->polygonlist, polygonlist.data(), sizeof(TYPE_POLYGON_PT) * polygonlist.size());
                }
                /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            trans_receive_callback(usrpayload, userPayloadSize);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USING_GAEACTOR_TRANSMIT
            m_pGaeactorTransmit->publish(publisherchannel);
#endif
            freeData(usrpayload);
        }
#else

        transentityhexidxdata phexidxData;

        phexidxData.uildsrc = uildsrc;
        phexidxData.uilddst = uilddst;
        phexidxData.eDdisplayMode = eDdisplayMode;
        phexidxData.sensorinfo = sensorinfo;
        ::msg::AgentPositionInfo::msg_transentityhexidxdata _AgentRelationInfo = gaeactortransmit::GaeactorTransmit::SerializeHexidxData(phexidxData,hexidxslist,polygonlist);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        E_CHANNEL_TRANSMITDATA_TYPE datatype = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX;
        uint32_t irecvlen = _AgentRelationInfo.ByteSizeLong();
        std::vector<BYTE> buffer;
        buffer.resize(sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t) + irecvlen);
        memcpy(buffer.data(), &datatype, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(buffer.data() + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &irecvlen, sizeof(uint32_t));
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        BYTE * data = (BYTE *)(buffer.data()+sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t));
        _AgentRelationInfo.SerializeToArray(data, irecvlen);

        size_t userPayloadSize = buffer.size();
        void * usrpayload = (void *)buffer.data();
        trans_receive_callback(usrpayload, userPayloadSize);
#endif
    }
}

void GaeactorTransmitManager::dealEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE &echowaveinfo, const HEXIDX_HGT_ARRAY &hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave)
{
    //EASY_FUNCTION(profiler::colors::Magenta)
    transechowavedatasimple data;
    memset(&data,0,sizeof(transechowavedatasimple));
    data.uildval = uildval;

    data.sensoruildsrc = std::get<0>(echowaveinfo);
    data.entityuilddst = std::get<1>(echowaveinfo);
    data.sensingmediaildsrc = std::get<2>(echowaveinfo);
    data.sensorhexidx = std::get<3>(echowaveinfo);
    data.entityhexidx = std::get<4>(echowaveinfo);
    data.bEchoWave = bEchoWave;
    data.hexidxlistcount = hexidxslist.size();
    data.geolistcount = geolatlnglist.size();
    if(data.hexidxlistcount == 2)
    {
        data.hexidxlist[0] = hexidxslist.at(0).PARAM_seq_hexidx_element;
        data.hexidxlist[1] = hexidxslist.at(1).PARAM_seq_hexidx_element;
    }

    if(data.geolistcount == 2)
    {
        data.geolist[0].mLat = geolatlnglist.at(0).lat;
        data.geolist[0].mLon = geolatlnglist.at(0).lng;
        data.geolist[1].mLat = geolatlnglist.at(1).lat;
        data.geolist[1].mLon = geolatlnglist.at(1).lng;
    }
    if(m_pInteractionsDataCircularBufferThread)
    {
        m_pInteractionsDataCircularBufferThread->pushbackData( (const BYTE *)&data, sizeof(transechowavedatasimple),0);
        m_pInteractionsDataCircularBufferThread->wake();
    }
}


GaeactorTransmitManager::GaeactorTransmitManager(const QString& transmit_channel_title, QObject *parent)
    :QObject(parent)
    ,m_pGaeactorAgentCores(nullptr)
    ,m_pGaeactorAgentSensors(nullptr)
    ,m_pPosDataCircularBufferThread(nullptr)
    ,m_pWaveDataCircularBufferThread(nullptr)
    ,m_pInteractionsDataCircularBufferThread(nullptr)
    ,m_pEventDataCircularBufferThread(nullptr)
    ,m_pSensorUpdateDataCircularBufferThread(nullptr)
#ifdef USING_GAEACTOR_EXPORT_LIB
    ,m_receive_callback(nullptr)
#endif
{
    qRegisterMetaType<QVector<QByteArray>>("QVector<QByteArray>");
    init(transmit_channel_title);
}

void GaeactorTransmitManager::init(const QString &transmit_channel_title)
{
#ifdef USING_GAEACTOR_TRANSMIT
    m_pGaeactorTransmit = new gaeactortransmit::GaeactorTransmit(this);
    set_transmit_channel_id(transmit_channel_title);
    std::vector<std::tuple<std::string,std::string>> servicelist;
    //servicelist.push_back(std::make_tuple("gaeactor",transmit_channel_title.toStdString()));
    servicelist.push_back(std::make_tuple("gaeactor-viewer",transmit_channel_title.toStdString()));
    servicelist.push_back(std::make_tuple("gaeactor-test",transmit_channel_title.toStdString()));
    servicelist.push_back(std::make_tuple("gaeactor-hub",transmit_channel_title.toStdString()));

    m_pGaeactorTransmit->initDeployType(E_DEPLOYMODE_TYPE_LOCAL_RECV_SEND,servicelist);
#endif
    m_pGaeactorAgentCores  = new gaeactoragentcores::GaeactorAgentCores(this);
    m_pGaeactorAgentSensors  = new gaeactoragentsensors::GaeactorAgentSensors(this);

    m_pGaeactorAgentCores->registDisplayCallback(std::bind(&GaeactorTransmitManager::displayPosCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    m_pGaeactorAgentCores->registEventUpdateCallback(std::bind(&GaeactorTransmitManager::dealEventUpdateCallback, this, std::placeholders::_1));
    m_pGaeactorAgentSensors->registDisplayCallback(std::bind(&GaeactorTransmitManager::displayHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    m_pGaeactorAgentSensors->registEventUpdateCallback(std::bind(&GaeactorTransmitManager::dealEventUpdateCallback, this, std::placeholders::_1));

//    gaeactorinteractions::GaeactorInteractions::getInstance().registHexidxDisplayCallback(std::bind(&GaeactorTransmitManager::displayInteractionsHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
//    gaeactorinteractions::GaeactorInteractions::getInstance().registDisplayCallback(std::bind(&GaeactorTransmitManager::displayInteractionsEchoWaveHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
//    gaeactorinteractions::GaeactorInteractions::getInstance().registDisplayListCallback(std::bind(&GaeactorTransmitManager::displayInteractionsListEchoWaveHexidxCallback, this, std::placeholders::_1));
    gaeactorauditions::GaeactorAuditions::getInstance().registHexidxDisplayCallback(std::bind(&GaeactorTransmitManager::displayAuditionsHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    gaeactorauditions::GaeactorAuditions::getInstance().registDisplayCallback(std::bind(&GaeactorTransmitManager::displayAuditionsEchoWaveHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    gaeactoreventengine::GaeactorEventEngine::getInstance().registEventUpdateCallback(std::bind(&GaeactorTransmitManager::dealEventUpdateCallback, this, std::placeholders::_1));

    gaeactoreventengine::GaeactorEventEngine::getInstance().registSensorUpdateCallback(std::bind(&GaeactorTransmitManager::dealSensorUpdateCallback, this, std::placeholders::_1, std::placeholders::_2));
#ifdef USING_GAEACTOR_TRANSMIT
    m_pGaeactorTransmit->setDataCallback(std::bind(&GaeactorTransmitManager::data_receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

    m_pGaeactorTransmit->allocShareChannel(200);
#endif

//    gaeactorinteractions::GaeactorInteractions::getInstance().setCheckEnable(true);
    gaeactorauditions::GaeactorAuditions::getInstance().setCheckEnable(true);
    gaeactoreventengine::GaeactorEventEngine::getInstance().setCheckEnable(true);

    initBuffer();


    //StartEvent();
#ifndef DATA_DEAL_MODE_QUEUE
    for(int i = 0 ; i< USING_THREAD_NUM; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorTransmitManager::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorTransmitManager::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                   &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();

    }
#endif
}

void GaeactorTransmitManager::initBuffer()
{
#ifdef USING_GAEACTOR_TRANSMIT
    m_pPosDataCircularBufferThread = new dealStorageDataThread(m_pGaeactorTransmit,MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY);
    //m_pWaveDataCircularBufferThread = new dealStorageDataThread(MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX_ARRAY);
    m_pInteractionsDataCircularBufferThread = new dealStorageDataThread(m_pGaeactorTransmit,MAX_BUFFER_LEN,E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY);
    m_pEventDataCircularBufferThread = new dealStorageDataThread(m_pGaeactorTransmit,MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY);
    m_pSensorUpdateDataCircularBufferThread = new dealStorageDataThread(m_pGaeactorTransmit,1024, E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY);
#else
//    m_pPosDataCircularBufferThread = new dealStorageDataThread(nullptr,MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY);
//    //m_pWaveDataCircularBufferThread = new dealStorageDataThread(MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX_ARRAY);
//    m_pInteractionsDataCircularBufferThread = new dealStorageDataThread(nullptr,MAX_BUFFER_LEN,E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY);
//    m_pEventDataCircularBufferThread = new dealStorageDataThread(nullptr,MAX_BUFFER_LEN, E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY);
//    m_pSensorUpdateDataCircularBufferThread = new dealStorageDataThread(nullptr,1024, E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY);

#endif
}

void GaeactorTransmitManager::transformHexPosData(const BYTE *pdata, const UINT32 &ilen)
{
#if 0
    QByteArray by((const char *)pdata, ilen);
    dealtransformHexPosDataSlot(std::move(by));
#else
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
    m_pGaeactorAgentCores->inputPosData(*pos_hexidx_data);
#endif
}

void GaeactorTransmitManager::transformHexPosArrayData(const BYTE *pdata, const UINT32 &ilen)
{
#if 0
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    UINT32 iCount = ilen / sizeof(gaeactoragentcores::pos_hexidx);
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data_current = pos_hexidx_data;
    QVector<QByteArray> bylist;
    bylist.resize(iCount);
    for(int index = 0; index < iCount;index ++)
    {
        QByteArray by((const char *)pos_hexidx_data_current, sizeof(gaeactoragentcores::pos_hexidx));
        bylist[index] = std::move(by);
        pos_hexidx_data_current++;
    }
    dealtransformHexPosArrayDataSlot(std::move(bylist));
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    UINT32 iCount = ilen / sizeof(gaeactoragentcores::pos_hexidx);
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data_current = pos_hexidx_data;
    for(int index = 0; index < iCount;index ++)
    {
        const char *pdata = (const char *)pos_hexidx_data_current;
        const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
        m_pGaeactorAgentCores->inputPosData(*pos_hexidx_data);
        pos_hexidx_data_current++;
    }
#endif

}

void GaeactorTransmitManager::transformHexPosattData(const BYTE *pdata, const UINT32 &ilen)
{
#if 0
    QByteArray by((const char *)pdata, ilen);
    dealtransformHexPosattDataSlot(std::move(by));
#else
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
    m_pGaeactorAgentCores->inputPosattData(*pos_hexidx_data);
#endif
}

void GaeactorTransmitManager::transformHexPosattArrayData(const BYTE *pdata, const UINT32 &ilen)
{
#if 0
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    UINT32 ipackCount = ilen / sizeof(gaeactoragentcores::posatt_hexidx);
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data_current = pos_hexidx_data;
    QVector<QByteArray> bylist;
    bylist.resize(ipackCount);
    for(int index = 0; index < ipackCount;index ++)
    {
        QByteArray by((const char *)pos_hexidx_data_current, sizeof(gaeactoragentcores::posatt_hexidx));
        bylist[index] = std::move(by);
        pos_hexidx_data_current++;
    }
    dealtransformHexPosattArrayDataSlot(std::move(bylist));
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    UINT32 ipackCount = ilen / sizeof(gaeactoragentcores::posatt_hexidx);
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data_current = pos_hexidx_data;
    QVector<QByteArray> bylist;
    bylist.resize(ipackCount);
    for(int index = 0; index < ipackCount;index ++)
    {
        const char *pdata = (const char *)pos_hexidx_data_current;
        const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
        m_pGaeactorAgentCores->inputPosattData(*pos_hexidx_data);
        pos_hexidx_data_current++;
    }
#endif
}

void GaeactorTransmitManager::transformHexSensorData(const BYTE *pdata, const UINT32 &ilen)
{
#if 0
    QByteArray by((const char *)pdata, ilen);
    dealtransformSensorDataSlot(std::move(by));
#else
    const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(pdata);
    m_pGaeactorAgentSensors->inputWaveData(*data);
#endif
}

void GaeactorTransmitManager::transformHexSensorArrayData(const BYTE *pdata, const UINT32 &ilen)
{
    //EASY_FUNCTION(profiler::colors::DarkCyan)
#if 0
    UINT32 ireadLenCount = 0;

    UINT32 ipackCount = 0;

    if(ireadLenCount + sizeof(UINT32) > ilen)
    {
        std::stringstream ss;
        ss<<"decode trans sensor array pack count error";
        TRACE_LOG_PRINT_EX2(ss);
        return;
    }
    memcpy(&ipackCount, pdata, sizeof(UINT32));
    ireadLenCount += sizeof(UINT32);

    const BYTE* pTransSensorArrayData_ptr = pdata;

    QVector<QByteArray> bylist;
    bylist.resize(ipackCount);
    for(int index = 0; index < ipackCount;index ++)
    {
        UINT32 iPackLen;
        if(ireadLenCount + sizeof(UINT32) > ilen)
        {
            std::stringstream ss;
            ss<<"decode trans sensor array pack len error";
            TRACE_LOG_PRINT_EX2(ss);
            break;
        }
        memcpy(&iPackLen, pTransSensorArrayData_ptr + ireadLenCount, sizeof(UINT32));
        ireadLenCount += sizeof(UINT32);

        if(ireadLenCount + iPackLen > ilen)
        {
            std::stringstream ss;
            ss<<"decode trans sensor array pack len error";
            TRACE_LOG_PRINT_EX2(ss);
            break;
        }
        QByteArray by;
        by.resize(iPackLen);
        memcpy(by.data(), pTransSensorArrayData_ptr + ireadLenCount, iPackLen);
        ireadLenCount += iPackLen;
        bylist[index] = std::move(by);
    }
    if(ireadLenCount != ilen )
    {
        std::stringstream ss;
        ss<<"trans sensor array error";
        TRACE_LOG_PRINT_EX2(ss);
    }
    else
    {
        dealtransformSensorArrayDataSlot(std::move(bylist));
    }
#else

    UINT32 ireadLenCount = 0;

    UINT32 ipackCount = 0;

    if(ireadLenCount + sizeof(UINT32) > ilen)
    {
        std::stringstream ss;
        ss<<"decode trans sensor array pack count error";
        TRACE_LOG_PRINT_EX2(ss);
        return;
    }
    memcpy(&ipackCount, pdata, sizeof(UINT32));
    ireadLenCount += sizeof(UINT32);

    const BYTE* pTransSensorArrayData_ptr = pdata;

    QVector<QByteArray> bylist;
    bylist.resize(ipackCount);
    for(int index = 0; index < ipackCount;index ++)
    {
        UINT32 iPackLen;
        if(ireadLenCount + sizeof(UINT32) > ilen)
        {
            std::stringstream ss;
            ss<<"decode trans sensor array pack len error";
            TRACE_LOG_PRINT_EX2(ss);
            break;
        }
        memcpy(&iPackLen, pTransSensorArrayData_ptr + ireadLenCount, sizeof(UINT32));
        ireadLenCount += sizeof(UINT32);

        if(ireadLenCount + iPackLen > ilen)
        {
            std::stringstream ss;
            ss<<"decode trans sensor array pack len error";
            TRACE_LOG_PRINT_EX2(ss);
            break;
        }

        const char *pdata = (const char *)(pTransSensorArrayData_ptr + ireadLenCount);
        const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(pdata);
        m_pGaeactorAgentSensors->inputWaveData(*data);

        ireadLenCount += iPackLen;
    }
    if(ireadLenCount != ilen )
    {
        std::stringstream ss;
        ss<<"trans sensor array error";
        TRACE_LOG_PRINT_EX2(ss);
    }
#endif
}

void GaeactorTransmitManager::transformGetSensorPathData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata, ilen);
    dealtransformGetSensorPathDataSlot(std::move(by));
}




//#include <time.h>
//#ifdef WIN32
//#include <windows.h>
//#else
//#include <sys/time.h>
//#endif
//#ifdef WIN32
//int gettimeofday(struct timeval *tp, void *tzp)
//{
//    time_t clock;
//    struct tm tm;
//    SYSTEMTIME wtm;

//    GetLocalTime(&wtm);
//    tm.tm_year   = wtm.wYear  - 1900;
//    tm.tm_mon    = wtm.wMonth - 1;
//    tm.tm_mday   = wtm.wDay;
//    tm.tm_hour   = wtm.wHour;
//    tm.tm_min    = wtm.wMinute;
//    tm.tm_sec    = wtm.wSecond;
//    tm.tm_isdst  = -1;

//    clock = mktime(&tm);
//    tp->tv_sec   = clock;
//    tp->tv_usec  = wtm.wMilliseconds * 1000;
//    return (0);
//}
//#endif

//void GaeactorTransmitManager::PrintTime()
//{
//    struct timeval timer;
//    gettimeofday(&timer, NULL);
//    volatile uint64_t current_time = (uint64_t)(timer.tv_sec)*1000+ (uint)(timer.tv_usec);
//    printf("current_time:%lld\n", current_time);
//}

//// 定时事件回调函数
//void GaeactorTransmitManager::onTime(evutil_socket_t sock, short event, void *arg)
//{
//    GaeactorTransmitManager * pGaeactorTransmitManager = reinterpret_cast<GaeactorTransmitManager*>(arg);
//    if(pGaeactorTransmitManager)
//    {
//        static int64_t last = 0;
//        auto timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//        //printf("current_time:%lld %lld\n",timestamp, timestamp-last);
//        last = timestamp;
//        //pGaeactorTransmitManager->PrintTime();
//        struct timeval tv;
//        tv.tv_sec = 0;
//        tv.tv_usec = 5;
//        // 重新添加定时事件（定时事件触发后默认自动删除）
//        event_add((struct event*)arg, &tv);
//    }
//}

//void GaeactorTransmitManager::startEvt()
//{
//    // 初始化
//    event_init();

//    struct event evTime;
//    // 设置定时事件
//    evtimer_set(&tickEvt, &GaeactorTransmitManager::onTime, (void*)this);

//    struct timeval tv;
//    tv.tv_sec = 0;
//    tv.tv_usec = 5;
//    // 添加定时事件
//    event_add(&evTime, &tv);

//    // 事件循环
//    event_dispatch();
//}

//void GaeactorTransmitManager::StartEvent()
//{
//    // 初始化
//    //event_init();
//    baseEvt = event_base_new();

//    // 设置定时事件
//    evtimer_set(&tickEvt, &GaeactorTransmitManager::onTime, &tickEvt);

//    struct timeval tv;
//    tv.tv_sec = 0;
//    tv.tv_usec = 5;
//    // 添加定时事件
//    //event_add(&tickEvt, &tv);
//    event_base_set(baseEvt, &tickEvt);
//    evtimer_add(&tickEvt, &tv);

//    // 事件循环
//    //event_dispatch();
//    event_base_loop(baseEvt, 0);
//}

void GaeactorTransmitManager::pushbuffer(const TYPE_ULID &ulid, E_DATA_TYPE type , QByteArray &&by,const TYPE_ULID &sensingmediaid)
{
    if(type == E_DATA_TYPE_SENSOR)
    {
        QWriteLocker locker(&m_snesordatabuf_mutex);
        auto id_pair = qMakePair(ulid, sensingmediaid);
        auto itor = m_snesordatabuf.find(id_pair);
        if(itor != m_snesordatabuf.end())
        {
            std::get<0>(itor->second) = std::move(by);
            std::get<1>(itor->second) = type;
            std::get<2>(itor->second) = true;
        }
        else
        {
            m_snesordatabuf.insert(std::make_pair(std::move(id_pair), std::make_tuple(std::move(by), type , true)));
        }
    }
    else
    {
        QWriteLocker locker(&m_entitydatabuf_mutex);
        auto itor = m_entitydatabuf.find(ulid);
        if(itor != m_entitydatabuf.end())
        {
            std::get<0>(itor->second) = std::move(by);
            std::get<1>(itor->second) = type;
            std::get<2>(itor->second) = true;
        }
        else
        {
            m_entitydatabuf.insert(std::make_pair(ulid, std::make_tuple(std::move(by), type , true)));
        }

    }


}
#ifndef DATA_DEAL_MODE_QUEUE
void GaeactorTransmitManager::data_deal_thread_func(void *pParam)
{
    if (pParam == nullptr)
    {
        return;
    }

    threadParam *pObject = reinterpret_cast<threadParam*>(pParam);
    if(pObject)
    {
        {
            QReadLocker locker(&m_entitydatabuf_mutex);
            auto itor = m_entitydatabuf.begin();
            while(itor != m_entitydatabuf.end())
            {
                QByteArray &by = std::get<0>(itor->second);
                E_DATA_TYPE &type = std::get<1>(itor->second);
                bool &bUpdate = std::get<2>(itor->second);
                if(bUpdate)
                {
                    switch (type) {
                    case E_DATA_TYPE_POS:
                    {
                        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_pos_data_func_callback, this, std::placeholders::_1),\
                                                                                         std::move(by));
                        //                        deal_pos_data_func_callback(by);
                    }break;
                    case E_DATA_TYPE_POS_ATT:
                    {
                        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_posatt_data_func_callback, this, std::placeholders::_1),\
                                                                                         std::move(by));
                        //                        deal_posatt_data_func_callback(by);
                    }break;
                    default:
                        break;
                    }
                    bUpdate = false;
                }
                itor++;
            }
        }

        {
            QReadLocker locker(&m_snesordatabuf_mutex);
            auto itor = m_snesordatabuf.begin();
            while(itor != m_snesordatabuf.end())
            {
                QByteArray &by = std::get<0>(itor->second);
                E_DATA_TYPE &type = std::get<1>(itor->second);
                bool &bUpdate = std::get<2>(itor->second);
                if(bUpdate)
                {
                    switch (type) {
                    case E_DATA_TYPE_SENSOR:
                    {
                        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_sensor_data_func_callback, this, std::placeholders::_1),\
                                                                                         std::move(by));
                        //                        deal_sensor_data_func_callback(by);
                    }break;
                    default:
                        break;
                    }
                    bUpdate = false;
                }
                itor++;
            }
        }
        stdutils::OriDateTime::sleep(DATA_DEAL_INTERVAL);
    }
    return;
}
#endif

void GaeactorTransmitManager::dealtransformHexPosDataSlot(QByteArray &&by)
{
    //EASY_FUNCTION(profiler::colors::DarkCyan)
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(by.data());
    m_pGaeactorAgentCores->inputPosData(*pos_hexidx_data);
#else
    ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_pos_data_func_callback, this, std::placeholders::_1),\
                                                                     std::move(by));
#endif
#else
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(by.data());
    pushbuffer(pos_hexidx_data->PARAM_protocol_head.PARAM_source_ulid, E_DATA_TYPE_POS, std::move(by));
#endif
}

void GaeactorTransmitManager::dealtransformHexPosArrayDataSlot(QVector<QByteArray> &&bylist)
{
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist.at(index);
        const char *pdata = by.data();
        const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
        m_pGaeactorAgentCores->inputPosData(*pos_hexidx_data);
    }
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
//    ThreadPoolTaskManager::getInstance().appendArrayProcessor<QVector<QByteArray>>(std::bind(&GaeactorTransmitManager::deal_pos_array_data_func_callback, this, std::placeholders::_1),\
//                                                                     std::move(bylist));
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_pos_data_func_callback, this, std::placeholders::_1),\
                                                                         std::move(by));
    }
#endif
#else
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        const char *pdata = by.data();
        const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
        TYPE_ULID ulid = pos_hexidx_data->PARAM_protocol_head.PARAM_source_ulid;
        pushbuffer(ulid, E_DATA_TYPE_POS, std::move(by));
    }
#endif
}


void GaeactorTransmitManager::dealtransformHexPosattDataSlot(QByteArray &&by)
{
    //EASY_FUNCTION(profiler::colors::DarkCyan)
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(by.data());
    m_pGaeactorAgentCores->inputPosattData(*pos_hexidx_data);
#else
    ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_posatt_data_func_callback, this, std::placeholders::_1),\
                                                                     std::move(by));
#endif
#else
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(by.data());
    TYPE_ULID ulid = pos_hexidx_data->PARAM_protocol_head.PARAM_source_ulid;
    pushbuffer(ulid, E_DATA_TYPE_POS_ATT, std::move(by));
#endif
}

void GaeactorTransmitManager::dealtransformHexPosattArrayDataSlot(QVector<QByteArray> &&bylist)
{
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist.at(index);
        const char *pdata = by.data();
        const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
        m_pGaeactorAgentCores->inputPosattData(*pos_hexidx_data);
    }
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
//    ThreadPoolTaskManager::getInstance().appendArrayProcessor<QVector<QByteArray>>(std::bind(&GaeactorTransmitManager::deal_posatt_array_data_func_callback, this, std::placeholders::_1),\
//                                                                     std::move(bylist));
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_posatt_data_func_callback, this, std::placeholders::_1),\
                                                                         std::move(by));
    }
#endif
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        const char *pdata = by.data();
        const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
        TYPE_ULID ulid = pos_hexidx_data->PARAM_protocol_head.PARAM_source_ulid;
        pushbuffer(ulid, E_DATA_TYPE_POS_ATT, std::move(by));
    }
#endif
}

void GaeactorTransmitManager::dealtransformSensorDataSlot(QByteArray &&by)
{
    //EASY_FUNCTION(profiler::colors::DarkCyan)
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(by.data());
    m_pGaeactorAgentSensors->inputWaveData(*data);
#else
    ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_sensor_data_func_callback, this, std::placeholders::_1),\
                                                                     std::move(by));
#endif
#else
    const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(by.data());
    TYPE_ULID ulid = data->PARAM_protocol_head.PARAM_source_ulid;
    TYPE_ULID sensingmediaid = data->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_source_sensingmediaid;
    pushbuffer(ulid, E_DATA_TYPE_SENSOR, std::move(by),sensingmediaid);
#endif

}

void GaeactorTransmitManager::dealtransformSensorArrayDataSlot(QVector<QByteArray> &&bylist)
{
#ifdef DATA_DEAL_MODE_QUEUE
#ifdef DEAL_DIRECT
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist.at(index);
        const char *pdata = by.data();
        const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(pdata);
        m_pGaeactorAgentSensors->inputWaveData(*data);
    }
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    //    ThreadPoolTaskManager::getInstance().appendArrayProcessor<QVector<QByteArray>>(std::bind(&GaeactorTransmitManager::deal_sensor_array_data_func_callback, this, std::placeholders::_1),\
    //                                                                     std::move(bylist));
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        ThreadPoolTaskManager::getInstance().appendProcessor<QByteArray>(std::bind(&GaeactorTransmitManager::deal_sensor_data_func_callback, this, std::placeholders::_1),\
                                                                         std::move(by));
    }
#endif
#else
    //EASY_FUNCTION(profiler::colors::DarkCyan)
    for(int index = 0; index < bylist.size();index ++)
    {
        QByteArray & by = bylist[index];
        const char *pdata = by.data();
        const gaeactoragentsensors::wave_smd_hexidx *data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(pdata);
        TYPE_ULID ulid = data->PARAM_protocol_head.PARAM_source_ulid;
        TYPE_ULID sensingmediaid = data->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_source_sensingmediaid;
        pushbuffer(ulid, E_DATA_TYPE_SENSOR, std::move(by),sensingmediaid);
    }
#endif

}

void GaeactorTransmitManager::dealtransformGetSensorPathDataSlot(QByteArray &&by)
{
    //EASY_FUNCTION(profiler::colors::RichGreen)
    const char *pdata = by.data();
    const transdata_sensor_path *sensor_path_data = reinterpret_cast<const transdata_sensor_path *>(pdata);
    std::cout << "deal requset path " << sensor_path_data->srcid << " to " << sensor_path_data->dstid << std::endl;
    auto path = getSensorPath(sensor_path_data->srcid, sensor_path_data->dstid);

    size_t ibufsize = sizeof(tagEdgeVertexIndex) * path.m_path.size();
    size_t isize = sizeof(transdata_path_info) + ibufsize;

    CHANNEL_INFO* publisherchannel = applyforShareChannel();
    size_t userPayloadSize = isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);

    void * usrpayload = loanTransmitBuffer(publisherchannel, userPayloadSize);

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_PATH_ARRAY;
        BYTE *pDstData = static_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_path_info *pData = reinterpret_cast<transdata_path_info *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);

        pData->src = sensor_path_data->srcid;
        pData->dst = sensor_path_data->dstid;
        pData->m_bValid = path.m_bValid;
        pData->value = path.value;
        pData->end = path.end;
        pData->path_buffer_count = path.m_path.size();

        if(ibufsize > 0)
        {
            tagEdgeVertexIndex *p_byte_buffer = (tagEdgeVertexIndex*)(pData->path_seq_buffer);
            memcpy(p_byte_buffer, path.m_path.data(), ibufsize);
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        trans_receive_callback(usrpayload, userPayloadSize);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USING_GAEACTOR_TRANSMIT
        m_pGaeactorTransmit->publish(publisherchannel);
#endif
        freeData(usrpayload);
    }
}

void GaeactorTransmitManager::deal_pos_data_func_callback(const QByteArray &databy)
{
    //EASY_FUNCTION(profiler::colors::RichGreen)
    const char *pdata = databy.data();
    const gaeactoragentcores::pos_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::pos_hexidx *>(pdata);
    m_pGaeactorAgentCores->inputPosData(*pos_hexidx_data);
}

void GaeactorTransmitManager::deal_posatt_data_func_callback(const QByteArray &databy)
{
    //EASY_FUNCTION(profiler::colors::RichGreen)
    const char *pdata = databy.data();
    const gaeactoragentcores::posatt_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentcores::posatt_hexidx *>(pdata);
    m_pGaeactorAgentCores->inputPosattData(*pos_hexidx_data);
}

void GaeactorTransmitManager::deal_pos_array_data_func_callback(const QVector<QByteArray> &bylist)
{
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist[index];
        deal_pos_data_func_callback(by);
    }
}

void GaeactorTransmitManager::deal_posatt_array_data_func_callback(const QVector<QByteArray> &bylist)
{
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist[index];
        deal_posatt_data_func_callback(by);
    }
}

void GaeactorTransmitManager::deal_sensor_data_func_callback(const QByteArray &databy)
{
    //EASY_FUNCTION(profiler::colors::RichGreen)
    const char *pdata = databy.data();
    const gaeactoragentsensors::wave_smd_hexidx *pos_hexidx_data = reinterpret_cast<const gaeactoragentsensors::wave_smd_hexidx *>(pdata);
    m_pGaeactorAgentSensors->inputWaveData(*pos_hexidx_data);
}

void GaeactorTransmitManager::deal_sensor_array_data_func_callback(const QVector<QByteArray> &bylist)
{
    for(int index = 0; index < bylist.size();index ++)
    {
        const QByteArray & by = bylist[index];
        deal_sensor_data_func_callback(by);
    }
}

#ifdef USING_GAEACTOR_EXPORT_LIB
void GaeactorTransmitManager::setReceive_callback(receive_callback newReceive_callback)
{
    m_receive_callback = std::move(newReceive_callback);
}

void GaeactorTransmitManager::trans_receive_callback(void *usrpayload, size_t userPayloadSize)
{
    EASY_FUNCTION(profiler::colors::Navy)
#if 0
    std::vector<BYTE> buffer;
    size_t userPayloadSize = isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
    buffer.resize(userPayloadSize);
    memcpy(buffer.data(), usrpayload, userPayloadSize);
    if(m_receive_callback)
    {
        const BYTE *pData = nullptr;
        const BYTE *pOriginData = (const BYTE *)buffer.data();
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType;
        memcpy(&channelTransmitDataType, pOriginData, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        pData = pOriginData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
        m_receive_callback(nullptr, channelTransmitDataType, pData, isize, pOriginData, userPayloadSize);
    }
#else
    size_t isize = userPayloadSize - (sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32)) ;
    if(m_receive_callback)
    {
        const BYTE *pData = nullptr;
        const BYTE *pOriginData = (const BYTE *)usrpayload;
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType;
        memcpy(&channelTransmitDataType, pOriginData, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        pData = pOriginData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
        m_receive_callback(channelTransmitDataType, pData, isize , pOriginData, userPayloadSize);
    }
#endif
}

void GaeactorTransmitManager::trans_type_receive_callback(E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType, const BYTE *pData, size_t isize)
{
    std::vector<BYTE> buffer;
    size_t userPayloadSize = isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
    buffer.resize(userPayloadSize);

    BYTE *pDstData = static_cast<BYTE*>(buffer.data());
    memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
    memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
    BYTE *pEventdata = reinterpret_cast<BYTE *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));

    memset(pEventdata, 0, isize);
    memcpy(pEventdata, pData, isize);

    this->trans_receive_callback(pDstData, userPayloadSize);
}

#endif

QString GaeactorTransmitManager::transmit_channel_id() const
{
#ifdef USING_GAEACTOR_TRANSMIT
    return m_pGaeactorTransmit->transmit_channel_id();
#endif
    return QString();
}

void GaeactorTransmitManager::set_transmit_channel_id(const QString &new_transmit_channel_id)
{
#ifdef USING_GAEACTOR_TRANSMIT
    m_pGaeactorTransmit->set_transmit_channel_id(new_transmit_channel_id);
#endif
}

void GaeactorTransmitManager::deal_step_refresh_event()
{
    gaeactoreventengine::GaeactorEventEngine::getInstance().refreshEvents();
}

void GaeactorTransmitManager::set_refresh_event_enable(bool bEnbale)
{
    gaeactoreventengine::GaeactorEventEngine::getInstance().setEnableDeal(bEnbale);
}

tagPathInfo GaeactorTransmitManager::getSensorPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst)
{
    return gaeactoreventengine::GaeactorEventEngine::getInstance().getPath(uildsrc, dst);
}

CHANNEL_INFO *GaeactorTransmitManager::applyforShareChannel()
{
#ifdef USING_GAEACTOR_TRANSMIT
    if(m_pGaeactorTransmit)
    {
        return m_pGaeactorTransmit->applyforShareChannel();
    }
#endif
    return nullptr;
}

void *GaeactorTransmitManager::loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen)
{
    void * usrpayload = nullptr;
#ifdef USING_GAEACTOR_TRANSMIT
    if(m_pGaeactorTransmit)
    {
        usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(channelinfo,iLen);
    }
#else
    size_t isize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
    usrpayload = (void*)malloc(isize);
    memset(usrpayload,0,isize);
#endif
    return usrpayload;
}


void GaeactorTransmitManager::freeData(void *pSrcData)
{
#ifdef USING_GAEACTOR_TRANSMIT

#else
    free(pSrcData);
#endif
}

#ifdef USING_GAEACTOR_TRANSMIT
dealStorageDataThread::dealStorageDataThread(gaeactortransmit::GaeactorTransmit *pGaeactorTransmit, UINT32 buffer_size, E_CHANNEL_TRANSMITDATA_TYPE type)
#else
dealStorageDataThread::dealStorageDataThread(UINT32 buffer_size, E_CHANNEL_TRANSMITDATA_TYPE type)
#endif
    :m_pDataCircularBuffer(nullptr)
    ,m_pCurrentDataDealBuffer(nullptr)
    ,m_idealDataMaxBufferLen(buffer_size)
    ,m_hDataDealThread(nullptr)
    ,m_type(type)
#ifdef USING_GAEACTOR_TRANSMIT
    ,m_pGaeactorTransmit(pGaeactorTransmit)
#endif
{
    create_resource();
}

dealStorageDataThread::~dealStorageDataThread()
{
    release_resource();
}

void dealStorageDataThread::create_resource()
{
    if(nullptr == m_pDataCircularBuffer)
    {
        try
        {
            m_pDataCircularBuffer = new stdutils::DATA_STORAGE_CIRCULAR_BUFFER(m_idealDataMaxBufferLen*2);
        }
        catch (...)
        {
            m_pDataCircularBuffer = nullptr;
            return ;
        }
    }

    if(nullptr == m_pCurrentDataDealBuffer)
    {
        try
        {
            m_pCurrentDataDealBuffer = new BYTE[m_idealDataMaxBufferLen];
            memset(m_pCurrentDataDealBuffer, 0, sizeof(BYTE) * m_idealDataMaxBufferLen);
        }
        catch (...)
        {
            m_pCurrentDataDealBuffer = nullptr;
            return ;
        }
    }

    if(m_type == E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY ||\
                                                                            m_type == E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY ||\
              m_type == E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY)
    {
#ifdef _MSC_VER
        m_hDataDealThread = new stdutils::OriThread(std::bind(&dealStorageDataThread::data_deal_thread_func,this,std::placeholders::_1),\
                                                    this,\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread = new stdutils::OriThread(std::bind(&dealStorageDataThread::data_deal_thread_func,this,std::placeholders::_1),\
                                                    this,\
                                                    99);
#endif
        m_hDataDealThread->start();
    }
}

void dealStorageDataThread::release_resource()
{
    if(nullptr != m_pDataCircularBuffer)
    {
        delete m_pDataCircularBuffer;
        m_pDataCircularBuffer = nullptr;
    }

    if(m_pCurrentDataDealBuffer)
    {
        delete []m_pCurrentDataDealBuffer;
        m_pCurrentDataDealBuffer = nullptr;
    }

    m_idealDataMaxBufferLen = 0;


    if (m_hDataDealThread != nullptr)
    {
        delete m_hDataDealThread;
        m_hDataDealThread = nullptr;
    }
}

void dealStorageDataThread::data_deal_thread_func(void *pParam)
{
    if (pParam == nullptr)
    {
        return;
    }

    dealStorageDataThread *pObject = reinterpret_cast<dealStorageDataThread*>(pParam);
    if(pObject)
    {
        //        if(m_pDataCircularBuffer && m_pDataCircularBuffer->isEmpty())
        //        {
        //            m_dealfullCond.wait(&m_dealmutex);
        //        }
        popfrontDataToDeal();
#ifdef _MSC_VER
        stdutils::OriDateTime::sleep(BUFFER_THREAD_TRANSFER_INTERVAL);
#else
        stdutils::OriDateTime::sleep(BUFFER_THREAD_TRANSFER_INTERVAL);
#endif
    }
    return;
}


bool dealStorageDataThread::popfrontDataToDeal()
{
    m_dealfullCond.wait(&m_dealmutex);
    if((nullptr == m_pDataCircularBuffer) || (nullptr == m_pCurrentDataDealBuffer))
    {
        return false;
    }

    if(m_pDataCircularBuffer->isEmpty())
    {
        return false;
    }
    uint32_t iPopLen = m_idealDataMaxBufferLen;
    memset(m_pCurrentDataDealBuffer, 0, m_idealDataMaxBufferLen);
    int struct_size = 0;
    if(m_pDataCircularBuffer->PopFront(m_pCurrentDataDealBuffer, iPopLen))
    {
        switch (m_type) {
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
        {
            struct_size = sizeof(transentityhexidxpostdata);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX_ARRAY:
            break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION_ARRAY:
            break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY:
        {
            struct_size = sizeof(transechowavedatasimple);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
        {
            struct_size = sizeof(transeventlistdatasimple);
        }
        break;
        case E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY:
        {
            struct_size = sizeof(trans_sensor_update_info);
        }break;
        default:
            break;
        }


        if(iPopLen >= struct_size)
        {
            //总共有多少包
            int totalsize = iPopLen / struct_size;
            //单次最多传输的包数
            int permaxsize = MAX_SEND_BUFFER_SIZE / struct_size;
            //单次最多传输的字节数
            int perdealdatasize = permaxsize * struct_size;
            uint32_t idealLen = 0;
#if 0
            //需要传输的次数
            int needdealtimes = totalsize / permaxsize;
            //最后一次传输的包数
            int remainder = totalsize  % permaxsize;
            if(remainder != 0)
            {
                needdealtimes++;
            }

            if(m_type == E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY)
            {
                std::cout<<" array num "<<iPopLen / struct_size<<" res "<<iPopLen % struct_size<< " size: "<<iPopLen<<" kB: "<<(double)iPopLen/1024.0f<<" need repeat "<<needdealtimes<<" prenum "<<permaxsize<<" reminder "<<remainder<<std::endl;
            }
#endif
            while (idealLen != iPopLen)
            {
                //计算当次传输的字节数
                int iperdeallen = 0;
                if(idealLen + perdealdatasize < iPopLen)
                {
                    iperdeallen = perdealdatasize;
                    //std::cout<<"----------------------repeat deal "<<tracestr<<" array num "<<iperdeallen / struct_size << " size "<<iperdeallen<<" kB "<<(double)(iperdeallen)/1024.0f<<std::endl;
                }
                else
                {
                    iperdeallen = iPopLen - idealLen;
                    //std::cout<<"+++++++++++++++++++++repeat deal "<<tracestr<<" array num "<<iperdeallen / struct_size << " size "<<iperdeallen<<" kB "<<(double)(iperdeallen)/1024.0f<<std::endl;
                }
#ifdef USING_GAEACTOR_TRANSMIT
                //从偏移位置开始传输指定字节数的数据
                 CHANNEL_INFO* publisherchannel = m_pGaeactorTransmit->applyforShareChannel();
                if(publisherchannel && (publisherchannel->m_instance.empty() || publisherchannel->m_event.empty() || publisherchannel->m_service.empty()))
                {
                    std::cout<<" get sharechannel failed"<<std::endl;
                }
                else
                {
                    m_pGaeactorTransmit->transmitData(publisherchannel, m_type, (const BYTE *)(m_pCurrentDataDealBuffer + idealLen), iperdeallen);

                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    GaeactorTransmitManager::getInstance().trans_type_receive_callback(m_type, (const BYTE *)(m_pCurrentDataDealBuffer + idealLen), iperdeallen);
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                }
#else


                switch (m_type) {
                case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
                {
                    uint32_t num = iperdeallen / struct_size;
                    BYTE *pdata = (BYTE *)(m_pCurrentDataDealBuffer + idealLen);
                    ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentPositionInfo_array;
                    transentityhexidxpostdata* ptransdatasrc = (transentityhexidxpostdata*)(pdata);
                    for (int index = 0; index < num; index++)
                    {
                        transentityhexidxpostdata* ptransdata = (ptransdatasrc + index);
                        if(ptransdata)
                        {
                            *_AgentPositionInfo_array.add_transentityhexidxpostdata() = gaeactortransmit::GaeactorTransmit::SerializePositionData(*ptransdata);
                        }
                    }

                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    E_CHANNEL_TRANSMITDATA_TYPE datatype = m_type;
                    uint32_t irecvlen = _AgentPositionInfo_array.ByteSizeLong();
                    std::vector<BYTE> buffer;
                    buffer.resize(sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t) + irecvlen);
                    memcpy(buffer.data(), &datatype, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
                    memcpy(buffer.data() + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &irecvlen, sizeof(uint32_t));
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    BYTE * data = (BYTE *)(buffer.data()+sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t));
                    _AgentPositionInfo_array.SerializeToArray(data, irecvlen);

                    GaeactorTransmitManager::getInstance().trans_receive_callback((void *)buffer.data(),buffer.size());
                }
                break;
                case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
                {
                    struct_size = sizeof(transeventlistdatasimple);
                }
                break;
                default:
                {
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    GaeactorTransmitManager::getInstance().trans_type_receive_callback(m_type, (const BYTE *)(m_pCurrentDataDealBuffer + idealLen), iperdeallen);
                    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                }
                break;
                }
#endif
                //累计已传输的字节数
                idealLen += iperdeallen;
            }
        }
    }

    return true;
}

bool dealStorageDataThread::pushbackData(const BYTE *pData, UINT32 iDataLen, const TIMESTAMP_TYPE& iTimeStamp)
{
    if(nullptr == m_pDataCircularBuffer)
    {
        return false;
    }
    m_pDataCircularBuffer->PushBack(pData, iDataLen, iTimeStamp);
    return true;
}

void dealStorageDataThread::wake()
{
    m_dealfullCond.wakeAll();
}
