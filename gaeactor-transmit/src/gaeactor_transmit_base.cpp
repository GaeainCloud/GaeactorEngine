#include "gaeactor_transmit_base.h"

#include <QDebug>

#include <iostream>
#ifdef USING_SINGLE_CHANNEL
#include "loghelper.h"
#include "src/OriginalDateTime.h"

//为场数据缓冲区分配50M缓冲
#define TRANS_DATA_BUFFER_SIZE (50*1024*1024)
#define DEAL_DATA_BUFFER_SIZE (5*1024*1024)

namespace gaeactortransmit
{
DataCircularBuffer::DataCircularBuffer()
    :m_pImportDataBuffer(nullptr),
    m_iImportDataBufferLen(0),
    m_iImportDataBufferWriteIndex(1),
    m_iImportDataBufferReadIndex(0)
{
}

DataCircularBuffer::~DataCircularBuffer()
{
    if(m_pImportDataBuffer)
    {
        delete []m_pImportDataBuffer;
    }

    m_iImportDataBufferLen = 0;
    m_iImportDataBufferReadIndex	= 0;
    m_iImportDataBufferWriteIndex	= 1;
    m_iBufferBytesCount = 0;
}

void DataCircularBuffer::init(int maxcount)
{
    m_iImportDataBufferLen = maxcount;
    try
    {
        m_pImportDataBuffer = new TRANS_DATA[m_iImportDataBufferLen];

    }
    catch (...)
    {
        m_iImportDataBufferLen = 0;
        m_pImportDataBuffer = nullptr;
    };

    if (m_pImportDataBuffer)
    {
        memset(m_pImportDataBuffer, 0, m_iImportDataBufferLen*sizeof(TRANS_DATA));
    }
    m_iImportDataBufferReadIndex	= 0;
    m_iImportDataBufferWriteIndex	= 1;
    m_iBufferBytesCount = 0;
}

bool DataCircularBuffer::write_data(const TRANS_DATA_PTR pParam, UINT32 iParamLen)
{
    QMutexLocker locker(&m_databuf_mutex);
    UINT32 iSaveLen = sizeof(iParamLen)+iParamLen;
    if (pParam == NULL ||
        iParamLen <= 0 ||
        iSaveLen>=m_iImportDataBufferLen)
    {
        LOG_PRINT_STR_EX("input request invalid");
        return false;
    }

    // 判断数据被覆盖，写入指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferReadIndex\
         -m_iImportDataBufferWriteIndex)%m_iImportDataBufferLen < iSaveLen)
    {
        // 丢失该包，防止丢失整个缓存
        LOG_PRINT_STR_EX("buffer no enough to read");
        return false;
    }


    //拷贝长度
    if (m_iImportDataBufferWriteIndex+ sizeof(iParamLen) <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer+m_iImportDataBufferWriteIndex, \
               (TRANS_DATA_PTR)&iParamLen, sizeof(iParamLen));
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer + m_iImportDataBufferWriteIndex, (TRANS_DATA_PTR)&iParamLen, sizeof(TRANS_DATA) * (m_iImportDataBufferLen - m_iImportDataBufferWriteIndex));
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer, ((TRANS_DATA_PTR)&iParamLen) + (m_iImportDataBufferLen - m_iImportDataBufferWriteIndex), sizeof(TRANS_DATA)*(sizeof(iParamLen)-(m_iImportDataBufferLen-m_iImportDataBufferWriteIndex)));
    }

    m_iImportDataBufferWriteIndex += sizeof(iParamLen);
    m_iImportDataBufferWriteIndex %= m_iImportDataBufferLen;

    //拷贝数据
    if (m_iImportDataBufferWriteIndex+iParamLen <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer+m_iImportDataBufferWriteIndex, \
               (TRANS_DATA_PTR)pParam, sizeof(TRANS_DATA)*iParamLen);
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer+m_iImportDataBufferWriteIndex, (TRANS_DATA_PTR)pParam, sizeof(TRANS_DATA)*(m_iImportDataBufferLen-m_iImportDataBufferWriteIndex));
        memcpy((TRANS_DATA_PTR)m_pImportDataBuffer, (TRANS_DATA_PTR)pParam +(m_iImportDataBufferLen-m_iImportDataBufferWriteIndex), sizeof(TRANS_DATA)*(iParamLen-(m_iImportDataBufferLen-m_iImportDataBufferWriteIndex)));
    }

    m_iImportDataBufferWriteIndex += iParamLen;
    m_iImportDataBufferWriteIndex %= m_iImportDataBufferLen;
    m_iBufferBytesCount += sizeof(iParamLen) + iParamLen;
    m_packslenlist.push_back(iParamLen);
    return true;
}

bool DataCircularBuffer::read_data(const TRANS_DATA_PTR pBuffer, UINT32 iBufferLen)
{
    QMutexLocker locker(&m_databuf_mutex);

    if (pBuffer == NULL ||
        iBufferLen <= 0 ||
        iBufferLen >= m_iImportDataBufferLen)
    {
        LOG_PRINT_STR_EX("input request invalid");
        return false;
    }
    UINT32 ireadBufferLen = 0;

    // 检测数据是否够长度，读取指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferWriteIndex\
         -(m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen < sizeof(ireadBufferLen))
    {
        // 数据不够
        LOG_PRINT_STR_EX("buffer no enough to read packlen");
        return false;
    }

    if (m_iImportDataBufferReadIndex+1+sizeof(ireadBufferLen) <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)&ireadBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(ireadBufferLen));
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)&ireadBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy(((TRANS_DATA_PTR)&ireadBufferLen) + m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(sizeof(ireadBufferLen)-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    if (ireadBufferLen <= 0 || ireadBufferLen != iBufferLen || m_packslenlist.front() !=  iBufferLen)
    {
        LOG_PRINT_STR_EX("can read len is unmatch request len");
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    // 读取参数数据
    // 检测数据是否够长度，读取指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferWriteIndex\
         -(m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen < iBufferLen + sizeof(iBufferLen))
    {
        // 数据不够
        LOG_PRINT_STR_EX("buffer no enough to read packdata");
        return false;
    }

    m_iImportDataBufferReadIndex += sizeof(iBufferLen);
    m_iImportDataBufferReadIndex %= m_iImportDataBufferLen;

    if (m_iImportDataBufferReadIndex+1+iBufferLen <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*iBufferLen);
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy((TRANS_DATA_PTR)pBuffer+m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(iBufferLen-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    m_iImportDataBufferReadIndex += iBufferLen;
    m_iImportDataBufferReadIndex %= m_iImportDataBufferLen;
    m_iBufferBytesCount -= (sizeof(iBufferLen) + iBufferLen);
    m_packslenlist.pop_front();
    return true;
}

bool DataCircularBuffer::read_data_step(const TRANS_DATA_PTR pBuffer, UINT32 &iBufferLen)
{
    QMutexLocker locker(&m_databuf_mutex);

    if (pBuffer == NULL)
    {
        LOG_PRINT_STR_EX("input request invalid");
        return false;
    }
    UINT32 ireadBufferLen = 0;

    // 检测数据是否够长度，读取指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferWriteIndex\
         -(m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen < sizeof(ireadBufferLen))
    {
        // 数据不够
        LOG_PRINT_STR_EX("buffer no enough to read packlen");
        return false;
    }

    if (m_iImportDataBufferReadIndex+1+sizeof(ireadBufferLen) <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)&ireadBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(ireadBufferLen));
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)&ireadBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy(((TRANS_DATA_PTR)&ireadBufferLen) + m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(sizeof(ireadBufferLen)-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    if (ireadBufferLen <= 0)
    {
        LOG_PRINT_STR_EX("can read len is unmatch request len");
        return false;
    }

    //////////////////////////////////////////////////////////////////////////
    // 读取参数数据
    // 检测数据是否够长度，读取指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferWriteIndex\
         -(m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen < ireadBufferLen + sizeof(ireadBufferLen))
    {
        // 数据不够
        LOG_PRINT_STR_EX("buffer no enough to read packdata");
        return false;
    }

    m_iImportDataBufferReadIndex += sizeof(ireadBufferLen);
    m_iImportDataBufferReadIndex %= m_iImportDataBufferLen;

    if (m_iImportDataBufferReadIndex+1+ireadBufferLen <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*ireadBufferLen);
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy((TRANS_DATA_PTR)pBuffer+m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(iBufferLen-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    m_iImportDataBufferReadIndex += ireadBufferLen;
    m_iImportDataBufferReadIndex %= m_iImportDataBufferLen;
    m_iBufferBytesCount -= (sizeof(ireadBufferLen) + ireadBufferLen);
    iBufferLen = ireadBufferLen;
    return true;
}

bool DataCircularBuffer::read_data_direct(const TRANS_DATA_PTR pBuffer, UINT32 iBufferLen, UINT32 iPackcount)
{

    QMutexLocker locker(&m_databuf_mutex);

    if (pBuffer == NULL ||
        iBufferLen <= 0 ||
        iBufferLen >= m_iImportDataBufferLen ||
        iPackcount <= 0)
    {
        LOG_PRINT_STR_EX("input request invalid");
        return false;
    }

    if (m_iImportDataBufferReadIndex+1+iBufferLen <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*iBufferLen);
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)pBuffer, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy((TRANS_DATA_PTR)pBuffer+m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(iBufferLen-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    m_iImportDataBufferReadIndex += iBufferLen;
    m_iImportDataBufferReadIndex %= m_iImportDataBufferLen;
    m_iBufferBytesCount -= iBufferLen;
    for(int i = 0; i < iPackcount; i++)
    {
        m_packslenlist.pop_front();
    }
    return true;
}

bool DataCircularBuffer::get_next_pack_len(UINT32 &iBufferLen)
{
    QMutexLocker locker(&m_databuf_mutex);
    // 检测数据是否够长度，读取指针判断
    if ((m_iImportDataBufferLen+m_iImportDataBufferWriteIndex\
         -(m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen < sizeof(iBufferLen))
    {
        // 数据不够
        LOG_PRINT_STR_EX("buffer no enough to read packlen");
        return false;
    }

    if (m_iImportDataBufferReadIndex+1+sizeof(iBufferLen) <= m_iImportDataBufferLen)
    {
        // 末端拷贝
        memcpy((TRANS_DATA_PTR)&iBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(iBufferLen));
    }
    else
    {
        // 两端拷贝
        memcpy((TRANS_DATA_PTR)&iBufferLen, (TRANS_DATA_PTR)m_pImportDataBuffer+(m_iImportDataBufferReadIndex+1), sizeof(TRANS_DATA)*(m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1)));
        memcpy(((TRANS_DATA_PTR)&iBufferLen) + m_iImportDataBufferLen-(m_iImportDataBufferReadIndex+1), (TRANS_DATA_PTR)m_pImportDataBuffer, sizeof(TRANS_DATA)*(sizeof(iBufferLen)-(m_iImportDataBufferLen -(m_iImportDataBufferReadIndex+1))));
    }

    if (iBufferLen <= 0 || m_packslenlist.front() !=  iBufferLen)
    {
        LOG_PRINT_STR_EX("can read len is unmatch request len");
        return false;
    }
    return true;
}

bool DataCircularBuffer::get_buffer_packs_len(std::list<UINT32> &_packslenlist)
{
    QMutexLocker locker(&m_databuf_mutex);
    if(m_packslenlist.empty())
    {
        return false;
    }
    _packslenlist = m_packslenlist;
    return true;
}


UINT32 DataCircularBuffer::get_buffer_bytes_count()
{
    QMutexLocker locker(&m_databuf_mutex);
    return m_iBufferBytesCount;
}

void DataCircularBuffer::clear_packslen()
{
    QMutexLocker locker(&m_databuf_mutex);
    m_packslenlist.clear();
}

bool DataCircularBuffer::isEmpty()
{
    UINT32 iReadyDataLen = 0;
    QMutexLocker locker(&m_databuf_mutex);
    // 获取数据长度
    iReadyDataLen = (m_iImportDataBufferLen+m_iImportDataBufferWriteIndex - (m_iImportDataBufferReadIndex+1))%m_iImportDataBufferLen;
    if (iReadyDataLen<=0)
    {
        // 数据不够
        return true;
    }
    return false;
}


DataProcessor::DataProcessor(QObject *parent)
    :QObject(parent)
    ,m_hDataDealThread(nullptr)
    ,m_data_deal_func_callback(nullptr)
    ,m_curDealDataBuffer(nullptr)
{
    m_transDataCircularBuffer.init(TRANS_DATA_BUFFER_SIZE);
    try{

        m_curDealDataBuffer = new DataCircularBuffer::TRANS_DATA[DEAL_DATA_BUFFER_SIZE];
    }
    catch(...)
    {
        m_curDealDataBuffer = nullptr;
    }


    memset(m_curDealDataBuffer, 0, DEAL_DATA_BUFFER_SIZE * sizeof(DataCircularBuffer::TRANS_DATA));
#ifdef _MSC_VER
    m_hDataDealThread = new stdutils::OriThread(std::bind(&DataProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                nullptr,\
                                                THREAD_PRIORITY_TIME_CRITICAL);
#else
    m_hDataDealThread = new stdutils::OriThread(std::bind(&DataProcessor::data_deal_thread_func,this,std::placeholders::_1),\
                                                nullptr,\
                                                99);
#endif
    m_hDataDealThread->start();
}

DataProcessor::~DataProcessor()
{
    m_dealfullCond.wakeAll();
    //清理DataDealThread

    if (m_hDataDealThread != nullptr)
    {
        delete m_hDataDealThread;
        m_hDataDealThread = nullptr;
    }

    if(m_curDealDataBuffer)
    {
        delete []m_curDealDataBuffer;
    }
}

void DataProcessor::dealWake()
{
    m_dealfullCond.wakeAll();
}

void DataProcessor::appendData(const BYTE *pData, UINT32 iLen)
{
    m_transDataCircularBuffer.write_data((DataCircularBuffer::TRANS_DATA_PTR)pData, iLen);
    //dealWake();
}

void DataProcessor::set_data_deal_func_callback(data_deal_func_callback _data_deal_func_callback)
{
    m_data_deal_func_callback = std::move(_data_deal_func_callback);
}


void DataProcessor::data_deal_thread_func(void *pParam)
{
//    m_dealfullCond.wait(&m_dealmutex);

    deal_data();
    stdutils::OriDateTime::sleep(1);
}

void DataProcessor::deal_data()
{
#if 1
    std::list<UINT32> _packslenlist;
    m_transDataCircularBuffer.get_buffer_packs_len(_packslenlist);

    for(int i = 0 ;i < _packslenlist.size();i++)
    {
        memset(m_curDealDataBuffer, 0, DEAL_DATA_BUFFER_SIZE);
        UINT32 iBufferLen = _packslenlist.front();
        bool bRet = m_transDataCircularBuffer.read_data(m_curDealDataBuffer, iBufferLen);
        if(bRet && m_data_deal_func_callback)
        {
            _packslenlist.pop_front();
            m_data_deal_func_callback(m_curDealDataBuffer, iBufferLen);
        }
    }
#else
    while(!m_transDataCircularBuffer.isEmpty())
    {
        memset(m_curDealDataBuffer, 0, DEAL_DATA_BUFFER_SIZE);
        UINT32 iBufferLen = 0;
        bool bRet = m_transDataCircularBuffer.read_data_step(m_curDealDataBuffer, iBufferLen);
        if(bRet && iBufferLen != 0 && m_data_deal_func_callback)
        {
            std::cout<<"will append data to deal ----------------- "<<iBufferLen<<std::endl;
            m_data_deal_func_callback(m_curDealDataBuffer, iBufferLen);
        }
    }
    m_transDataCircularBuffer.clear_packslen();
#endif
}
#else
namespace gaeactortransmit
{
#endif
GaeactorTransmitBase::GaeactorTransmitBase(const E_TRANSMIT_TYPE &transmitType, QObject *parent)
    :m_transmitType(transmitType)
    ,m_preceive_callback(nullptr)
{

}

GaeactorTransmitBase::~GaeactorTransmitBase()
{

}

void GaeactorTransmitBase::transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen)
{

}

void *GaeactorTransmitBase::loanTransmitBuffer(UINT32 iLen)
{
    return nullptr;
}

void GaeactorTransmitBase::publish()
{

}

void GaeactorTransmitBase::init(CHANNEL_INFO *channelinfo)
{
    m_channelinfo = channelinfo;
}

CHANNEL_INFO *GaeactorTransmitBase::channelinfo()
{
    return m_channelinfo;
}

E_TRANSMIT_TYPE GaeactorTransmitBase::transmitType() const
{
    return m_transmitType;
}

void GaeactorTransmitBase::setTransmitType(E_TRANSMIT_TYPE newTransmitType)
{
    m_transmitType = newTransmitType;
}

void GaeactorTransmitBase::setDataCallback(receive_callback func)
{
    m_preceive_callback = std::move(func);
}

const std::string &GaeactorTransmitBase::getUlidStr() const
{
    return m_ulidstr;
}

void GaeactorTransmitBase::setUlidstr(const std::string &newUlidstr)
{
    m_ulidstr = newUlidstr;
}

void GaeactorTransmitBase::setPRuntime(iox::runtime::PoshRuntime *newPRuntime)
{
    m_pRuntime = newPRuntime;
}

iox::runtime::PoshRuntime *GaeactorTransmitBase::pRuntime() const
{
    return m_pRuntime;
}


} // namespace gaeactortransmit
