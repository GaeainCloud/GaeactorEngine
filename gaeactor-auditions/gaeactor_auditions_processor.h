#ifndef GAEACTOR_AUDITIONS_PROCESSOR_H
#define GAEACTOR_AUDITIONS_PROCESSOR_H
#include "gaeactor_processor_interface.h"
#include <QObject>
#include "gaeactor_auditions_define.h"
#include <QHash>
#include <QList>

#include <iostream>
#include "src/OriginalMutex.h"

namespace stdutils {
class OriThread;
}
namespace gaeactorauditions
{
#define USING_THREAD_NUM AUDITIONS_THREAD_NUM
class GaeactorAuditionsProcessor : public QObject
{
    Q_OBJECT
public:
    static GaeactorAuditionsProcessor & getInstance();
    virtual ~GaeactorAuditionsProcessor();
    void refreshAuditions();
    void registDisplayCallback(echowave_display_hexidx_update_callback func);
    void registHexidxDisplayCallback(display_hexidx_update_callback func);
    bool isCheckable() const;
    void setCheckEnable(bool newBChecking);
    void setEnableDeal(bool bEnbale);
private:
    explicit GaeactorAuditionsProcessor(QObject *parent = nullptr);

    void data_deal_thread_func(void * pParam);
private:
    echowave_display_hexidx_update_callback m_echowave_display_hexidx_update_callback;
    display_hexidx_update_callback m_phexidx_update_callback;

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
#endif // GAEACTOR_AUDITIONS_PROCESSOR_H
