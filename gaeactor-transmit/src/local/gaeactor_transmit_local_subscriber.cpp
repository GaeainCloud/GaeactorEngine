#include "gaeactor_transmit_local_subscriber.h"

#include <QDebug>

#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/listener.hpp"

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include <iostream>
#include "src/OriginalDateTime.h"
#include "loghelper.h"

namespace gaeactortransmit
{
GaeactorTransmitLocalSubscriber::GaeactorTransmitLocalSubscriber(QObject *parent)
    :GaeactorTransmitReceiver(parent)
//    ,m_pListener(nullptr)
    ,m_pSubscriber(nullptr)
{
#ifdef USING_SINGLE_CHANNEL
    m_DataProcessor = new DataProcessor(this);
    m_DataProcessor->set_data_deal_func_callback(std::bind(&GaeactorTransmitLocalSubscriber::data_deal_func_callback, this, std::placeholders::_1, std::placeholders::_2));
#endif
}

GaeactorTransmitLocalSubscriber::~GaeactorTransmitLocalSubscriber()
{
//    if(m_pListener)
//    {
//        delete m_pListener;
//    }

    if(m_pSubscriber)
    {
        delete m_pSubscriber;
    }
#ifdef USING_SINGLE_CHANNEL
	if(m_DataProcessor)
    {
        m_DataProcessor->deleteLater();
    }
#endif
}

void GaeactorTransmitLocalSubscriber::init(CHANNEL_INFO *channelinfo)
{    
    GaeactorTransmitBase::init(channelinfo);
    m_pSubscriber = new iox::popo::UntypedSubscriber({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_service),\
                                                      iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_instance),\
                                                      iox::capro::IdString_t(iox::cxx::TruncateToCapacity, channelinfo->m_event)});

//    m_pListener = new iox::popo::Listener();

//    m_pListener->attachEvent(*m_pSubscriber,
//                                       iox::popo::SubscriberEvent::DATA_RECEIVED,
//                                       iox::popo::createNotificationCallback(onSampleReceivedCallback,*this))
//        .or_else([](auto) {
//            std::cerr << "unable to attach subscriberLeft" << std::endl;
//            std::exit(EXIT_FAILURE);
//        });

    //std::cout<<"subscriber create succeed --> service:"<<channelinfo.m_service<<" instance:"<<channelinfo.m_instance<<" event:"<<channelinfo.m_event<<std::endl;
}

void GaeactorTransmitLocalSubscriber::onSampleReceivedCallback(iox::popo::UntypedSubscriber *subscriber, GaeactorTransmitLocalSubscriber *self)
{
    if(self)
    {
        self->dealSubscriberInfo(subscriber);
    }
}

void GaeactorTransmitLocalSubscriber::dealSubscriberInfo(iox::popo::UntypedSubscriber *const subscriber)
{
#if 1
    auto takeResult = subscriber->take();
    if (!takeResult.has_error())
    {
        auto instanceString = subscriber->getServiceDescription().getInstanceIDString();
        auto eventString = subscriber->getServiceDescription().getEventIDString();

        const void*& usrpayload = takeResult.value();

        auto chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(usrpayload);
        BYTE* pSrcData = static_cast<BYTE*>(const_cast<void*>(usrpayload));

#ifdef USING_SINGLE_CHANNEL
        UINT32 iLen;
        memcpy(&iLen, pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), sizeof(UINT32));
        uint32_t userPayloadSize = iLen;
        if(m_DataProcessor && chunkHeader->userPayloadSize() == userPayloadSize)
        {
            m_DataProcessor->appendData(pSrcData, userPayloadSize);
        }
#else
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType;

        UINT32 iLen;
        BYTE *pData = nullptr;

        memcpy(&channelTransmitDataType, pSrcData, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(&iLen, pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), sizeof(UINT32));
        pData = pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
        uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
        if(m_preceive_callback && chunkHeader->userPayloadSize() == userPayloadSize)
        {
            m_preceive_callback(m_channelinfo,channelTransmitDataType,pData,iLen,pSrcData,userPayloadSize);
        }
#endif
        subscriber->release(usrpayload);

//        static int64_t lsttimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//        static uint64_t icount = 0;
//        icount += chunkHeader->userPayloadSize();
//        int64_t currenttimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();

//        if (currenttimestamp - lsttimestamp > 1000)
//        {
//            LOG_PRINT_STR_EX("total recv count "+QString::number(icount))
//        }
    }
    else
    {
        auto ErrorTypes = takeResult.get_error();
        //! [error]
        if (ErrorTypes == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
        {
            LOG_PRINT_STR_EX("No chunk available.")
            std::cout<<"No chunk available."<<std::endl;
        }
        else
        {
            LOG_PRINT_STR_EX("Error receiving chunk.")
            std::cout<<"Error receiving chunk."<<std::endl;
        }
        //! [error]
    }
#else
    subscriber->take().and_then([subscriber](const void* usrpayload) {
        auto instanceString = subscriber->getServiceDescription().getInstanceIDString();

        auto chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(usrpayload);
        // store the sample in the corresponding cache
        if (instanceString == iox::capro::IdString_t("FrontLeft"))
        {

            BYTE* pSrcData = static_cast<BYTE*>(const_cast<void*>(usrpayload));
            UINT32 iLen;
            BYTE *pData = nullptr;

            memcpy(&iLen,pSrcData, sizeof(UINT32));
            pData = pSrcData + sizeof(UINT32);

            m_preceive_callback(m_channelinfo,pData,iLen);
        }

        subscriber->release(usrpayload);
    });
#endif
}
#ifdef USING_SINGLE_CHANNEL
void GaeactorTransmitLocalSubscriber::data_deal_func_callback(const BYTE *pSrcData, UINT32 userPayloadSize)
{
    E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType;
    UINT32 iLen;
    const BYTE *pData = nullptr;
    memcpy(&channelTransmitDataType, pSrcData, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
    memcpy(&iLen, pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), sizeof(UINT32));
    pData = pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);

    if(m_preceive_callback )
    {
        //std::cout<<"recv datalen:"<<userPayloadSize<<std::endl;
        m_preceive_callback(m_channelinfo,channelTransmitDataType,pData,iLen,pSrcData,userPayloadSize);
    }
}
#endif

void GaeactorTransmitLocalSubscriber::attachListenerEvent(iox::popo::Listener *newPListener)
{
    m_pListener = newPListener;

    m_pListener->attachEvent(*m_pSubscriber,
                           iox::popo::SubscriberEvent::DATA_RECEIVED,
                           iox::popo::createNotificationCallback(onSampleReceivedCallback,*this))
        .or_else([](auto) {
            std::cerr << "unable to attach subscriberLeft" << std::endl;
            std::exit(EXIT_FAILURE);
        });

}

void GaeactorTransmitLocalSubscriber::detachListenerEvent()
{
    if(m_pListener)
    {
        m_pListener->detachEvent(*m_pSubscriber,iox::popo::SubscriberEvent::DATA_RECEIVED);
    }

}

iox::popo::UntypedSubscriber *GaeactorTransmitLocalSubscriber::pSubscriber() const
{
    return m_pSubscriber;
}

void GaeactorTransmitLocalSubscriber::printMempoolInfo()
{
//    const iox::rp::RelativePointer<iox::mepoo::MemoryManager> m_memoryMgr = m_pRuntime->getMiddlewareSubscriber({iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo.m_service),\
//                                                                                                                 iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo.m_instance),\
//                                                                                                                 iox::capro::IdString_t(iox::cxx::TruncateToCapacity, m_channelinfo.m_event)})->m_chunkReceiverData.m_memoryInfo;
//    int mempoolnum = m_memoryMgr->getNumberOfMemPools();

//    //    m_mempoolconf.addMemPool({ CHUNK_SIZE, NUM_CHUNKS_IN_POOL });
//    //    m_memoryMgr->configureMemoryManager(m_mempoolconf, m_memoryAllocator, m_memoryAllocator);

//    for (int index = 0; index < mempoolnum; index++)
//    {
//        auto memPool = m_memoryMgr->getMemPoolInfo(index);
//        std::cout << "  MemPool [ ChunkSize = " << memPool.m_chunkSize
//                  << ", ChunkPayloadSize = " << memPool.m_chunkSize - sizeof(iox::mepoo::ChunkHeader)
//                  << ", ChunkUsed = " << memPool.m_usedChunks
//                  << ", ChunkCount = " << memPool.m_numChunks << " ]"<<std::endl;
//    }
}
}
