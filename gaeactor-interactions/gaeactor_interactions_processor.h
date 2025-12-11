#ifndef GAEACTOR_INTERACTIONS_PROCESSOR_H
#define GAEACTOR_INTERACTIONS_PROCESSOR_H

#include "gaeactor_processor_interface.h"
#include <QObject>
#include "gaeactor_interactions_define.h"
#include <QHash>
#include <QList>
#include <iostream>
#include <thread>
#include "src/OriginalMutex.h"

namespace stdutils {
class OriThread;
}
namespace gaeactorinteractions
{
#define USING_THREAD_NUM INTERACTIONS_THREAD_NUM
class GaeactorInteractionsProcessor : public QObject
{
    Q_OBJECT
public:
    static GaeactorInteractionsProcessor & getInstance();
    virtual ~GaeactorInteractionsProcessor();
    void refreshInteractions(int id);
    void registDisplayCallback(echowave_display_hexidx_update_callback func);

    void registDisplayListCallback(echowave_list_display_hexidx_update_callback func);
    void registHexidxDisplayCallback(display_hexidx_update_callback func);
    bool isCheckable() const;
    void setCheckEnable(bool newBChecking);

    void setEnableDeal(bool bEnbale);
private:
    void dealEventLoop(int id);

    void data_deal_thread_func(void *pParam);
private:
    explicit GaeactorInteractionsProcessor(QObject *parent = nullptr);
private:

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
#endif // GAEACTOR_INTERACTIONS_PROCESSOR_H
