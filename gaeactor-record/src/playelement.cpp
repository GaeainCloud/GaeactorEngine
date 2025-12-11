#pragma execution_character_set("utf-8")
#include "playelement.h"
#include "src/storage/OriginalDataInputManager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

#include <QDir>
#include <QTextStream>

#define PRE_PARSE_SIZE  (1800*1000)


#define USING_COMPRESS_STATE
#include "Helper/datadeal_thread.h"
#include "Helper/jsonprocessor.h"

PlayElement::PlayElement(const QString& elename,QObject *parent)
    : QObject(parent),
    m_devices(nullptr),
    m_pOriginalDataInputManager(nullptr),
    m_currentitem(nullptr),
    m_iDataFrames(0),
    m_elename(elename),
    m_pDataDecodeThread(nullptr)
{
    connect(this,&PlayElement::sendDataSignal,this,&PlayElement::sendDataSlot);
    m_pThread = new QThread(this);
    connect(m_pThread, &QThread::started, this,&PlayElement::dealData,Qt::DirectConnection);
    connect(this,&PlayElement::decodeEndSig,this,&PlayElement::decodeEndSlot);
}

PlayElement::~PlayElement()
{
    if(m_pOriginalDataInputManager)
    {
        m_pOriginalDataInputManager->clear();
        delete m_pOriginalDataInputManager;
        m_pOriginalDataInputManager = nullptr;
    }

    if(m_pDataDecodeThread)
    {
        delete m_pDataDecodeThread;
        m_pDataDecodeThread = nullptr;
    }

    if(m_pThread)
    {
        m_pThread->quit();
        m_pThread->wait();
        m_pThread->deleteLater();
    }
}


bool PlayElement::slotplay(const tagReplayItemInfo *item)
{    
    m_iDataFrames = 0;
    m_currentitem = item;
    if(initNetDevice())
    {
        if(nullptr == m_pOriginalDataInputManager)
        {
            m_pOriginalDataInputManager = new originaldatastoragestd::OriginalDataInputParseManager();
            auto obj = aos::AosKernelCommon::read_json_file_object("./apps/appcontainers/Understream/cfg/decodeplugin.json");
            auto objfilters = aos::AosKernelCommon::read_json_file_object("./apps/appcontainers/Understream/cfg/decodefilter.json");

            std::vector<std::string> decodepluginlist;
            std::vector<QJsonObject> decodepluginlistSettings;

            auto decodeplugins = obj.value("decodeplugins").toArray();
            for(auto item:decodeplugins)
            {
                auto pluginobj = item.toObject();
                decodepluginlist.push_back(pluginobj.value("pluginname").toString().toStdString());
                decodepluginlistSettings.push_back(pluginobj.value("settting").toObject());
            }

            auto msgkeyfilters = objfilters.value("msgkeyfilters").toArray();
            for(auto item:msgkeyfilters)
            {
                m_msgkeyfilterss.push_back(item.toString());
            }

            auto packAliasfilters = objfilters.value("packAliasfilters").toArray();
            for(auto item:packAliasfilters)
            {
                m_packAliasfilters.push_back(item.toString());
            }
            m_pOriginalDataInputManager->setdecodepluginlist(decodepluginlist,decodepluginlistSettings);

            if(nullptr == m_pDataDecodeThread)
            {
                m_pDataDecodeThread = new datadeal_thread(datadeal_thread::E_DEAL_TYPE_DECODE,this, &PlayElement::dataDecodeCallback,nullptr);
            }
        }
        emit setEnableSignal(false);

        const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(m_currentitem);
        if(pitem)
        {
            m_pOriginalDataInputManager->clear();
            m_pOriginalDataInputManager->initialize(pitem->m_absoluteFilePath.toStdString().c_str(),pitem->m_filename.toStdString().c_str(),this,&PlayElement::dealDataSourceCallback);
#ifdef JUMP_BY_DATAPOS
            m_iDataFrames = m_pOriginalDataInputManager->jumpToDataPos(pitem->m_startPos)-1;
#else
            m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(static_cast<unsigned long long>(pitem->m_startPosMillseconds))-1;
#endif
            //调用一次播放，调用两次是停止，默认停止好一些，避免aos.exe/PhugiaEntities.exe/SoloQ.exe未启动完成

#if 1
            m_pOriginalDataInputManager->stop();
#else
            play();
#endif
            requestDealData();
        }
        return true;
    }
    return false;
}

bool PlayElement::prepareStateplayerData(const QString &savedir,\
                                         originaldatastoragestd::OriginalDataInputParseManager * pOriginalDataInputManager, \
                                         qint64 istartTimeStampSection, \
                                         qint64 iendTimeStampSection,
                                         qint64 istart,
                                         qint64 iend)
{
    if(pOriginalDataInputManager)
    {
        std::vector<std::pair<long long,std::vector<char>>>  querydata = pOriginalDataInputManager->querySectionData(istart,iend,false);

        QString pluginsavedir = QString("%1/").arg(savedir);
        QDir dir1(pluginsavedir);
        if(!dir1.exists())
        {
            if(!dir1.mkpath(pluginsavedir))
            {
                return false;
            }
        }
        QString filename = QString("%1_%2.csv").arg(QDateTime::fromMSecsSinceEpoch(istartTimeStampSection).toUTC().toString("yyyy-MM-dd-hh-mm-ss-zzz")).arg(QDateTime::fromMSecsSinceEpoch(iendTimeStampSection).toUTC().toString("yyyy-MM-dd-hh-mm-ss-zzz"));

        QString filepath = pluginsavedir+ filename;
        QFile aFile(filepath);
        if (aFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qint64 TimeStamp = 0;
            QTextStream aStream(&aFile); //用文本流读取文件
            for(auto item:querydata)
            {
                QJsonObject stateobj = decodeData_to_jsonobj((unsigned char *)item.second.data(), item.second.size(), true);
                bool bAppend = false;
                if(stateobj.contains("packAlias"))
                {
                    QString packAlias =  stateobj.value("packAlias").toString();
                    if(m_packAliasfilters.isEmpty())
                    {
                        bAppend = true;
                    }
                    else
                    {
                        if(m_packAliasfilters.contains(packAlias))
                        {
                            bAppend = true;
                        }
                    }
                }
                else
                {
                    bAppend = true;
                }

                if(bAppend)
                {
                    QDateTime dt = QDateTime::fromMSecsSinceEpoch(item.first).toUTC();
                    qint64 interval = (TimeStamp == 0) ? 0 : item.first - TimeStamp;
                    QString timedir = dt.toString("yyyy-MM-dd hh:mm:ss.zzz")+","+QString::number(item.first)+","+QString::number(interval)+": ";
                    TimeStamp = item.first;

                    timedir += aos::AosKernelCommon::json_object_to_string(stateobj);
                    timedir += "\n";
                    aStream<<timedir; //写入文本流
                }
            }
        }
        aFile.close();
        return true;
    }
    return false;
}

QJsonObject PlayElement::decodeData_to_jsonobj(unsigned char *pData, unsigned int iDataLen, bool bCompressed)
{
    if(bCompressed)
    {
        return JsonProcessor::deserialized_to_jsonobj((const unsigned char * const )pData, iDataLen);
    }
    return aos::AosKernelCommon::string_to_json_object(QString(QByteArray((char *)pData, iDataLen)));
}

void PlayElement::deal_callback_data(unsigned char *pData, unsigned int iDataLen, long long iTimeStamp, long long iGlobeFileReadBeginValidDataPos, long long iDataSendTimeStamp)
{
    auto inputstorageType = m_pOriginalDataInputManager->getInputStorageType();
//    inputstorageType = originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS;
    switch (inputstorageType)
    {
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
    {
        QByteArray array ((const char *)pData,iDataLen);
//        const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(pDataBack->m_currentitem);
//        if(pitem)
//        {
//            QDateTime datetime  = QDateTime::fromMSecsSinceEpoch(iTimeStamp);
//            QDateTime currentDateTime =  QDateTime::currentDateTime().toUTC();;
//            qDebug()<<pDataBack->formatTimeStr( iDataSendTimeStamp - pitem->m_iTimeStampStart)<<"_"<<pDataBack->formatTimeStr(pDataBack->m_payStart.msecsTo(currentDateTime))<<"_"<<pDataBack->formatTimeStr(pDataBack->m_payStart.msecsTo(datetime));
//            qDebug()<<datetime.toString("yyyy-MM-dd hh:mm:ss.zzz")<<"len "<< iDataLen<< ":"<<array.toHex(' ');
//        }
        emit this->sendDataSignal(array, iGlobeFileReadBeginValidDataPos,iDataSendTimeStamp);
    }
        break;
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
    {
        this->decodestateData(pData,iDataLen,iTimeStamp,iGlobeFileReadBeginValidDataPos,iDataSendTimeStamp, true);
    }
        break;
    default:
        return;
    }
}

QString formatTimeStr(qint64 msecs)
{
    qint64 hh = msecs/(3600*1000);
    qint64 mm = (msecs-hh*(3600*1000))/(60*1000);
    qint64 ss = (msecs-hh*(3600*1000)-mm*(60*1000))/1000;
    qint64 ms = (msecs-hh*(3600*1000)-mm*(60*1000)-ss*1000)%1000;
    QString timestr = QString("%1:%2:%3.%4").arg(QString::number(hh),2,'0').arg(QString::number(mm),2,'0').arg(QString::number(ss),2,'0').arg(QString::number(ms),3,'0');
    return timestr;
}


bool PlayElement::readPart(const QString& savedir,\
                         originaldatastoragestd::OriginalDataInputParseManager * pOriginalDataInputManager,\
                         qint64 istartTimeStampSection ,\
                         qint64 iendTimeStampSection,
                         qint64 istart,
                         qint64 iend, qint64 ibegin)
{
    std::map<std::string, originaldatastoragestd::tagParesFrames> paresFramesMap = pOriginalDataInputManager->querySectionParesFrameData(static_cast<unsigned long long>(istart), static_cast<unsigned long long>(iend),true);

    std::map<std::string, originaldatastoragestd::tagParesFrames>::const_iterator ctor = paresFramesMap.cbegin();
    while(ctor != paresFramesMap.cend())
    {
        const std::string & key = ctor->first;
        const originaldatastoragestd::tagParesFrames &paresFrames = ctor->second;

       QString pluginsavedir = QString("%1%2/").arg(savedir).arg(QString::fromStdString(key));
       QDir dir1(pluginsavedir);
       if(!dir1.exists())
       {
           if(!dir1.mkpath(pluginsavedir))
           {
               ctor++;
               continue;
           }
       }
       QString filename = QString("%1_%2.csv").arg(QDateTime::fromMSecsSinceEpoch(istartTimeStampSection).toUTC().toString("yyyy-MM-dd-hh-mm-ss-zzz")).arg(QDateTime::fromMSecsSinceEpoch(iendTimeStampSection).toUTC().toString("yyyy-MM-dd-hh-mm-ss-zzz"));

       QString filepath = pluginsavedir+ filename;
       QFile aFile(filepath);
       if (aFile.open(QIODevice::WriteOnly | QIODevice::Text))
       {           
           qint64 TimeStamp = 0;
           QTextStream aStream(&aFile); //用文本流读取文件
           for(size_t index = 0; index < paresFrames.m_paresData.size();index++)
           {
               const originaldatastoragestd::tagTimeStampMsgkeyMap& timestampmsgkeymap = paresFrames.m_paresData.at(index);
               QDateTime dt = QDateTime::fromMSecsSinceEpoch(timestampmsgkeymap.m_TimeStamp).toUTC();

               qint64 interval = (TimeStamp == 0) ? 0 : timestampmsgkeymap.m_TimeStamp - TimeStamp;
               QString timedir = dt.toString("yyyy-MM-dd hh:mm:ss.zzz")+","+QString::number(timestampmsgkeymap.m_TimeStamp)+","+QString::number(interval)+":\n";
               std::vector<originaldatastoragestd::tagMsgkeyFramesMap>::const_iterator itor3 =  timestampmsgkeymap.msgkeyFramesMap.cbegin();
               bool bAppendFrame = false;
               QString timetostart = formatTimeStr(timestampmsgkeymap.m_TimeStamp-ibegin);
               while(itor3 != timestampmsgkeymap.msgkeyFramesMap.cend())
               {
                   QString msgkeytmp = QString(QByteArray(itor3->m_msgkey.data(),itor3->m_msgkey.size()));
                   bool bAppend = false;
                   if(m_msgkeyfilterss.isEmpty())
                   {
                       bAppend = true;
                   }
                   else
                   {
                       if(m_msgkeyfilterss.contains(msgkeytmp))
                       {
                           bAppend = true;
                       }
                   }
                   if(bAppend)
                   {
                       bAppendFrame = true;
                       timedir+= "\t"+QString(QByteArray(itor3->m_msgkey.data(),itor3->m_msgkey.size()))+"\n";
                       const std::vector<originaldatastoragestd::tagFrame> &frames = itor3->m_frames.m_frames;
                       std::vector<originaldatastoragestd::tagFrame>::const_iterator itor2 = frames.cbegin();
                       while (itor2 != frames.cend())
                       {
                           //                        QString str = QString("\tAllPackIndex: %1 TimeStampIndex: %2 TimeStamp: %3 Msgkey: %4 Data: %5\n").arg(QString::number(itor2->m_sectionmsgindex))
                           //                                .arg(QString::number(itor2->m_timestampmsgindex))
                           //                                .arg(QString::number(itor2->m_TimeStamp))
                           //                                .arg(QString(itor2->m_msgkey.toHex()))
                           //                                .arg(QString(itor2->m_frame.toHex()));
                           QString str = QString("\tallId: %1 TimeId: %2  timestamp:%3 utctime:%4 starttime:%5 frame: %6\n").arg(QString::number(itor2->m_sectionmsgindex))
                                   .arg(QString::number(itor2->m_timestampmsgindex))
                                   .arg(QString::number(itor2->m_TimeStamp))
                                   .arg(dt.toString("yyyy-MM-dd hh:mm:ss.zzz"))
                                   .arg(timetostart)
                                   .arg(QString(QByteArray(itor2->m_frame.data(),itor2->m_frame.size()).toHex()));
                           timedir+=str;

                           itor2++;
                       }
                   }
                   itor3++;
               }
               if(bAppendFrame)
               {
                   TimeStamp = timestampmsgkeymap.m_TimeStamp;
                   aStream<<timedir; //写入文本流
               }
           }
       }
       aFile.close();//关闭文件
       ctor++;
    }
    return true;
}

void PlayElement::requestDealData()
{
    if (m_pThread->isRunning() == true)
    {
        m_pThread->quit();
        m_pThread->wait();
        m_pThread->start();
    }
    else
    {
        m_pThread->start();
    }
}

void PlayElement::queryOriginData(const QString &pchFileName,\
                                  tagReplayItemInfo *ptagReplayItemInfo,\
                                  originaldatastoragestd::OriginalDataInputParseManager *pOriginalDataInputManager)
{
    if(ptagReplayItemInfo && pOriginalDataInputManager)
    {
        auto inputstorageType = pOriginalDataInputManager->getInputStorageType();
//        inputstorageType = originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS;

        int pre_size = PRE_PARSE_SIZE;
        switch (inputstorageType)
        {
        case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
        case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
        {
        }
            break;
        case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
        case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
        {
            pre_size = 5*60*1000;
        }
            break;
        default:
            return;
        }
        QStringList pathList = pchFileName.split("/");
        QString dirname = QString("%1/%2/%3").arg(pathList.at(pathList.length()-3)).arg(pathList.at(pathList.length()-2)).arg(pathList.at(pathList.length()-1));
        QString savedir = QDir::currentPath()+"/DecodeLog/"+dirname+"/";
        savedir = savedir.replace(' ','_');
        QDir dir1(savedir);
        if(!dir1.exists())
        {
            if(!dir1.mkpath(savedir))
            {
                return;
            }
        }

        qint64 istartTimeStamp = ptagReplayItemInfo->m_iTimeStampStart;
        qint64 iendTimeStamp = ptagReplayItemInfo->m_iTimeStampEnd;
        istartTimeStamp = istartTimeStamp<ptagReplayItemInfo->m_iTimeStampStart?ptagReplayItemInfo->m_iTimeStampStart:istartTimeStamp;
        iendTimeStamp = istartTimeStamp>ptagReplayItemInfo->m_iTimeStampEnd?ptagReplayItemInfo->m_iTimeStampEnd:iendTimeStamp;

        if((iendTimeStamp-istartTimeStamp)<pre_size)
        {
            qint64 istart = istartTimeStamp- ptagReplayItemInfo->m_iTimeStampStart;
            qint64 iend = iendTimeStamp - ptagReplayItemInfo->m_iTimeStampStart;

            switch (inputstorageType)
            {
            case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
            case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
            {
                if(!readPart(savedir,\
                             pOriginalDataInputManager,\
                             istartTimeStamp ,\
                             iendTimeStamp,
                             istart,
                             iend,
                             ptagReplayItemInfo->m_iTimeStampStart))
                {
                    return;
                }
            }
                break;
            case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
            case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
            {
                if(!prepareStateplayerData(savedir,\
                                           pOriginalDataInputManager,\
                                           istartTimeStamp ,\
                                           iendTimeStamp,
                                           istart,
                                           iend))
                {
                    return;
                }
            }
                break;
            default:
                return;
            }
        }
        else
        {
            int readTimes = (iendTimeStamp-istartTimeStamp)/pre_size;
            int readindex = 0;
            for(readindex = 0;readindex<readTimes;readindex++)
            {
                qint64 istartTimeStampSection = istartTimeStamp+pre_size*readindex;
                qint64 iendTimeStampSection = istartTimeStampSection + pre_size;
                qint64 istart = istartTimeStampSection- ptagReplayItemInfo->m_iTimeStampStart;
                qint64 iend = iendTimeStampSection - ptagReplayItemInfo->m_iTimeStampStart;
                switch (inputstorageType)
                {
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
                {
                    if(!readPart(savedir,\
                                 pOriginalDataInputManager,\
                                 istartTimeStampSection ,\
                                 iendTimeStampSection,
                                 istart,
                                 iend,
                                 ptagReplayItemInfo->m_iTimeStampStart))
                    {
                        return;
                    }
                }
                    break;
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
                {
                    if(!prepareStateplayerData(savedir,\
                                               pOriginalDataInputManager,\
                                               istartTimeStampSection ,\
                                               iendTimeStampSection,
                                               istart,
                                               iend))
                    {
                        return;
                    }
                }
                    break;
                default:
                    return;
                }
            }
            {

                qint64 istartTimeStampSection = istartTimeStamp+pre_size*readindex ;
                qint64 iendTimeStampSection = istartTimeStampSection + (iendTimeStamp-istartTimeStamp)%pre_size;
                qint64 istart = istartTimeStampSection- ptagReplayItemInfo->m_iTimeStampStart;
                qint64 iend = iendTimeStampSection - ptagReplayItemInfo->m_iTimeStampStart;

                switch (inputstorageType)
                {
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
                {
                    if(!readPart(savedir,\
                                 pOriginalDataInputManager,\
                                 istartTimeStampSection ,\
                                 iendTimeStampSection,
                                 istart,
                                 iend,
                                 ptagReplayItemInfo->m_iTimeStampStart))
                    {
                        return;
                    }
                }
                    break;
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
                case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
                {
                    if(!prepareStateplayerData(savedir,\
                                               pOriginalDataInputManager,\
                                               istartTimeStampSection ,\
                                               iendTimeStampSection,
                                               istart,
                                               iend))
                    {
                        return;
                    }
                }
                    break;
                default:
                    return;
                }
            }
        }
    }
    emit decodeEndSig();
}

void PlayElement::start()
{
    if(m_pOriginalDataInputManager )
    {
        m_pOriginalDataInputManager->start();
    }
}

void PlayElement::stop()
{
    if(m_pOriginalDataInputManager )
    {
        m_pOriginalDataInputManager->stop();
    }
}

void PlayElement::setReadSpeed(float readSpeed)
{
    if(m_pOriginalDataInputManager )
    {
        m_pOriginalDataInputManager->setReadSpeed(readSpeed);
    }
}

float PlayElement::getReadSpeed()
{
    if(!m_pOriginalDataInputManager )
    {
        return 1.0f;
    }
    return m_pOriginalDataInputManager->getReadSpeed();
}


quint64 PlayElement::jumpToDataPos(quint64 iJumpDataPos)
{
    if(m_pOriginalDataInputManager)
    {
        m_iDataFrames = m_pOriginalDataInputManager->jumpToDataPos(iJumpDataPos)-1;
    }
    return m_iDataFrames;
}

quint64 PlayElement::jumpToDataMillisecondPos(quint64 iJumpDataMillisecondPos)
{
    if(m_pOriginalDataInputManager)
    {
        const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(m_currentitem);
        if(pitem)
        {
            m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPos-pitem->m_iTimeStampStart)-1;
        }
    }
    return m_iDataFrames;
}

quint64 PlayElement::jumpToDataMillisecondPosOffset(quint64 iJumpDataMillisecondPosOffset)
{
    if(m_pOriginalDataInputManager)
    {
        m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPosOffset)-1;
    }
    return m_iDataFrames;
}

quint64 PlayElement::jumpToDataMillisecondPosOffsetPercent(double percent)
{
    if(m_pOriginalDataInputManager)
    {
        m_pOriginalDataInputManager->stop();
        quint64 iJumpDataMillisecondPos = 0;
         const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(m_currentitem);
         if(pitem)
         {
             iJumpDataMillisecondPos = pitem->getTotalMSecs() *percent;
         }
        m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPos)-1;
        m_pOriginalDataInputManager->start();
    }
    return m_iDataFrames;

}

quint64 PlayElement::jumpToDataPosPercent(double percent)
{
    if(m_pOriginalDataInputManager)
    {
        m_pOriginalDataInputManager->stop();
        quint64 iJumpDataPos = 0;
         const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(m_currentitem);
         if(pitem)
         {
             iJumpDataPos = pitem->m_iValidDataTotalLen * percent;
         }
        m_iDataFrames = m_pOriginalDataInputManager->jumpToDataPos(iJumpDataPos)-1;
        m_pOriginalDataInputManager->start();
    }
    return m_iDataFrames;
}

void PlayElement::sendDataSlot(QByteArray array, qint64 iGlobeFileReadValidDataPos, qint64 iDataSendTimeStamp)
{
    if(m_devices)
    {
        this->m_devices->WriteData(array,0);
    }
}

void PlayElement::dealData()
{
    if(m_currentitem)
    {
        originaldatastoragestd::OriginalDataInputParseManager * pOriginalDataInputManager = new originaldatastoragestd::OriginalDataInputParseManager();
        auto obj = aos::AosKernelCommon::read_json_file_object("./apps/appcontainers/Understream/cfg/decodeplugin.json");
        std::vector<std::string> decodepluginlist;
        auto decodeplugins = obj.value("decodeplugins").toArray();

        std::vector<QJsonObject> decodepluginlistSettings;
        for(auto item:decodeplugins)
        {
            auto pluginobj = item.toObject();
            decodepluginlist.push_back(pluginobj.value("pluginname").toString().toStdString());
            decodepluginlistSettings.push_back(pluginobj.value("settting").toObject());
        }
        pOriginalDataInputManager->setdecodepluginlist(decodepluginlist,decodepluginlistSettings);
        pOriginalDataInputManager->clear();
        pOriginalDataInputManager->initialize(m_currentitem->m_absoluteFilePath.toStdString().c_str(),m_currentitem->m_filename.toStdString().c_str(),this,&PlayElement::dealDataSourceCallback);
        pOriginalDataInputManager->jumpToDataMillisecondPos(m_currentitem->m_startPosMillseconds)-1;
        pOriginalDataInputManager->start();
        auto inputstorageType = originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_NULL;
        do
        {
            inputstorageType = pOriginalDataInputManager->getInputStorageType();
            QThread::sleep(5);
        }while(inputstorageType ==  originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_NULL);
        pOriginalDataInputManager->jumpToDataMillisecondPos(m_currentitem->m_startPosMillseconds)-1;
        pOriginalDataInputManager->stop();
        queryOriginData(m_currentitem->m_absoluteFilePath,const_cast<tagReplayItemInfo *>(m_currentitem) ,pOriginalDataInputManager);

        if(pOriginalDataInputManager)
        {
            delete pOriginalDataInputManager;
        }
    }
}

void PlayElement::decodeEndSlot()
{
    QMessageBox::information(nullptr,tr("Tips"),tr("end decode"));
}

QString PlayElement::getElename() const
{
    return m_elename;
}

const tagReplayItemInfo *PlayElement::getCurrentitem() const
{
    return m_currentitem;
}

quint64 PlayElement::getIDataFrames() const
{
    return m_iDataFrames;
}


void PlayElement::play()
{
    if(m_pOriginalDataInputManager )
    {
        if(m_pOriginalDataInputManager->getBRunning())
        {
            m_pOriginalDataInputManager->stop();
        }
        else
        {
            m_pOriginalDataInputManager->start();
        }
    }
}

bool PlayElement::getPlayStatus()
{
    if(nullptr == m_pOriginalDataInputManager )
    {
        return false;
    }
    return m_pOriginalDataInputManager->getBRunning();
}

aosStream::BaseDevice* PlayElement::AddDevice(uint64_t uDeviceID, QObject *parent, QJsonObject &device_jo)
{
    auto type = device_jo[aosStream::js_streams_devices_type].toString();
    auto info_jo = device_jo[aosStream::js_streams_devices_info].toObject();
    if(aosStream::js_sdt_tcpServer == type)
    {
        if(!info_jo.contains(aosStream::js_sdi_localPort)){
            return  nullptr;
        }
        aosStream::TCPServerInfo info;
        info.uDeviceID = uDeviceID;
        info.enableCustomLocalSetting = false;
        if(info_jo.contains(aosStream::js_sdi_localHost)){
            info.localHost = info_jo[aosStream::js_sdi_localHost].toString();
        }

        info.localPort = info_jo[aosStream::js_sdi_localPort].toInt();
        return aosStream::CreateTCPServer(info, this);
    }
    else if(aosStream::js_sdt_tcpClient == type)
    {
        if(!info_jo.contains(aosStream::js_sdi_targetIP) || !info_jo.contains(aosStream::js_sdi_targetPort) ){
            return  nullptr;
        }
        aosStream::TCPClientInfo info;
        info.uDeviceID = uDeviceID;
        info.enableCustomLocalSetting = false;
        info.serverHost = info_jo[aosStream::js_sdi_targetIP].toString();
        info.serverPort = info_jo[aosStream::js_sdi_targetPort].toInt();
        return aosStream::CreateTCPClient(info, this);
    }
    else  if(aosStream::js_sdt_udp == type)
    {
        aosStream::UDPInfo info;
        info.uDeviceID = uDeviceID;
        info.enableCustomLocalSetting = true;
        if(info_jo.contains(aosStream::js_sdi_localHost)) {
            info.localHost = info_jo[aosStream::js_sdi_localHost].toString();
        }
        if(info_jo.contains(aosStream::js_sdi_localPort)){
            info.localPort = info_jo[aosStream::js_sdi_localPort].toInt();
        }
        if(info_jo.contains(aosStream::js_sdi_muticastAddr)){
            info.muticastAddr = info_jo[aosStream::js_sdi_muticastAddr].toString();
        }
        if(info_jo.contains(aosStream::js_sdi_targetIP)){
            info.targetHost = info_jo[aosStream::js_sdi_targetIP].toString();
        }
        if(info_jo.contains(aosStream::js_sdi_targetPort)){
            info.targetPort = info_jo[aosStream::js_sdi_targetPort].toInt();
        }
        return aosStream::CreateUDP(info, this);
    }
    else  if(aosStream::js_sdt_com == type)
    {
        if(!info_jo.contains(aosStream::js_sdi_com_id)) {
            return  nullptr;
        }

        aosStream::SerialPortInfo info;
        info.uDeviceID = uDeviceID;
        info.name = info_jo[aosStream::js_sdi_com_id].toString();
        if(info_jo.contains(aosStream::js_sdi_com_baudRate)){
            info.baudRate = info_jo[aosStream::js_sdi_com_baudRate].toInt();
        }
        if(info_jo.contains(aosStream::js_sdi_com_dataBits)){
            info.dataBits = (QSerialPort::DataBits)info_jo[aosStream::js_sdi_com_dataBits].toInt();
        }
        if(info_jo.contains(aosStream::js_sdi_com_stopBits)){
            info.stopBits = (QSerialPort::StopBits)info_jo[aosStream::js_sdi_com_stopBits].toInt();
        }
        if(info_jo.contains(aosStream::js_sdi_com_parity)){
            info.parity = (QSerialPort::Parity)info_jo[aosStream::js_sdi_com_parity].toInt();
        }
        if(info_jo.contains(aosStream::js_sdi_com_flowControl)){
            info.flowControl = (QSerialPort::FlowControl)info_jo[aosStream::js_sdi_com_flowControl].toInt();
        }
        //        info.dataBits = QSerialPort::Data8;
        //        info.stopBits = QSerialPort::OneStop;
        //        info.parity = QSerialPort::NoParity;
        //        info.flowControl = QSerialPort::NoFlowControl;
        return aosStream::CreateSerialPort(info, this);
    }else  if(aosStream::js_sdt_webServer == type)
    {
        if(!info_jo.contains(aosStream::js_sdi_web_port)){
            return  nullptr;
        }
        aosStream::webServerInfo info;
        info.uDeviceID = uDeviceID;
        info.localHost = info_jo[aosStream::js_sdi_web_listenAddr].toString();
        if(info_jo.contains(aosStream::js_sdi_web_port)){
            info.localPort = info_jo[aosStream::js_sdi_web_port].toInt();
        }
        return aosStream::CreateWebServer(info, this);
    }else  if(aosStream::js_sdt_webClient == type){
        if(!info_jo.contains(aosStream::js_sdi_web_port)){
            return  nullptr;
        }
        aosStream::webClientInfo info;
        info.uDeviceID = uDeviceID;
        info.targetHost = info_jo[aosStream::js_sdi_web_connectAddr].toString();
        if(info_jo.contains(aosStream::js_sdi_web_port)){
            info.targetPort = info_jo[aosStream::js_sdi_web_port].toInt();
        }
        return aosStream::CreateWebClient(info, this);
    }else  if(aosStream::js_sdt_pipeClient == type){
//        QString pipeName;
//        //add plugin
//        if(device_jo.contains(aosStream::js_streams_plugin))
//        {
//            auto plugins_ja = device_jo[aosStream::js_streams_plugin].toArray();
//            if(!plugins_ja.isEmpty())
//            {
//                for(int i = 0; i < plugins_ja.size(); ++i)
//                {
//                    auto plugin_jo = plugins_ja[i].toObject();
//                    auto ret = AddPlugin(plugin_jo, pipeName);
//                    if(0 != ret){
//                        return  nullptr;
//                    }
//                }
//            }
//        }
//        // pipe name maybe come from plugin
//        if(pipeName.length() <= 0){
//            if(info_jo.contains(aosStream::js_sdi_pipeName)){
//                pipeName = info_jo[aosStream::js_sdi_pipeName].toString();
//            }else if(info_jo.contains(aosStream::js_sdi_pipeNameVar)){
//                pipeName = GetRandPipeName();
//            }
//        }

//        if(pipeName.length() <= 0){
//            return  nullptr;
//        }
//        aosStream::pipeClientInfo info;
//        info.uDeviceID = uDeviceID;
//        info.name = pipeName;
//        return aosStream::CreatePipeClient(info, this);
        return nullptr;
    }else  if(aosStream::js_sdt_pipeServer == type){
        QString pipeName;
        if(info_jo.contains(aosStream::js_sdi_pipeName)){
            pipeName = info_jo[aosStream::js_sdi_pipeName].toString();
        }else if(info_jo.contains(aosStream::js_sdi_pipeNameVar)){
            pipeName = GetRandPipeName();
        }
        if(pipeName.length() <= 0){
            return  nullptr;
        }
        aosStream::pipeServerInfo info;
        info.uDeviceID = uDeviceID;
        info.name = pipeName;
        return aosStream::CreatePipeServer(info, this);
    }

    return  nullptr;
}

QString PlayElement::GetRandPipeName()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());//为随机值设定一个seed

    const char chrs[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int chrs_size = sizeof(chrs);

    int length = 16;
    char* ch = new char[length + 1];
    memset(ch, 0, length + 1);
    int randomx = 0;
    for (int i = 0; i < length; ++i)
    {
        randomx= rand() % (chrs_size - 1);
        ch[i] = chrs[randomx];
    }

    QString ret(ch);
    delete[] ch;
    return ret;
}

bool PlayElement::initNetDevice()
{
    if(m_currentitem == nullptr)
    {
        return  false;
    }
    if(m_devices)
    {
        delete m_devices;
        m_devices = nullptr;
    }
    const tagReplayItemInfo *pitem = dynamic_cast<const tagReplayItemInfo*>(m_currentitem);
    if(pitem)
    {
        try
        {
            QJsonObject device_jo = pitem->m_netconfig;
            m_devices = AddDevice(0,this,device_jo);
        }
        catch (...)
        {
            m_devices = nullptr;
            return false;
        }
        if(m_devices)
        {
            m_devices->Init();
        }
    }
    return (m_devices!=nullptr)?true:false;
}


bool PlayElement::dealDataSourceCallback(void *pObject, unsigned char *pData, unsigned int iDataLen, long long iTimeStamp, long long iGlobeFileReadBeginValidDataPos,long long iDataSendTimeStamp)
{
    if(!pObject)
    {
        return false;
    }
    PlayElement* pDataBack = reinterpret_cast<PlayElement*>(pObject);
    if(pDataBack)
    {
        pDataBack->deal_callback_data(pData, iDataLen, iTimeStamp, iGlobeFileReadBeginValidDataPos, iDataSendTimeStamp);
    }
    return true;
}

void PlayElement::decodestateData( unsigned char *pData, unsigned int iDataLen, long long iTimeStamp,long long iGlobeFileReadValidDataPos,long long iDataSendTimeStamp, bool bCompressed)
{
    if(m_pDataDecodeThread)
    {
        m_pDataDecodeThread->appendDecodeData(pData,iDataLen,iTimeStamp,iGlobeFileReadValidDataPos,iDataSendTimeStamp,bCompressed);
    }
}

void PlayElement::dataDecodeCallback(QObject *pObject, const QByteArray &msg, long long iTimeStamp, long long iGlobeFileReadValidDataPos, long long iDataSendTimeStamp)
{
    if(!pObject)
    {
        return;
    }
    PlayElement* pDataBack = reinterpret_cast<PlayElement*>(pObject);
    if(pDataBack)
    {
//        const tagReplayItemInfo *pitem =dynamic_cast<const tagReplayItemInfo*>(pDataBack->m_currentitem);
//        if(pitem)
//        {
//            QDateTime datetime  = QDateTime::fromMSecsSinceEpoch(iTimeStamp);
//            QDateTime currentDateTime =  QDateTime::currentDateTime().toUTC();;
//            qDebug()<<pDataBack->formatTimeStr( iDataSendTimeStamp - pitem->m_iTimeStampStart)<<"_"<<pDataBack->formatTimeStr(pDataBack->m_payStart.msecsTo(currentDateTime))<<"_"<<pDataBack->formatTimeStr(pDataBack->m_payStart.msecsTo(datetime));
//            qDebug()<<datetime.toString("yyyy-MM-dd hh:mm:ss.zzz")<<"len "<< iDataLen<< ":"<<array.toHex(' ');
//        }
        emit pDataBack->sendDataSignal(msg, iGlobeFileReadValidDataPos,iDataSendTimeStamp);
    }
}




