#include "gaeactor_event_engine_processor.h"

#include <QDebug>
#include "gaeactor_event_engine_interface.h"
#include "src/OriginalDateTime.h"
#include "src/OriginalThread.h"
#include "loghelper.h"
#include <sstream>
#include "easy/profiler.h"
#include "runningmodeconfig.h"
#define THREAD_EXIT_MAX_TIME (3000)
namespace gaeactoreventengine
{
GaeactorEventEngineProcessor::GaeactorEventEngineProcessor(QObject *parent)
{
    m_bChecking.store(false);
    m_needdeal.store(false);

    for(int i = 0 ; i< USING_THREAD_NUM; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorEventEngineProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                        &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorEventEngineProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                        &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();
    }

    setCheckEnable(true);
}

GaeactorEventEngineProcessor::~GaeactorEventEngineProcessor()
{
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
}

tagPathInfo GaeactorEventEngineProcessor::getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst)
{
    return tagPathInfo();
}

void GaeactorEventEngineProcessor::setCheckEnable(bool newBChecking)
{
    m_bChecking.store(newBChecking);
}

bool GaeactorEventEngineProcessor::isCheckable() const
{
    return m_bChecking.load();
}

void GaeactorEventEngineProcessor::setEnableDeal(bool bEnbale)
{
    m_needdeal.store(bEnbale);
    if(bEnbale)
    {
        m_dealfullCond.wakeAll();
    }
}

bool GaeactorEventEngineProcessor::isEnableDeal()
{
    return m_needdeal.load();
}

void GaeactorEventEngineProcessor::refreshEvents()
{    
    EASY_FUNCTION(profiler::colors::Green)
    IDENTIFI_EVENT_INFO eventinfo;
    gaeactorenvironment::GaeactorProcessorInterface::getInstance().refresh_event(eventinfo);
    if(m_event_update_callback)
    {
        m_event_update_callback(eventinfo);
    }

}

void GaeactorEventEngineProcessor::refreshEvents_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo)
{
    EASY_FUNCTION(profiler::colors::Green)
    IDENTIFI_EVENT_INFO eventinfo;
    gaeactorenvironment::GaeactorProcessorInterface::getInstance().refresh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, eventinfo);
    if(m_event_update_callback)
    {
        m_event_update_callback(eventinfo);
    }
}

void GaeactorEventEngineProcessor::refreshEvents_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret)
{
    EASY_FUNCTION(profiler::colors::Green)
    IDENTIFI_EVENT_INFO eventinfo;
    gaeactorenvironment::GaeactorProcessorInterface::getInstance().refresh_events_by_sensors(sensorid,sensingmediaid,hexidxslistret,eventinfo);
    if(m_event_update_callback)
    {
        m_event_update_callback(eventinfo);
    }
}

void GaeactorEventEngineProcessor::registEventUpdateCallback(event_update_callback func)
{
    m_event_update_callback = std::move(func);
}

void GaeactorEventEngineProcessor::registSensorUpdateCallback(sensor_update_callback func)
{
}

GaeactorEventEngineProcessor &GaeactorEventEngineProcessor::getInstance()
{
    static GaeactorEventEngineProcessor gaeactoreventengineprocessor;
    return gaeactoreventengineprocessor;
}

void GaeactorEventEngineProcessor::data_deal_thread_func(void *pParam)
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
            if(!m_needdeal.load())
            {
                m_dealfullCond.wait(&m_dealmutex);
            }
            refreshEvents();
#ifdef _MSC_VER
            stdutils::OriDateTime::sleep(1);
#else
            stdutils::OriDateTime::sleep(16);
#endif
        }
        else
        {
            stdutils::OriDateTime::sleep(3000);
        }
    }
    return;
}

}
