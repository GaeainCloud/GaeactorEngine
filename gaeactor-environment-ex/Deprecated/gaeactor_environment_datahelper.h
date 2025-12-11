#ifndef GAEACTOR_ENVIRONMENT_DATAHELPER_H
#define GAEACTOR_ENVIRONMENT_DATAHELPER_H

#include "head_define.h"
#include <QObject>
#include <QMutex>
#include "transformdata_define.h"
#include <QReadWriteLock>

typedef struct tagSensorEntityInterceveInfo
{
    //    enum E_INTERCEVE_STATUS:INT32
    //    {
    //        E_INTERCEVE_STATUS_UNKOWN,
    //        E_INTERCEVE_STATUS_ADD,
    //        E_INTERCEVE_STATUS_UPDATE,
    //        E_INTERCEVE_STATUS_REMOVING,
    //        E_INTERCEVE_STATUS_REMOVED
    //    };

    tagSensorEntityInterceveInfo()
    {
        //        m_interceve_status.store(E_INTERCEVE_STATUS_UNKOWN);
        m_resolutionhexidx = 0;
        m_srchexidx = 0;
        setInterceve_status(false);
    }

    tagSensorEntityInterceveInfo(H3INDEX _resolutionhexidx,
                                 H3INDEX _srchexidx,
                                 bool _bInterceveValid)
    {
        //        m_interceve_status.store(E_INTERCEVE_STATUS_UNKOWN);
        m_resolutionhexidx = _resolutionhexidx;
        m_srchexidx = _srchexidx;
        setInterceve_status(_bInterceveValid);
    }

    tagSensorEntityInterceveInfo(const tagSensorEntityInterceveInfo &other)
    {
        //        m_interceve_status.store(other.m_interceve_status.load());
        m_resolutionhexidx = other.m_resolutionhexidx;
        m_srchexidx = other.m_srchexidx;
        m_bInterceveValid = other.m_bInterceveValid;
    }

    inline tagSensorEntityInterceveInfo& operator = (const tagSensorEntityInterceveInfo &other)
    {
        if(this == &other)
            return *this;
        //        m_interceve_status.store(other.m_interceve_status.load());
        m_resolutionhexidx = other.m_resolutionhexidx;
        m_srchexidx = other.m_srchexidx;
        m_bInterceveValid = other.m_bInterceveValid;
        return *this;
    }

    inline tagSensorEntityInterceveInfo& operator = (tagSensorEntityInterceveInfo &&other)
    {
        if(this == &other)
            return *this;
        //        m_interceve_status.store(other.m_interceve_status.load());
        m_resolutionhexidx = other.m_resolutionhexidx;
        m_srchexidx = other.m_srchexidx;
        m_bInterceveValid = other.m_bInterceveValid;
        return *this;
    }

    void setInterceve_status(bool bStatus)
    {
        m_bInterceveValid = bStatus;
    }

    //    bool isAddStatus()
    //    {
    //        bool bret = false;
    //        if(m_interceve_status.load() == E_INTERCEVE_STATUS_ADD)
    //        {
    //            m_interceve_status.store(E_INTERCEVE_STATUS_UPDATE);
    //            bret = true;
    //        }
    //        return bret;
    //    }

    //    bool isUpdateStatus()
    //    {
    //        return m_interceve_status.load() == E_INTERCEVE_STATUS_UPDATE;
    //    }

    //    bool isRemoving()
    //    {
    //        bool bret = false;
    //        if(m_interceve_status.load() == E_INTERCEVE_STATUS_REMOVING)
    //        {
    //            m_interceve_status.store(E_INTERCEVE_STATUS_REMOVED);
    //            bret = true;
    //        }
    //        return bret;
    //    }
public:
    H3INDEX m_resolutionhexidx;
    H3INDEX m_srchexidx;
    bool m_bInterceveValid;

    //    std::atomic<E_INTERCEVE_STATUS> m_interceve_status;
}SENSOR_ENTITY_INTERCEVE_INFO;

typedef std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>,SENSOR_ENTITY_INTERCEVE_INFO> SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP;

typedef struct tagHexidxTimeStampInfo
{
    tagHexidxTimeStampInfo()
    {
        m_timestamp = 0;
        m_bHexidxValid = true;
    }

    tagHexidxTimeStampInfo(UINT64 _timestamp,
                           bool _bHexidxValid,
                           const transdata_param_seq_hexidx &_HexidxInfo)
    {
        m_timestamp = _timestamp;
        m_bHexidxValid = _bHexidxValid;
        m_HexidxInfo = _HexidxInfo;
    }
public:
    UINT64 m_timestamp;
    bool m_bHexidxValid;
    transdata_param_seq_hexidx m_HexidxInfo;
}HEXIDX_TIME_STAMP_INFO;

typedef std::unordered_map<H3INDEX,HEXIDX_TIME_STAMP_INFO> HEXIDX_TIME_STAMP_INFO_HASHMAP;

typedef std::unordered_map<TYPE_ULID,HEXIDX_TIME_STAMP_INFO_HASHMAP> SENSOR_HEXIDX_INFO_HASHMAP;

typedef struct tagSensorProperty
{
    tagSensorProperty()
    {
        m_silent_time = 0;
        m_bClose = false;
        m_lastUpdateTimestamp = 0;
        m_eWaveSensorSourceType = E_WAVE_SENSOR_SOURCE_TYPE_NULL;
        m_bInterceveInfoValid = true;
    }

    tagSensorProperty(UINT32 _silent_time,
                      bool _bSilence,
                      UINT64 _timestamp,
                      E_WAVE_SENSOR_SOURCE_TYPE _eWaveSensorSourceType,
                      bool bInterceveInfoValid)
    {
        m_silent_time = _silent_time;
        m_bClose = _bSilence;
        m_lastUpdateTimestamp = _timestamp;
        m_eWaveSensorSourceType = _eWaveSensorSourceType;
        m_bInterceveInfoValid = bInterceveInfoValid;
    }

public:
    UINT32 m_silent_time;
    bool m_bClose;
    UINT64 m_lastUpdateTimestamp;
    E_WAVE_SENSOR_SOURCE_TYPE m_eWaveSensorSourceType;
    bool m_bInterceveInfoValid;
}SENSOR_PROPERTY;

struct tagSensorInfo
{
    tagSensorInfo()
    {
        m_bValid = false;
    }
    tagSensorInfo(bool _bValid,
                  const transdata_sensorposinfo& _sensorinfo, const std::vector<transdata_param_seq_polygon> &_polygon)
    {
        m_bValid = _bValid;
        m_sensorinfo = _sensorinfo;
        m_polygons = _polygon;
    }
    bool m_bValid;
    transdata_sensorposinfo m_sensorinfo;
    std::vector<transdata_param_seq_polygon> m_polygons;

};

typedef struct tagSensorInterceveEntityInfo
{

    tagSensorInterceveEntityInfo(SENSOR_HEXIDX_INFO_HASHMAP& _sensor_hexidx_info_map,
                                 SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP& _sensor_entity_interceve_info_hashmap,
                                 SENSOR_PROPERTY& _sensor_property,
                                 const TYPE_ULID &sensingmediaid,
                                 tagSensorInfo & _sensorinfo)
    {
        m_sensor_hexidx_info_map = std::move(_sensor_hexidx_info_map);
        m_sensor_entity_interceve_info_hashmap = std::move(_sensor_entity_interceve_info_hashmap);
        m_sensor_property  = std::move(_sensor_property);
        m_sensor_infos.insert(std::make_pair(sensingmediaid,std::move(_sensorinfo)));
        m_sensor_hexidx_info_map_bchange =false;
    }

    tagSensorInterceveEntityInfo(const tagSensorInterceveEntityInfo &other)
    {
        m_sensor_hexidx_info_map = other.m_sensor_hexidx_info_map;
        m_sensor_entity_interceve_info_hashmap = other.m_sensor_entity_interceve_info_hashmap;
        m_sensor_property  = other.m_sensor_property;
        m_sensor_infos  = other.m_sensor_infos;
        m_sensor_hexidx_info_map_bchange  = other.m_sensor_hexidx_info_map_bchange;
    }

    inline tagSensorInterceveEntityInfo& operator = (const tagSensorInterceveEntityInfo &other)
    {
        if(this == &other)
            return *this;
        m_sensor_hexidx_info_map = other.m_sensor_hexidx_info_map;
        m_sensor_entity_interceve_info_hashmap = other.m_sensor_entity_interceve_info_hashmap;
        m_sensor_property  = other.m_sensor_property;
        m_sensor_infos  = other.m_sensor_infos;
        m_sensor_hexidx_info_map_bchange  = other.m_sensor_hexidx_info_map_bchange;
        return *this;
    }

    inline tagSensorInterceveEntityInfo& operator = (tagSensorInterceveEntityInfo &&other)
    {
        if(this == &other)
            return *this;
        m_sensor_hexidx_info_map = other.m_sensor_hexidx_info_map;
        m_sensor_entity_interceve_info_hashmap = other.m_sensor_entity_interceve_info_hashmap;
        m_sensor_property  = other.m_sensor_property;
        m_sensor_infos  = other.m_sensor_infos;
        m_sensor_hexidx_info_map_bchange  = other.m_sensor_hexidx_info_map_bchange;
        return *this;
    }

    SENSOR_PROPERTY m_sensor_property;

    std::unordered_map<TYPE_ULID,tagSensorInfo> m_sensor_infos;

    bool m_sensor_hexidx_info_map_bchange;
    SENSOR_HEXIDX_INFO_HASHMAP& get_sensor_hexidx_info_map_lock_ForRead()
    {
        m_sensor_hexidx_info_map_mutex.lockForRead();
        return m_sensor_hexidx_info_map;
    }

    SENSOR_HEXIDX_INFO_HASHMAP& get_sensor_hexidx_info_map_lock_ForWrite()
    {
        m_sensor_hexidx_info_map_mutex.lockForWrite();
        return m_sensor_hexidx_info_map;
    }

    SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP& get_sensor_entity_interceve_info_hashmap_lock_ForRead()
    {
        m_sensor_entity_interceve_info_hashmap_mutex.lockForRead();
        return m_sensor_entity_interceve_info_hashmap;
    }

    SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP& get_sensor_entity_interceve_info_hashmap_lock_ForWrite()
    {
        m_sensor_entity_interceve_info_hashmap_mutex.lockForWrite();
        return m_sensor_entity_interceve_info_hashmap;
    }

    void set_sensor_hexidx_info_map_unlock()
    {
        m_sensor_hexidx_info_map_mutex.unlock();
    }

    void set_sensor_entity_interceve_info_hashmap_unlock()
    {
        m_sensor_entity_interceve_info_hashmap_mutex.unlock();
    }

private:
    QReadWriteLock m_sensor_hexidx_info_map_mutex;
    QReadWriteLock m_sensor_entity_interceve_info_hashmap_mutex;

    SENSOR_HEXIDX_INFO_HASHMAP m_sensor_hexidx_info_map;
    SENSOR_ENTITY_INTERCEVE_INFO_HASHMAP m_sensor_entity_interceve_info_hashmap;

}SENSOR_INTERCEVE_ENTITY_INFO;

typedef std::unordered_map<TYPE_ULID, SENSOR_INTERCEVE_ENTITY_INFO> SENSORS_INTERCEVE_HASHMAP;

typedef struct tagEntityHexidxInfo
{
    H3INDEX m_h3Index;
    transdata_entityposinfo m_entityinfo;
    bool m_isSensor;
    bool m_bEntityValid;
    tagEntityHexidxInfo()
    {
        m_h3Index = 0;
        m_isSensor = false;
        m_bEntityValid  = false;
    }

    tagEntityHexidxInfo(H3INDEX _h3Index,
                        transdata_entityposinfo& entityinfo,
                        bool _isSensor,
                        bool _bHexidxValid)
    {
        m_h3Index = _h3Index;
        m_entityinfo = entityinfo;
        m_isSensor = _isSensor;
        m_bEntityValid  = _bHexidxValid;
    }
}ENTITY_HEXIDX_INFO;

//typedef std::tuple<H3INDEX, bool,bool> ENTITY_HEXIDX_INFO;
typedef std::unordered_map<TYPE_ULID, ENTITY_HEXIDX_INFO> ENTITY_HEXIDX_HASHMAP;


namespace gaeactorenvironment {
class  GaeactorEnvironmentDatahelper : public QObject
{
    Q_OBJECT
public:
    explicit GaeactorEnvironmentDatahelper(QObject *parent = nullptr);
    virtual ~GaeactorEnvironmentDatahelper();

};
}
#endif // GAEACTOR_ENVIRONMENT_DATAHELPER_H
