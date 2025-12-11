#include "gaeactor_agent_sensors_processor.h"
#include <QDebug>
#include "gaeactor_event_engine_interface.h"
#include "easy/profiler.h"
#include "src/OriginalDateTime.h"
#include "src/OriginalThread.h"
#include "runningmodeconfig.h"

#ifdef USING_SENSORS_DEAL_THREAD
#define THREAD_EXIT_MAX_TIME (3000)
#define PRE_THREAD_MAX_DEAL_SIZE (300)
#endif
namespace gaeactoragentsensors
{
GaeactorAgentSensorsProcessor::GaeactorAgentSensorsProcessor(QObject *parent)
{
#ifdef USING_SENSORS_DEAL_THREAD
    m_bChecking.store(true);
    for(int i = 0 ; i< USING_THREAD_NUM; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAgentSensorsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAgentSensorsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                         &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();
    }
#endif
}

GaeactorAgentSensorsProcessor::~GaeactorAgentSensorsProcessor()
{
#ifdef USING_SENSORS_DEAL_THREAD
    m_dealfullCond.wakeAll();

    //清理DataDealThread
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

void GaeactorAgentSensorsProcessor::dealWaveData(const wave_smd_hexidx &wave_smd_hexidx_data)
{
    EASY_FUNCTION(profiler::colors::Yellow)
    const TYPE_ULID & sensorulid = wave_smd_hexidx_data.PARAM_protocol_head.PARAM_source_ulid;
    transdata_sensorposinfo _sensorinfo;
    memcpy(&_sensorinfo, &wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx,sizeof(transdata_sensorposinfo));

    TYPE_ULID &sensingmediaid = _sensorinfo.PARAM_source_sensingmediaid;

    UINT32 iBufferSize = wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count;

    payload_hexidx *ppayload_hexidx = nullptr;
    payload_polygon *ppayload_polygon = nullptr;

    HEXIDX_HGT_ARRAY hexidxslist;
    std::vector<param_seq_polygon> polygon_list;
    if(iBufferSize != 0)
    {
        BYTE* pBuf = (BYTE*)wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_seq_buffer;
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        UINT16  PARAM_qty_hexidx = GET_HEXIDX_NUM(pBuf);
        if(PARAM_qty_hexidx != 0)
        {
            ppayload_hexidx = GET_HEXIDX_STRUCT_PTR(pBuf);
            hexidxslist.resize(ppayload_hexidx->PARAM_qty_hexidx);
            memcpy(hexidxslist.data(), ppayload_hexidx->PARAM_seq_hexidx, sizeof(transdata_param_seq_hexidx) * ppayload_hexidx->PARAM_qty_hexidx);
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        UINT16  PARAM_qty_polygon = GET_POLYGON_NUM(pBuf);
        if(PARAM_qty_polygon != 0)
        {
            ppayload_polygon = GET_POLYGON_STRUCT_PTR(pBuf);
            polygon_list.resize(ppayload_polygon->PARAM_qty_polygon);
            memcpy(polygon_list.data(), ppayload_polygon->PARAM_seq_polygon, sizeof(transdata_param_seq_polygon) * ppayload_polygon->PARAM_qty_polygon);
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    }



#ifdef USING_SENSORS_DEAL_THREAD
    QPair<TYPE_ULID,TYPE_ULID>  sensor_sensingmedia_id_pair = qMakePair(sensorulid, sensingmediaid);
    {
        QWriteLocker locker2(&m_sensors_hexidx_infos_mutex);
        //查询是否在该hexidx下存在该实体
        auto itor3 = m_sensors_hexidx_infos.find(sensor_sensingmedia_id_pair);
        if(itor3 != m_sensors_hexidx_infos.end())
        {
            //更新实体当前的原始hexidx 信息
            std::tuple<HEXIDX_HGT_ARRAY,transdata_sensorposinfo,std::vector<param_seq_polygon>> &hexidxold = itor3->second;
            std::get<0>(hexidxold) = std::move(hexidxslist);
            std::get<1>(hexidxold) = std::move(_transdata_sensorposinfo);
            std::get<2>(hexidxold) = std::move(polygon_list);
        }
        else
        {
            //该hexidx 新插入该实体信息
            m_sensors_hexidx_infos.insert(std::make_pair(sensor_sensingmedia_id_pair, std::make_tuple(std::move(hexidxslist), std::move(_transdata_sensorposinfo), std::move(polygon_list))));
        }
    }
    m_dealfullCond.wakeAll();
#else
    //更新实体当前的原始hexidx 信息
    sensingmediaid = (sensingmediaid == 0)?sensorulid:sensingmediaid;
    /////////////////////////////////////////////////////////////////////////////
//    auto usage = (int)_sensorinfo.PARAM_wave_usage;
//    QString cctrace;
//    switch (usage)
//    {
//    case 0x00:cctrace = "UNDEFINED"; break;
//    case 0x01:cctrace = "ECHOSENSE"; break;
//    case 0x02:cctrace = "INTFERNCE"; break;
//    case 0x03:cctrace = "SUPRESSOR"; break;
//    case 0x04:cctrace = "LASERBURN"; break;
//    case 0x05:cctrace = "VIBRATION"; break;
//    case 0x06:cctrace = "MCRWVBURN"; break;
//    case 0x07:cctrace = "SHOCKWAVE"; break;
//    case 0x08:cctrace = "DETECTION"; break;
//    case 0x09:cctrace = "EXPLOSION"; break;
//    case 0xfd-1:cctrace = "SPACEOVLP-1"; break;
//    case 0xfd:cctrace = "SPACEOVLP"; break;
//    case 0xfe:cctrace = "__COMM___"; break;
//    case 0xff:cctrace = "_INFRARED"; break;
//    default:
//        int cc = 0;
//        cc = 1;
//        break;
//    }

//    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//    static UINT64 lasttimestampmap = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//    UINT64 interval = fabs(currentTimeStamp - lasttimestampmap);
//    //if(usage == 0x01)
//    {
//        lasttimestampmap = currentTimeStamp;
//        std::cout <<_sensorinfo.PARAM_source_sensingmediaid<<" usage "<< cctrace.toStdString()<<" deal sensor index "<< _sensorinfo.PARAM_sensor_pack_index << " hexindex size:" << hexidxslist.size() << " polygonlist size:" << polygon_list.size() <<" "<<_sensorinfo.PARAM_timestamp<<" "<<currentTimeStamp - _sensorinfo.PARAM_timestamp<< std::endl;
//    }

//    HEXIDX_ARRAY hexidxslistret;
//    hexidxslistret.resize(hexidxslist.size());
//    for (int i = 0; i < hexidxslist.size(); i++)
//    {
//        hexidxslistret[i] = (hexidxslist.at(i).PARAM_seq_hexidx_element);
//    }
    IDENTIFI_EVENT_INFO eventinfo;
    m_phexidx_update_callback(sensorulid,sensingmediaid, hexidxslist, polygon_list, _sensorinfo, E_DISPLAY_MODE_WAVE);
    bool bDeal = true;

    if((_sensorinfo.PARAM_wave_usage == 0xFD - 1) || (runningmode::RunningModeConfig::getInstance().get_MODE_USING_ECHO_GENERATE_DIRECT() && _sensorinfo.PARAM_wave_usage == 0x01))
    {
        bDeal = false;
    }

    if(bDeal)
    {
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_sensor(sensorulid,sensingmediaid,hexidxslist,std::move(_sensorinfo),std::move(polygon_list),eventinfo);
    }

    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_GENERATE_DETECT_EVENT_AFTER_AGENT_STEP_UPDATE_SENSOR())
    {
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            if(_sensorinfo.PARAM_wave_usage == 0x08)
            {
                gaeactoreventengine::GaeactorEventEngine::getInstance().refreshEvents_by_sensors(sensorulid,sensingmediaid,hexidxslist);
//                if(m_event_update_callback)
//                {
//                    m_event_update_callback(eventinfo);
//                }
            }
        }
        else
        {
            if(m_event_update_callback)
            {
                m_event_update_callback(eventinfo);
            }
        }
    }


#endif
}

#ifdef USING_SENSORS_DEAL_THREAD
bool GaeactorAgentSensorsProcessor::isCheckable() const
{
    return m_bChecking.load();
}

void GaeactorAgentSensorsProcessor::setCheckEnable(bool newBChecking)
{
    m_bChecking.store(newBChecking);
}

void GaeactorAgentSensorsProcessor::refreshIntervenes(int id)
{
    EASY_FUNCTION(profiler::colors::Green)
    bool bDeal = true;
    {
        QReadLocker locker(&m_sensors_hexidx_infos_mutex);
        bDeal = m_sensors_hexidx_infos.empty() ? false : true;
    }

    if(!bDeal)
    {
        m_dealfullCond.wait(&m_dealmutex);
    }
//    else
    {
        bool bValid = false;

        std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>,std::tuple<HEXIDX_HGT_ARRAY,transdata_sensorposinfo,std::vector<param_seq_polygon>>> _sensor_hexidx_infos;
        {
            QWriteLocker locker(&m_sensors_hexidx_infos_mutex);
            auto itor3 = m_sensors_hexidx_infos.begin();
            while(itor3 != m_sensors_hexidx_infos.end())
            {
                if(_sensor_hexidx_infos.size() < PRE_THREAD_MAX_DEAL_SIZE)
                {
                    bValid = true;
                    _sensor_hexidx_infos.insert(std::make_pair(itor3->first,std::move(itor3->second)));
                    itor3 = m_sensors_hexidx_infos.erase(itor3);
                    continue;
                }
                else
                {
                    break;
                }
                itor3++;
            }
        }
        if(bValid)
        {
            TYPE_ULID sensorulid;
            TYPE_ULID sensingmediaid;
            auto itor3 = _sensor_hexidx_infos.begin();
            while (itor3 != _sensor_hexidx_infos.end())
            {
                //更新实体当前的原始hexidx 信息
                sensorulid = itor3->first.first;
                sensingmediaid = itor3->first.second;
                sensingmediaid = (sensingmediaid == 0)?sensorulid:sensingmediaid;

                const HEXIDX_HGT_ARRAY &hexidxslist = std::get<0>(itor3->second);
                /////////////////////////////////////////////////////////////////////////////

//                UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//                static std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>, UINT64> timestampmap;

//                QPair<TYPE_ULID,TYPE_ULID>  pp = qMakePair(sensorulid, sensingmediaid);
//                auto timestampmap_itor = timestampmap.find(pp);
//                if(timestampmap_itor != timestampmap.end())
//                {
//                    std::cout <<sensingmediaid<< " ++++++++++++++++deal sensingmedia "<<fabs(currentTimeStamp - timestampmap_itor->second)<<" ms "/*<< hexidxslist.size() */<<std::endl;
//                    timestampmap_itor->second = currentTimeStamp;
//                }
//                else
//                {
//                    timestampmap.insert(std::make_pair(pp, currentTimeStamp));
//                }
                /////////////////////////////////////////////////////////////////////////////
                const transdata_sensorposinfo &_sensorinfo = std::get<1>(itor3->second);
                const std::vector<transdata_param_seq_polygon>& _polygon = std::get<2>(itor3->second);
                if(_sensorinfo.PARAM_wave_usage == 0xFD - 1)
                {
                    HEXIDX_ARRAY hexidxslistret;
                    hexidxslistret.resize(hexidxslist.size());
                    for (int i = 0; i < hexidxslist.size(); i++)
                    {
                        hexidxslistret[i] = (hexidxslist.at(i).PARAM_seq_hexidx_element);
                    }
                    m_phexidx_update_callback(sensorulid,sensingmediaid, hexidxslistret, _polygon, _sensorinfo, E_DISPLAY_MODE_WAVE);
                    if(hexidxslistret.size() == 0)
                    {
                        std::cout <<sensingmediaid<< " +++++ "<< hexidxslist.size() <<std::endl;
                    }
                }
                else
                {
                    {
                        gaeactorenvironment::GaeactorProcessor::getInstance().update_hexindex_sensor(sensorulid,sensingmediaid,hexidxslist,_sensorinfo,_polygon);
                        //                        std::cout.precision(17);
                        //                        std::cout << sensingmediaid<< " "<<std::get<0>(hexidxslistret).size()<< " "<<std::get<1>(hexidxslistret).size()<< " "<<(int)std::get<2>(hexidxslistret).PARAM_wave_usage <<std::endl;

                        HEXIDX_ARRAY hexidxslistretTmp;

                        hexidxslistretTmp.resize(hexidxslist.size());
                        for (int i = 0; i < hexidxslist.size(); i++)
                        {
                            hexidxslistretTmp[i] = (hexidxslist.at(i).PARAM_seq_hexidx_element);
                        }
                        transdata_sensorposinfo _sensorinfotmp = _sensorinfo;
                        if(hexidxslistretTmp.empty())
                        {
                            _sensorinfotmp = gaeactorenvironment::GaeactorProcessor::getInstance().gettransdata_sensorposinfo_by_sensingmedia(sensorulid,sensingmediaid);
                        }
                        m_phexidx_update_callback(sensorulid,sensingmediaid, hexidxslistretTmp, _polygon,  _sensorinfotmp, E_DISPLAY_MODE_WAVE);
                    }
                }
                itor3++;
            }
        }
    }
}

void GaeactorAgentSensorsProcessor::data_deal_thread_func(void * pParam)
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
    return;
}
#endif

void GaeactorAgentSensorsProcessor::registDisplayCallback(display_hexidx_update_callback func)
{
    m_phexidx_update_callback = std::move(func);
}

void GaeactorAgentSensorsProcessor::registEventUpdateCallback(event_update_callback func)
{
    m_event_update_callback = std::move(func);
}

GaeactorAgentSensorsProcessor &GaeactorAgentSensorsProcessor::getInstance()
{
    static GaeactorAgentSensorsProcessor gaeactoragentsensorsprocessor;
    return gaeactoragentsensorsprocessor;
}
}
