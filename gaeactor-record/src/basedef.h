#ifndef BASEDEFINE_H
#define BASEDEFINE_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

//#define START_SOLOQ

//#define LOAD_RECORD_MAP_FILE
//#define JUMP_BY_DATAPOS
#define SLIDER_MIN (0)
#define SLIDER_MAX (1000000.0f)
#define SLIDER_RANGE (SLIDER_MAX - SLIDER_MIN)

#define SPEED_STEP (0.02f)
//#define ENABLE_SEND


enum ENUM_REPLAY_TYPE
{
    ENUM_REPLAY_TYPE_NULL,
    ENUM_REPLAY_TYPE_SINGLE_FILE,
    ENUM_REPLAY_TYPE_MISSION,
};
struct tagReplayItemInfoBase
{
        int m_index;
        QString m_titlename;
        ENUM_REPLAY_TYPE m_eType;

        QJsonObject m_netconfig;
        tagReplayItemInfoBase()
        {
            m_index = -1;
            m_titlename = "";
            m_eType = ENUM_REPLAY_TYPE_NULL;

            m_netconfig.insert("type","tcpServer");
            QJsonObject info;
            info.insert("localHost","127.0.0.1");
            info.insert("localPort",32606);
            m_netconfig.insert("info",info);
            m_netconfig.insert("plugin",QJsonArray());
        }
        virtual ~tagReplayItemInfoBase(){}
};
struct tagReplayItemInfo:public tagReplayItemInfoBase
{
    bool m_bSelected;
    quint64 m_size;
    QString m_date;
    QString m_filename;
    QString m_absoluteFilePath;

    qint64 m_iTimeStampStart;
    qint64 m_iTimeStampEnd;
    quint64 m_iValidDataTotalLen;
    quint64 m_iFrames;

    quint64 m_startPos;
    quint64 m_startFrames;
    qint64 m_startPosMillseconds;
    tagReplayItemInfo()
    {
        m_size = 0;
        m_bSelected = false;
        m_date = "";
        m_filename = "";
        m_absoluteFilePath = "";
        m_iTimeStampStart = 0;
        m_iTimeStampEnd = 0;
        m_iValidDataTotalLen = 0;
        m_iFrames = 0;
        m_startPos = 0;
        m_startFrames = 0;
        m_eType = ENUM_REPLAY_TYPE_SINGLE_FILE;
        m_startPosMillseconds = 0;
    }

    qint64 getTotalMSecs() const
    {
        QDateTime startTime = QDateTime::fromMSecsSinceEpoch(m_iTimeStampStart);
        QDateTime endTime = QDateTime::fromMSecsSinceEpoch(m_iTimeStampEnd);
        qint64 msecs = startTime.msecsTo(endTime);
        return msecs;
    }
};


struct tagReplayItemMissionInfo:public tagReplayItemInfoBase
{
    QJsonObject m_redocrddata;
    QString m_dateStr;
    QString m_signatureStr;
    qint64 m_mtimeutc;

    tagReplayItemMissionInfo()
    {
        m_dateStr = "";
        m_signatureStr = "";
        m_mtimeutc = 0;
        m_eType = ENUM_REPLAY_TYPE_MISSION;
    }
    tagReplayItemMissionInfo(int index,
                             QJsonObject redocrddata,
                             QString dateStr,
                             QString signatureStr,
                             qint64 mtimeutc)
    {
        m_index = index;
        m_redocrddata = redocrddata;
        m_dateStr = dateStr;
        m_signatureStr = signatureStr;
        m_mtimeutc = mtimeutc;
        m_eType = ENUM_REPLAY_TYPE_MISSION;
    }
};


#endif // BASEDEFINE_H
