#ifndef GAEACTOR_INTERFACE_H
#define GAEACTOR_INTERFACE_H

#include "gaeactor_engine_global.h"
#include <stdint.h>
#include <vector>
#include <functional>

namespace gaeactor_engine
{
#pragma pack(1)

typedef struct tagLngLatHgtRef{
    double lng;  ///< longitude in degs
    double lat;  ///< latitude in degs
    double hgt;  ///< height in m
    int ref;
}LNG_LAT_HGT_REF;


typedef struct tagPolygonInfo{
    uint64_t polygon_id;
    uint8_t polygon_usage_type;
    int polygon_slient_time_gap;
}POLYGOININFO;

typedef struct tagEntityInfo{
    uint64_t entity_id;
    double lng;
    double lat;
    double alt;
    int ref;
    double roll;
    double pitch;
    double yaw;
    double speed;
}ENTITY_INFO;


typedef struct tagSensorInfo{
    uint64_t entity_id;
    POLYGOININFO polygon_info;
    std::vector<LNG_LAT_HGT_REF> polygon_lnglat_degs;
}SENSOR_INFO;


typedef struct tagEntitySensorInfo{
    ENTITY_INFO entity_info;
    POLYGOININFO polygon_info;
    std::vector<LNG_LAT_HGT_REF> polygon_lnglat_degs;
}ENTITY_SENSOR_INFO;


enum E_EVENT_TYPE: int32_t
{
    E_EVENT_TYPE_NULL,
    E_EVENT_TYPE_ADD,
    E_EVENT_TYPE_REMOVE,
    E_EVENT_TYPE_UPDATE,
};


typedef struct tageventInfo
{
    E_EVENT_TYPE m_event_mode;
    uint64_t m_sensorid;
    uint64_t m_entityid;
    uint64_t m_polygon_id;
    ENTITY_INFO m_sensorposinfo;
    ENTITY_INFO m_entityposinfo;
    POLYGOININFO m_sensor_polygon_info;
    bool m_entityisSensorProprety;
    double m_distance;
    uint64_t m_timestamp;
}EVENT_INFO;

#pragma pack()



#define AGENT_ENTITY_PROPERTY_NORMAL  (0x00)
#define AGENT_ENTITY_PROPERTY_SENSOR  (0x03)


typedef uint16_t E_ENTITY_PROPERTY;


class GAEACTOR_ENGINE_EXPORT GaeactorInferface
{
public:

    typedef std::function<void (const ENTITY_INFO &)> position_update_callback;
    typedef std::function<void (const SENSOR_INFO &)> sensor_update_callback;
    typedef std::function<void (const std::vector<EVENT_INFO> &)> event_update_callback;
    GaeactorInferface();
    ~GaeactorInferface();

    void updateEntitySensorInfo(const ENTITY_SENSOR_INFO& entity_sensor_info,
                                const bool& bClear = false);


    void updateEntityInfo(const ENTITY_INFO& entityinfo,
                          const E_ENTITY_PROPERTY& entity_type,
                          const bool& bClear = false);

    void updateSensorInfo(const SENSOR_INFO& sensorinfo,
                          const bool& bClear = false);

    void deal_step_refresh_event();

    void set_position_update_callback(position_update_callback _position_update_callback);
    void set_sensor_update_callback(sensor_update_callback _sensor_update_callback);
    void set_event_update_callback(event_update_callback _event_update_callback);

private:
    void receive_callback(const int32_t &channelTransmitDataType, const uint8_t* pdata, const uint32_t& ilen, const uint8_t *pOrignaldata, const uint32_t &iOrignallen);

private:

    bool transmitEntityPosAttData(const ENTITY_INFO& entity_info,
                                  const E_ENTITY_PROPERTY& entiity_type,
                                  const bool& bClear);

    bool transmitSensorData(const uint64_t& entity_id,
                            const POLYGOININFO& polygon_info,
                            const std::vector<LNG_LAT_HGT_REF>& polygon_latlng_degs,
                            const bool& bClear);
private:

    uint64_t m_pos_pack_index = 0;
    uint64_t m_sensor_pack_index = 0;

    position_update_callback m_position_update_callback;
    sensor_update_callback m_sensor_update_callback;
    event_update_callback m_event_update_callback;
};
};

#endif

