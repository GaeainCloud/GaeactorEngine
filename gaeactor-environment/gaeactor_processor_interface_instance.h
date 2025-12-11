#ifndef GAEACTOR_PROCESSOR_INTERFACE_INSTANCE_H
#define GAEACTOR_PROCESSOR_INTERFACE_INSTANCE_H
#include "head_define.h"
#include "gaeactor_environment_global.h"
#include "gaeactor_environment_define.h"
#include <QObject>


namespace gaeactorenvironment_normal {
class GaeactorProcessor;
};

namespace gaeactorenvironment {
class GaeactorProcessor;
};

namespace gaeactorenvironment {

class GAEACTOR_ENVIRONMENT_EXPORT GaeactorProcessorInterfaceInstance:public QObject
{
    Q_OBJECT
public:
    explicit GaeactorProcessorInterfaceInstance(QObject *parent=nullptr);
    virtual ~GaeactorProcessorInterfaceInstance();
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

    void record_sensor_overlap(bool brecord);

    std::vector<std::tuple<H3INDEX, bool, uint32_t>> getCellbuffers();
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO>  getCellbuffersInfo();
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO>  getCellbuffersSensorInfo(bool bAll = true);
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> getCellbuffersTargetSensorInfo(const HEXIDX_ARRAY& hexidxslist,bool bAll = true);

    void refersh_silent_timeout();

    void refersh_event(IDENTIFI_EVENT_INFO& eventinfo);

    void refersh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo &eninfo, IDENTIFI_EVENT_INFO& identifi_event_info);
    void refersh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret,IDENTIFI_EVENT_INFO& eventinfo);
private:
    gaeactorenvironment_normal::GaeactorProcessor *m_pGaeactorProcessor_normal;
    gaeactorenvironment::GaeactorProcessor *m_pGaeactorProcessor;
};
}

#endif // GAEACTOR_PROCESSOR_INTERFACE_H
