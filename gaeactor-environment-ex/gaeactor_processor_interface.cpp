#include "gaeactor_processor_interface.h"
#include "gaeactor_processor_interface_instance.h"
namespace gaeactorenvironment_ex
{
GaeactorProcessorInterface& GaeactorProcessorInterface::getInstance()
{
    static GaeactorProcessorInterface gaeactorprocessor;
    return gaeactorprocessor;
}
GaeactorProcessorInterface::GaeactorProcessorInterface()
    :m_pGaeactorProcessorInterfaceInstance(nullptr)
{
    m_pGaeactorProcessorInterfaceInstance = new GaeactorProcessorInterfaceInstance();
}

GaeactorProcessorInterfaceInstance *GaeactorProcessorInterface::pGaeactorProcessorInterfaceInstance() const
{
    return m_pGaeactorProcessorInterfaceInstance;
}

GaeactorProcessorInterface::~GaeactorProcessorInterface()
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        delete m_pGaeactorProcessorInterfaceInstance;
    }
}
void GaeactorProcessorInterface::update_hexindex_entity(const TYPE_ULID& uildsrc,
                                                        const H3INDEX& new_h3Index,
                                                        const FLOAT64 &hgt,
                                                        const transdata_entityposinfo& _entityinfo,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->update_hexindex_entity(uildsrc, new_h3Index, hgt, _entityinfo, eventinfo);
    }
}
void GaeactorProcessorInterface::update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                                        const HEXIDX_HGT_ARRAY  &_hexidxs,
                                                        transdata_sensorposinfo&& _sensorinfo,
                                                        POLYGON_LIST &&_polygon,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon), eventinfo);

    }
}

void GaeactorProcessorInterface::force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs, transdata_sensorposinfo &&_sensorinfo, POLYGON_LIST &&_polygon)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->force_cover_update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon));
    }
}

void GaeactorProcessorInterface::force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist, const UINT32 &_silent_time)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->force_cover_update_hexindex_sensor_ex(sensorulid, sensingmediaid, hexidxslist,_silent_time);
    }
}

void GaeactorProcessorInterface::reset()
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->reset();
    }
}

void GaeactorProcessorInterface::record_sensor_overlap(bool brecord)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->record_sensor_overlap(brecord);
    }
}

std::vector<std::tuple<H3INDEX, bool, uint32_t> > GaeactorProcessorInterface::getCellbuffers()
{
    return m_pGaeactorProcessorInterfaceInstance->getCellbuffers();
}

std::vector<HEXINDEX_SENSORS_ENTITYS_INFO>   GaeactorProcessorInterface::getCellbuffersInfo()
{
    return m_pGaeactorProcessorInterfaceInstance->getCellbuffersInfo();
}

std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> GaeactorProcessorInterface::getCellbuffersSensorInfo(bool bAll)
{
    return m_pGaeactorProcessorInterfaceInstance->getCellbuffersSensorInfo(bAll);
}

std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> GaeactorProcessorInterface::getCellbuffersTargetSensorInfo(const HEXIDX_ARRAY &hexidxslist, bool bAll)
{
    return m_pGaeactorProcessorInterfaceInstance->getCellbuffersTargetSensorInfo(hexidxslist,bAll);
}

void GaeactorProcessorInterface::refresh_silent_timeout()
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->refresh_silent_timeout();
    }
}


void GaeactorProcessorInterface::refresh_event(IDENTIFI_EVENT_INFO &identifi_event_info)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->refresh_event(identifi_event_info);
    }
}

void GaeactorProcessorInterface::refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo, IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->refresh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, identifi_event_info);
    }
}

void GaeactorProcessorInterface::refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(m_pGaeactorProcessorInterfaceInstance)
    {
        m_pGaeactorProcessorInterfaceInstance->refresh_events_by_sensors(sensorid, sensingmedia_id, hexidxslistret, identifi_event_info);
    }
}


}
