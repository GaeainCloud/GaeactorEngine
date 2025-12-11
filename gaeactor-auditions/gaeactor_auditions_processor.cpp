#include "gaeactor_auditions_processor.h"

#include <QDebug>
#include "src/OriginalDateTime.h"
#include "src/OriginalThread.h"
#include "loghelper.h"
#include <sstream>
#include "easy/profiler.h"
#include "runningmodeconfig.h"
#define THREAD_EXIT_MAX_TIME (3000)

namespace gaeactorauditions
{
GaeactorAuditionsProcessor::GaeactorAuditionsProcessor(QObject *parent)
{
    m_bChecking.store(false);
    m_needdeal.store(false);

    for(int i = 0 ; i< USING_THREAD_NUM; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAuditionsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorAuditionsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       99);
#endif

        m_hDataDealThread[i]->start();
    }
}

GaeactorAuditionsProcessor::~GaeactorAuditionsProcessor()
{
    m_dealfullCond.wakeAll();

    //清理DataDealThread
    for(int i = 0; i < USING_THREAD_NUM; i++)
    {
        if (m_hDataDealThread[i] != nullptr)
        {
            delete m_hDataDealThread[i];
            m_hDataDealThread[i] = nullptr;
        }
    }
}

bool GaeactorAuditionsProcessor::isCheckable() const
{
    return m_bChecking.load();
}

void GaeactorAuditionsProcessor::setCheckEnable(bool newBChecking)
{
    m_bChecking.store(newBChecking);
}

void GaeactorAuditionsProcessor::setEnableDeal(bool bEnbale)
{
    m_needdeal.store(bEnbale);
    if(bEnbale)
    {
        m_dealfullCond.wakeAll();
    }
}

void GaeactorAuditionsProcessor::refreshAuditions()
{
    EASY_FUNCTION(profiler::colors::Green)
    {
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().refresh_silent_timeout();
    }
}

void GaeactorAuditionsProcessor::registDisplayCallback(echowave_display_hexidx_update_callback func)
{
    m_echowave_display_hexidx_update_callback = std::move(func);
}

void GaeactorAuditionsProcessor::registHexidxDisplayCallback(display_hexidx_update_callback func)
{
    m_phexidx_update_callback = std::move(func);
}

GaeactorAuditionsProcessor &GaeactorAuditionsProcessor::getInstance()
{
    static GaeactorAuditionsProcessor gaeactorauditionsprocessor;
    return gaeactorauditionsprocessor;
}


void GaeactorAuditionsProcessor::data_deal_thread_func(void * pParam)
{
    if (pParam == nullptr)
    {
        return;
    }
    threadParam *pObject = reinterpret_cast<threadParam*>(pParam);
    if(pObject)
    {
        if(m_bChecking.load())
        {
            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE())
            {
                std::stringstream ss;
                ss<<"****************** MODE_USING_CLEAR_OLD_SENSOR_SENSINGMEDIA_FORCE_COVER_MODE stop refreshAuditions therad , sensor sensingmedia clean by other data thread ******************\n";
                TRACE_LOG_PRINT_EX2(ss);
                m_dealfullCond.wait(&m_dealmutex);
            }
            else
            {
                refreshAuditions();
            }
#ifdef _MSC_VER
            {
                stdutils::OriDateTime::sleep(1);
            }
#else
            stdutils::OriDateTime::sleep(10);
#endif
        }
        else
        {
            //                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            stdutils::OriDateTime::sleep(3000);
        }
    }
    return;
}
}
