#include "gaeactor_processor_interface.h"
#include "gaeactor_processor.h"
#include "gaeactor_processor_normal.h"
#include "runningmodeconfig.h"
namespace gaeactorenvironment
{

GaeactorProcessorInterface& GaeactorProcessorInterface::getInstance()
{
    static GaeactorProcessorInterface gaeactorprocessor;
    return gaeactorprocessor;
}

GaeactorProcessorInterface::~GaeactorProcessorInterface()
{

}


void GaeactorProcessorInterface::update_hexindex_entity(const TYPE_ULID& uildsrc,
                                                        const H3INDEX& new_h3Index,
                                                        const FLOAT64 &hgt,
                                                        const transdata_entityposinfo& _entityinfo,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().update_hexindex_entity(uildsrc, new_h3Index, hgt, _entityinfo);

    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().update_hexindex_entity(uildsrc, new_h3Index, hgt,_entityinfo, eventinfo);

    }
}
void GaeactorProcessorInterface::update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                                        const HEXIDX_HGT_ARRAY  &_hexidxs,
                                                        transdata_sensorposinfo&& _sensorinfo,
                                                        POLYGON_LIST &&_polygon,
                                                        IDENTIFI_EVENT_INFO& eventinfo)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon));
    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon), eventinfo);
    }
}


std::vector<UINT8> GaeactorProcessorInterface::get_using_resolutions()
{
    if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        return gaeactorenvironment_normal::GaeactorProcessor::getInstance().get_using_resolutions();
    }
    return gaeactorenvironment::GaeactorProcessor::getInstance().get_using_resolutions();
}

UINT8 GaeactorProcessorInterface::get_using_max_resolution()
{
    if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        return gaeactorenvironment_normal::GaeactorProcessor::getInstance().get_using_max_resolution();
    }
    return gaeactorenvironment::GaeactorProcessor::getInstance().get_using_max_resolution();
}

UINT8 GaeactorProcessorInterface::get_using_min_resolution()
{
    if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        return gaeactorenvironment_normal::GaeactorProcessor::getInstance().get_using_min_resolution();
    }
    return gaeactorenvironment::GaeactorProcessor::getInstance().get_using_min_resolution();
}

void GaeactorProcessorInterface::force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &_hexidxs, transdata_sensorposinfo &&_sensorinfo, POLYGON_LIST &&_polygon)
{
    gaeactorenvironment::GaeactorProcessor::getInstance().force_cover_update_hexindex_sensor(sensorulid, sensingmediaid, _hexidxs, std::move(_sensorinfo),  std::move(_polygon));

}

void GaeactorProcessorInterface::force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist, const UINT32 &_silent_time)
{
    gaeactorenvironment::GaeactorProcessor::getInstance().force_cover_deal_hexindex_sensor(sensorulid, sensingmediaid, hexidxslist,_silent_time);

}

void GaeactorProcessorInterface::reset()
{
    gaeactorenvironment::H3IndexBufferManager::getInstance().reset();
}

std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t> > GaeactorProcessorInterface::getCellbuffers()
{
    return gaeactorenvironment::H3IndexBufferManager::getInstance().getCellbuffers();
}

std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>>>   GaeactorProcessorInterface::getCellbuffersInfo()
{
    return gaeactorenvironment::H3IndexBufferManager::getInstance().getCellbuffersInfo();
}

void GaeactorProcessorInterface::refresh_silent_timeout()
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().refresh_silent_timeout();
    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().refresh_silent_timeout();
    }
}


void GaeactorProcessorInterface::refresh_event(IDENTIFI_EVENT_INFO &identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().refresh_event(identifi_event_info);
    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().refresh_event(identifi_event_info);
    }
}

void GaeactorProcessorInterface::refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo& eninfo, IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().refresh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, identifi_event_info);
    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().refresh_events_by_cores(entityid, h3Indexsrc, hgt, eninfo, identifi_event_info);
    }
}

void GaeactorProcessorInterface::refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& identifi_event_info)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        gaeactorenvironment_normal::GaeactorProcessor::getInstance().refresh_events_by_sensors(sensorid, sensingmedia_id, hexidxslistret, identifi_event_info);
    }
    else
    {
        gaeactorenvironment::GaeactorProcessor::getInstance().refresh_events_by_sensors(sensorid, sensingmedia_id, hexidxslistret, identifi_event_info);
    }
}

GaeactorProcessorInterface::GaeactorProcessorInterface()
{
}

}
