#include "gaeactormanager.h"
#include <QTimer>
#include <iostream>

#include "LocationHelper.h"
#include "gaeactor_transmit_interface.h"
#include "KlusterWebSocketClient.h"

#include "./proto/protoc/AgentPositionInfo.pb.h"
//#define AUTO_CALLBACK
GaeactorManager &GaeactorManager::getInstance()
{
    static GaeactorManager gaeactormanager;
    return gaeactormanager;
}

GaeactorManager::~GaeactorManager()
{

}


void *GaeactorManager::loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen)
{
    void * usrpayload = nullptr;
#ifdef USING_GAEACTOR_TRANSMIT
    if(m_pGaeactorTransmit)
    {
        usrpayload = m_pGaeactorTransmit->loanTransmitBuffer(channelinfo,iLen);
    }
#else
    size_t isize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
    usrpayload = (void*)malloc(isize);
    memset(usrpayload,0,isize);
#endif
    return usrpayload;
}


void GaeactorManager::freeData(void *pSrcData)
{
#ifdef USING_GAEACTOR_TRANSMIT

#else
    free(pSrcData);
#endif
}

void GaeactorManager::dealentityHexidex(TYPE_ULID ulid, const LAT_LNG &pos, INT32 alt, FLOAT32 roll, FLOAT32 pitch, FLOAT32 yaw,bool bSensor, bool bClear)
{
    size_t isize = sizeof(transdata_posatt_hexidx);
#ifdef USING_GAEACTOR_TRANSMIT
    void * usrpayload = loanTransmitBuffer(std::get<1>(m_Transmitchannel_AgentCores), isize);
#else
    void * usrpayload = loanTransmitBuffer(nullptr, isize);
#endif
    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_posatt_hexidx *pData = reinterpret_cast<transdata_posatt_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);

        pData->PARAM_protocol_head.PARAM_source_ulid = ulid;
        H3INDEX h3index = 0;
        if(!bClear)
        {
            LocationHelper::getIndexInfo(h3index, pos.lat,pos.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);
        }
        PROPERTY_SET_TYPE(pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property,bSensor ? AGENT_ENTITY_PROPERTY_SENSOR : AGENT_ENTITY_PROPERTY_NORMAL);
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx = h3index;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = pos.lat*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = pos.lng*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_amsl = alt*1000;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_ref = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = roll;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = pitch;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = yaw;
#ifdef USING_GAEACTOR_TRANSMIT
        m_pGaeactorTransmit->publish(std::get<1>(m_Transmitchannel_AgentCores));

        transdata_entityposinfo _transdata_entityposinfo;
        _transdata_entityposinfo.PARAM_pos_hexidx = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx;
        _transdata_entityposinfo.PARAM_longitude = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude;   //1/10000000
        _transdata_entityposinfo.PARAM_latitude = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude;   //1/10000000
        _transdata_entityposinfo.PARAM_amsl = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_amsl;   //1/10000000
        _transdata_entityposinfo.PARAM_ref = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_ref;   //1/10000000
        _transdata_entityposinfo.PARAM_roll = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll;   //1/10000000
        _transdata_entityposinfo.PARAM_pitch = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch;   //1/10000000
        _transdata_entityposinfo.PARAM_yaw = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw;   //1/10000000
        m_displayPosCallback(ulid,ulid ,_transdata_entityposinfo, E_DISPLAY_MODE_ENTITY);
#else
//        if(m_wsclient_)
//        {
//            m_wsclient_->SendBinaryMessage((const char *)usrpayload, isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
//        }
        transdata_entityposinfo _transdata_entityposinfo;
        _transdata_entityposinfo.PARAM_pos_hexidx = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx;
        _transdata_entityposinfo.PARAM_longitude = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude;   //1/10000000
        _transdata_entityposinfo.PARAM_latitude = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude;   //1/10000000
        _transdata_entityposinfo.PARAM_amsl = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_amsl;   //1/10000000
        _transdata_entityposinfo.PARAM_ref = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_ref;   //1/10000000
        _transdata_entityposinfo.PARAM_roll = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll;   //1/10000000
        _transdata_entityposinfo.PARAM_pitch = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch;   //1/10000000
        _transdata_entityposinfo.PARAM_yaw = pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw;   //1/10000000
        m_displayPosCallback(ulid,ulid ,_transdata_entityposinfo, E_DISPLAY_MODE_ENTITY);
#endif
        freeData(usrpayload);
    }
}

void GaeactorManager::clearentityHexidex(TYPE_ULID ulid)
{
    dealentityHexidex(ulid, LAT_LNG(), 0, 0.0, 0.0, 0.0,false, true);
}

void GaeactorManager::dealHexidex(TYPE_ULID ulid, const LAT_LNG& pos, const HEXIDX_HGT_ARRAY &hexidxslist, const std::vector<transdata_param_seq_polygon>& _polygon, int slient_time_gap, bool bClear)
{
    dealentityHexidex(ulid, pos, 0, 0.0, 0.0, 0.0,true, bClear);

    //////////////////////////////////////////////////////////////////////////////////////////
    size_t ibufsize = sizeof(UINT16) + sizeof(transdata_param_seq_hexidx) * hexidxslist.size() + sizeof(UINT16) + sizeof(transdata_param_seq_polygon) * _polygon.size();
    ibufsize = bClear ? 0 : ibufsize;
    size_t isize = sizeof(transdata_wave_smd_hexidx) + ibufsize;

#ifdef USING_GAEACTOR_TRANSMIT
    void * usrpayload = loanTransmitBuffer(std::get<1>(m_Transmitchannel_AgentSensors), isize);
#else
    void * usrpayload = loanTransmitBuffer(nullptr, isize);
#endif

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_wave_smd_hexidx *pData = reinterpret_cast<transdata_wave_smd_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);
        pData->PARAM_protocol_head.PARAM_source_ulid = ulid;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_snd_rcv = 0x01;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_silent_time_gap = slient_time_gap;
        pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count = ibufsize;

        if(!bClear && ibufsize>0)
        {
            BYTE *p_byte_buffer = (BYTE*)(pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_seq_buffer);
            transdata_payload_hexidx *ptransdata_payload_hexidx = GET_HEXIDX_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_hexidx->PARAM_qty_hexidx = hexidxslist.size();
            if(!hexidxslist.empty())
            {
                memcpy(ptransdata_payload_hexidx->PARAM_seq_hexidx, hexidxslist.data(), sizeof(transdata_param_seq_hexidx) * hexidxslist.size());
            }
            transdata_payload_polygon *ptransdata_payload_polygon = GET_POLYGON_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_polygon->PARAM_qty_polygon = _polygon.size();
            if(!_polygon.empty())
            {
                memcpy(ptransdata_payload_polygon->PARAM_seq_polygon, _polygon.data(), sizeof(transdata_param_seq_polygon) * _polygon.size());
            }
        }
#ifdef USING_GAEACTOR_TRANSMIT
        m_pGaeactorTransmit->publish(std::get<1>(m_Transmitchannel_AgentSensors));
#ifdef AUTO_CALLBACK
        transdata_entityposinfo _transdata_entityposinfo;
        _transdata_entityposinfo.PARAM_pos_hexidx = LocationHelper::getIndexInfo(pos.lat,pos.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);
        _transdata_entityposinfo.PARAM_longitude = pos.lng*LON_LAT_ACCURACY;   //1/10000000
        _transdata_entityposinfo.PARAM_latitude = pos.lat*LON_LAT_ACCURACY;   //1/10000000
        m_displayPosCallback(ulid,ulid ,_transdata_entityposinfo, E_DISPLAY_MODE_ENTITY);
        m_displaycallback(ulid,ulid ,hexidxslist,_polygon, E_DISPLAY_MODE_WAVE);
#endif
#else
//        if(m_wsclient_)
//        {
//            m_wsclient_->SendBinaryMessage((const char *)usrpayload, isize + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
//        }
        transdata_entityposinfo _transdata_entityposinfo;
        LocationHelper::getIndexInfo(_transdata_entityposinfo.PARAM_pos_hexidx,pos.lat,pos.lng,INDEX_MAPPING_RESOLUTION_ENTITY_POS);
        _transdata_entityposinfo.PARAM_longitude = pos.lng*LON_LAT_ACCURACY;   //1/10000000
        _transdata_entityposinfo.PARAM_latitude = pos.lat*LON_LAT_ACCURACY;   //1/10000000
        m_displayPosCallback(ulid,ulid ,_transdata_entityposinfo, E_DISPLAY_MODE_ENTITY);
        m_displaycallback(ulid,ulid ,hexidxslist,_polygon,TYPE_SENSORINFO(),E_DISPLAY_MODE_WAVE);
#endif
        freeData(usrpayload);

    }
}

void GaeactorManager::clearHexidex(TYPE_ULID ulid)
{
    dealentityHexidex(ulid, LAT_LNG(), 0, 0.0, 0.0, 0.0,true, true);
    dealHexidex(ulid, LAT_LNG(), HEXIDX_HGT_ARRAY(), std::vector<transdata_param_seq_polygon>(), 0 ,true);
}


void GaeactorManager::registDisplayCallback(display_hexidx_update_callback func)
{
    m_displaycallback = std::move(func);
}

void GaeactorManager::registDisplayPosCallback(display_pos_update_callback func)
{
    m_displayPosCallback = std::move(func);
}

void GaeactorManager::registIntersectionDisplayCallback(intersection_display_hexidx_update_callback func)
{
    m_intersection_display_hexidx_update_callback = std::move(func);
}

void GaeactorManager::registEchoWaveDisplayCallback(echowave_display_hexidx_update_callback func)
{
    m_echowave_display_hexidx_update_callback = std::move(func);
}
void GaeactorManager::binary_receive_data_call_back(const BYTE*pdata, const UINT32& ilen)
{

    const char *frame_head = (const char*)pdata;
    uint32_t *crcval_recv = (uint32_t*)(pdata + sizeof(uint32_t));

    const char *dataptr = (const char *)(pdata + sizeof(uint32_t) + sizeof(uint32_t));

    E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = *((E_CHANNEL_TRANSMITDATA_TYPE*)(dataptr));
    uint32_t irecvlen = *(uint32_t*)(dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
    const char *protocoldata_ptr = dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t);

    uint32_t ipayloadlen = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + irecvlen;
    //std::cout<<"recv data "<<message.size()<<" datatype "<<channelTransmitDataType<<" recv len "<< irecvlen<<std::endl;
    receive_callback(channelTransmitDataType, (const BYTE*)protocoldata_ptr, irecvlen, (const BYTE*)dataptr, ipayloadlen);
}

void GaeactorManager::receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pdata, const UINT32 &ilen, const BYTE *pOrignaldata, const UINT32 &iOrignallen)
{
    switch (channelTransmitDataType) {
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX:
    {
        transformHexData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX:
    {
        transformHexAttData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
    {
        transformHexAttArrayData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION:
    {
        transformHexIntersectionData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE:
    {
        transformEchowaveData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY:
    {
        transformEchowaveArrayData(pdata, ilen);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT:
    {
        transformEventlistData(pdata, ilen);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
    {
        transformEventlistArrayData(pdata, ilen);
    }break;
    default:
        break;
    }
}

GaeactorManager::GaeactorManager(QObject *parent)
    :QObject(parent)
    ,m_wsclient_(nullptr)
{
#ifdef USING_GAEACTOR_TRANSMIT
    m_pGaeactorTransmit = new gaeactortransmit::GaeactorTransmit(this);
    std::vector<std::tuple<std::string,std::string>> servicelist;
    servicelist.push_back(std::make_tuple("gaeactor",""));
    m_pGaeactorTransmit->initDeployType(E_DEPLOYMODE_TYPE_LOCAL_RECV_SEND,servicelist);
//    m_pGaeactorTransmit->initDeployType(E_DEPLOYMODE_TYPE_LOCAL_RECV,servicelist);
    m_pGaeactorTransmit->set_transmit_channel_id("");

    m_pGaeactorTransmit->setDataCallback(std::bind(&GaeactorManager::receive_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    m_Transmitchannel_AgentCores = m_pGaeactorTransmit->getReusePublisherChannel();
    m_Transmitchannel_AgentSensors = m_pGaeactorTransmit->getReusePublisherChannel();
#else
    QString ipstr = "ws://127.0.0.1:31769";
    std::cout << "using remote data" << ipstr.toStdString() << std::endl;

    m_wsclient_ = new KlusterWebSocketClient(QUrl(ipstr));

    m_wsclient_->setBinaryDataCallback(std::bind(&GaeactorManager::binary_receive_data_call_back,this,std::placeholders::_1,std::placeholders::_2));
    m_wsclient_->Connect();
#endif
    connect(this, &GaeactorManager::dealtransformHexDataSig, this, &GaeactorManager::dealtransformHexDataSlot);
    connect(this, &GaeactorManager::dealtransformHexAttDataSig, this, &GaeactorManager::dealtransformHexAttDataSlot);
    connect(this, &GaeactorManager::dealtransformHexAttArrayDataSig, this, &GaeactorManager::dealtransformHexAttArrayDataSlot);

    connect(this, &GaeactorManager::dealtransformHexIntersectionDataSig, this, &GaeactorManager::dealtransformHexIntersectionDataSlot);
    connect(this, &GaeactorManager::dealtransformEchowaveDataSig, this, &GaeactorManager::dealtransformEchowaveDataSlot);
    connect(this, &GaeactorManager::dealtransformEchowaveArrayDataSig, this, &GaeactorManager::dealtransformEchowaveArrayDataSlot);
    connect(this, &GaeactorManager::dealtransformEventlistDataSig, this, &GaeactorManager::dealtransformEventlistDataSlot);
    connect(this, &GaeactorManager::dealtransformEventlistArrayDataSig, this, &GaeactorManager::dealtransformEventlistArrayDataSlot);
}

void GaeactorManager::transformHexData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformHexDataSig(by);
}

void GaeactorManager::transformHexAttData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformHexAttDataSig(by);
}

void GaeactorManager::transformHexAttArrayData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformHexAttArrayDataSig(by);
}

void GaeactorManager::transformHexIntersectionData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformHexIntersectionDataSig(by);
}

void GaeactorManager::transformEchowaveData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformEchowaveDataSig(by);
}

void GaeactorManager::transformEchowaveArrayData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata, ilen);
    emit dealtransformEchowaveArrayDataSig(by);
}


void GaeactorManager::transformEventlistData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata,ilen);
    emit dealtransformEventlistDataSig(by);
}

void GaeactorManager::transformEventlistArrayData(const BYTE *pdata, const UINT32 &ilen)
{
    QByteArray by((const char *)pdata, ilen);
    emit dealtransformEventlistArrayDataSig(by);
}


void GaeactorManager::dealtransformHexDataSlot(const QByteArray&by)
{

    const char * pdata = by.data();

    transentityhexidxdata transdata;

    HEXIDX_HGT_ARRAY hexidxslist;
    POLYGON_LIST polygonlist;
#if 0
    memcpy(&transdata, pdata, sizeof(transentityhexidxdata));

    if (transdata.buffersize != 0)
    {
        BYTE* pBuf = (BYTE*)(pdata + sizeof(transentityhexidxdata));
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        UINT16  PARAM_qty_hexidx = TRANSMIT_GET_HEXIDX_NUM(pBuf);
        if (PARAM_qty_hexidx != 0)
        {
            transentityhexidxlistdata * ptransentityhexidxlistdata = TRANSMIT_GET_HEXIDX_STRUCT_PTR(pBuf);
            hexidxslist.resize(ptransentityhexidxlistdata->flexarrycount);
            memcpy(hexidxslist.data(), ptransentityhexidxlistdata->hexidxlist, sizeof(transdata_param_seq_hexidx) * ptransentityhexidxlistdata->flexarrycount);
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        UINT16  PARAM_qty_polygon = TRANSMIT_GET_POLYGON_NUM(pBuf);
        if (PARAM_qty_polygon != 0)
        {
            transentitypolygonlistdata *ptransentitypolygonlistdata = TRANSMIT_GET_POLYGON_STRUCT_PTR(pBuf);
            polygonlist.resize(ptransentitypolygonlistdata->flexarrycount);
            memcpy(polygonlist.data(), ptransentitypolygonlistdata->polygonlist, sizeof(TYPE_POLYGON_PT) * ptransentitypolygonlistdata->flexarrycount);
        }
    }
#else

    ::msg::AgentPositionInfo::msg_transentityhexidxdata _AgentRelationInfo;
    _AgentRelationInfo.ParseFromArray(pdata, by.size());
    size_t size = _AgentRelationInfo.ByteSizeLong();
    int cc = by.size();
    if (by.size() == _AgentRelationInfo.ByteSizeLong())
    {
        transdata.uildsrc = QString::fromStdString(_AgentRelationInfo.uildsrc()).toULongLong();
        transdata.uilddst = QString::fromStdString(_AgentRelationInfo.uilddst()).toULongLong();
        transdata.eDdisplayMode = (E_DISPLAY_MODE)_AgentRelationInfo.eddisplaymode();

        transdata.sensorinfo.PARAM_sensor_pack_index = QString::fromStdString(_AgentRelationInfo.sensorinfo().param_sensor_pack_index()).toULongLong();
        transdata.sensorinfo.PARAM_timestamp = QString::fromStdString(_AgentRelationInfo.sensorinfo().param_timestamp()).toULongLong();
        transdata.sensorinfo.PARAM_sim_timestamp = QString::fromStdString(_AgentRelationInfo.sensorinfo().param_sim_timestamp()).toULongLong();
        transdata.sensorinfo.PARAM_source_sensingmediaid = QString::fromStdString(_AgentRelationInfo.sensorinfo().param_source_sensingmediaid()).toULongLong();
        transdata.sensorinfo.PARAM_elecmag_wave_properties = _AgentRelationInfo.sensorinfo().param_elecmag_wave_properties();
        transdata.sensorinfo.PARAM_sound_wave_properties = _AgentRelationInfo.sensorinfo().param_sound_wave_properties();
        transdata.sensorinfo.PARAM_wave_freq_mean_sgnfcnt = _AgentRelationInfo.sensorinfo().param_wave_freq_mean_sgnfcnt();
        transdata.sensorinfo.PARAM_wave_freq_std_dvtn_sgnfcnt = _AgentRelationInfo.sensorinfo().param_wave_freq_std_dvtn_sgnfcnt();
        transdata.sensorinfo.PARAM_wave_freq_distbtn = _AgentRelationInfo.sensorinfo().param_wave_freq_distbtn();
        transdata.sensorinfo.PARAM_wave_frequency_scale = _AgentRelationInfo.sensorinfo().param_wave_frequency_scale();
        transdata.sensorinfo.PARAM_wave_usage = _AgentRelationInfo.sensorinfo().param_wave_usage();
        transdata.sensorinfo.PARAM_wave_snd_rcv = _AgentRelationInfo.sensorinfo().param_wave_snd_rcv();
        transdata.sensorinfo.PARAM_wave_silent_time_gap = _AgentRelationInfo.sensorinfo().param_wave_silent_time_gap();
        transdata.sensorinfo.PARAM_wave_field_media_id = _AgentRelationInfo.sensorinfo().param_wave_field_media_id();
        transdata.sensorinfo.PARAM_wave_modsig_id = QString::fromStdString(_AgentRelationInfo.sensorinfo().param_wave_modsig_id()).toULongLong();
        transdata.sensorinfo.PARAM_wave_power_in_watts = _AgentRelationInfo.sensorinfo().param_wave_power_in_watts();
        transdata.sensorinfo.PARAM_wave_propagation_model = _AgentRelationInfo.sensorinfo().param_wave_propagation_model();
        transdata.sensorinfo.PARAM_wave_direction_pitch = _AgentRelationInfo.sensorinfo().param_wave_direction_pitch();
        transdata.sensorinfo.PARAM_wave_direction_azimuth = _AgentRelationInfo.sensorinfo().param_wave_direction_azimuth();
        transdata.sensorinfo.PARAM_wave_gaingraph = _AgentRelationInfo.sensorinfo().param_wave_gaingraph();

        hexidxslist.resize(_AgentRelationInfo.hexidxslist_size());
        for (int index = 0; index < _AgentRelationInfo.hexidxslist_size(); index++)
        {
            hexidxslist[index] = transdata_param_seq_hexidx{QString::fromStdString(_AgentRelationInfo.hexidxslist().Get(index).param_seq_hexidx_element()).toULongLong(),
                                                            transdata_param_seq_hexidx_hgt{_AgentRelationInfo.hexidxslist().Get(index).param_seq_hexidx_hgt().param_seq_hexidx_hgt(),
                                                                                           _AgentRelationInfo.hexidxslist().Get(index).param_seq_hexidx_hgt().param_seq_hexidx_hgt0(),
                                                                                           _AgentRelationInfo.hexidxslist().Get(index).param_seq_hexidx_hgt().param_seq_hexidx_hgtn(),
                                                                                           _AgentRelationInfo.hexidxslist().Get(index).param_seq_hexidx_hgt().param_wave_info_entropy()}};
        }

        polygonlist.resize(_AgentRelationInfo.polygonlist_size());
        for (int index = 0; index < _AgentRelationInfo.polygonlist_size(); index++)
        {
            polygonlist[index] = TYPE_POLYGON_PT{(LON_LAT_TYPE)_AgentRelationInfo.polygonlist().Get(index).param_longitude(),
                                                 (LON_LAT_TYPE)_AgentRelationInfo.polygonlist().Get(index).param_latitude(),
                                                 _AgentRelationInfo.polygonlist().Get(index).param_amsl(),
                                                 _AgentRelationInfo.polygonlist().Get(index).param_ref() };
        }
    }
#endif
    m_displaycallback(transdata.uildsrc, transdata.uilddst, hexidxslist, polygonlist, transdata.sensorinfo, transdata.eDdisplayMode);
}

void GaeactorManager::dealtransformHexAttDataSlot(const QByteArray &by)
{

    const char* pdata = by.data();

    transentityhexidxpostdata transdata;
#if 0
    memcpy(&transdata, pdata, sizeof(transentityhexidxpostdata));
#else
    ::msg::AgentPositionInfo::msg_transentityhexidxpostdata _AgentRelationInfo;
    _AgentRelationInfo.ParseFromArray(pdata, by.size());
    size_t size = _AgentRelationInfo.ByteSizeLong();
    int cc = by.size();
    if (by.size() == _AgentRelationInfo.ByteSizeLong())
    {
        transdata.uildsrc = QString::fromStdString(_AgentRelationInfo.uildsrc()).toULongLong();
        transdata.uilddst = QString::fromStdString(_AgentRelationInfo.uilddst()).toULongLong();
        transdata.eDdisplayMode = (E_DISPLAY_MODE)_AgentRelationInfo.eddisplaymode();

        transdata.entityinfo.PARAM_pos_pack_index = QString::fromStdString(_AgentRelationInfo.entityinfo().param_pos_pack_index()).toULongLong();
        transdata.entityinfo.PARAM_timestamp = QString::fromStdString(_AgentRelationInfo.entityinfo().param_timestamp()).toULongLong();
        transdata.entityinfo.PARAM_sim_timestamp = QString::fromStdString(_AgentRelationInfo.entityinfo().param_sim_timestamp()).toULongLong();
        transdata.entityinfo.PARAM_sensor_property = _AgentRelationInfo.entityinfo().param_sensor_property();
        transdata.entityinfo.PARAM_pos_hexidx = QString::fromStdString(_AgentRelationInfo.entityinfo().param_pos_hexidx()).toULongLong();
        transdata.entityinfo.PARAM_longitude = _AgentRelationInfo.entityinfo().param_longitude();
        transdata.entityinfo.PARAM_latitude = _AgentRelationInfo.entityinfo().param_latitude();
        transdata.entityinfo.PARAM_amsl = _AgentRelationInfo.entityinfo().param_amsl();
        transdata.entityinfo.PARAM_ref = _AgentRelationInfo.entityinfo().param_ref();
        transdata.entityinfo.PARAM_roll = _AgentRelationInfo.entityinfo().param_roll();
        transdata.entityinfo.PARAM_pitch = _AgentRelationInfo.entityinfo().param_pitch();
        transdata.entityinfo.PARAM_yaw = _AgentRelationInfo.entityinfo().param_yaw();
        transdata.entityinfo.PARAM_speed = _AgentRelationInfo.entityinfo().param_speed();

        transdata.entityinfo.PARAM_reserved[0] = _AgentRelationInfo.entityinfo().param_reserved1();
        transdata.entityinfo.PARAM_reserved[1] = _AgentRelationInfo.entityinfo().param_reserved2();
        transdata.entityinfo.PARAM_reserved[2] = _AgentRelationInfo.entityinfo().param_reserved3();
        transdata.entityinfo.PARAM_reserved[3] = _AgentRelationInfo.entityinfo().param_reserved4();
        transdata.entityinfo.PARAM_reserved[4] = _AgentRelationInfo.entityinfo().param_reserved5();
        transdata.entityinfo.PARAM_reserved[5] = _AgentRelationInfo.entityinfo().param_reserved6();

    }
#endif

    m_displayPosCallback(transdata.uildsrc, transdata.uilddst, transdata.entityinfo, transdata.eDdisplayMode);
}

void GaeactorManager::dealtransformHexAttArrayDataSlot(const QByteArray &by)
{
    const char* pdata = by.data();
#if 0
    uint32_t num = by.length() / sizeof(transentityhexidxpostdata);
    //  std::cout << "recv pos data num " << num << std::endl;
    transentityhexidxpostdata* ptransdatasrc = (transentityhexidxpostdata*)(pdata);
    for (int index = 0; index < num; index++)
    {
        transentityhexidxpostdata* ptransdata = (ptransdatasrc + index);
        m_displayPosCallback(ptransdata->uildsrc, ptransdata->uilddst, ptransdata->entityinfo, ptransdata->eDdisplayMode);
    }
#else
    ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentRelationInfo_array;
    _AgentRelationInfo_array.ParseFromArray(pdata, by.size());
    size_t size = _AgentRelationInfo_array.ByteSizeLong();
    int cc = by.size();
    if (by.size() == _AgentRelationInfo_array.ByteSizeLong())
    {
        for (int index = 0; index < _AgentRelationInfo_array.transentityhexidxpostdata_size(); index++)
        {
            auto& _AgentRelationInfo = _AgentRelationInfo_array.transentityhexidxpostdata().Get(index);

            transentityhexidxpostdata transdata;

            transdata.uildsrc = QString::fromStdString(_AgentRelationInfo.uildsrc()).toULongLong();
            transdata.uilddst = QString::fromStdString(_AgentRelationInfo.uilddst()).toULongLong();
            transdata.eDdisplayMode = (E_DISPLAY_MODE)_AgentRelationInfo.eddisplaymode();

            transdata.entityinfo.PARAM_pos_pack_index = QString::fromStdString(_AgentRelationInfo.entityinfo().param_pos_pack_index()).toULongLong();
            transdata.entityinfo.PARAM_timestamp = QString::fromStdString(_AgentRelationInfo.entityinfo().param_timestamp()).toULongLong();
            transdata.entityinfo.PARAM_sim_timestamp = QString::fromStdString(_AgentRelationInfo.entityinfo().param_sim_timestamp()).toULongLong();
            transdata.entityinfo.PARAM_sensor_property = _AgentRelationInfo.entityinfo().param_sensor_property();
            transdata.entityinfo.PARAM_pos_hexidx = QString::fromStdString(_AgentRelationInfo.entityinfo().param_pos_hexidx()).toULongLong();
            transdata.entityinfo.PARAM_longitude = _AgentRelationInfo.entityinfo().param_longitude();
            transdata.entityinfo.PARAM_latitude = _AgentRelationInfo.entityinfo().param_latitude();
            transdata.entityinfo.PARAM_amsl = _AgentRelationInfo.entityinfo().param_amsl();
            transdata.entityinfo.PARAM_ref = _AgentRelationInfo.entityinfo().param_ref();
            transdata.entityinfo.PARAM_roll = _AgentRelationInfo.entityinfo().param_roll();
            transdata.entityinfo.PARAM_pitch = _AgentRelationInfo.entityinfo().param_pitch();
            transdata.entityinfo.PARAM_yaw = _AgentRelationInfo.entityinfo().param_yaw();
            transdata.entityinfo.PARAM_speed = _AgentRelationInfo.entityinfo().param_speed();

            transdata.entityinfo.PARAM_reserved[0] = _AgentRelationInfo.entityinfo().param_reserved1();
            transdata.entityinfo.PARAM_reserved[1] = _AgentRelationInfo.entityinfo().param_reserved2();
            transdata.entityinfo.PARAM_reserved[2] = _AgentRelationInfo.entityinfo().param_reserved3();
            transdata.entityinfo.PARAM_reserved[3] = _AgentRelationInfo.entityinfo().param_reserved4();
            transdata.entityinfo.PARAM_reserved[4] = _AgentRelationInfo.entityinfo().param_reserved5();
            transdata.entityinfo.PARAM_reserved[5] = _AgentRelationInfo.entityinfo().param_reserved6();

            m_displayPosCallback(transdata.uildsrc, transdata.uilddst, transdata.entityinfo, transdata.eDdisplayMode);
        }
    }
#endif
}

void GaeactorManager::dealtransformHexIntersectionDataSlot(const QByteArray&by)
{

    /*
    struct transintersectiondata
    {
        TYPE_ULID uildsrc;
        TYPE_ULID uilddst;
        E_DISPLAY_MODE eDdisplayMode;
        UINT32 flexarrycount;
        struct hexlist
        {
            TYPE_ULID sensoruildsrc;
            TYPE_ULID entityuilddst;
            H3INDEX hexidx;
        } hexidxList[];
    };
    */
    const char * pdata = by.data();
    transintersectiondata transdata;
    memcpy(&transdata, pdata, sizeof(transintersectiondata));
    transintersectiondata::hexlist * pHexidxlist = (transintersectiondata::hexlist *)(pdata + sizeof(transintersectiondata));
    std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> > hexidxslistinfo;
    if(transdata.flexarrycount != 0)
    {
        hexidxslistinfo.resize(transdata.flexarrycount);

        for(int index = 0; index < transdata.flexarrycount;index++)
        {
            hexidxslistinfo[index] = std::make_tuple(pHexidxlist[index].sensoruildsrc, pHexidxlist[index].entityuilddst, pHexidxlist[index].hexidx);
        }
        m_intersection_display_hexidx_update_callback(transdata.uildsrc, transdata.uilddst, hexidxslistinfo, transdata.eDdisplayMode);
    }
}

void GaeactorManager::dealtransformEchowaveDataSlot(const QByteArray&by)
{

    /*
    struct transechowavedata
    {
        struct geolatlng
        {
            double mLat;
            double mLon;
        };

        TYPE_ULID uildval;
        TYPE_ULID sensoruildsrc;
        TYPE_ULID entityuilddst;
        H3INDEX sensorhexidx;
        H3INDEX entityhexidx;
        bool bEchoWave;
        UINT32 hexidxlistcount;
        UINT32 geolistcount;
        UINT32 flexarrycount;
        BYTE flexarray[];//H3INDEX[]+geolatlng[]
    };
    */
    const char * pdata = by.data();
    transechowavedata transdata;
    memcpy(&transdata, pdata, sizeof(transechowavedata));
    BYTE * pflexarray = (BYTE *)(pdata + sizeof(transechowavedata));
    HEXIDX_HGT_ARRAY hexidxslist;
    hexidxslist.resize(transdata.hexidxlistcount);
    QVector<LAT_LNG> geolatlnglist;
    geolatlnglist.resize(transdata.geolistcount);
    H3INDEX * phexidxlist = reinterpret_cast<H3INDEX*>(pflexarray);
    for(int index = 0; index < transdata.hexidxlistcount;index++)
    {
        hexidxslist[index].PARAM_seq_hexidx_element = phexidxlist[index];
    }

    pflexarray = pflexarray + sizeof(H3INDEX)*transdata.hexidxlistcount;

    transechowavedata::geolatlng * pgeolatlnglist = reinterpret_cast<transechowavedata::geolatlng*>(pflexarray);
    for(int index = 0; index < transdata.geolistcount; index++)
    {
        geolatlnglist[index].lat =(pgeolatlnglist[index].mLat);
        geolatlnglist[index].lng =(pgeolatlnglist[index].mLon);
    }
    m_echowave_display_hexidx_update_callback(transdata.uildval,\
                                              std::make_tuple(transdata.sensoruildsrc, transdata.entityuilddst,transdata.sensingmediaildsrc, transdata.sensorhexidx, transdata.entityhexidx),\
                                              hexidxslist, geolatlnglist,
                                              transdata.bEchoWave);
}

void GaeactorManager::dealtransformEchowaveArrayDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    uint32_t num = by.length() / sizeof(transechowavedatasimple);
    transechowavedatasimple* ptransdatasrc = (transechowavedatasimple*)(pdata);

    for (int index = 0; index < num; index++)
    {
        HEXIDX_HGT_ARRAY hexidxslist;
        transechowavedatasimple* ptransdata = (ptransdatasrc + index);
        hexidxslist.resize(ptransdata->hexidxlistcount);
        QVector<LAT_LNG> geolatlnglist;
        geolatlnglist.resize(ptransdata->geolistcount);
        for (int id = 0; id < ptransdata->hexidxlistcount; id++)
        {
            hexidxslist[id].PARAM_seq_hexidx_element = ptransdata->hexidxlist[id];
        }
        for (int id = 0; id < ptransdata->geolistcount; id++)
        {
            geolatlnglist[id].lat = (ptransdata->geolist[id].mLat);
            geolatlnglist[id].lng = (ptransdata->geolist[id].mLon);
        }
        m_echowave_display_hexidx_update_callback(ptransdata->uildval, \
                                                  std::make_tuple(ptransdata->sensoruildsrc, ptransdata->entityuilddst,ptransdata->sensingmediaildsrc, ptransdata->sensorhexidx, ptransdata->entityhexidx), \
                                                  hexidxslist, geolatlnglist,
                                                  ptransdata->bEchoWave);
    }
}

void GaeactorManager::dealtransformEventlistDataSlot(const QByteArray&by)
{

    /*
    struct transeventlistdata
    {

        struct transevententitydata
        {
            INT32 lng;   //1/10000000
            INT32 lat;   //1/10000000
            INT32 alt;   //1/1000
            INT32 ref;
            FLOAT32 roll;  //1/1000
            FLOAT32 pitch; //1/1000
            FLOAT32 yaw;  //1/1000
        };

        struct transeventdata
        {
            TYPE_ULID sensoruildsrc;
            TYPE_ULID entityuilddst;
            H3INDEX sensorhexidx;
            H3INDEX entityhexidx;
            E_EVENT_MODE eventType;
            transevententitydata sensorinfo;
            transevententitydata entityinfo;
        };
        E_EVENT_MODE eventType;
        UINT32 eventcount;
        transeventdata eventlist[];
    };
    */
    const char * pdata = by.data();
    transeventlistdata transdata;
    memcpy(&transdata, pdata, sizeof(transeventlistdata));
    transeventdata * peventlist = (transeventdata *)(pdata + sizeof(transeventlistdata));

    std::vector<EVENT_INFO>  eventinfolist;
    for(int index = 0; index < transdata.eventcount; index++)
    {
        eventinfolist.emplace_back(peventlist[index].eventifo);
    }

    m_eventlist_update_callback(transdata.eventType, eventinfolist);
}

void GaeactorManager::dealtransformEventlistArrayDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    uint32_t num = by.length() / sizeof(transeventlistdatasimple);
    transeventlistdatasimple* ptransdatasrc = (transeventlistdatasimple*)(pdata);

    for (int index = 0; index < num; index++)
    {
        std::vector<EVENT_INFO>  eventinfolist;
        transeventlistdatasimple* ptransdata = (ptransdatasrc + index);
        eventinfolist.emplace_back(ptransdata->event.eventifo);
        m_eventlist_update_callback(ptransdata->eventType, eventinfolist);
    }
}

CHANNEL_INFO* GaeactorManager::getchannel(TYPE_ULID ulid)
{
#ifdef USING_GAEACTOR_TRANSMIT

    auto itor = m_ulidchannel.find(ulid);
    if(itor == m_ulidchannel.end())
    {
        m_ulidchannel.insert(std::make_pair(ulid,m_pGaeactorTransmit->getReusePublisherChannel()));
    }
    return std::get<1>(m_ulidchannel.at(ulid));
#endif
    return nullptr;
}

void GaeactorManager::registEventlistUpdateCallback(const eventlist_update_callback &newEventlist_update_callback)
{
    m_eventlist_update_callback = std::move(newEventlist_update_callback);
}
