#include "gaeactormanager.h"
#include <QTimer>
#include <iostream>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QDir>

#include "KlusterWebSocketClient.h"
#include "configmanager.h"
#include "settingsconfig.h"

#include "gaeactor_comm_interface.h"

#include "LocationHelper.h"

#include "gaeactor_transmit_interface.h"

#include "./proto/protoc/SimParamsInfo.pb.h"
#include "src/storage/OriginalDataOutputManager.h"
#include "src/OriginalDateTime.h"
#include "loghelper.h"
#include <sstream>

GaeactorManager &GaeactorManager::getInstance()
{
	static GaeactorManager gaeactormanager;
	return gaeactormanager;
}


GaeactorManager::GaeactorManager(QObject *parent)
    :QObject(parent),
    m_wsclient_(nullptr),
    m_localip(""),
    m_hub_websocketport(0),
    m_businglocalwsaddr(true),
    m_pOriginalDataOutputManager_send(nullptr)
{
    m_record.store(false);
}

GaeactorManager::~GaeactorManager()
{
    if(m_pOriginalDataOutputManager_send)
    {
        m_pOriginalDataOutputManager_send->clear();
    }
    if(m_wsclient_)
    {
        m_wsclient_->deleteLater();
    }
    if(SettingsConfig::getInstance().record_local_memory())
    {
        std::cout << "remove subscribe channedl data" << SettingsConfig::getInstance().getTopicChannelFormat(ECAL_CHANNEL_BINARYDATA_STR) << std::endl;

        gaeactorcomm::GaeactorComm::getInstance().removeTopicChannel(m_binary_channel);
    }
}

void GaeactorManager::init()
{
    QString dir_time_str;
    if(ConfigManager::getInstance().m_path.contains("OriginalData") && ConfigManager::getInstance().m_path.endsWith("reviewdata"))
    {
        dir_time_str = ConfigManager::getInstance().m_path;
    }
    else
    {
        QString savedir = QDir::currentPath()+"/OriginalData";
        QString dir_name_str = QString("%1/%2").arg(savedir).arg(QDateTime::currentDateTime().toUTC().toString("yyyyMMdd"));
        dir_time_str = dir_name_str+"/"+ConfigManager::getInstance().m_path;
    }
    QString dir_send_str = dir_time_str+"/gaeactor_out";


    m_pOriginalDataOutputManager_send = new originaldatastoragestd::OriginalDataOutputManager();
    m_pOriginalDataOutputManager_send->initialize(dir_send_str.toStdString().c_str(),originaldatastoragestd::OriginalDataOutputManager::ENUM_OUTPUTSTORAGE_TYPE_SEND);
    m_pOriginalDataOutputManager_send->start();

    if(SettingsConfig::getInstance().record_local_memory())
    {
        m_binary_channel = gaeactorcomm::GaeactorComm::getInstance().allocProtobufSubscribeTopicChannel<::msg::SimParamsInfo::msg_BinaryData_Array>(SettingsConfig::getInstance().getTopicChannelFormat(ECAL_CHANNEL_BINARYDATA_STR).c_str());
        gaeactorcomm::GaeactorComm::getInstance().set_protobuf_data_callback<::msg::SimParamsInfo::msg_BinaryData_Array>(m_binary_channel,
                                                                                                                   std::bind(&GaeactorManager::deal_binarydata_callback,
                                                                                                                             this,
                                                                                                                             std::placeholders::_1,
                                                                                                                             std::placeholders::_2));
        std::cout << "add subscribe channedl data" << SettingsConfig::getInstance().getTopicChannelFormat(ECAL_CHANNEL_BINARYDATA_STR) << std::endl;
    }
    else
    {
        QString ipstr;
        if(m_businglocalwsaddr && m_localip.isEmpty() && m_hub_websocketport == 0)
        {
            ipstr = SettingsConfig::getInstance().lavic_desktop_cfg().display_remote_url;
        }
        else
        {
            ipstr = QString("ws://%1:%2").arg(m_localip).arg(m_hub_websocketport);
        }
        m_wsclient_ = new KlusterWebSocketClient(QUrl(ipstr));
        m_wsclient_->setBinaryDataCallback(std::bind(&GaeactorManager::binary_receive_data_call_back,this,std::placeholders::_1,std::placeholders::_2));
        m_wsclient_->Connect();
        std::cout << "using remote data" << ipstr.toStdString() << std::endl;
    }

}

void GaeactorManager::setRunning(bool bRunning)
{
    setRecord(bRunning);
}

void GaeactorManager::receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pdata, const UINT32 &ilen, const BYTE*pOriginaldata, const UINT32& iOriginallen)
{
    recording_data(pOriginaldata, iOriginallen);
}

void GaeactorManager::deal_binarydata_callback(const COMM_CHANNEL_INFO &channelinfo, const msg::SimParamsInfo::msg_BinaryData_Array *pdata)
{
    ::msg::SimParamsInfo::msg_BinaryData_Array *pmsg_binary_Array = const_cast<::msg::SimParamsInfo::msg_BinaryData_Array *>(pdata);
    if(pmsg_binary_Array)
    {
        uint64_t packindex = pmsg_binary_Array->packindex();
//        UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//        static UINT64 lasttimestampmap = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//        UINT64 interval = fabs(currentTimeStamp - lasttimestampmap);
//        lasttimestampmap = currentTimeStamp;
//        std::stringstream ss;
//        ss <<"--------------trans interval "<<packindex<<" "<<interval<<" ms\n";
//        std::cout<<ss.str();

        if(m_array_packindex != 0)
        {
            if(packindex - m_array_packindex != 1)
            {
                std::stringstream ss2;
                ss2 <<"[error] --------------trans array pack index error "<<m_array_packindex<<" "<<packindex<<"\n";
                std::cout<<ss2.str();
            }
        }
        m_array_packindex = packindex;
        for(int k = 0; k < pmsg_binary_Array->binarydatas_size(); k++)
        {
            ::msg::SimParamsInfo::msg_BinaryData *pmsg = pmsg_binary_Array->mutable_binarydatas(k);
            if(pmsg)
            {
                UINT64 itemindex = pmsg->packindex();
                const BYTE* pOriginaldata = (const BYTE*)(pmsg->binarydata().c_str());
                UINT32 iOriginallen = pmsg->binarysize();
                TIMESTAMP_TYPE timestamp = pmsg->timestamp();
                if(iOriginallen == pmsg->binarydata().size())
                {
                    if(m_pOriginalDataOutputManager_send)
                    {
                        calc_trans_speed(iOriginallen);

                        if(m_item_packindex != 0)
                        {
                            if(itemindex - m_item_packindex != 1)
                            {
                                std::stringstream ss2;
                                ss2 <<"[error] --------------trans item pack index error "<<m_item_packindex<<" "<<itemindex<<"\n";
                                std::cout<<ss2.str();
                            }
                        }
                        m_item_packindex = itemindex;
                        m_pOriginalDataOutputManager_send->inputDataAndTimeStamp(pOriginaldata,iOriginallen,timestamp);
                    }
                }
                else
                {
                    std::stringstream ss2;
                    ss2 <<"[error] --------------trans data length error "<<pmsg->binarydata().size()<<" "<<iOriginallen<<"\n";
                    std::cout<<ss2.str();
                }
            }
        }
    }
}

void GaeactorManager::calc_trans_speed(const UINT32 &iOriginallen)
{
    static UINT64 datasize = 0;
    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    static UINT64 lasttimestampmapms = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    UINT64 intervalms = fabs(currentTimeStamp - lasttimestampmapms);
    datasize+=iOriginallen;
    if(intervalms > 1000)
    {
        lasttimestampmapms = currentTimeStamp;

        std::stringstream ss;
        ss <<"++++++++++++++++++++++++++++++++ "<<intervalms<<" ms transfer datasize:"<<datasize<<" bytes --- "<<datasize/1024.0f<<"  kb/s  --- "<<datasize/(1024.0f*1024.0f)<<"  mb/s \n";
        TRACE_LOG_PRINT_EX2(ss);
        datasize = 0;
    }
}

void GaeactorManager::recording_data(const BYTE *pOriginaldata, const UINT32 &iOriginallen)
{
    if(m_pOriginalDataOutputManager_send)
    {
        calc_trans_speed(iOriginallen);
        m_pOriginalDataOutputManager_send->inputData(pOriginaldata,iOriginallen);
    }
}

void GaeactorManager::set_usinglocalwsaddr(bool newBusinglocalwsaddr)
{
    m_businglocalwsaddr = newBusinglocalwsaddr;
}

const UINT16 &GaeactorManager::hub_websocketport() const
{
    return m_hub_websocketport;
}

void GaeactorManager::setHub_websocketport(UINT16 newHub_websocketport)
{
    m_hub_websocketport = newHub_websocketport;
}

void GaeactorManager::setLocalip(const QString &newLocalip)
{
    m_localip = newLocalip;
}

const QString &GaeactorManager::localip() const
{
    return m_localip;
}

void GaeactorManager::setRecord(bool newRecord)
{
    m_record.store(newRecord);
}

void GaeactorManager::binary_receive_data_call_back(const BYTE *pdata, const UINT32 &ilen)
{
    const BYTE*pOriginaldata = (const BYTE*)pdata;
    const UINT32& iOriginallen = ilen;

    const char *frame_head = (const char*)pdata;
    uint32_t *crcval_recv = (uint32_t*)(pdata + sizeof(uint32_t));

    const char *dataptr = (const char *)(pdata + sizeof(uint32_t) + sizeof(uint32_t));

    E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = *((E_CHANNEL_TRANSMITDATA_TYPE*)(dataptr));
    uint32_t irecvlen = *(uint32_t*)(dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
    const char *protocoldata_ptr = dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t);

    uint32_t ipayloadlen = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + irecvlen;
    //std::cout<<"recv data "<<message.size()<<" datatype "<<channelTransmitDataType<<" recv len "<< irecvlen<<std::endl;
#if 0
    receive_callback(channelTransmitDataType, (const BYTE*)protocoldata_ptr, irecvlen, (const BYTE*)dataptr, ipayloadlen);
#else
    receive_callback(channelTransmitDataType, (const BYTE*)protocoldata_ptr, irecvlen, (const BYTE*)pOriginaldata, iOriginallen);
#endif

}
