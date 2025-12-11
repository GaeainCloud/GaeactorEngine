#include "gaeactorInterface.h"

#include <QDateTime>

#include "LocationHelper.h"
#include "gaeactor_transmit_interface.h"

#include "gaeactortransmitmanager.h"

namespace gaeactor_engine
{

GaeactorInferface::GaeactorInferface()
    :m_position_update_callback(nullptr),
    m_sensor_update_callback(nullptr),
    m_event_update_callback(nullptr)
{
    GaeactorTransmitManager::getInstance().setReceive_callback(std::bind(&GaeactorInferface::receive_callback,
                                                                         this,
                                                                         std::placeholders::_1,
                                                                         std::placeholders::_2,
                                                                         std::placeholders::_3,
                                                                         std::placeholders::_4,
                                                                         std::placeholders::_5));

}

GaeactorInferface::~GaeactorInferface()
{

}

void GaeactorInferface::updateEntitySensorInfo(const ENTITY_SENSOR_INFO& entity_sensor_info,
                                               const bool& bClear)
{
    E_ENTITY_PROPERTY entity_type = entity_sensor_info.polygon_lnglat_degs.empty() ? AGENT_ENTITY_PROPERTY_NORMAL : AGENT_ENTITY_PROPERTY_SENSOR;

    transmitEntityPosAttData(entity_sensor_info.entity_info, entity_type, bClear);

    transmitSensorData(entity_sensor_info.entity_info.entity_id, entity_sensor_info.polygon_info, entity_sensor_info.polygon_lnglat_degs, bClear);
}


void GaeactorInferface::updateEntityInfo(const ENTITY_INFO& entityinfo,
                                         const E_ENTITY_PROPERTY& entity_type,
                                         const bool& bClear)
{
    transmitEntityPosAttData(entityinfo, entity_type, bClear);
}


void GaeactorInferface::GaeactorInferface::updateSensorInfo(const SENSOR_INFO& sensorinfo,
                                                            const bool& bClear)
{
    transmitSensorData(sensorinfo.entity_id, sensorinfo.polygon_info, sensorinfo.polygon_lnglat_degs, bClear);
}


void GaeactorInferface::deal_step_refresh_event()
{
    GaeactorTransmitManager::getInstance().deal_step_refresh_event();
}

void GaeactorInferface::set_position_update_callback(position_update_callback _position_update_callback)
{
    m_position_update_callback = std::move(_position_update_callback);
}

void GaeactorInferface::set_sensor_update_callback(sensor_update_callback _sensor_update_callback)
{
    m_sensor_update_callback = std::move(_sensor_update_callback);
}

void GaeactorInferface::set_event_update_callback(event_update_callback _event_update_callback)
{
    m_event_update_callback = std::move(_event_update_callback);
}

void GaeactorInferface::receive_callback(const int32_t &channelTransmitDataType, const uint8_t *pdata, const uint32_t &ilen, const uint8_t *pOrignaldata, const uint32_t &iOrignallen)
{
    switch (channelTransmitDataType) {
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_ATT_HEXIDX_ARRAY:
    {
        if(m_position_update_callback)
        {            

            ::msg::AgentPositionInfo::msg_transentityhexidxpostdata_array _AgentRelationInfo_array;
            _AgentRelationInfo_array.ParseFromArray(pdata, ilen);
            size_t size = _AgentRelationInfo_array.ByteSizeLong();

            //std::cout <<"protobuf size "<<size<<" len "<<cc<<"\n";
            if (ilen == _AgentRelationInfo_array.ByteSizeLong())
            {
                for (int index = 0; index < _AgentRelationInfo_array.transentityhexidxpostdata_size(); index++)
                {
                    auto& _AgentRelationInfo = _AgentRelationInfo_array.transentityhexidxpostdata().Get(index);

                    transentityhexidxpostdata transdata;
                    gaeactortransmit::GaeactorTransmit::DeserializePositionData(_AgentRelationInfo,transdata);
                    ENTITY_INFO _ENTITY_INFO{transdata.uildsrc,
                                             (double)transdata.entityinfo.PARAM_latitude / LON_LAT_ACCURACY,
                                             (double)transdata.entityinfo.PARAM_latitude / LON_LAT_ACCURACY,
                                             (double)transdata.entityinfo.PARAM_amsl / 1000.0,
                                             transdata.entityinfo.PARAM_ref,
                                             transdata.entityinfo.PARAM_roll,
                                             transdata.entityinfo.PARAM_pitch,
                                             transdata.entityinfo.PARAM_yaw,
                                             transdata.entityinfo.PARAM_speed/1000.0,
                                             };

                    m_position_update_callback(_ENTITY_INFO);
                }
            }
        }
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_HEXIDX:
    {
        if(m_sensor_update_callback)
        {
            ::msg::AgentPositionInfo::msg_transentityhexidxdata _AgentRelationInfo;
            _AgentRelationInfo.ParseFromArray(pdata, ilen);
            if (ilen == _AgentRelationInfo.ByteSizeLong())
            {
                HEXIDX_HGT_ARRAY hexidxslist;
                POLYGON_LIST polygonlist;
                transentityhexidxdata transdata;
                gaeactortransmit::GaeactorTransmit::DeserializeHexidxData(_AgentRelationInfo, transdata, hexidxslist, polygonlist);
                SENSOR_INFO _SENSOR_INFO;
                _SENSOR_INFO.entity_id = transdata.uildsrc;
                _SENSOR_INFO.polygon_info.polygon_id = transdata.uilddst;
                _SENSOR_INFO.polygon_info.polygon_usage_type = transdata.sensorinfo.PARAM_wave_usage;
                _SENSOR_INFO.polygon_info.polygon_slient_time_gap = transdata.sensorinfo.PARAM_wave_silent_time_gap;
                _SENSOR_INFO.polygon_lnglat_degs.resize(polygonlist.size());
                for( int i = 0; i < polygonlist.size(); i++)
                {
                    _SENSOR_INFO.polygon_lnglat_degs[i].lat = (double)polygonlist[i].PARAM_latitude / LON_LAT_ACCURACY;
                    _SENSOR_INFO.polygon_lnglat_degs[i].lng = (double)polygonlist[i].PARAM_longitude / LON_LAT_ACCURACY;
                    _SENSOR_INFO.polygon_lnglat_degs[i].hgt = (double)polygonlist[i].PARAM_amsl / 1000.0;
                    _SENSOR_INFO.polygon_lnglat_degs[i].ref = (double)polygonlist[i].PARAM_ref;
                }
                m_sensor_update_callback(_SENSOR_INFO);
            }
        }
    }
    break;
    case E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_EVENT_ARRAY:
    {
        if(m_event_update_callback)
        {
            std::vector<EVENT_INFO> events;
            uint32_t num = ilen / sizeof(transeventlistdatasimple);
            events.resize(num);
            ::msg::AgentPositionInfo::msg_transeventlistdatasimple_array _AgentPositionInfo_array;
            transeventlistdatasimple* ptransdatasrc = (transeventlistdatasimple*)(pdata);
            for (int index = 0; index < num; index++)
            {
                transeventlistdatasimple* ptransdata = (ptransdatasrc + index);
                if(ptransdata)
                {
                    EVENT_INFO _EVENT_INFO;

                    _EVENT_INFO.m_event_mode = (E_EVENT_TYPE)ptransdata->eventType;
                    _EVENT_INFO.m_sensorid = ptransdata->event.eventifo.m_sensorid;
                    _EVENT_INFO.m_entityid = ptransdata->event.eventifo.m_entityid;
                    _EVENT_INFO.m_polygon_id = ptransdata->event.eventifo.m_sensingmediaid;
                    _EVENT_INFO.m_sensorposinfo = ENTITY_INFO{ptransdata->event.eventifo.m_sensorid,
                                                              (double)ptransdata->event.eventifo.m_sensorposinfo.PARAM_longitude / LON_LAT_ACCURACY,
                                                              (double)ptransdata->event.eventifo.m_sensorposinfo.PARAM_latitude / LON_LAT_ACCURACY,
                                                              (double)ptransdata->event.eventifo.m_sensorposinfo.PARAM_amsl / 1000.0,
                                                              ptransdata->event.eventifo.m_sensorposinfo.PARAM_ref,
                                                              ptransdata->event.eventifo.m_sensorposinfo.PARAM_roll,
                                                              ptransdata->event.eventifo.m_sensorposinfo.PARAM_pitch,
                                                              ptransdata->event.eventifo.m_sensorposinfo.PARAM_yaw,
                                                              ptransdata->event.eventifo.m_sensorposinfo.PARAM_speed/1000.0};
                    _EVENT_INFO.m_entityposinfo = ENTITY_INFO{ptransdata->event.eventifo.m_entityid,
                                                              (double)ptransdata->event.eventifo.m_entityposinfo.PARAM_longitude / LON_LAT_ACCURACY,
                                                              (double)ptransdata->event.eventifo.m_entityposinfo.PARAM_latitude / LON_LAT_ACCURACY,
                                                              (double)ptransdata->event.eventifo.m_entityposinfo.PARAM_amsl / 1000.0,
                                                              ptransdata->event.eventifo.m_entityposinfo.PARAM_ref,
                                                              ptransdata->event.eventifo.m_entityposinfo.PARAM_roll,
                                                              ptransdata->event.eventifo.m_entityposinfo.PARAM_pitch,
                                                              ptransdata->event.eventifo.m_entityposinfo.PARAM_yaw,
                                                              ptransdata->event.eventifo.m_entityposinfo.PARAM_speed/1000.0};

                    _EVENT_INFO.m_sensor_polygon_info.polygon_id =ptransdata->event.eventifo.m_sensingmediaid;
                    _EVENT_INFO.m_sensor_polygon_info.polygon_usage_type = ptransdata->event.eventifo.m_sensorproprety.PARAM_wave_usage;
                    _EVENT_INFO.m_sensor_polygon_info.polygon_slient_time_gap = ptransdata->event.eventifo.m_sensorproprety.PARAM_wave_silent_time_gap;


                    _EVENT_INFO.m_entityisSensorProprety = ptransdata->event.eventifo.m_entityisSensorProprety;
                    _EVENT_INFO.m_distance = ptransdata->event.eventifo.m_distance;
                    _EVENT_INFO.m_timestamp = ptransdata->event.eventifo.m_timestamp;

                    events[index] = std::move(_EVENT_INFO);
                }
            }
            m_event_update_callback(events);
        }
    }
    break;
    default:
        break;
    }
}




bool GaeactorInferface::transmitEntityPosAttData(const ENTITY_INFO& entity_info,
                                                 const E_ENTITY_PROPERTY& entiity_type,
                                                 const bool& bClear)
{
    size_t isize = sizeof(transdata_posatt_hexidx);

    size_t i_malloc_size = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + isize;
    void * usrpayload = (void*)malloc(i_malloc_size);
    memset(usrpayload,0,isize);

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_POSATT;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_posatt_hexidx *pData = reinterpret_cast<transdata_posatt_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);

        pData->PARAM_protocol_head.PARAM_source_ulid = entity_info.entity_id;
        H3INDEX h3index = 0;
        if(!bClear)
        {
            LocationHelper::getIndexInfo(h3index, entity_info.lat, entity_info.lng, INDEX_MAPPING_RESOLUTION_ENTITY_POS);
        }
#ifdef TRANS_INDEX_TIMESTAMP
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_pack_index = m_pos_pack_index++;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_timestamp = QDateTime::currentMSecsSinceEpoch();
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sim_timestamp = 0;
#endif
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_sensor_property = entiity_type;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pos_hexidx = h3index;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_latitude = entity_info.lat*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_longitude = entity_info.lng*LON_LAT_ACCURACY;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_amsl = entity_info.alt*1000;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_ref = entity_info.ref;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_roll = entity_info.roll;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_pitch = entity_info.pitch;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_yaw = entity_info.yaw;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_speed = entity_info.speed*1000;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_attached = false;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_attached_parent_agentid = 0;

        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[0] = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[1] = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[2] = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[3] = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[4] = 0;
        pData->PARAM_payload_pos_hexidx.m_entityposinfo.PARAM_reserved[5] = 0;

        {
            const BYTE *pData = nullptr;
            const BYTE *pOriginData = (const BYTE *)usrpayload;
            pData = pOriginData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + isize;
            GaeactorTransmitManager::getInstance().data_receive_callback(channelTransmitDataType, pData, isize, pOriginData, userPayloadSize);
            free(usrpayload);
        }

        return true;
    }
    return false;
}

double degreeToRadian(double degree)
{
    return degree * M_PI / 180.0;
}

double radianToDegree(double radian)
{
    return radian * 180.0 / M_PI;
}


bool GaeactorInferface::transmitSensorData(const TYPE_ULID& entity_id,
                                           const POLYGOININFO& polygon_info,
                                           const std::vector<LNG_LAT_HGT_REF>& polygon_latlng_degs,
                                           const bool& bClear)
{
    std::vector<transdata_param_seq_polygon> _polygon;
    HEXIDX_ARRAY _h3indexlist;
    if(!bClear)
    {
        std::vector<LatLng> _polygon_latlng_rads;
        _polygon.resize(polygon_latlng_degs.size());
        _polygon_latlng_rads.resize(polygon_latlng_degs.size());
        for(int index = 0; index < polygon_latlng_degs.size(); index++)
        {
            _polygon_latlng_rads[index].lat = degreeToRadian(polygon_latlng_degs[index].lat);
            _polygon_latlng_rads[index].lng = degreeToRadian(polygon_latlng_degs[index].lng);

            transdata_param_seq_polygon polygonitem{(LON_LAT_TYPE)(polygon_latlng_degs[index].lng * LON_LAT_ACCURACY),
                                                    (LON_LAT_TYPE)(polygon_latlng_degs[index].lat * LON_LAT_ACCURACY),
                                                    (FLOAT64)(polygon_latlng_degs[index].hgt) * 1000,
                                                    polygon_latlng_degs[index].ref};
            _polygon[index] = std::move(polygonitem);
        }

        if(!_polygon_latlng_rads.empty())
        {
#ifdef USING_TARGET_RESOLUTION
            LocationHelper::getPolygonResulutionIndex(_h3indexlist,data, INDEX_MAPPING_RESOLUTION_SENSINGMEDIA);
#else
            int res = LocationHelper::getPolygonRes(_polygon_latlng_rads);
            LocationHelper::getPolygonResulutionIndex(_h3indexlist, _polygon_latlng_rads, res);
#endif
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    size_t ibufsize =  sizeof(UINT16) + sizeof(transdata_param_seq_hexidx) * _h3indexlist.size() + sizeof(UINT16) + sizeof(transdata_param_seq_polygon) * _polygon.size();
    size_t isize = sizeof(transdata_wave_smd_hexidx) + ibufsize;

    size_t i_malloc_size = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + isize;
    void * usrpayload = (void*)malloc(i_malloc_size);
    memset(usrpayload,0,isize);

    if(usrpayload)
    {
        E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = E_CHANNEL_TRANSMITDATA_TYPE_ENTITY_SENSOR;
        BYTE *pDstData = reinterpret_cast<BYTE*>(usrpayload);
        memcpy(pDstData, &channelTransmitDataType, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
        memcpy(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), &isize, sizeof(UINT32));
        transdata_wave_smd_hexidx *pData = reinterpret_cast<transdata_wave_smd_hexidx *>(pDstData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32));
        memset(pData,0,isize);
        pData->PARAM_protocol_head.PARAM_source_ulid = entity_id;
#ifdef TRANS_INDEX_TIMESTAMP
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_sensor_pack_index = m_sensor_pack_index++;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_timestamp = QDateTime::currentMSecsSinceEpoch();
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_sim_timestamp = 0;
#endif
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_source_sensingmediaid = polygon_info.polygon_id;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_silent_time_gap = polygon_info.polygon_slient_time_gap;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_field_media_id = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_modsig_id = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_usage = polygon_info.polygon_usage_type;
        pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_buffer_count = ibufsize;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_direction_pitch = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_direction_azimuth = 0;


        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_elecmag_wave_properties = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_sound_wave_properties = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_freq_mean_sgnfcnt = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_freq_std_dvtn_sgnfcnt = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_freq_distbtn = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_frequency_scale = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_power_in_watts = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_propagation_model = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_gaingraph = 0;
        pData->PARAM_payload_wave_smd_hexidx.m_sensorinfo.PARAM_wave_snd_rcv = 0;

        //std::cout << " usage "<<sensingmediaid<<" "<<hexidxslist.size()<<" "<<_polygon.size()<<" "<<(int)usage_type<<std::endl;

        if(ibufsize > 0)
        {
            BYTE *p_byte_buffer = (BYTE*)(pData->PARAM_payload_wave_smd_hexidx.PARAM_payload_buffer.PARAM_seq_buffer);
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            transdata_payload_hexidx *ptransdata_payload_hexidx = GET_HEXIDX_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_hexidx->PARAM_qty_hexidx = _h3indexlist.size();
            if(!_h3indexlist.empty())
            {
                for(int i = 0; i < _h3indexlist.size();i++)
                {
                    ptransdata_payload_hexidx->PARAM_seq_hexidx[i].PARAM_seq_hexidx_element = _h3indexlist.at(i);
                }
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            transdata_payload_polygon *ptransdata_payload_polygon = GET_POLYGON_STRUCT_PTR(p_byte_buffer);
            ptransdata_payload_polygon->PARAM_qty_polygon = _polygon.size();
            if(!_polygon.empty())
            {
                memcpy(ptransdata_payload_polygon->PARAM_seq_polygon, _polygon.data(), sizeof(transdata_param_seq_polygon) * _polygon.size());
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
        {
            const BYTE *pData = nullptr;
            const BYTE *pOriginData = (const BYTE *)usrpayload;
            pData = pOriginData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + isize;
            GaeactorTransmitManager::getInstance().data_receive_callback(channelTransmitDataType, pData, isize, pOriginData, userPayloadSize);
            free(usrpayload);
        }
        return true;
    }
    return false;
}
};

