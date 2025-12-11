#ifndef GAEACTOR_TRANSMIT_MANAGER_H
#define GAEACTOR_TRANSMIT_MANAGER_H
#include <QObject>

#include "head_define.h"


#ifdef USING_GAEACTOR_EXPORT_LIB
#include "gaeactor_global.h"
#endif


#ifdef USING_GAEACTOR_TRANSMIT
#include "gaeactor_transmit_interface.h"
#else
#include "gaeactor_transmit_define.h"
#endif

#include "src/OriginalMutex.h"
#include "src/OriginalDynamicBuffer.h"
#include "src/OriginalThread.h"
#include <QMutex>
namespace gaeactoragentcores {
class GaeactorAgentCores;
}
namespace gaeactoragentsensors {
class GaeactorAgentSensors;
}
#ifdef USING_GAEACTOR_TRANSMIT
namespace gaeactortransmit{
class GaeactorTransmit;
}
#endif

class dealStorageDataThread
{
public:
#ifdef USING_GAEACTOR_TRANSMIT
    dealStorageDataThread(gaeactortransmit::GaeactorTransmit *pGaeactorTransmit,UINT32 buffer_size, E_CHANNEL_TRANSMITDATA_TYPE type);
#else
    dealStorageDataThread(UINT32 buffer_size, E_CHANNEL_TRANSMITDATA_TYPE type);
#endif
    ~dealStorageDataThread();
    void create_resource();
    void release_resource();
    void data_deal_thread_func(void *pParam);

    bool popfrontDataToDeal();
    bool pushbackData(const BYTE *pData, UINT32 iDataLen, const TIMESTAMP_TYPE &iTimeStamp);
    void wake();
private:
    stdutils::DATA_STORAGE_CIRCULAR_BUFFER  * m_pDataCircularBuffer;
    BYTE *m_pCurrentDataDealBuffer;
    UINT32 m_idealDataMaxBufferLen;
    stdutils::OriThread* m_hDataDealThread;
    E_CHANNEL_TRANSMITDATA_TYPE m_type;
    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;
#ifdef USING_GAEACTOR_TRANSMIT
    gaeactortransmit::GaeactorTransmit *m_pGaeactorTransmit;
#endif
};



#include <QReadWriteLock>



#define USING_THREAD_NUM 1
class GAEACTOR_EXPORT GaeactorTransmitManager : public QObject
{
    Q_OBJECT
public:
    enum E_DATA_TYPE
    {
        E_DATA_TYPE_POS,
        E_DATA_TYPE_POS_ATT,
        E_DATA_TYPE_SENSOR
    };

    static GaeactorTransmitManager & getInstance(QString transmit_channel_title ="");
    virtual ~GaeactorTransmitManager();

    QString transmit_channel_id() const;
    void set_transmit_channel_id(const QString &new_transmit_channel_id);
    void deal_step_refresh_event();
    void set_refresh_event_enable(bool bEnbale);
    void data_receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE*pdata, const UINT32& ilen, const BYTE *pOrignaldata, const UINT32 &iOrignallen);
#ifdef USING_GAEACTOR_EXPORT_LIB
    void setReceive_callback(receive_callback newReceive_callback);
    void trans_receive_callback(void* usrpayload, size_t isize);
    void trans_type_receive_callback(E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType, const BYTE *pData, size_t isize);
#endif
private:

    void dealEventUpdateCallback(IDENTIFI_EVENT_INFO &eventlist);
    void dealSensorUpdateCallback(const TYPE_ULID& ulidsrc , E_EVENT_MODE type);
    void displayInteractionsHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayInteractionsEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist,const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave);
    void displayInteractionsListEchoWaveHexidxCallback(const std::list<std::tuple<TYPE_ULID, EVENT_TUPLE, HEXIDX_HGT_ARRAY, QVector<LAT_LNG>, bool> > &result);

    void displayAuditionsHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayAuditionsEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave);



    void displayPosCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const transdata_entityposinfo& posinfo, E_DISPLAY_MODE eDdisplayMode);

    void displayHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist,const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave);



    void dealHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
    void dealEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE &echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist, const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave);


private:
    explicit GaeactorTransmitManager(const QString &transmit_channel_title ="", QObject *parent = nullptr);

    void init(const QString& transmit_channel_title);
    void initBuffer();
    void transmitinentifieventinfo(IDENTIFI_EVENT_INFO &eventinfo);
    void transformHexPosData(const BYTE*pdata, const UINT32& ilen);
    void transformHexPosArrayData(const BYTE*pdata, const UINT32& ilen);

    void transformHexPosattData(const BYTE*pdata, const UINT32& ilen);
    void transformHexPosattArrayData(const BYTE*pdata, const UINT32& ilen);
    void transformHexSensorData(const BYTE*pdata, const UINT32& ilen);
    void transformHexSensorArrayData(const BYTE*pdata, const UINT32& ilen);

    void transformGetSensorPathData(const BYTE*pdata, const UINT32& ilen);

    //void PrintTime();
    // 定时事件回调函数
    //    static void onTime(evutil_socket_t sock, short event, void *arg);
    //    void startEvt();
    //    void StartEvent();

    void pushbuffer(const TYPE_ULID &ulid, E_DATA_TYPE type, QByteArray &&by, const TYPE_ULID &sensingmediaid = 0);
#ifndef data_deal_thread_func
    void data_deal_thread_func(void *pParam);
#endif

    tagPathInfo getSensorPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst);

    CHANNEL_INFO* applyforShareChannel();
    void* loanTransmitBuffer(const CHANNEL_INFO* channelinfo,UINT32 iLen);
    void freeData(void *pSrcData);
private:
    void dealtransformHexPosDataSlot(QByteArray &&by);
    void dealtransformHexPosArrayDataSlot(QVector<QByteArray>&& bylist);

    void dealtransformHexPosattDataSlot(QByteArray&& by);
    void dealtransformHexPosattArrayDataSlot(QVector<QByteArray>&& bylist);
    void dealtransformSensorDataSlot(QByteArray&& by);
    void dealtransformSensorArrayDataSlot(QVector<QByteArray>&& bylist);


    void dealtransformGetSensorPathDataSlot(QByteArray&& by);
private slots:


    void deal_pos_data_func_callback(const QByteArray &databy);
    void deal_posatt_data_func_callback(const QByteArray& databy);

    void deal_pos_array_data_func_callback(const QVector<QByteArray>& bylist);
    void deal_posatt_array_data_func_callback(const QVector<QByteArray>& bylist);

    void deal_sensor_data_func_callback(const QByteArray &databy);
    void deal_sensor_array_data_func_callback(const QVector<QByteArray>& bylist);
private:
    gaeactoragentcores::GaeactorAgentCores* m_pGaeactorAgentCores;
    gaeactoragentsensors::GaeactorAgentSensors* m_pGaeactorAgentSensors;

    dealStorageDataThread *m_pPosDataCircularBufferThread;
    dealStorageDataThread *m_pWaveDataCircularBufferThread;
    dealStorageDataThread *m_pInteractionsDataCircularBufferThread;
    dealStorageDataThread *m_pEventDataCircularBufferThread;
    dealStorageDataThread *m_pSensorUpdateDataCircularBufferThread;

    //    struct event tickEvt;
    //    struct event_base *baseEvt;


    QReadWriteLock m_entitydatabuf_mutex;
    std::unordered_map<TYPE_ULID,std::tuple<QByteArray, E_DATA_TYPE, bool>> m_entitydatabuf;
    QReadWriteLock m_snesordatabuf_mutex;
    std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>,std::tuple<QByteArray, E_DATA_TYPE, bool>> m_snesordatabuf;

    std::atomic_bool m_bChecking{false};
#ifndef DATA_DEAL_MODE_QUEUE
    struct threadParam
    {
        int id;
    }m_hDataDealThreadParam[USING_THREAD_NUM];
    stdutils::OriThread* m_hDataDealThread[USING_THREAD_NUM];
#endif
#ifdef USING_GAEACTOR_TRANSMIT
    gaeactortransmit::GaeactorTransmit *m_pGaeactorTransmit;
#endif
#ifdef USING_GAEACTOR_EXPORT_LIB
    receive_callback m_receive_callback;
#endif
};
#endif // GAEACTOR_TRANSMIT_MANAGER_H
