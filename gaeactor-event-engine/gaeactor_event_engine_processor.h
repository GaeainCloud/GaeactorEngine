#ifndef GAEACTOR_EVENT_ENGINE_PROCESSOR_H
#define GAEACTOR_EVENT_ENGINE_PROCESSOR_H
#include "gaeactor_processor_interface.h"

#include <QObject>
#include "gaeactor_event_engine_define.h"
#include <QHash>
#include <QList>

#include <iostream>
#include <thread>
#include "src/OriginalMutex.h"

namespace stdutils {
class OriThread;
}
namespace gaeactoreventengine
{
#define USING_THREAD_NUM EVENTS_THREAD_NUM
class GaeactorEventEngineProcessor : public QObject
{
    Q_OBJECT
public:
    static GaeactorEventEngineProcessor & getInstance();
    virtual ~GaeactorEventEngineProcessor();

    tagPathInfo getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst);

    void refreshEvents();

    void refreshEvents_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo &eninfo);
    void refreshEvents_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret);

    void registEventUpdateCallback(event_update_callback func);

    void registSensorUpdateCallback(sensor_update_callback func);

    void setCheckEnable(bool newBChecking);

    bool isCheckable() const;

    void setEnableDeal(bool bEnbale);

    bool isEnableDeal();
private:
    explicit GaeactorEventEngineProcessor(QObject *parent = nullptr);
    void data_deal_thread_func(void *pParam);
private:
    event_update_callback m_event_update_callback;
    std::atomic_bool m_bChecking{false};
    struct threadParam
    {
        int id;
    }m_hDataDealThreadParam[USING_THREAD_NUM];
    stdutils::OriThread* m_hDataDealThread[USING_THREAD_NUM];

    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;
    std::atomic<bool> m_needdeal;
};
}
#endif // GAEACTOR_EVENT_ENGINE_PROCESSOR_H
