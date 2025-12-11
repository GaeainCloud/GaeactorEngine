#include "gaeactor_interactions_processor.h"

#include <QDebug>
#include "src/OriginalDateTime.h"
#include "src/OriginalThread.h"
#include "loghelper.h"

#define ARC_SENSOR_TO_EITITY
#define USING_SPAN_ANGLE (15)
//#define SHOW_SENSOR_TO_ENTITY_WAVE
//#define GENERATEECHOWAVE
#define THREAD_EXIT_MAX_TIME (3000)
namespace gaeactorinteractions
{
GaeactorInteractionsProcessor::GaeactorInteractionsProcessor(QObject *parent)
{
    m_bChecking.store(false);
    m_needdeal.store(false);
    for(int i = 0 ; i< 0; i++)
    {
        m_hDataDealThreadParam[i].id = i;
#ifdef _MSC_VER
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorInteractionsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       THREAD_PRIORITY_TIME_CRITICAL);
#else
        m_hDataDealThread[i] = new stdutils::OriThread(std::bind(&GaeactorInteractionsProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                                                                                                                       &m_hDataDealThreadParam[i],\
                                                       99);
#endif
        m_hDataDealThread[i]->start();
    }
}

GaeactorInteractionsProcessor::~GaeactorInteractionsProcessor()
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

void GaeactorInteractionsProcessor::data_deal_thread_func(void *pParam)
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
            if(!m_needdeal.load())
            {
                //                LOG_PRINT_STR_EX("thread id: "+ QString::number(id) +" GaeactorInteractionsProcessor thred wait ")
                m_dealfullCond.wait(&m_dealmutex);
                //                LOG_PRINT_STR_EX("thread id: "+ QString::number(id) +" GaeactorInteractionsProcessor thred wake up ")
            }
            //            static volatile uint64_t lasttimestamp = 0;
            //            static volatile uint64_t dealcount = 0;

            //            uint64_t currenttimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();

            //            dealcount++;
            //            if(fabs(currenttimestamp- lasttimestamp)>1000)
            //            {
            //                double freq = 0.0f;
            //                freq = (double)1000.0f/(dealcount);
            //                std::cout<<"interval "<<currenttimestamp - lasttimestamp<<" deal count "<<dealcount<<std::endl;
            //                lasttimestamp = currenttimestamp;
            //                dealcount = 0;
            //            }
            refreshInteractions(pObject->id);
            //            m_needdeal.store(false);
#ifdef _MSC_VER
            stdutils::OriDateTime::sleep(50);
#else
            stdutils::OriDateTime::sleep(50);
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

bool GaeactorInteractionsProcessor::isCheckable() const
{
    return m_bChecking.load();
}

void GaeactorInteractionsProcessor::setCheckEnable(bool newBChecking)
{
    m_bChecking.store(newBChecking);
}

void GaeactorInteractionsProcessor::setEnableDeal(bool bEnbale)
{
    m_needdeal.store(bEnbale);
    if(bEnbale)
    {
        m_dealfullCond.wakeAll();
    }
}

void GaeactorInteractionsProcessor::refreshInteractions(int id)
{
}

void GaeactorInteractionsProcessor::registDisplayCallback(echowave_display_hexidx_update_callback func)
{
}

void GaeactorInteractionsProcessor::registDisplayListCallback(echowave_list_display_hexidx_update_callback func)
{
}

void GaeactorInteractionsProcessor::registHexidxDisplayCallback(display_hexidx_update_callback func)
{
}

GaeactorInteractionsProcessor &GaeactorInteractionsProcessor::getInstance()
{
    static GaeactorInteractionsProcessor gaeactorinteractionsprocessor;
    return gaeactorinteractionsprocessor;
}
}
