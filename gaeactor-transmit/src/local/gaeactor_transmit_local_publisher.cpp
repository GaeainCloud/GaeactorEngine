#include "gaeactor_transmit_local_publisher.h"

#include <QDebug>
#include <chrono>

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"

#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "src/OriginalDateTime.h"
#include "loghelper.h"
namespace gaeactortransmit
{
GaeactorTransmitLocalPublisher::GaeactorTransmitLocalPublisher(QObject *parent)
    :GaeactorTransmitSender(parent)
    ,m_pPublisher(nullptr)
#ifndef USING_SINGLE_CHANNEL
    ,m_usrpayload(nullptr)
#endif
{
#ifdef USING_SINGLE_CHANNEL
    m_DataProcessor = new DataProcessor(this);
    m_DataProcessor->set_data_deal_func_callback(std::bind(&GaeactorTransmitLocalPublisher::data_deal_func_callback, this, std::placeholders::_1, std::placeholders::_2));
#else
    m_pushing.store(false);
    m_applying.store(false);
#endif
}

GaeactorTransmitLocalPublisher::~GaeactorTransmitLocalPublisher()
{
    if(m_pPublisher)
    {
        delete m_pPublisher;
    }
#ifdef USING_SINGLE_CHANNEL
    if(m_DataProcessor)
    {
        m_DataProcessor->deleteLater();
    }
#endif
}

void GaeactorTransmitLocalPublisher::init(CHANNEL_INFO *channelinfo)
{
    GaeactorTransmitBase::init(channelinfo);
    m_pPublisher = new iox::popo::UntypedPublisher({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_service),\
                                                    iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_instance),\
                                                    iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_event)});

//    std::cout<<"publisher create succeed --> service:"<<channelinfo.m_service<<" instance:"<<channelinfo.m_instance<<" event:"<<channelinfo.m_event<<std::endl;
}

void GaeactorTransmitLocalPublisher::transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen)
{
#ifdef USING_SINGLE_CHANNEL
    if(m_DataProcessor)
    {
        m_DataProcessor->appendData(pData, iLen);
    }
#else
    if(m_pPublisher)
    {
        uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
        uint32_t userPayloadAlignment = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
        uint32_t userHeaderSize = iox::CHUNK_NO_USER_HEADER_SIZE;
        uint32_t userHeaderAlignment = iox::CHUNK_NO_USER_HEADER_ALIGNMENT;

//        QMutexLocker locker(&m_pPublisherMutex);

        if(!m_pushing.load())
        {
            m_pushing.store(true);
            auto result = m_pPublisher->loan(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
            if (!result.has_error())
            {
                void * usrpayload = result.value();

                BYTE *pDstData = static_cast<BYTE*>(usrpayload);
                memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
                memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &iLen, sizeof(UINT32));
                memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32), pData, iLen);

                m_pPublisher->publish(usrpayload);

                stdutils::OriDateTime::sleep(1);
//                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
                auto ErrorTypes = result.get_error();
                {
                    LOG_PRINT_STR_EX("Error receiving chunk.")
                }
            }
            m_pushing.store(false);
            m_applying.store(false);
        }
        else
        {
            LOG_PRINT_STR_EX(" this publisher is pushing ")
        }
    }
#endif
}

void *GaeactorTransmitLocalPublisher::loanTransmitBuffer(UINT32 iLen)
{
    void *_usrpayload = nullptr;
#ifndef USING_SINGLE_CHANNEL
    if(m_pPublisher)
    {
        uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
        uint32_t userPayloadAlignment = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
        uint32_t userHeaderSize = iox::CHUNK_NO_USER_HEADER_SIZE;
        uint32_t userHeaderAlignment = iox::CHUNK_NO_USER_HEADER_ALIGNMENT;

        if(!m_pushing.load())
        {
            m_pushing.store(true);
            auto result = m_pPublisher->loan(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
            if (!result.has_error())
            {
                m_usrpayload = result.value();
                _usrpayload = m_usrpayload;
            }
            else
            {
                m_usrpayload = nullptr;
            }
        }
        else
        {
            LOG_PRINT_STR_EX(" this publisher is pushing ")
        }
    }
#endif
    return _usrpayload;
}

void GaeactorTransmitLocalPublisher::publish()
{
#ifndef USING_SINGLE_CHANNEL
	if(m_pushing.load() && m_usrpayload)
    {
        m_pPublisher->publish(m_usrpayload);
//        stdutils::OriDateTime::sleep(1);
        m_pushing.store(false);
        m_applying.store(false);
    }
#endif
}

std::string GaeactorTransmitLocalPublisher::getEventItemName(TYPE_ULID ulid)
{
    int64_t currenttimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMicros();
    TYPE_ULID _ulid = ulid;
    std::string str = QString::number(_ulid).toStdString();

    return m_channelinfo->m_event + str + std::to_string(m_publisherItems.size());
}

bool GaeactorTransmitLocalPublisher::appendPublisherItem(const std::string &item)
{
    auto itor = std::find_if(m_publisherItems.begin(),
                             m_publisherItems.end(),
                             [&](const std::list<std::string>::value_type &vt){
                                 return vt == item;
                             });

    if(itor == m_publisherItems.end())
    {
        m_publisherItems.emplace_back(std::move(item));
        return true;
    }
    return false;
}

bool GaeactorTransmitLocalPublisher::removePbulisherItem(const std::string &item)
{
    auto itor = std::find_if(m_publisherItems.begin(),
                             m_publisherItems.end(),
                             [&](const std::list<std::string>::value_type &vt){
                                 return vt == item;
                             });
    if(itor != m_publisherItems.end())
    {
        m_publisherItems.erase(itor);
        return true;
    }
    return false;

}

bool GaeactorTransmitLocalPublisher::clearPublisherItems()
{
    m_publisherItems.clear();
    return true;
}

UINT32 GaeactorTransmitLocalPublisher::getPublisherCount()
{
    return m_publisherItems.size();
}


void GaeactorTransmitLocalPublisher::printMempoolInfo()
{
    const iox::rp::RelativePointer<iox::mepoo::MemoryManager> m_memoryMgr = m_pRuntime->getMiddlewarePublisher({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo->m_service),\
                                                                                                                iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo->m_instance),\
                                                                                                                iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo->m_event)})->m_chunkSenderData.m_memoryMgr;
    int mempoolnum = m_memoryMgr->getNumberOfMemPools();

    //    m_mempoolconf.addMemPool({ CHUNK_SIZE, NUM_CHUNKS_IN_POOL });
    //    m_memoryMgr->configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);

    for (int index = 0; index < mempoolnum; index++)
    {
        auto memPool = m_memoryMgr->getMemPoolInfo(index);
        std::cout << "  MemPool [ ChunkSize = " << memPool.m_chunkSize
                  << ", ChunkPayloadSize = " << memPool.m_chunkSize - sizeof(iox::mepoo::ChunkHeader)
                  << ", ChunkUsed = " << memPool.m_usedChunks
                  << ", ChunkCount = " << memPool.m_numChunks << " ]"<<std::endl;
    }
}

#ifdef USING_SINGLE_CHANNEL
void GaeactorTransmitLocalPublisher::data_deal_func_callback(const BYTE *pData, UINT32 iLen)
{
    if(m_pPublisher)
    {
        uint32_t userPayloadSize = iLen;
        uint32_t userPayloadAlignment = iox::CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT;
        uint32_t userHeaderSize = iox::CHUNK_NO_USER_HEADER_SIZE;
        uint32_t userHeaderAlignment = iox::CHUNK_NO_USER_HEADER_ALIGNMENT;

        auto result = m_pPublisher->loan(userPayloadSize, userPayloadAlignment, userHeaderSize, userHeaderAlignment);
        if (!result.has_error())
        {
            void * usrpayload = result.value();
            BYTE *pDstData = static_cast<BYTE*>(usrpayload);
            memcpy(pDstData, pData, iLen);

            m_pPublisher->publish(usrpayload);
            //LOG_PRINT_STR_EX("PUBLISH datalen : "+ QString::number(iLen))
            //stdutils::OriDateTime::sleep(1);
            //                std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        else
        {
            auto ErrorTypes = result.get_error();
            {
                LOG_PRINT_STR_EX("Error receiving chunk.")
            }
        }
    }
}
#else
bool GaeactorTransmitLocalPublisher::isPushing() const
{
    return m_pushing.load();
    }

bool GaeactorTransmitLocalPublisher::applying()
{
    QMutexLocker locker(&m_applyingMutex);
    bool ret = m_applying.load();
    m_applying.store(true);
    return ret;
}

void GaeactorTransmitLocalPublisher::setApplying(const bool &newApplying)
{
    m_applying.store(newApplying);
}
#endif
}
