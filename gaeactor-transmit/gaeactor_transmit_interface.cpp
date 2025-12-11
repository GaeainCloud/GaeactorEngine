#include "gaeactor_transmit_interface.h"
#include <QDebug>
//#include "gaeactor_transmit_processor.h"

#include "demservice/DemserviceStandalone.h"
namespace gaeactortransmit
{
GaeactorTransmit::GaeactorTransmit(QObject *parent)
    :QObject(parent)
{
    //m_pGaeactorTransmitProcessor = new GaeactorTransmitProcessor(this);
}

GaeactorTransmit::~GaeactorTransmit()
{
//    if(m_pGaeactorTransmitProcessor)
//    {
//        m_pGaeactorTransmitProcessor->deleteLater();
//    }
}

void GaeactorTransmit::initDeployType(const E_DEPLOYMODE_TYPE &deployType, const std::vector<std::tuple<std::string,std::string>> &servicenamelist)
{
    m_deployType = deployType;
//    if(m_pGaeactorTransmitProcessor)
//    {
//        m_pGaeactorTransmitProcessor->initDeployType(m_deployType,servicenamelist);
//    }
}

QString GaeactorTransmit::transmit_channel_id() const
{
#if 1
    return QString();
#else
    return m_pGaeactorTransmitProcessor->transmit_channel_id();
#endif
}

void GaeactorTransmit::set_transmit_channel_id(const QString &new_transmit_channel_id)
{
    //m_pGaeactorTransmitProcessor->set_transmit_channel_id(new_transmit_channel_id);
}

std::tuple<std::string, CHANNEL_INFO*> GaeactorTransmit::requireIndependentPublisherChannel()
{
    return std::tuple<std::string, CHANNEL_INFO*>();
//    return m_pGaeactorTransmitProcessor->requireIndependentPublisherChannel();
}

std::tuple<std::string, CHANNEL_INFO*>  GaeactorTransmit::getReusePublisherChannel()
{
    return std::tuple<std::string, CHANNEL_INFO*>();
//    return m_pGaeactorTransmitProcessor->getReusePublisherChannel();
}

void GaeactorTransmit::allocShareChannel(UINT32 iCount)
{
    return;
//    return m_pGaeactorTransmitProcessor->allocShareChannel(iCount);
}

CHANNEL_INFO* GaeactorTransmit::applyforShareChannel()
{
    return nullptr;
//    return m_pGaeactorTransmitProcessor->applyforShareChannel();
}

bool GaeactorTransmit::removePublisherUseItem(const std::tuple<std::string, CHANNEL_INFO*> &itemChannelInfo)
{
    return true;
//    return m_pGaeactorTransmitProcessor->removePublisherUseItem(itemChannelInfo);
}


bool GaeactorTransmit::transmitData(const CHANNEL_INFO *channelinfo, const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE *pData, UINT32 iLen)
{
    return true;
//    return m_pGaeactorTransmitProcessor->transmitData(channelinfo,channelTransmitDataType, pData, iLen);
}

void *GaeactorTransmit::loanTransmitBuffer(const CHANNEL_INFO *channelinfo, UINT32 iLen)
{
    return nullptr;
//    return m_pGaeactorTransmitProcessor->loanTransmitBuffer(channelinfo, iLen);
}

void GaeactorTransmit::publish(const CHANNEL_INFO *channelinfo)
{
    return;
//    return m_pGaeactorTransmitProcessor->publish(channelinfo);
}

void GaeactorTransmit::setDataCallback(receive_callback func)
{
    return;
//    return m_pGaeactorTransmitProcessor->setDataCallback(func);
}

void GaeactorTransmit::printMempoolInfo()
{
    return;
//    return m_pGaeactorTransmitProcessor->printMempoolInfo();
}



msg::AgentPositionInfo::msg_transentityhexidxpostdata GaeactorTransmit::SerializePositionData(const transentityhexidxpostdata &_data)
{
    ::msg::AgentPositionInfo::msg_transentityhexidxpostdata _AgentPositionInfo_item;
    _AgentPositionInfo_item.set_uildsrc(QString::number(_data.uildsrc).toStdString());
    _AgentPositionInfo_item.set_uilddst(QString::number(_data.uilddst).toStdString());
    _AgentPositionInfo_item.set_eddisplaymode((::msg::AgentPositionInfo::E_DISPLAY_MODE)(_data.eDdisplayMode));
    *_AgentPositionInfo_item.mutable_entityinfo() = SerializeEntityData(_data.entityinfo);
    return _AgentPositionInfo_item;
}

void GaeactorTransmit::DeserializePositionData(const msg::AgentPositionInfo::msg_transentityhexidxpostdata &_AgentRelationInfo, transentityhexidxpostdata &positiondata)
{
    positiondata.uildsrc = QString::fromStdString(_AgentRelationInfo.uildsrc()).toULongLong();
    positiondata.uilddst = QString::fromStdString(_AgentRelationInfo.uilddst()).toULongLong();
    positiondata.eDdisplayMode = (E_DISPLAY_MODE)_AgentRelationInfo.eddisplaymode();
    DeserializeEntityData(_AgentRelationInfo.entityinfo(),positiondata.entityinfo);
}


::msg::AgentPositionInfo::msg_transentityhexidxdata GaeactorTransmit::SerializeHexidxData(const transentityhexidxdata& _data,const HEXIDX_HGT_ARRAY &hexidxslist,const POLYGON_LIST &polygonlist)
{
    ::msg::AgentPositionInfo::msg_transentityhexidxdata _AgentRelationInfo;
    _AgentRelationInfo.set_uildsrc(QString::number(_data.uildsrc).toStdString());
    _AgentRelationInfo.set_uilddst(QString::number(_data.uilddst).toStdString());
    _AgentRelationInfo.set_eddisplaymode((::msg::AgentPositionInfo::E_DISPLAY_MODE)(_data.eDdisplayMode));
    *_AgentRelationInfo.mutable_sensorinfo() = SerializeSensorData(_data.sensorinfo);


    for(int index = 0; index < hexidxslist.size(); index++)
    {
        ::msg::AgentPositionInfo::msg_transdata_param_seq_hexidx _msg_transdata_param_seq_hexidx;
        _msg_transdata_param_seq_hexidx.set_param_seq_hexidx_element(QString::number(hexidxslist[index].PARAM_seq_hexidx_element).toStdString());

        ::msg::AgentPositionInfo::msg_transdata_param_seq_hexidx_hgt _msg_transdata_param_seq_hexidx_hgt;
        _msg_transdata_param_seq_hexidx_hgt.set_param_seq_hexidx_hgt(hexidxslist[index].PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt);
        _msg_transdata_param_seq_hexidx_hgt.set_param_seq_hexidx_hgt0(hexidxslist[index].PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt0);
        _msg_transdata_param_seq_hexidx_hgt.set_param_seq_hexidx_hgtn(hexidxslist[index].PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgtn);
        _msg_transdata_param_seq_hexidx_hgt.set_param_wave_info_entropy(hexidxslist[index].PARAM_seq_hexidx_hgt.PARAM_wave_info_entropy);
        *_msg_transdata_param_seq_hexidx.mutable_param_seq_hexidx_hgt() = _msg_transdata_param_seq_hexidx_hgt;
        *_AgentRelationInfo.add_hexidxslist() = _msg_transdata_param_seq_hexidx;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    for(int index = 0; index < polygonlist.size(); index++)
    {
        ::msg::AgentPositionInfo::msg_TYPE_POLYGON_PT polygonlist_item;
        polygonlist_item.set_param_longitude(polygonlist[index].PARAM_longitude);
        polygonlist_item.set_param_latitude(polygonlist[index].PARAM_latitude);
        polygonlist_item.set_param_amsl(polygonlist[index].PARAM_amsl);
        //TO DO SET terrain_elevation                            
        {
            double PARAM_longitude = polygonlist[index].PARAM_longitude / LON_LAT_ACCURACY;
            double PARAM_latitude = polygonlist[index].PARAM_latitude / LON_LAT_ACCURACY;
            double terrain_elevation = DemserviceStandalone::getInstance().getElevation(PARAM_longitude, PARAM_latitude);
            polygonlist_item.set_param_terrain_elevation(terrain_elevation);
        }
        polygonlist_item.set_param_ref(polygonlist[index].PARAM_ref);
        *_AgentRelationInfo.add_polygonlist() = polygonlist_item;
    }
    return _AgentRelationInfo;
}

void GaeactorTransmit::DeserializeHexidxData(const ::msg::AgentPositionInfo::msg_transentityhexidxdata& _AgentRelationInfo,transentityhexidxdata &hexidxdata,HEXIDX_HGT_ARRAY &hexidxslist,POLYGON_LIST &polygonlist)
{
    hexidxdata.uildsrc = QString::fromStdString(_AgentRelationInfo.uildsrc()).toULongLong();
    hexidxdata.uilddst = QString::fromStdString(_AgentRelationInfo.uilddst()).toULongLong();
    hexidxdata.eDdisplayMode = (E_DISPLAY_MODE)_AgentRelationInfo.eddisplaymode();
    DeserializeSensorData(_AgentRelationInfo.sensorinfo(), hexidxdata.sensorinfo);
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


::msg::AgentPositionInfo::msg_transdata_entityposinfo GaeactorTransmit::SerializeEntityData(const transdata_entityposinfo& _entityinfo)
{
    ::msg::AgentPositionInfo::msg_transdata_entityposinfo _msg_transdata_entityposinfo;
    _msg_transdata_entityposinfo.set_param_pos_pack_index(QString::number(_entityinfo.PARAM_pos_pack_index).toStdString());
    _msg_transdata_entityposinfo.set_param_timestamp(QString::number(_entityinfo.PARAM_timestamp).toStdString());
    _msg_transdata_entityposinfo.set_param_sim_timestamp(QString::number(_entityinfo.PARAM_sim_timestamp).toStdString());
    _msg_transdata_entityposinfo.set_param_sensor_property(_entityinfo.PARAM_sensor_property);
    _msg_transdata_entityposinfo.set_param_pos_hexidx(QString::number(_entityinfo.PARAM_pos_hexidx).toStdString());
    _msg_transdata_entityposinfo.set_param_longitude(_entityinfo.PARAM_longitude);
    _msg_transdata_entityposinfo.set_param_latitude(_entityinfo.PARAM_latitude);
    _msg_transdata_entityposinfo.set_param_amsl(_entityinfo.PARAM_amsl);
    //TO DO SET terrain_elevation                            
    {
        double PARAM_longitude = _entityinfo.PARAM_longitude / LON_LAT_ACCURACY;
        double PARAM_latitude = _entityinfo.PARAM_latitude / LON_LAT_ACCURACY;
        double terrain_elevation = DemserviceStandalone::getInstance().getElevation(PARAM_longitude, PARAM_latitude);
        _msg_transdata_entityposinfo.set_param_terrain_elevation(terrain_elevation);
    }
    _msg_transdata_entityposinfo.set_param_ref(_entityinfo.PARAM_ref);
    _msg_transdata_entityposinfo.set_param_roll(_entityinfo.PARAM_roll);
    _msg_transdata_entityposinfo.set_param_pitch(_entityinfo.PARAM_pitch);
    _msg_transdata_entityposinfo.set_param_yaw(_entityinfo.PARAM_yaw);
    _msg_transdata_entityposinfo.set_param_speed(_entityinfo.PARAM_speed);
    int32_t attached_value = _entityinfo.PARAM_attached ? 1 : 0;
    _msg_transdata_entityposinfo.set_param_attached(attached_value);
    _msg_transdata_entityposinfo.set_param_attached_agentid(QString::number(_entityinfo.PARAM_attached_parent_agentid).toStdString());
    _msg_transdata_entityposinfo.set_param_reserved1(_entityinfo.PARAM_reserved[0]);
    _msg_transdata_entityposinfo.set_param_reserved2(_entityinfo.PARAM_reserved[1]);
    _msg_transdata_entityposinfo.set_param_reserved3(_entityinfo.PARAM_reserved[2]);
    _msg_transdata_entityposinfo.set_param_reserved4(_entityinfo.PARAM_reserved[3]);
    _msg_transdata_entityposinfo.set_param_reserved5(_entityinfo.PARAM_reserved[4]);
    _msg_transdata_entityposinfo.set_param_reserved6(_entityinfo.PARAM_reserved[5]);
    return _msg_transdata_entityposinfo;
}

void GaeactorTransmit::DeserializeEntityData(const ::msg::AgentPositionInfo::msg_transdata_entityposinfo& _entityinfo, transdata_entityposinfo &entityinfo)
{
    entityinfo.PARAM_pos_pack_index = QString::fromStdString(_entityinfo.param_pos_pack_index()).toULongLong();
    entityinfo.PARAM_timestamp = QString::fromStdString(_entityinfo.param_timestamp()).toULongLong();
    entityinfo.PARAM_sim_timestamp = QString::fromStdString(_entityinfo.param_sim_timestamp()).toULongLong();
    entityinfo.PARAM_sensor_property = _entityinfo.param_sensor_property();
    entityinfo.PARAM_pos_hexidx = QString::fromStdString(_entityinfo.param_pos_hexidx()).toULongLong();
    entityinfo.PARAM_longitude = _entityinfo.param_longitude();
    entityinfo.PARAM_latitude = _entityinfo.param_latitude();
    entityinfo.PARAM_amsl = _entityinfo.param_amsl();
    entityinfo.PARAM_ref = _entityinfo.param_ref();
    entityinfo.PARAM_roll = _entityinfo.param_roll();
    entityinfo.PARAM_pitch = _entityinfo.param_pitch();
    entityinfo.PARAM_yaw = _entityinfo.param_yaw();
    entityinfo.PARAM_speed = _entityinfo.param_speed();
    entityinfo.PARAM_attached = _entityinfo.param_attached() == 1?true:false;
    entityinfo.PARAM_attached_parent_agentid = QString::fromStdString(_entityinfo.param_attached_agentid()).toULongLong();
    entityinfo.PARAM_reserved[0] = _entityinfo.param_reserved1();
    entityinfo.PARAM_reserved[1] = _entityinfo.param_reserved2();
    entityinfo.PARAM_reserved[2] = _entityinfo.param_reserved3();
    entityinfo.PARAM_reserved[3] = _entityinfo.param_reserved4();
    entityinfo.PARAM_reserved[4] = _entityinfo.param_reserved5();
    entityinfo.PARAM_reserved[5] = _entityinfo.param_reserved6();
}


::msg::AgentPositionInfo::msg_transdata_sensorposinfo GaeactorTransmit::SerializeSensorData(const transdata_sensorposinfo& _sensorinfo)
{
    ::msg::AgentPositionInfo::msg_transdata_sensorposinfo _msg_transdata_sensorposinfo;
    _msg_transdata_sensorposinfo.set_param_sensor_pack_index(QString::number(_sensorinfo.PARAM_sensor_pack_index).toStdString());
    _msg_transdata_sensorposinfo.set_param_timestamp(QString::number(_sensorinfo.PARAM_timestamp).toStdString());
    _msg_transdata_sensorposinfo.set_param_sim_timestamp(QString::number(_sensorinfo.PARAM_sim_timestamp).toStdString());
    _msg_transdata_sensorposinfo.set_param_source_sensingmediaid(QString::number(_sensorinfo.PARAM_source_sensingmediaid).toStdString());
    _msg_transdata_sensorposinfo.set_param_elecmag_wave_properties(_sensorinfo.PARAM_elecmag_wave_properties);
    _msg_transdata_sensorposinfo.set_param_sound_wave_properties(_sensorinfo.PARAM_sound_wave_properties);
    _msg_transdata_sensorposinfo.set_param_wave_freq_mean_sgnfcnt(_sensorinfo.PARAM_wave_freq_mean_sgnfcnt);
    _msg_transdata_sensorposinfo.set_param_wave_freq_std_dvtn_sgnfcnt(_sensorinfo.PARAM_wave_freq_std_dvtn_sgnfcnt);
    _msg_transdata_sensorposinfo.set_param_wave_freq_distbtn(_sensorinfo.PARAM_wave_freq_distbtn);
    _msg_transdata_sensorposinfo.set_param_wave_frequency_scale(_sensorinfo.PARAM_wave_frequency_scale);
    _msg_transdata_sensorposinfo.set_param_wave_usage(_sensorinfo.PARAM_wave_usage);
    _msg_transdata_sensorposinfo.set_param_wave_snd_rcv(_sensorinfo.PARAM_wave_snd_rcv);
    _msg_transdata_sensorposinfo.set_param_wave_silent_time_gap(_sensorinfo.PARAM_wave_silent_time_gap);
    _msg_transdata_sensorposinfo.set_param_wave_field_media_id(_sensorinfo.PARAM_wave_field_media_id);
    _msg_transdata_sensorposinfo.set_param_wave_modsig_id(QString::number(_sensorinfo.PARAM_wave_modsig_id).toStdString());
    _msg_transdata_sensorposinfo.set_param_wave_power_in_watts(_sensorinfo.PARAM_wave_power_in_watts);
    _msg_transdata_sensorposinfo.set_param_wave_propagation_model(_sensorinfo.PARAM_wave_propagation_model);
    _msg_transdata_sensorposinfo.set_param_wave_direction_pitch(_sensorinfo.PARAM_wave_direction_pitch);
    _msg_transdata_sensorposinfo.set_param_wave_direction_azimuth(_sensorinfo.PARAM_wave_direction_azimuth);
    _msg_transdata_sensorposinfo.set_param_wave_gaingraph(_sensorinfo.PARAM_wave_gaingraph);
    _msg_transdata_sensorposinfo.set_param_wave_direction_radius(_sensorinfo.PARAM_wave_direction_radius);
    return _msg_transdata_sensorposinfo;
}

void GaeactorTransmit::DeserializeSensorData(const ::msg::AgentPositionInfo::msg_transdata_sensorposinfo& _sensorinfo, transdata_sensorposinfo &sensorinfo)
{
    sensorinfo.PARAM_sensor_pack_index = QString::fromStdString(_sensorinfo.param_sensor_pack_index()).toULongLong();
    sensorinfo.PARAM_timestamp = QString::fromStdString(_sensorinfo.param_timestamp()).toULongLong();
    sensorinfo.PARAM_sim_timestamp = QString::fromStdString(_sensorinfo.param_sim_timestamp()).toULongLong();
    sensorinfo.PARAM_source_sensingmediaid = QString::fromStdString(_sensorinfo.param_source_sensingmediaid()).toULongLong();
    sensorinfo.PARAM_elecmag_wave_properties = _sensorinfo.param_elecmag_wave_properties();
    sensorinfo.PARAM_sound_wave_properties = _sensorinfo.param_sound_wave_properties();
    sensorinfo.PARAM_wave_freq_mean_sgnfcnt = _sensorinfo.param_wave_freq_mean_sgnfcnt();
    sensorinfo.PARAM_wave_freq_std_dvtn_sgnfcnt = _sensorinfo.param_wave_freq_std_dvtn_sgnfcnt();
    sensorinfo.PARAM_wave_freq_distbtn = _sensorinfo.param_wave_freq_distbtn();
    sensorinfo.PARAM_wave_frequency_scale = _sensorinfo.param_wave_frequency_scale();
    sensorinfo.PARAM_wave_usage = _sensorinfo.param_wave_usage();
    sensorinfo.PARAM_wave_snd_rcv = _sensorinfo.param_wave_snd_rcv();
    sensorinfo.PARAM_wave_silent_time_gap = _sensorinfo.param_wave_silent_time_gap();
    sensorinfo.PARAM_wave_field_media_id = _sensorinfo.param_wave_field_media_id();
    sensorinfo.PARAM_wave_modsig_id = QString::fromStdString(_sensorinfo.param_wave_modsig_id()).toULongLong();
    sensorinfo.PARAM_wave_power_in_watts = _sensorinfo.param_wave_power_in_watts();
    sensorinfo.PARAM_wave_propagation_model = _sensorinfo.param_wave_propagation_model();
    sensorinfo.PARAM_wave_direction_pitch = _sensorinfo.param_wave_direction_pitch();
    sensorinfo.PARAM_wave_direction_azimuth = _sensorinfo.param_wave_direction_azimuth();
    sensorinfo.PARAM_wave_gaingraph = _sensorinfo.param_wave_gaingraph();
    sensorinfo.PARAM_wave_direction_radius = _sensorinfo.param_wave_direction_radius();
}

::msg::AgentPositionInfo::msg_transeventdata GaeactorTransmit::SerializeEventData(const transeventdata &_data)
{
    ::msg::AgentPositionInfo::msg_transeventdata _msg_transeventdata_item;
    _msg_transeventdata_item.set_eventtype((::msg::AgentPositionInfo::E_EVENT_MODE)_data.eventType);
    ::msg::AgentPositionInfo::msg_transdata_eventInfo _msg_transdata_eventInfo_item;

    _msg_transdata_eventInfo_item.set_m_sensorid(QString::number(_data.eventifo.m_sensorid).toStdString());
    _msg_transdata_eventInfo_item.set_m_entityid(QString::number(_data.eventifo.m_entityid).toStdString());
    _msg_transdata_eventInfo_item.set_m_sensingmediaid(QString::number(_data.eventifo.m_sensingmediaid).toStdString());
    *_msg_transdata_eventInfo_item.mutable_m_sensorposinfo() = SerializeEntityData(_data.eventifo.m_sensorposinfo);
    *_msg_transdata_eventInfo_item.mutable_m_entityposinfo() = SerializeEntityData(_data.eventifo.m_entityposinfo);
    *_msg_transdata_eventInfo_item.mutable_m_sensorproprety() = SerializeSensorData(_data.eventifo.m_sensorproprety);
    _msg_transdata_eventInfo_item.set_m_entityissensorproprety(_data.eventifo.m_entityisSensorProprety);
    _msg_transdata_eventInfo_item.set_m_distance(_data.eventifo.m_distance);
    *_msg_transdata_eventInfo_item.mutable_m_entityproprety() = SerializeSensorData(_data.eventifo.m_entityproprety);
    _msg_transdata_eventInfo_item.set_m_timestamp(QString::number(_data.eventifo.m_timestamp).toStdString());

    *_msg_transeventdata_item.mutable_eventifo()=_msg_transdata_eventInfo_item;
    return _msg_transeventdata_item;
}

void GaeactorTransmit::DeserializeEventData(const ::msg::AgentPositionInfo::msg_transeventdata &event, EVENT_INFO &eventifo)
{
    eventifo.m_sensorid = QString::fromStdString(event.eventifo().m_sensorid()).toULongLong();
    eventifo.m_entityid = QString::fromStdString(event.eventifo().m_entityid()).toULongLong();
    eventifo.m_sensingmediaid = QString::fromStdString(event.eventifo().m_sensingmediaid()).toULongLong();
    gaeactortransmit::GaeactorTransmit::DeserializeEntityData(event.eventifo().m_sensorposinfo(), eventifo.m_sensorposinfo);
    gaeactortransmit::GaeactorTransmit::DeserializeEntityData(event.eventifo().m_entityposinfo(), eventifo.m_entityposinfo);
    gaeactortransmit::GaeactorTransmit::DeserializeSensorData(event.eventifo().m_sensorproprety(), eventifo.m_sensorproprety);
    eventifo.m_entityisSensorProprety = event.eventifo().m_entityissensorproprety();
    eventifo.m_distance = event.eventifo().m_distance();
    gaeactortransmit::GaeactorTransmit::DeserializeSensorData(event.eventifo().m_entityproprety(), eventifo.m_entityproprety);
    eventifo.m_timestamp = QString::fromStdString(event.eventifo().m_timestamp()).toULongLong();
}

}
