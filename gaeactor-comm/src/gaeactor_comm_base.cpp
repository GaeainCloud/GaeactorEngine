#include "gaeactor_comm_base.h"
#include <ecal/ecal_publisher.h>
#include <ecal/ecal_subscriber.h>
#include "src/OriginalThread.h"
namespace gaeactorcomm
{
GaeactorCommBase::GaeactorCommBase(const COMM_CHANNEL_INFO &_channel_info)
    :m_channel_info(_channel_info)
    ,m_pPublisher(nullptr)
    ,m_pSubscriber(nullptr)
{

//#ifdef _MSC_VER
//    m_hDataDealThread = new stdutils::OriThread(std::bind(&GaeactorCommBase::deal_thread_func,this,std::placeholders::_1), nullptr, THREAD_PRIORITY_NORMAL);
//#else
//    m_hDataDealThread = new stdutils::OriThread(std::bind(&GaeactorCommBase::deal_thread_func,this,std::placeholders::_1), nullptr, 0);
//#endif
//    m_hDataDealThread->start();

}

GaeactorCommBase::~GaeactorCommBase()
{
    if(m_pPublisher)
    {
        delete m_pPublisher;
    }

    if(m_pSubscriber)
    {
        delete m_pSubscriber;
    }
}

bool GaeactorCommBase::sendData(const BYTE *pData, UINT32 iLen)
{
    return true;
}

bool GaeactorCommBase::sendData(const std::string &pData)
{
    return true;
}

//void GaeactorCommBase::deal_process()
//{

//}

void GaeactorCommBase::set_binary_data_callback(comm_binary_receive_callback func)
{
    m_comm_binary_receive_callback = std::move(func);
}

void GaeactorCommBase::set_string_data_callback(comm_string_receive_callback func)
{
    m_comm_string_receive_callback = std::move(func);
}

//void GaeactorCommBase::deal_thread_func(void *pParam)
//{
//    deal_process();
//}

//bool GaeactorCommBase::isrunning() const
//{
//    return m_brunning.load();
//}

//void GaeactorCommBase::setBrunning(bool newBrunning)
//{
//    m_brunning = newBrunning;
//}

} // namespace gaeactorcomm
