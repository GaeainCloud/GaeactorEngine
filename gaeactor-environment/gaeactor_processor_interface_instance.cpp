#include "gaeactor_processor.h"
#include "gaeactor_processor_interface_instance.h"
#include "gaeactor_processor_normal.h"
#include "runningmodeconfig.h"
namespace gaeactorenvironment
{

GaeactorProcessorInterfaceInstance::GaeactorProcessorInterfaceInstance(QObject *parent)
    :QObject(parent)
    ,m_pGaeactorProcessor_normal(nullptr)
    ,m_pGaeactorProcessor(nullptr)
{
    m_pGaeactorProcessor_normal = new gaeactorenvironment_normal::GaeactorProcessor();
    m_pGaeactorProcessor = new gaeactorenvironment::GaeactorProcessor();
}

GaeactorProcessorInterfaceInstance::~GaeactorProcessorInterfaceInstance()
{
    std::cout<<"GaeactorProcessorInterfaceInstance release"<<std::endl;
    if(m_pGaeactorProcessor_normal)
    {
        delete m_pGaeactorProcessor_normal;
    }
    if(m_pGaeactorProcessor)
    {
        delete m_pGaeactorProcessor;
    }
}
void GaeactorProcessorInterfaceInstance::update_hexindex_entity(const TYPE_ULID& uildsrc,
                                                        const H3INDEX& new_h3Index,
                                                        const FLOAT64 &hgt,
                                                        const transdata_entityposinfo& _entityinfo,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->update_hexindex_entity(uildsrc, new_h3Index, hgt, _entityinfo);
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->update_hexindex_entity(uildsrc, new_h3Index, hgt,_entityinfo, eventinfo);
        }
    }
}
void GaeactorProcessorInterfaceInstance::update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                                        const HEXIDX_HGT_ARRAY  &_hexidxs,
                                                        transdata_sensorposinfo&& _sensorinfo,
                                                        POLYGON_LIST &&_polygon,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon));
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon), eventinfo);
        }
    }
}

void GaeactorProcessorInterfaceInstance::force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs, transdata_sensorposinfo &&_sensorinfo, POLYGON_LIST &&_polygon)
{
    if(m_pGaeactorProcessor)
    {
        m_pGaeactorProcessor->force_cover_update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon));
    }
}

void GaeactorProcessorInterfaceInstance::force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist, const UINT32 &_silent_time)
{
    if(m_pGaeactorProcessor)
    {
        m_pGaeactorProcessor->force_cover_deal_hexindex_sensor(sensorulid, sensingmediaid, hexidxslist,_silent_time);
    }
}

void GaeactorProcessorInterfaceInstance::reset()
{
    if(m_pGaeactorProcessor)
    {
        m_pGaeactorProcessor->reset();
        m_pGaeactorProcessor->init();
    }
}

void GaeactorProcessorInterfaceInstance::record_sensor_overlap(bool brecord)
{
    if(m_pGaeactorProcessor)
    {
        m_pGaeactorProcessor->getBuffer().record_sensor_overlap(brecord);
    }
}

std::vector<std::tuple<H3INDEX, bool, uint32_t> > GaeactorProcessorInterfaceInstance::getCellbuffers()
{
    return m_pGaeactorProcessor->getBuffer().getCellbuffers();
}

std::vector<HEXINDEX_SENSORS_ENTITYS_INFO>   GaeactorProcessorInterfaceInstance::getCellbuffersInfo()
{
    return m_pGaeactorProcessor->getBuffer().getCellbuffersInfo();
}

std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> GaeactorProcessorInterfaceInstance::getCellbuffersSensorInfo(bool bAll)
{
    return m_pGaeactorProcessor->getBuffer().getCellbuffersSensorInfo(bAll);
}

std::vector<std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID, transdata_param_seq_hexidx_hgt> >, std::vector<std::tuple<TYPE_ULID, FLOAT64> > > > GaeactorProcessorInterfaceInstance::getCellbuffersTargetSensorInfo(const HEXIDX_ARRAY &hexidxslist, bool bAll)
{
    return m_pGaeactorProcessor->getBuffer().getCellbuffersTargetSensorInfo(hexidxslist);
}

void GaeactorProcessorInterfaceInstance::refersh_silent_timeout()
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->refersh_silent_timeout();
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->refersh_silent_timeout();
        }
    }
}


void GaeactorProcessorInterfaceInstance::refersh_event(IDENTIFI_EVENT_INFO &identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->refersh_event(identifi_event_info);
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->refersh_event(identifi_event_info);
        }
    }
}

void GaeactorProcessorInterfaceInstance::refersh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo, IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->refersh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, identifi_event_info);
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->refersh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, identifi_event_info);
        }
    }
}

void GaeactorProcessorInterfaceInstance::refersh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(m_pGaeactorProcessor_normal)
        {
            m_pGaeactorProcessor_normal->refersh_events_by_sensors(sensorid, sensingmedia_id, hexidxslistret, identifi_event_info);
        }
    }
    else
    {
        if(m_pGaeactorProcessor)
        {
            m_pGaeactorProcessor->refersh_events_by_sensors(sensorid, sensingmedia_id, hexidxslistret, identifi_event_info);
        }
    }
}


}
