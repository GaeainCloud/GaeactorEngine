#ifndef GAEACTOR_AGENT_SENSORS_PROCESSOR_H
#define GAEACTOR_AGENT_SENSORS_PROCESSOR_H
#include "gaeactor_processor_interface.h"

#include <QObject>
#include "gaeactor_agent_sensors_define.h"
#include <QHash>
#include <QList>
#include <QReadWriteLock>
#include "src/OriginalMutex.h"

//#define USING_SENSORS_DEAL_THREAD

namespace stdutils {
class OriThread;
}

namespace gaeactoragentsensors
{
#define USING_THREAD_NUM SENSORS_THREAD_NUM

class GaeactorAgentSensorsProcessor : public QObject
{
    Q_OBJECT
public:

    static GaeactorAgentSensorsProcessor & getInstance();
    virtual ~GaeactorAgentSensorsProcessor();
    void dealWaveData(const wave_smd_hexidx &wave_smd_hexidx_data);
    void registDisplayCallback(display_hexidx_update_callback func);
    void registEventUpdateCallback(event_update_callback func);
private:
    explicit GaeactorAgentSensorsProcessor(QObject *parent = nullptr);
#ifdef USING_SENSORS_DEAL_THREAD
public:
    bool isCheckable() const;
    void setCheckEnable(bool newBChecking);
private:
    void refreshIntervenes(int id);
    void data_deal_thread_func(void *pParam);
#endif
private:
    display_hexidx_update_callback m_phexidx_update_callback;
    event_update_callback m_event_update_callback;

#ifdef USING_SENSORS_DEAL_THREAD
    QReadWriteLock m_sensors_hexidx_infos_mutex;
    std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>,std::tuple<HEXIDX_HGT_ARRAY,transdata_sensorposinfo,std::vector<param_seq_polygon>>> m_sensors_hexidx_infos;

    std::atomic_bool m_bChecking{false};
    struct threadParam
    {
        int id;
    }m_hDataDealThreadParam[USING_THREAD_NUM];
    stdutils::OriThread* m_hDataDealThread[USING_THREAD_NUM];

    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;
#endif
};
}
#endif // GAEACTOR_AGENT_CORES_PROCESSOR_H
