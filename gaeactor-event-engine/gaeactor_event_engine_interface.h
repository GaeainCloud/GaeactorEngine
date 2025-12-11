#ifndef GAEACTOR_EVENT_ENGINE_INTERFACE_H
#define GAEACTOR_EVENT_ENGINE_INTERFACE_H

#include "gaeactor_event_engine_global.h"
#include <QObject>
#include "gaeactor_event_engine_define.h"
#include <QHash>
#include <QList>
#include <QVector>

namespace gaeactoreventengine {


class GAEACTOR_EVENT_ENGINE_EXPORT GaeactorEventEngine : public QObject
{
    Q_OBJECT
public:
    static GaeactorEventEngine & getInstance();
    virtual ~GaeactorEventEngine();


    tagPathInfo getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst);

    void setCheckEnable(bool newBChecking);

    bool isCheckable() const;

    void setEnableDeal(bool bEnbale);

    bool isEnableDeal() const;

    void refreshEvents();
    void refreshEvents_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc,const FLOAT64 &hgt, const transdata_entityposinfo &eninfo);
    void refreshEvents_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret);
    void registEventUpdateCallback(event_update_callback func);
    void registSensorUpdateCallback(sensor_update_callback func);
private:
    explicit GaeactorEventEngine(QObject *parent = nullptr);

};
}
#endif // GAEACTOR_EVENT_ENGINE_INTERFACE_H
