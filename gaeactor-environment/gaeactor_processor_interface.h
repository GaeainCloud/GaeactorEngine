#ifndef GAEACTOR_PROCESSOR_INTERFACE_H
#define GAEACTOR_PROCESSOR_INTERFACE_H
#include "head_define.h"
#include "gaeactor_environment_global.h"


namespace gaeactorenvironment {

class GAEACTOR_ENVIRONMENT_EXPORT GaeactorProcessorInterface
{
public:
    static GaeactorProcessorInterface& getInstance();
    virtual ~GaeactorProcessorInterface();
    void update_hexindex_entity(const TYPE_ULID& uildsrc,
                                const H3INDEX& h3,
                                const FLOAT64 &hgt,
                                const transdata_entityposinfo& _entityinfo,
                                IDENTIFI_EVENT_INFO& eventinfo);

    void update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                const HEXIDX_HGT_ARRAY  &_hexidxs,
                                transdata_sensorposinfo&& _sensorinfo,
                                POLYGON_LIST &&_polygon,
                                IDENTIFI_EVENT_INFO& eventinfo);


    void force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid,
                                            const HEXIDX_HGT_ARRAY &_hexidxs,
                                            transdata_sensorposinfo &&_sensorinfo,
                                            POLYGON_LIST &&_polygon);

    void force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid,
                                             const HEXIDX_HGT_ARRAY &hexidxslist,
                                             const UINT32 &_silent_time);

    void reset();

    std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t>> getCellbuffers();
    std::unordered_map<UINT64, std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>>>  getCellbuffersInfo();

    void refresh_silent_timeout();

    void refresh_event(IDENTIFI_EVENT_INFO& eventinfo);

    void refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo &eninfo, IDENTIFI_EVENT_INFO& identifi_event_info);
    void refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& eventinfo);

    std::vector<UINT8> get_using_resolutions();
    UINT8 get_using_max_resolution();
    UINT8 get_using_min_resolution();
private:
    explicit GaeactorProcessorInterface();
};
}

#endif // GAEACTOR_PROCESSOR_INTERFACE_H
