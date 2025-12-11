#include "gaeactor_agent_cores_processor.h"


#include "gaeactor_event_engine_interface.h"
#include "easy/profiler.h"
#include "src/OriginalDateTime.h"
#include "src/OriginalThread.h"
#include "loghelper.h"
#include "runningmodeconfig.h"

#ifdef USING_CORES_DEAL_THREAD
#define THREAD_EXIT_MAX_TIME (3000)
#define PRE_THREAD_MAX_DEAL_SIZE (300)
//#define USING_HEXIDX_ENTITY
//#define CLEAR_SENSOR_TO_SENSOR_INTERSECTION
#endif

namespace gaeactoragentcores {
GaeactorAgentCoresProcessor::GaeactorAgentCoresProcessor(QObject *parent)
{
#ifdef USING_CORES_DEAL_THREAD
    m_bChecking.store(true);
    for(int i = 0 ; i< USING_THREAD_NUM; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAgentCoresProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAgentCoresProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();
    }
#endif
}

GaeactorAgentCoresProcessor::~GaeactorAgentCoresProcessor()
{
#ifdef USING_CORES_DEAL_THREAD
    m_dealfullCond.wakeAll();

    for(int i = 0; i < USING_THREAD_NUM; i++)
    {
        if (m_hDataDealThread[i] != nullptr)
        {
            delete m_hDataDealThread[i];
            m_hDataDealThread[i] = nullptr;
        }
    }
#endif
}



void GaeactorAgentCoresProcessor::appenddata(const H3INDEX &h3Indexsrc, const TYPE_ULID & entityulid, transdata_entityposinfo&& eninfo)
{
#ifdef USING_CORES_DEAL_THREAD
    if(h3Indexsrc == 0)
    {
        {
            gaeactorenvironment::GaeactorProcessor::getInstance().clear_hexindex_entity(entityulid);
        }
        m_phexidx_update_callback(entityulid,0,eninfo,E_DISPLAY_MODE_ENTITY);
    }
    else
    {
        ///////////////////////////////////////////////////////////////
        bool bSensor = false;
        QList<UINT64> resolutions;
        std::unordered_map<UINT64,H3INDEX> resolution_target_hexidxs;
        {
            QWriteLocker locker2(&m_entity_hexidx_infos_mutex);

            //查询是否在该hexidx下存在该实体
            auto itor3 = m_entity_hexidx_infos.find(entityulid);
            if(itor3 != m_entity_hexidx_infos.end())
            {
                //更新实体当前的原始hexidx 信息
                std::tuple<H3INDEX, std::unordered_map<UINT64,H3INDEX>,transdata_entityposinfo, bool> &hexidxold = itor3->second;
                std::get<0>(hexidxold) = h3Indexsrc;
                std::get<1>(hexidxold) = std::move(resolution_target_hexidxs);
                std::get<2>(hexidxold) = eninfo;
                std::get<3>(hexidxold) = bSensor;
            }
            else
            {
                //该hexidx 新插入该实体信息
                m_entity_hexidx_infos.insert(std::make_pair(entityulid, std::make_tuple(h3Indexsrc,std::move(resolution_target_hexidxs),std::move(eninfo), bSensor)));
            }
        }
        m_dealfullCond.wakeAll();
    }
#else

//    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//    static UINT64 lasttimestampmap = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//    UINT64 interval = fabs(currentTimeStamp - lasttimestampmap);
//    if(interval > 1000)
//    {
//        lasttimestampmap = currentTimeStamp;
//        std::cout <<"deal pos index "<< eninfo.PARAM_pos_pack_index << std::endl;
//    }

    bool bDeal = true;
    if(!runningmode::RunningModeConfig::getInstance().get_USING_DEAL_ATTACHED_GENERATE() && eninfo.PARAM_attached)
    {
        bDeal = false;
    }
    if(bDeal)
    {
        IDENTIFI_EVENT_INFO eventinfo;

        const FLOAT64 &hgt = eninfo.PARAM_amsl;
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_entity(entityulid,h3Indexsrc,hgt,eninfo,eventinfo);
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_GENERATE_DETECT_EVENT_AFTER_AGENT_STEP_UPDATE_POS())
        {
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                gaeactoreventengine::GaeactorEventEngine::getInstance().refreshEvents_by_cores(entityulid,h3Indexsrc, hgt, eninfo);
//                if(m_event_update_callback)
//                {
//                    m_event_update_callback(eventinfo);
//                }
            }
            else
            {
                if(m_event_update_callback)
                {
                    m_event_update_callback(eventinfo);
                }
            }
        }
    }

    if(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES)
    {
        if(runningmode::RunningModeConfig::getInstance().get_USING_TRANS_MODE_POSDATA_POI())
        {
            m_phexidx_update_callback(entityulid,0,eninfo,E_DISPLAY_MODE_ENTITY);
        }
    }
    else
    {
        m_phexidx_update_callback(entityulid,0,eninfo,E_DISPLAY_MODE_ENTITY);
    }
#endif
}

void GaeactorAgentCoresProcessor::dealPosData(const pos_hexidx &pos_hexidx_data)
{
    //EASY_FUNCTION(profiler::colors::Orange)
    const TYPE_ULID & entityulid = pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid;
    const H3INDEX &h3Indexsrc = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx;
    transdata_entityposinfo eninfo;
    eninfo.PARAM_sensor_property = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_sensor_property;
    eninfo.PARAM_pos_hexidx = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx;
    eninfo.PARAM_longitude = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_longitude;
    eninfo.PARAM_latitude = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_latitude;
    eninfo.PARAM_amsl = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_amsl;
    eninfo.PARAM_ref = pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_ref;
    appenddata(h3Indexsrc, entityulid, std::move(eninfo));
}

void GaeactorAgentCoresProcessor::dealPosattData(const posatt_hexidx &pos_hexidx_data)
{
    EASY_FUNCTION(profiler::colors::Orange)
    const TYPE_ULID & entityulid = pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid;
    const H3INDEX &h3Indexsrc = pos_hexidx_data.PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx;
    transdata_entityposinfo eninfo;
    memcpy(&eninfo, &pos_hexidx_data.PARAM_payload_pos_hexidx, sizeof(transdata_entityposinfo));
    appenddata(h3Indexsrc, entityulid, std::move(eninfo));
}


#ifdef USING_CORES_DEAL_THREAD
bool GaeactorAgentCoresProcessor::isCheckable() const
{
    return m_bChecking.load();
}

void GaeactorAgentCoresProcessor::setCheckEnable(bool newBChecking)
{
    m_bChecking.store(newBChecking);
}

void GaeactorAgentCoresProcessor::refreshIntervenes(int id)
{
    EASY_FUNCTION(profiler::colors::Orange)

    bool bDeal = true;
    {
        QReadLocker locker(&m_entity_hexidx_infos_mutex);
        bDeal = m_entity_hexidx_infos.empty() ? false : true;
    }

    if(!bDeal)
    {
//        LOG_PRINT_STR_EX("thread id: "+ QString::number(id) + " wait entity data "+QString::number(m_entity_hexidx_infos.size()));
        m_dealfullCond.wait(&m_dealmutex);
//        LOG_PRINT_STR_EX("thread id:"+ QString::number(id) + " wacke up entity data  "+QString::number(m_entity_hexidx_infos.size()));
    }
//    else
    {
        bool bValid = false;

        std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, std::unordered_map<UINT64,H3INDEX>,transdata_entityposinfo, bool>> _entity_hexidx_infos;
        {
            QWriteLocker locker(&m_entity_hexidx_infos_mutex);
//            if(!m_entity_hexidx_infos.empty())
//            {
//                LOG_PRINT_STR_EX("thread id: "+ QString::number(id) + " entity before moved SIZE "+QString::number(m_entity_hexidx_infos.size())+ " deal count "+ QString::number(dealcount));
//            }
            auto itor3 = m_entity_hexidx_infos.begin();
            while(itor3 != m_entity_hexidx_infos.end())
            {
                if(_entity_hexidx_infos.size() < PRE_THREAD_MAX_DEAL_SIZE)
                {
                    bValid = true;
                    _entity_hexidx_infos.insert(std::make_pair(itor3->first,std::move(itor3->second)));
                    itor3 = m_entity_hexidx_infos.erase(itor3);
                    continue;
                }
                else
                {
                    break;
                }
                itor3++;
            }
            //            LOG_PRINT_STR_EX("thread id: "+ QString::number(id) + " entity moved SIZE "+QString::number(m_entity_hexidx_infos.size()));
        }
        if(bValid)
        {
//            LOG_PRINT_STR_EX("thread id: "+ QString::number(id) + " to deal "+ QString::number(dealcount)+" deal size "+QString::number(_entity_hexidx_infos.size()) + " leaf size "+QString::number(m_entity_hexidx_infos.size()));
            INTERSECTION_HEXIDX_LIST intersectionHexidx;
            TYPE_ULID entityulid;
            std::unordered_map<UINT64,H3INDEX> resolution_target_hexidxs;
            H3INDEX h3Indexsrc = 0;
            auto itor3 = _entity_hexidx_infos.begin();
            while (itor3 != _entity_hexidx_infos.end())
            {
                //更新实体当前的原始hexidx 信息
                entityulid = itor3->first;
                resolution_target_hexidxs = std::move(std::get<1>(itor3->second));
                h3Indexsrc = std::get<0>(itor3->second);
#ifdef CLEAR_SENSOR_TO_SENSOR_INTERSECTION
                bool &bSensorProperty = std::get<3>(itor3->second);
                if(!bSensorProperty)
#endif
                {
                    gaeactorenvironment::GaeactorProcessor::getInstance().update_hexindex_entity(entityulid,h3Indexsrc,std::get<2>(itor3->second));
                }
#ifdef USING_UI_SHOW
                m_phexidx_update_callback(entityulid,0,std::get<2>(itor3->second),E_DISPLAY_MODE_ENTITY);
#endif
                itor3++;
            }
        }
    }
}

void GaeactorAgentCoresProcessor::data_deal_thread_func(void *pParam)
{
    if (pParam == nullptr)
    {
        return;
    }

    threadParam *pObject = reinterpret_cast<threadParam*>(pParam);
    if(pObject)
    {
        if(m_bChecking.load())
        {
            refreshIntervenes(pObject->id);
#ifdef _MSC_VER
            //            stdutils::OriDateTime::sleep(1);
#else
            stdutils::OriDateTime::sleep(1);
#endif

        }
        else
        {
            //                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            stdutils::OriDateTime::sleep(3000);
        }
    }
}
#endif


void GaeactorAgentCoresProcessor::registDisplayCallback(display_pos_update_callback func)
{
    m_phexidx_update_callback = std::move(func);
}

void GaeactorAgentCoresProcessor::registEventUpdateCallback(event_update_callback func)
{
    m_event_update_callback = std::move(func);
}


GaeactorAgentCoresProcessor &gaeactoragentcores::GaeactorAgentCoresProcessor::getInstance()
{
    static GaeactorAgentCoresProcessor gaeactoragentcoresprocessor;
    return gaeactoragentcoresprocessor;
}
}
