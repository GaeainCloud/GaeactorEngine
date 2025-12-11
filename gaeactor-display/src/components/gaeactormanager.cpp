#include "gaeactormanager.h"
#include <QTimer>
#include <iostream>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include "KlusterWebSocketClient.h"
#include "components/function.h"
#include "settingsconfig.h"
#include "./src/OriginalDateTime.h"

#include "./proto/protoc/AgentPositionInfo.pb.h"
#include "LocationHelper.h"
#include "gaeactor_comm_interface.h"
#include "runningmodeconfig.h"

#define USING_PROTOBUF

GaeactorManager::GaeactorManager(E_RUNNING_MODE eRunningMode, QObject *parent)
	:QObject(parent),
    m_wsclient_(nullptr),
    m_eRunningMode(eRunningMode)
{
    qRegisterMetaType<E_CHANNEL_TRANSMITDATA_TYPE>("E_CHANNEL_TRANSMITDATA_TYPE");
	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
    {
        if (SettingsConfig::getInstance().lavic_desktop_cfg().display_remote)
		{
            QString ipstr = SettingsConfig::getInstance().lavic_desktop_cfg().display_remote_url;
			std::cout << "using remote data" << ipstr.toStdString() << std::endl;
            m_wsclient_ = new KlusterWebSocketClient(QUrl(ipstr));

            m_wsclient_->setBinaryDataCallback(std::bind(&GaeactorManager::binary_receive_data_call_back,this,std::placeholders::_1,std::placeholders::_2));
            m_wsclient_->Connect();
		}
		else
		{
            QString ipstr = SettingsConfig::getInstance().lavic_desktop_cfg().display_remote_url;
			std::cout << "using remote data" << ipstr.toStdString() << std::endl;
            m_wsclient_ = new KlusterWebSocketClient(QUrl(ipstr));
            m_wsclient_->setBinaryDataCallback(std::bind(&GaeactorManager::binary_receive_data_call_back,this,std::placeholders::_1,std::placeholders::_2));
            m_wsclient_->Connect();
		}
	}
	break;
	case GaeactorManager::E_RUNNING_MODE_REVIEW:
		break;
	default:
		break;
	}
}

GaeactorManager::~GaeactorManager()
{
#ifdef USING_GAEACTOR_TRANSMIT
	if (m_pGaeactorTransmit)
	{
		m_pGaeactorTransmit->deleteLater();
	}
#endif
}

void GaeactorManager::SendBinaryMessage(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType,const char * message, int isize)
{
    size_t payloadsize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + isize;
    void * usrpayload  = (void*)malloc(payloadsize);
    memset(usrpayload,0,payloadsize);

    BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
    memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
    memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));

    BYTE * pdata = pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
    memcpy(pdata, message, isize);

    if(m_wsclient_)
    {
        m_wsclient_->SendBinaryMessage((const char*)usrpayload, payloadsize);
    }
    free(usrpayload);
}

void GaeactorManager::dealSensorPath(const TYPE_ULID& src, const TYPE_ULID& dst)
{
	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
	{
		transdata_sensor_path _sensor_path;
		memset(&_sensor_path, 0, sizeof(transdata_sensor_path));
		_sensor_path.srcid = src;
		_sensor_path.dstid = dst;
#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_PATH, (const BYTE *)&_sensor_path, sizeof(transdata_sensor_path));
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_PATH, (const char *)&_sensor_path, sizeof(transdata_sensor_path));
#endif
	}break;
	default:break;
	}
}

void GaeactorManager::dealentityHexidex(TYPE_ULID ulid, const LAT_LNG &pos)
{
	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
	{
		transdata_pos_hexidx pos_hexidx_data;

		memset(&pos_hexidx_data, 0, sizeof(transdata_pos_hexidx));
		pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;

        LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx, pos.lat, pos.lng, INDEX_MAPPING_RESOLUTION_ENTITY_POS);
		//    m_pGaeactorTransmit->transmitData(getchannel(ulid), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));

#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const char *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#endif
	}break;
	default:break;
	}
}

void GaeactorManager::clearentityHexidex(TYPE_ULID ulid)
{
	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
	{
		transdata_pos_hexidx pos_hexidx_data;

		memset(&pos_hexidx_data, 0, sizeof(transdata_pos_hexidx));
		pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;

		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx = 0;

#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const char *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#endif

		std::string sensorulidstr = QString::number(ulid).toStdString();
		std::cout << "send clear entity hexidx: " << sensorulidstr << std::endl;
	}break;
	default:break;
	}
}

void GaeactorManager::dealHexidex(TYPE_ULID ulid, const LAT_LNG& pos, const HEXIDX_ARRAY &hexidxslist, const std::vector<transdata_param_seq_polygon>& _polygon, const UINT32 &slient_time_gap)
{

	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
	{
#if 0
		transdata_pos_hexidx pos_hexidx_data;
		size_t isize = sizeof(transdata_wave_smd_hexidx) + sizeof(transdata_param_seq_hexidx) * hexidxslist.size();
		transdata_wave_smd_hexidx *data = (transdata_wave_smd_hexidx *)std::malloc(isize);
		for (int i = 0; i < 100000; i++)
		{
			UINT64 CURRENTTIMSTAMP = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
			CURRENTTIMSTAMP += i;
			TYPE_ULID   _ulidTMP = ulid::Create(CURRENTTIMSTAMP, []() { return 128; });
			/////////////////////////////////////////////////////////////////////////////////////////
			memset(&pos_hexidx_data, 0, sizeof(transdata_pos_hexidx));
			pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulidTMP;
            pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx = LocationHelper::getIndexInfo(pos.lat, pos.lng, INDEX_MAPPING_RESOLUTION_ENTITY_POS);
			pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_latitude = pos.lat * 10000000;
			pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_longitude = pos.lng * 10000000;
			m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));

			//////////////////////////////////////////////////////////////////////////////////////////
			memset(data, 0, isize);
			transdata_wave_smd_hexidx & wave_smd_hexidx_data = *data;
			wave_smd_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = _ulidTMP;
			wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_wave_snd_rcv = 0x01;
			wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_wave_silent_time_gap = 255;
			wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_hexidx.PARAM_qty_hexidx = hexidxslist.size();
			for (int i = 0; i < hexidxslist.size(); i++)
			{
				wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_hexidx.PARAM_seq_hexidx[i].PARAM_seq_hexidx_element = hexidxslist.at(i);
			}

#ifdef USING_GAEACTOR_TRANSMIT
			m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentSensors), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#else
            this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#endif

		}
		free(data);
#else
		/////////////////////////////////////////////////////////////////////////////////////////
		transdata_pos_hexidx pos_hexidx_data;

		memset(&pos_hexidx_data, 0, sizeof(transdata_pos_hexidx));
		pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;
        LocationHelper::getIndexInfo(pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx, pos.lat, pos.lng, INDEX_MAPPING_RESOLUTION_ENTITY_POS);
		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_latitude = pos.lat * 10000000;
		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_longitude = pos.lng * 10000000;
		//    m_pGaeactorTransmit->transmitData(getchannel(ulid), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const char *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#endif

		//////////////////////////////////////////////////////////////////////////////////////////
		size_t ibufsize = sizeof(UINT16) + sizeof(transdata_param_seq_hexidx) * hexidxslist.size() + sizeof(UINT16) + sizeof(transdata_param_seq_polygon) * _polygon.size();
		size_t isize = sizeof(transdata_wave_smd_hexidx) + ibufsize;
		transdata_wave_smd_hexidx *data = (transdata_wave_smd_hexidx *)std::malloc(isize);
		memset(data, 0, isize);
		transdata_wave_smd_hexidx & wave_smd_hexidx_data = *data;
		wave_smd_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_snd_rcv = 0x01;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_silent_time_gap = slient_time_gap;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count = ibufsize;

		BYTE *p_byte_buffer = (BYTE*)(wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_seq_buffer);
		transdata_payload_hexidx *ptransdata_payload_hexidx = GET_HEXIDX_STRUCT_PTR(p_byte_buffer);
		ptransdata_payload_hexidx->PARAM_qty_hexidx = hexidxslist.size();
		for (int i = 0; i < hexidxslist.size(); i++)
		{
			ptransdata_payload_hexidx->PARAM_seq_hexidx[i].PARAM_seq_hexidx_element = hexidxslist.at(i);
		}

		transdata_payload_polygon *ptransdata_payload_polygon = GET_POLYGON_STRUCT_PTR(p_byte_buffer);
		ptransdata_payload_polygon->PARAM_qty_polygon = _polygon.size();
		memcpy(ptransdata_payload_polygon->PARAM_seq_polygon, _polygon.data(), sizeof(transdata_param_seq_polygon) * _polygon.size());
		//    m_pGaeactorTransmit->transmitData(getchannel(ulid), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentSensors), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const char *)data, isize);
#endif
		free(data);
#endif
	}break;
	default:break;
	}

}

void GaeactorManager::clearHexidex(TYPE_ULID ulid)
{
	switch (m_eRunningMode)
	{
	case GaeactorManager::E_RUNNING_MODE_REALTIME:
	{
		std::string sensorulidstr = QString::number(ulid).toStdString();

		std::cout << "send clear entity and sensor hexidx: " << sensorulidstr << std::endl;
		/////////////////////////////////////////////////////////////////////////////////////////
		transdata_pos_hexidx pos_hexidx_data;
		memset(&pos_hexidx_data, 0, sizeof(transdata_pos_hexidx));
		pos_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;
		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_pos_hexidx = 0;
		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_latitude = 0;
		pos_hexidx_data.PARAM_payload_pos_hexidx.PARAM_longitude = 0;
		//    m_pGaeactorTransmit->transmitData(getchannel(ulid), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentCores), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const BYTE *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POS, (const char *)&pos_hexidx_data, sizeof(transdata_pos_hexidx));
#endif
		//    stdutils::OriDateTime::sleep(1);
			//////////////////////////////////////////////////////////////////////////////////////////
		size_t isize = sizeof(transdata_wave_smd_hexidx) + sizeof(transdata_param_seq_hexidx) * 0;
		transdata_wave_smd_hexidx *data = (transdata_wave_smd_hexidx *)std::malloc(isize);
		memset(data, 0, isize);
		transdata_wave_smd_hexidx & wave_smd_hexidx_data = *data;
		wave_smd_hexidx_data.PARAM_protocol_head.PARAM_source_ulid = ulid;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_snd_rcv = 0x01;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_silent_time_gap = 0;
		wave_smd_hexidx_data.PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count = 0;
		//    m_pGaeactorTransmit->transmitData(getchannel(ulid), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#ifdef USING_GAEACTOR_TRANSMIT
		m_pGaeactorTransmit->transmitData(std::get<1>(m_Transmitchannel_AgentSensors), E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const BYTE *)data, isize);
#else
        this->SendBinaryMessage(E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR, (const char *)data, isize);
#endif
		free(data);
	}break;
	default:break;
	}
}

void GaeactorManager::receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pdata, const UINT32 &ilen, const BYTE*pOriginaldata, const UINT32& iOriginallen)
{
    QByteArray by((const char *)pdata, ilen);
    emit dealtransformSig(channelTransmitDataType, by);
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



GaeactorManagerHelper::GaeactorManagerHelper(QObject *parent /*= nullptr*/)
	:QObject(parent)
{

}

GaeactorManagerHelper::~GaeactorManagerHelper()
{

}

void GaeactorManagerHelper::registDisplayCallback(display_hexidx_update_callback func)
{
	m_displaycallback = std::move(func);
}

void GaeactorManagerHelper::registDisplayPosCallback(display_pos_update_callback func)
{
	m_displayPosCallback = std::move(func);
}

void GaeactorManagerHelper::registIntersectionDisplayCallback(intersection_display_hexidx_update_callback func)
{
	m_intersection_display_hexidx_update_callback = std::move(func);
}

void GaeactorManagerHelper::registEchoWaveDisplayCallback(echowave_display_hexidx_update_callback func)
{
	m_echowave_display_hexidx_update_callback = std::move(func);
}

void GaeactorManagerHelper::registEventlistUpdateCallback(const eventlist_update_callback &newEventlist_update_callback)
{
	m_eventlist_update_callback = std::move(newEventlist_update_callback);
}

void GaeactorManagerHelper::registPathUpdateCallback(const path_update_callback &newPath_update_callback)
{
	m_path_update_callback = std::move(newPath_update_callback);
}

void GaeactorManagerHelper::registSensorUpdateCallback(const sensor_update_callback &newsensor_update_callback)
{
    m_sensor_update_callback = std::move(newsensor_update_callback);
}

void GaeactorManagerHelper::registAgentrelationUpdateCallback(const agentrelation_update_callback &agentrelation_update_callback)
{
    m_agentrelation_update_callback = std::move(agentrelation_update_callback);
}

void GaeactorManagerHelper::registAgentCommSnrUpdateCallback(const agentcommsnr_update_callback &agentcommsnr_update_callback)
{
    m_agentcommsnr_update_callback = std::move(agentcommsnr_update_callback);
}

void GaeactorManagerHelper::registAgentCommStackUpdateCallback(const agentcommstack_update_callback &agentcommstack_update_callback)
{
    m_agentcommstack_update_callback = std::move(agentcommstack_update_callback);

}

void GaeactorManagerHelper::registSmdInfoUpdateCallback(const smdinfo_update_callback &smdinfo_update_callback)
{
    m_smdinfo_update_callback = std::move(smdinfo_update_callback);
}

void GaeactorManagerHelper::registPrejdugementUpdateCallback(const prejudgment_update_callback &_prejudgment_update_callback)
{
    m_prejudgment_update_callback = std::move(_prejudgment_update_callback);
}

void GaeactorManagerHelper::dealtransformSlot(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const QByteArray &by)
{
    switch (channelTransmitDataType) {
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX:
    {
        dealtransformHexDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX:
    {
        dealtransformHexAttDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
    {
        dealtransformHexAttArrayDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_INTERSECTION:
    {
        dealtransformHexIntersectionDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE:
    {
        dealtransformEchowaveDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ECHOWAVE_ARRAY:
    {
        dealtransformEchowaveArrayDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT:
    {
        dealtransformEventlistDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
    {
        dealtransformEventlistArrayDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_PATH_ARRAY:
    {
        dealtransformSensorPathArrayDataSlot(by);
    }break;
    case E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE:
    {
        dealtransformSensorUpdateDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_SENSOR_UPDATE_ARRAY:
    {
        dealtransformSensorUpdateArrayDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_RELATION:
    {
        dealtransformAgentRelationDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_SNR:
    {
        dealtransformAgentCommSnrDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_AGENT_COMMSTACK_RESULT:
    {
        dealtransformAgentCommStckRelustDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_SIMPARAMS:
    {
        dealtransformSimParamsDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_SMDINFO:
    {
        dealtransformSmdInfoDataSlot(by);
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_PREJUSTMENTLINE:
    {
        dealtransformPrejudgmentDataSlot(by);
    }
    break;
    default:
        break;
    }
}


void GaeactorManagerHelper::dealtransformHexDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();

	transentityhexidxdata transdata;

    HEXIDX_HGT_ARRAY hexidxslist;
	POLYGON_LIST polygonlist;
#ifndef USING_PROTOBUF
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
        gaeactortransmit::GaeactorTransmit::DeserializeHexidxData(_AgentRelationInfo, transdata, hexidxslist, polygonlist);
	}
#endif
	m_displaycallback(transdata.uildsrc, transdata.uilddst, hexidxslist, polygonlist, transdata.sensorinfo, transdata.eDdisplayMode);
}

void GaeactorManagerHelper::dealtransformHexAttDataSlot(const QByteArray &by)
{

	const char* pdata = by.data();

	transentityhexidxpostdata transdata;
#ifndef USING_PROTOBUF

	memcpy(&transdata, pdata, sizeof(transentityhexidxpostdata));
#else
	::msg::AgentPositionInfo::msg_transentityhexidxpostdata _AgentRelationInfo;
	_AgentRelationInfo.ParseFromArray(pdata, by.size());
	size_t size = _AgentRelationInfo.ByteSizeLong();
	int cc = by.size();
	if (by.size() == _AgentRelationInfo.ByteSizeLong())
	{
        gaeactortransmit::GaeactorTransmit::DeserializePositionData(_AgentRelationInfo, transdata);
	}
#endif
    UINT64 cur_time_stamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    bool bDeal = false;
    auto _posupdate_ctrl_itor = m_posupdate_ctrl.find(transdata.uildsrc);
    if(_posupdate_ctrl_itor == m_posupdate_ctrl.end())
    {
        m_posupdate_ctrl.insert(std::make_pair(transdata.uildsrc, cur_time_stamp));
        bDeal = true;
    }
    else
    {
        if(fabs(cur_time_stamp - _posupdate_ctrl_itor->second)>60)
        {
            _posupdate_ctrl_itor->second =  cur_time_stamp;
            bDeal = true;
        }
    }
    bDeal = true;
    if(bDeal)
    {
        m_displayPosCallback(transdata.uildsrc, transdata.uilddst, transdata.entityinfo, transdata.eDdisplayMode);
    }
}

void GaeactorManagerHelper::dealtransformHexAttArrayDataSlot(const QByteArray&by)
{
	const char* pdata = by.data();
#ifndef USING_PROTOBUF

	uint32_t num = by.length() / sizeof(transentityhexidxpostdata);
	//	std::cout << "recv pos data num " << num << std::endl;
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
    //std::cout <<"protobuf size "<<size<<" len "<<cc<<"\n";
	if (by.size() == _AgentRelationInfo_array.ByteSizeLong())
	{
		for (int index = 0; index < _AgentRelationInfo_array.transentityhexidxpostdata_size(); index++)
		{
			auto& _AgentRelationInfo = _AgentRelationInfo_array.transentityhexidxpostdata().Get(index);

            transentityhexidxpostdata transdata;
            gaeactortransmit::GaeactorTransmit::DeserializePositionData(_AgentRelationInfo,transdata);


            if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
            {
                m_displayPosCallback(transdata.uildsrc, transdata.uilddst, transdata.entityinfo, transdata.eDdisplayMode);

            }
            else
            {

//                UINT64 cur_time_stamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
//                bool bDeal = false;
//                auto _posupdate_ctrl_itor = m_posupdate_ctrl.find(transdata.uildsrc);
//                if(_posupdate_ctrl_itor == m_posupdate_ctrl.end())
//                {
//                    m_posupdate_ctrl.insert(std::make_pair(transdata.uildsrc, cur_time_stamp));
//                    bDeal = true;
//                }
//                else
//                {
//                    if(fabs(cur_time_stamp - _posupdate_ctrl_itor->second)>60)
//                    {
//                        _posupdate_ctrl_itor->second =  cur_time_stamp;
//                        bDeal = true;
//                    }
//                }
//                if(bDeal)
                {
                    m_displayPosCallback(transdata.uildsrc, transdata.uilddst, transdata.entityinfo, transdata.eDdisplayMode);
                }
            }
		}
	}
#endif

}

void GaeactorManagerHelper::dealtransformHexIntersectionDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	transintersectiondata transdata;
	memcpy(&transdata, pdata, sizeof(transintersectiondata));
	transintersectiondata::hexlist * pHexidxlist = (transintersectiondata::hexlist *)(pdata + sizeof(transintersectiondata));
	std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> > hexidxslistinfo;
	if (transdata.flexarrycount != 0)
	{
		hexidxslistinfo.resize(transdata.flexarrycount);

		for (int index = 0; index < transdata.flexarrycount; index++)
		{
			hexidxslistinfo[index] = std::make_tuple(pHexidxlist[index].sensoruildsrc, pHexidxlist[index].entityuilddst, pHexidxlist[index].hexidx);
		}
		m_intersection_display_hexidx_update_callback(transdata.uildsrc, transdata.uilddst, hexidxslistinfo, transdata.eDdisplayMode);
	}
}

void GaeactorManagerHelper::dealtransformEchowaveDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	transechowavedata transdata;
	memcpy(&transdata, pdata, sizeof(transechowavedata));
	BYTE * pflexarray = (BYTE *)(pdata + sizeof(transechowavedata));
    HEXIDX_HGT_ARRAY hexidxslist;
	hexidxslist.resize(transdata.hexidxlistcount);
    QVector<LAT_LNG> geolatlnglist;
	geolatlnglist.resize(transdata.geolistcount);
    transdata_param_seq_hexidx * phexidxlist = reinterpret_cast<transdata_param_seq_hexidx*>(pflexarray);
    hexidxslist.resize(transdata.hexidxlistcount);
    memcpy(hexidxslist.data(), phexidxlist, sizeof(transdata_param_seq_hexidx) * transdata.hexidxlistcount);

	pflexarray = pflexarray + sizeof(H3INDEX)*transdata.hexidxlistcount;

	transechowavedata::geolatlng * pgeolatlnglist = reinterpret_cast<transechowavedata::geolatlng*>(pflexarray);
	for (int index = 0; index < transdata.geolistcount; index++)
	{
        geolatlnglist[index].lat =(pgeolatlnglist[index].mLat);
        geolatlnglist[index].lng =(pgeolatlnglist[index].mLon);
	}
	m_echowave_display_hexidx_update_callback(transdata.uildval, \
		std::make_tuple(transdata.sensoruildsrc, transdata.entityuilddst, transdata.sensingmediaildsrc, transdata.sensorhexidx, transdata.entityhexidx), \
		hexidxslist, geolatlnglist,
		transdata.bEchoWave);
}

void GaeactorManagerHelper::dealtransformEchowaveArrayDataSlot(const QByteArray&by)
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
            geolatlnglist[id].lat = ptransdata->geolist[id].mLat;
            geolatlnglist[id].lng = ptransdata->geolist[id].mLon;
		}
		m_echowave_display_hexidx_update_callback(ptransdata->uildval, \
			std::make_tuple(ptransdata->sensoruildsrc, ptransdata->entityuilddst, ptransdata->sensingmediaildsrc, ptransdata->sensorhexidx, ptransdata->entityhexidx), \
			hexidxslist, geolatlnglist,
			ptransdata->bEchoWave);
	}
}

void GaeactorManagerHelper::dealtransformEventlistDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	transeventlistdata transdata;
	std::vector<EVENT_INFO>  eventinfolist;
#ifndef USING_PROTOBUF

	memcpy(&transdata, pdata, sizeof(transeventlistdata));
	transeventdata * peventlist = (transeventdata *)(pdata + sizeof(transeventlistdata));

	for (int index = 0; index < transdata.eventcount; index++)
	{
		eventinfolist.emplace_back(peventlist[index].eventifo);
	}
#else

	::msg::AgentPositionInfo::msg_transeventlistdata _transeventlistdata;
	_transeventlistdata.ParseFromArray(pdata, by.size());
	size_t size = _transeventlistdata.ByteSizeLong();
	int cc = by.size();
	if (by.size() == _transeventlistdata.ByteSizeLong())
	{
		transdata.eventType = (E_EVENT_MODE)_transeventlistdata.eventtype();
		transdata.eventcount = _transeventlistdata.eventlist_size();
		eventinfolist.resize(transdata.eventcount);
		for (int index = 0; index < transdata.eventcount; index++)
		{
			const ::msg::AgentPositionInfo::msg_transeventdata &_msg_transeventdata_item = _transeventlistdata.eventlist().Get(index);

            EVENT_INFO eventifo;
            gaeactortransmit::GaeactorTransmit::DeserializeEventData(_msg_transeventdata_item,eventifo);
			eventinfolist[index] = eventifo;
		}
	}
#endif
	m_eventlist_update_callback(transdata.eventType, eventinfolist);
}

void GaeactorManagerHelper::dealtransformEventlistArrayDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
#ifndef USING_PROTOBUF

	uint32_t num = by.length() / sizeof(transeventlistdatasimple);
	transeventlistdatasimple* ptransdatasrc = (transeventlistdatasimple*)(pdata);

	for (int index = 0; index < num; index++)
	{
		std::vector<EVENT_INFO>  eventinfolist;
		transeventlistdatasimple* ptransdata = (ptransdatasrc + index);
		eventinfolist.emplace_back(ptransdata->event.eventifo);
		m_eventlist_update_callback(ptransdata->eventType, eventinfolist);
	}
#else
	::msg::AgentPositionInfo::msg_transeventlistdatasimple_array _AgentRelationInfo_array;
	_AgentRelationInfo_array.ParseFromArray(pdata, by.size());
	size_t size = _AgentRelationInfo_array.ByteSizeLong();
	int cc = by.size();
	if (by.size() == _AgentRelationInfo_array.ByteSizeLong())
	{
		for (int index = 0; index < _AgentRelationInfo_array.transeventlistdatasimple_size(); index++)
		{
			auto& _AgentRelationInfo = _AgentRelationInfo_array.transeventlistdatasimple().Get(index);


			std::vector<EVENT_INFO>  eventinfolist;
			
            EVENT_INFO eventifo;
            gaeactortransmit::GaeactorTransmit::DeserializeEventData(_AgentRelationInfo.event(),eventifo);
            eventinfolist.emplace_back(eventifo);

			m_eventlist_update_callback((E_EVENT_MODE)_AgentRelationInfo.eventtype(), eventinfolist);
		}
	}

#endif
}

void GaeactorManagerHelper::dealtransformSensorPathArrayDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	transdata_path_info transdata;
	memcpy(&transdata, pdata, sizeof(transdata_path_info));
	tagEdgeVertexIndex * peventlist = (tagEdgeVertexIndex *)(pdata + sizeof(transdata_path_info));


	tagPathInfo pathinfo;
	pathinfo.m_bValid = transdata.m_bValid;
	pathinfo.value = transdata.value;
	pathinfo.end = transdata.end;
	if (transdata.path_buffer_count > 0)
	{
		pathinfo.m_path.resize(transdata.path_buffer_count);
		memcpy(pathinfo.m_path.data(), peventlist, sizeof(tagEdgeVertexIndex) * transdata.path_buffer_count);
	}

	m_path_update_callback(transdata.src, transdata.dst, pathinfo);
}

void GaeactorManagerHelper::dealtransformSensorUpdateDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	trans_sensor_update_info transdata;
	memcpy(&transdata, pdata, sizeof(trans_sensor_update_info));

	m_sensor_update_callback(transdata.src, transdata.mode);

}

void GaeactorManagerHelper::dealtransformSensorUpdateArrayDataSlot(const QByteArray&by)
{
	const char * pdata = by.data();
	uint32_t num = by.length() / sizeof(trans_sensor_update_info);
	//	std::cout << "recv pos data num " << num << std::endl;
	trans_sensor_update_info* ptransdatasrc = (trans_sensor_update_info*)(pdata);
	for (int index = 0; index < num; index++)
	{
		trans_sensor_update_info* ptransdata = (ptransdatasrc + index);
        m_sensor_update_callback(ptransdata->src, ptransdata->mode);
    }
}

void GaeactorManagerHelper::dealtransformAgentRelationDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();

    ::msg::AgentRelationInfo::msg_AgentRelationInfo agentrelationinfo;
        agentrelationinfo.ParseFromArray(pdata, by.size());
    size_t size = agentrelationinfo.ByteSizeLong();
    int cc = by.size();
    if(by.size() == agentrelationinfo.ByteSizeLong())
    {

        if(m_agentrelation_update_callback)
        {
            m_agentrelation_update_callback(&agentrelationinfo);
        }
    }
}

void GaeactorManagerHelper::dealtransformAgentCommSnrDataSlot(const QByteArray&by)
{
    const char * pdata = by.data();
    ::msg::AgentCommSnrInfo::msg_AgentCommSnrInfo agentrelationinfo;
    agentrelationinfo.ParseFromArray(pdata, by.size());
    size_t size = agentrelationinfo.ByteSizeLong();
    int cc = by.size();
    if(by.size() == agentrelationinfo.ByteSizeLong())
    {
//        std::cout <<"snr "<<agentrelationinfo.snr()<<" sinr "<<agentrelationinfo.sinr()<<"\n";
        m_agentcommsnr_update_callback(&agentrelationinfo);
    }
}

void GaeactorManagerHelper::dealtransformAgentCommStckRelustDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    ::msg::CommStackFrameResultElement::msg_CommStackFrameResultElement agentrelationinfo;
    agentrelationinfo.ParseFromArray(pdata, by.size());
    size_t size = agentrelationinfo.ByteSizeLong();
    int cc = by.size();
    if(by.size() == agentrelationinfo.ByteSizeLong())
    {
        //        std::cout <<"snr "<<agentrelationinfo.snr()<<" sinr "<<agentrelationinfo.sinr()<<"\n";
        m_agentcommstack_update_callback(&agentrelationinfo);
    }
}

void GaeactorManagerHelper::dealtransformSimParamsDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    ::msg::SimParamsInfo::msg_SimParamsInfo simparamsinfo;
    simparamsinfo.ParseFromArray(pdata, by.size());
    size_t size = simparamsinfo.ByteSizeLong();
    int cc = by.size();
    if(by.size() == simparamsinfo.ByteSizeLong())
    {
//        std::cout <<"------------------------simparms------------------------"<<"\n";
//        std::cout <<"dt "<<simparamsinfo.step_dt()<<"\n";
//        std::cout <<"interval "<<simparamsinfo.step_interval()<<"\n";
//        std::cout <<"freq "<<simparamsinfo.step_freq()<<"\n";
//        std::cout <<"1s/sim s "<<simparamsinfo.one_second_sim_step_second()<<"\n";
//        std::cout <<"num "<<simparamsinfo.agentinstance_num()<<"\n";
//        std::cout <<"utc_begin "<<simparamsinfo.mutable_simtimestamp()->utc_timestamp_begin()<<"\n";
//        std::cout <<"utc_cur "<<simparamsinfo.mutable_simtimestamp()->utc_timestamp_cur()<<"\n";
//        std::cout <<"physics_timestamp "<<simparamsinfo.mutable_simtimestamp()->physics_timestamp()<<"\n";
//        std::cout <<"sim_timestamp "<<simparamsinfo.mutable_simtimestamp()->sim_timestamp()<<"\n";
    }
}

void GaeactorManagerHelper::dealtransformSmdInfoDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    ::msg::AgentPositionInfo::msg_transdata_smd _msg_transdata_smd;
    _msg_transdata_smd.ParseFromArray(pdata, by.size());
    size_t size = _msg_transdata_smd.ByteSizeLong();
    int cc = by.size();
    if(by.size() == _msg_transdata_smd.ByteSizeLong())
    {
        m_smdinfo_update_callback(&_msg_transdata_smd);
    }
}

void GaeactorManagerHelper::dealtransformPrejudgmentDataSlot(const QByteArray &by)
{
    const char * pdata = by.data();
    ::msg::AgentPositionInfo::msg_transprejusdgmentline _msg_transdata_smd;
    _msg_transdata_smd.ParseFromArray(pdata, by.size());
    size_t size = _msg_transdata_smd.ByteSizeLong();
    int cc = by.size();
    if(by.size() == _msg_transdata_smd.ByteSizeLong())
    {
        m_prejudgment_update_callback(&_msg_transdata_smd);
    }
}
