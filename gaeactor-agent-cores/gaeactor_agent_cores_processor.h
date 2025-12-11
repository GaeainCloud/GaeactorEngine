#ifndef GAEACTOR_AGENT_CORES_PROCESSOR_H
#define GAEACTOR_AGENT_CORES_PROCESSOR_H

#include "gaeactor_processor_interface.h"
#include <QObject>
#include "gaeactor_agent_cores_define.h"
#include <QHash>
#include <QList>
#include <iostream>
#include <thread>
#include <QMutex>
#include <QReadWriteLock>
#include "src/OriginalMutex.h"

//#define USING_CORES_DEAL_THREAD

namespace stdutils {
class OriThread;
}
namespace gaeactoragentcores
{
#define USING_THREAD_NUM CORES_THREAD_NUM

class GaeactorAgentCoresProcessor : public QObject
{
    Q_OBJECT
public:
    static GaeactorAgentCoresProcessor & getInstance();
    virtual ~GaeactorAgentCoresProcessor();
    void dealPosData(const gaeactoragentcores::pos_hexidx & pos_hexidx_data);
    void dealPosattData(const gaeactoragentcores::posatt_hexidx & pos_hexidx_data);
    void appenddata(const H3INDEX &h3Indexsrc, const TYPE_ULID & entityulid, transdata_entityposinfo &&eninfo);
    void registDisplayCallback(display_pos_update_callback func);
    void registEventUpdateCallback(event_update_callback func);
private:
    explicit GaeactorAgentCoresProcessor(QObject *parent = nullptr);
#ifdef USING_CORES_DEAL_THREAD
public:
    bool isCheckable() const;
    void setCheckEnable(bool newBChecking);
private:
    void refreshIntervenes(int id);
    void data_deal_thread_func(void *pParam);
#endif
private:
    display_pos_update_callback m_phexidx_update_callback;
    event_update_callback m_event_update_callback;

#ifdef USING_CORES_DEAL_THREAD
    QReadWriteLock m_entity_hexidx_infos_mutex;
    std::unordered_map<TYPE_ULID, std::tuple<H3INDEX, std::unordered_map<UINT64,H3INDEX>,transdata_entityposinfo, bool>> m_entity_hexidx_infos;

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
