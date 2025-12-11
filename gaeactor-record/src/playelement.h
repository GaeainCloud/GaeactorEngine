#ifndef PLAYELEMENT_H
#define PLAYELEMENT_H
#include <QMap>
#include <QVector>
#include "basedef.h"


namespace aosStream {
class BaseDevice;
}

namespace originaldatastoragestd {
class OriginalDataInputManager;
}
class datadeal_thread;
class PlayElement : public QObject
{
    Q_OBJECT
public:
    PlayElement(const QString& elename,QObject *parent = nullptr);
    virtual ~PlayElement() override;
    bool slotplay(const tagReplayItemInfo *item);
    void start();
    void stop();
    void setReadSpeed(float readSpeed);
    float getReadSpeed();
    void play();
    bool getPlayStatus();

    quint64 jumpToDataPos(quint64 iJumpDataPos);
    quint64 jumpToDataMillisecondPos(quint64 iJumpDataMillisecondPos);
    quint64 jumpToDataMillisecondPosOffset(quint64 iJumpDataMillisecondPosOffset);
    quint64 jumpToDataMillisecondPosOffsetPercent(double percent);
    quint64 jumpToDataPosPercent(double percent);

    quint64 getIDataFrames() const;

    const tagReplayItemInfo *getCurrentitem() const;

    QString getElename() const;

protected:
    bool initNetDevice();
    static bool dealDataSourceCallback (void *pObject, unsigned char *pData, unsigned int iDataLen, long long iTimeStamp, long long iGlobeFileReadBeginValidDataPos,long long iDataSendTimeStamp);
    static void dataDecodeCallback(QObject *pObject, const QByteArray& msg, long long iTimeStamp,long long iGlobeFileReadValidDataPos,long long iDataSendTimeStamp);
    void decodestateData( unsigned char *pData, unsigned int iDataLen, long long iTimeStamp,long long iGlobeFileReadValidDataPos,long long iDataSendTimeStamp, bool bCompressed);
    aosStream::BaseDevice* AddDevice(uint64_t uDeviceID, QObject *parent, QJsonObject &device_jo);
    QString GetRandPipeName();
    void queryOriginData(const QString &pchFileName, \
                                      tagReplayItemInfo *ptagReplayItemInfo, \
                                      originaldatastoragestd::OriginalDataInputParseManager *pOriginalDataInputManager);
    bool readPart(const QString& savedir,\
                             originaldatastoragestd::OriginalDataInputParseManager * pOriginalDataInputManager,\
                             qint64 istartTimeStampSection ,\
                             qint64 iendTimeStampSection,
                             qint64 istart,
                             qint64 iend, qint64 ibegin);
    void requestDealData();
    bool prepareStateplayerData(const QString& savedir, \
                                originaldatastoragestd::OriginalDataInputParseManager * pOriginalDataInputManager, \
                                qint64 istartTimeStampSection , \
                                qint64 iendTimeStampSection,
                                qint64 istart,
                                qint64 iend);

    QJsonObject decodeData_to_jsonobj(unsigned char *pData, unsigned int iDataLen, bool bCompressed);
    void deal_callback_data(unsigned char *pData, unsigned int iDataLen, long long iTimeStamp, long long iGlobeFileReadBeginValidDataPos,long long iDataSendTimeStamp);
signals:
    void sendDataSignal(QByteArray array,qint64 iGlobeFileReadBeginValidDataPos,qint64 iDataSendTimeStamp);
    void setEnableSignal(bool bEnable);
    void decodeEndSig();
protected slots:
    void sendDataSlot(QByteArray array,qint64 iGlobeFileReadValidDataPos,qint64 iDataSendTimeStamp);
    void dealData();
    void decodeEndSlot();
protected:
    aosStream::BaseDevice* m_devices;
    originaldatastoragestd::OriginalDataInputManager * m_pOriginalDataInputManager;
    const tagReplayItemInfo * m_currentitem;
    quint64 m_iDataFrames;
    QString m_elename;
    QThread *m_pThread; // 线程处理数据
    datadeal_thread* m_pDataDecodeThread;

    QVector<QString> m_packAliasfilters;
    QVector<QString> m_msgkeyfilterss;

};



#endif // PLAYELEMENT_H
