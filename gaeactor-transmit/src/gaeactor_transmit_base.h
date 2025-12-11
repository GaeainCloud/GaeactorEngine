#ifndef GAEACTOR_TRANSMIT_BASE_H
#define GAEACTOR_TRANSMIT_BASE_H

#include <QObject>
#include "gaeactor_transmit_define.h"
#include <QHash>
#include <QList>
#include <QVector>
#define USING_SINGLE_CHANNEL

#ifdef USING_SINGLE_CHANNEL
#include <QMutex>
#include "src/OriginalThread.h"
#include "src/OriginalMutex.h"
#endif

#define MAX_PUBLISHER_NUM (1024)
#define PRE_PUBLISHER_USE_ITEM_NUM (2*100000/MAX_PUBLISHER_NUM)

namespace iox {
namespace runtime {
class PoshRuntime;
}
}

namespace gaeactortransmit {

#ifdef USING_SINGLE_CHANNEL
class DataCircularBuffer{
public:
    typedef BYTE TRANS_DATA;
    typedef BYTE* TRANS_DATA_PTR;
    DataCircularBuffer();
    ~DataCircularBuffer();
    void init(int maxcount = 10*1024*1024);
    // 存储数据
    bool write_data(TRANS_DATA_PTR const pParam, UINT32 iParamLen);

    // 读取数据
    bool read_data(TRANS_DATA_PTR const pBuffer, UINT32 iBufferLen);

    bool read_data_step(TRANS_DATA_PTR const pBuffer, UINT32& iBufferLen);

    bool read_data_direct(TRANS_DATA_PTR const pBuffer, UINT32 iBufferLen,UINT32 iPackcount);

    bool get_next_pack_len(UINT32 &iBufferLen);

    bool get_buffer_packs_len(std::list<UINT32> &_packslenlist);
    UINT32 get_buffer_bytes_count();

    void clear_packslen();

    bool isEmpty();
private:
    QMutex m_databuf_mutex;
    TRANS_DATA_PTR m_pImportDataBuffer;									// 输入数据缓存
    UINT32 m_iImportDataBufferLen;								// 输入数据缓存长度
    UINT32 m_iImportDataBufferWriteIndex;					// 输入数据缓存写入位置
    UINT32 m_iImportDataBufferReadIndex;					// 输入数据缓存读取位置的前1位置
    UINT32 m_iBufferBytesCount;
    std::list<UINT32> m_packslenlist;

};



typedef std::function<void (const BYTE* pData, UINT32 iLen)> data_deal_func_callback;

class DataProcessor:public QObject
{
    Q_OBJECT
public:

    DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();
    void dealWake();

    void appendData(const BYTE* pData, UINT32 iLen);

    void set_data_deal_func_callback(data_deal_func_callback _data_deal_func_callback);
private:
    void data_deal_thread_func(void *pParam);

    void deal_data();
private:
    stdutils::OriThread* m_hDataDealThread;
    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;

    DataCircularBuffer m_transDataCircularBuffer;

    data_deal_func_callback m_data_deal_func_callback;
    DataCircularBuffer::TRANS_DATA_PTR m_curDealDataBuffer;
};
#endif

class GaeactorTransmitBase : public QObject
{
    Q_OBJECT
public:
    explicit GaeactorTransmitBase(const E_TRANSMIT_TYPE& transmitType, QObject *parent = nullptr);
    virtual ~GaeactorTransmitBase();
    virtual void transmitData(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE * pData, UINT32 iLen);
    virtual void* loanTransmitBuffer(UINT32 iLen);
    virtual void publish();
    virtual void init(CHANNEL_INFO *channelinfo );

    CHANNEL_INFO* channelinfo();
    E_TRANSMIT_TYPE transmitType() const;
    void setTransmitType(E_TRANSMIT_TYPE newTransmitType);

    void setDataCallback(receive_callback func);
    const std::string& getUlidStr() const;
    void setUlidstr(const std::string &newUlidstr);



    void setPRuntime(iox::runtime::PoshRuntime *newPRuntime);

    iox::runtime::PoshRuntime *pRuntime() const;
protected:
    E_TRANSMIT_TYPE m_transmitType;
    receive_callback m_preceive_callback;
    std::string m_ulidstr;

    iox::runtime::PoshRuntime* m_pRuntime;

    CHANNEL_INFO *m_channelinfo;
#ifdef USING_SINGLE_CHANNEL
    DataProcessor * m_DataProcessor;
#endif
};
}
#endif // GAEACTOR_TRANSMIT_BASE_H
