#ifndef GAEACTOR_PROCESSOR_H
#define GAEACTOR_PROCESSOR_H
#include "base_define.h"
#include "head_define.h"
#include "gaeactor_environment_tbb_datahelper.h"
#include "internal_transformdata_define.h"
#include "transformdata_define.h"
#include "concurrentqueue.h"
#include <QReadWriteLock>
#include <QMutex>
#include <unordered_map>
#include <optional>
#include "easy/profiler.h"
#include "gaeactor_environment_define.h"


#define USING_CONCURRENT_HASH_MAP

namespace gaeactorenvironment_ex {

enum E_HEXINDEX_STATUS:BYTE
{
    E_HEXINDEX_STATUS_FREE = 0x00,
    E_HEXINDEX_STATUS_ENTITY = 0x01,
    E_HEXINDEX_STATUS_SENSOR = 0x02,
    E_HEXINDEX_STATUS_ALL = E_HEXINDEX_STATUS_ENTITY | E_HEXINDEX_STATUS_SENSOR
};

struct H3CellInfo
{
    UINT8 basecell;
    UINT8 resolution;
    //    UINT32 resolution_basecell;
    //    UINT64 resolution_basecell_digit_origin;
    UINT64 resolution_basecell_digit_valid;
    //    UINT64 digit_origin;
    UINT64 digit_valid;
};

class H3IndexBufferManager;
struct  HexIdexInfo
{
    HexIdexInfo();
    ~HexIdexInfo();
    HexIdexInfo(const HexIdexInfo &other);
    HexIdexInfo(HexIdexInfo &&other);
    HexIdexInfo &operator=(const HexIdexInfo &rhs);

    void init(const H3INDEX& h3, H3CellInfo &&h3cellinfo, H3IndexBufferManager *newPH3IndexBufferManager);

    bool updateData(const ENTITY_KEY &entity_key, const H3INDEX& h3, const FLOAT64 &hgt, const UINT8 &basecell, const UINT8 &resolution, const UINT64& digit_origin, E_DISPLAY_MODE eDdisplayMode, bool bRemove, bool bUpdateIntersect = true);

    bool updateData(const SENSOR_KEY &sensor_key, const transdata_param_seq_hexidx& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove);

    E_HEXINDEX_STATUS getHexidexStatus();
    bool isValid();

    void updateIntersect();
    void updateSensorOverlap(const H3INDEX &_h3Index, const UINT32 &isize);

public:

    SENESOR_LIST_CONCURRENT_HASHMAP sensorlist() const;

    ENTITY_LIST_CONCURRENT_HASHMAP entitylist() const;
private:

    template<typename T,typename U,typename M>
    void update_list(bool bRemove,T &_dstlist, const U& key, const M& val)
    {        
        EASY_FUNCTION(profiler::colors::Green)
        typename  T::accessor  _dstlist_accessor;
        if (bRemove)
        {
            if(_dstlist.find(_dstlist_accessor, key))
            {
                _dstlist.erase(_dstlist_accessor);
            }
        }
        else
        {
            if(_dstlist.find(_dstlist_accessor, key))
            {
                _dstlist_accessor->second = val;
            }
            else
            {
                _dstlist.insert(_dstlist_accessor,key);
                _dstlist_accessor->second = val;
            }
        }
    };


public:    
    H3IndexBufferManager *m_pH3IndexBufferManager;
    H3INDEX m_h3Index;
    H3CellInfo m_h3CellInfo;
    std::atomic_bool m_bValid;
    std::atomic<E_HEXINDEX_STATUS> m_eHexidexStatus;


private:
    SENESOR_LIST_CONCURRENT_HASHMAP m_sensorlist;
    ENTITY_LIST_CONCURRENT_HASHMAP m_entitylist;

};

typedef std::unordered_map<UINT64, HexIdexInfo> CELLBUFFERMAP;
typedef std::unordered_map<UINT64, CELLBUFFERMAP> RESOLUTION_BASECELL_CELLBUFFERMAP;

typedef HexIdexInfo* HEXIDXINFO_PTR;
DECLARE_TYPEDEF_TBB_HASH_MAP(UINT64, HexIdexInfo, CELLBUFFER_CONCURRENT_HASHMAP)
DECLARE_TYPEDEF_TBB_HASH_MAP(UINT64, CELLBUFFER_CONCURRENT_HASHMAP, RESOLUTION_BASECELL_CELLBUFFER_CONCURRENT_HASHMAP)
DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX, UINT64, SENSOR_OVERLAP_CONCURRENT_HASHMAP)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GaeactorProcessor;
class H3IndexBufferManager
{
public:
    H3IndexBufferManager();
    ~H3IndexBufferManager();
    void setGaeactorProcessor(GaeactorProcessor* pGaeactorProcessor);

    H3CellInfo getCellInfo(const H3INDEX& h3);

    std::vector<std::tuple<H3INDEX, bool, uint32_t>> getCellbuffers();
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO>   getCellbuffersInfo();
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> getCellbuffersSensorInfo(bool bAll = true);
    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> getCellbuffersTargetSensorInfo(const HEXIDX_ARRAY& hexidxslist,bool bAll = true);


    void appendIntersect(const H3INDEX& h3, tagConcurrentIntersectInfo&& info);
    INTERSECTINFO_CONCURRENT_HASHMAP getIntersectInfos();
    void removeIntersect(const H3INDEX& h3);
    void removeIntersect_ex(const ENTITY_KEY &entity_key, const H3INDEX& h3);
    void replaceIntersect(const ENTITY_KEY &entity_key, const H3INDEX& oldh3, const H3INDEX& newh3, tagConcurrentIntersectInfo&& info);

    void deal_hexindex_entity(const ENTITY_KEY &entity_key, const H3INDEX& h3, const FLOAT64 &hgt, bool bRemove);

    void deal_hexindex_entity_remove_old_append_new(const ENTITY_KEY &entity_key, const H3INDEX& oldh3, const H3INDEX& newh3, const FLOAT64 &hgt,
                                                    tbb::concurrent_hash_map<UINT8, std::tuple<H3INDEX, H3INDEX>> &_hexidx_array_need_replace);

    void deal_hexindex_sensor(const SENSOR_KEY &sensor_key, const transdata_param_seq_hexidx &h3, bool bRemove);

    void reset();

    void record_sensor_overlap(bool brecord);

    void updateSensorOverlap(const H3INDEX& _h3Index,const UINT32& isize);

    void lockIntersectForWrite();
    void lockIntersectForRead();
    void unlockIntersect();

    void trigger_refresh_event_by_entity_update(const TYPE_ULID& entityid, const tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>& _hexidx_array_need_replace,IDENTIFI_EVENT_INFO& eventinfo);

    void trigger_refresh_event_by_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid,
                                         const transdata_sensorposinfo& _sensorinfo,
                                         const bool& bNeedClear,
                                         const HEXIDX_HGT_ARRAY& remove_hexidxslist,
                                         const HEXIDX_HGT_ARRAY& reserve_hexidxslist,
                                         const HEXIDX_HGT_ARRAY& append_hexidxslist,
                                         IDENTIFI_EVENT_INFO& eventinfo);

public:
    GaeactorProcessor *m_pGaeactorProcessor;
    QReadWriteLock m_IntersectInfoMap_update_mutex;
    INTERSECTINFO_CONCURRENT_HASHMAP m_IntersectInfoMap;
    CELLBUFFER_CONCURRENT_HASHMAP* m_Cellbuffer;
    SENSOR_OVERLAP_CONCURRENT_HASHMAP m_sensor_overlapMap;
    bool m_brecord_sensor_overlap = false;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


struct tagHexindexEntityInfo
{
    H3INDEX m_h3Index;
    bool m_bValid;
    BufferInfoElement<tagEntityPosInfo>* m_pEntitySensorInfoElement;
};

DECLARE_TYPEDEF_TBB_HASH_MAP(TYPE_ULID, tagHexindexEntityInfo, ENTITYINFOS_CONCURRENT_HASHMAP)
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

DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX, HEXIDX_TIME_STAMP_INFO, HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP);
DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX, transdata_param_seq_hexidx_hgt, BUILD_HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class BufferInfoElement
{
public:
    T m_bufferinfo;
    BUFFERINFO_INDEX_TYPE m_info_buffer_index;

    void alloc(const BUFFERINFO_INDEX_TYPE _info_buffer_index, T &&_bufferinfo)
    {
        EASY_FUNCTION(profiler::colors::Green)
        m_bufferinfo = std::move(_bufferinfo);
        m_info_buffer_index = _info_buffer_index;
    }

    void update(T &&_sensorinfo)
    {
        if(isValid())
        {
            m_bufferinfo = std::move(_sensorinfo);
        }
    }

    void release()
    {
        m_info_buffer_index = INVALID_SENSORINFO_INDEX;
    }
    bool isValid()
    {
        return (m_info_buffer_index != INVALID_SENSORINFO_INDEX) ? true : false;
    }
};
class SensorInfoManager;
class tagHexindexlistSensorInfo
{

public:
    tagHexindexlistSensorInfo();
    ~tagHexindexlistSensorInfo();
    void setPSensorInfoManager(SensorInfoManager *newPSensorInfoManager);
    void deal_silent_time_out(HEXIDX_HGT_ARRAY &clearhexidxslist, UINT64 currentTimeStamp);
    void deal_clear_all_hexindexs(HEXIDX_HGT_ARRAY &clearhexidxslist);
    void deal_clear_hexindexs_by_h3list(HEXIDX_HGT_ARRAY &clearhexidxslist, const HEXIDX_HGT_ARRAY& hexidxslist);

    void deal_invalid_hexindexs_by_h3list(HEXIDX_HGT_ARRAY &invalidhexidxslist, const HEXIDX_HGT_ARRAY& hexidxslist);
    void deal_newappend_hexindexs_by_h3list(HEXIDX_HGT_ARRAY &newapendhexidxslist,const HEXIDX_HGT_ARRAY& hexidxslist,const UINT64& currentTimeStamp);
    void append_hexindexinfo(const H3INDEX& h3, const UINT64& currentTimeStamp, const transdata_param_seq_hexidx & _hexidxs_item = transdata_param_seq_hexidx());


    void deal_hexindex_sensor_remove_old_append_new(HEXIDX_HGT_ARRAY &remove_hexidxslist, HEXIDX_HGT_ARRAY &reserve_hexidxslist, HEXIDX_HGT_ARRAY &append_hexidxslist, const HEXIDX_HGT_ARRAY& new_hexidxslist, const UINT64& currentTimeStamp);

private:
    HEXIDX_TIME_STAMP_INFO_CONCURRENT_HASHMAP m_hexIndexslist;

public:
    BufferInfoElement<transdata_sensorposinfo>* m_pSensorInfoElement;
//    POLYGON_LIST m_polygon;
    bool m_bValid;

    UINT32 m_silent_time;
    UINT64 m_lastUpdateTimestamp;
    SensorInfoManager *m_pSensorInfoManager;
};
DECLARE_TYPEDEF_TBB_HASH_MAP(TYPE_ULID, tagHexindexlistSensorInfo, SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class tagHexindexSensorInfo
{
public:
    tagHexindexSensorInfo(H3IndexBufferManager *_pH3IndexBufferManager);
    ~tagHexindexSensorInfo();

    void setPH3IndexBufferManager(H3IndexBufferManager *newPH3IndexBufferManager);
    void append_HexindexlistSensorInfo(const TYPE_ULID &sensingmediaid, tagHexindexlistSensorInfo &&info);

    void clear_data();
    bool clear_hexindex_sensor(const TYPE_ULID &sensingmediaid);
    bool clear_hexindex_sensor_by_hexidexlist(const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist);
    transdata_sensorposinfo gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensingmediaid);

    bool is_contain_sensor(const TYPE_ULID &sensingmediaid);

    bool update_hexindex_sensor(const TYPE_ULID &sensingmediaid,
                                const HEXIDX_HGT_ARRAY  &_hexidxs,
                                transdata_sensorposinfo &&_sensorinfo,
                                POLYGON_LIST &&_polygon,
                                const UINT64& currentTimeStamp,
                                HEXIDX_HGT_ARRAY &remove_hexidxslist,
                                HEXIDX_HGT_ARRAY &reserve_hexidxslist,
                                HEXIDX_HGT_ARRAY &append_hexidxslist);

    void force_cover_hexindex_sensor(const TYPE_ULID &sensingmediaid, BufferInfoElement<transdata_sensorposinfo> *psensorInfoele, const HEXIDX_HGT_ARRAY& invalidhexidxslist);
    void force_cover_deal_hexindex_sensor(const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist);
    void force_cover_update_hexindex_sensor(const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY  &_hexidxs);

    void refresh_silent_timeout(UINT64 currentTimeStamp);
private:
    SENSOR_HEXIDX_INFO_CONCURRENT_HASHMAP m_sensor_hexidx_info_map;
public:
    bool m_bValid;
    TYPE_ULID m_sensorulid;

    H3IndexBufferManager *m_pH3IndexBufferManager;
//    SENSORINFO_INDEX_TYPE m_sensorinfo_index;

};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define SENSOR_INDEX_BUFFER_SIZE    (50000)
template <typename T>
class InfoManager
{
public:
    InfoManager()
        :m_element_index_size(0)
    {

    }
    ~InfoManager()
    {
        reset();
    }
    void reset()
    {
        m_element_index_buffer.clear();
    }

    void init(uint32_t _element_index_size)
    {
        m_element_index_size = _element_index_size;
        m_element_index_buffer.resize(m_element_index_size);
        for(int i = 0; i < m_element_index_size; i++)
        {
            m_buffer_unusing_queue.enqueue(i);
        }
    }

    BufferInfoElement<T> * alloc_info_element(T &&_sensorinfo)
    {
        EASY_FUNCTION(profiler::colors::Green)
        BUFFERINFO_INDEX_TYPE new_buffer_index = get_sensor_index();
        BufferInfoElement<T> * pSensorInfoElement = &this->m_element_index_buffer[new_buffer_index];
        pSensorInfoElement->alloc(new_buffer_index, std::move(_sensorinfo));
        return pSensorInfoElement;
    }

    void release_sensorinfo_element(BufferInfoElement<T> *release_ele)
    {
        if(release_ele && release_ele->isValid())
        {
            m_buffer_unusing_queue.enqueue(release_ele->m_info_buffer_index);
            if(release_ele->m_info_buffer_index >= 0 && release_ele->m_info_buffer_index < this->m_element_index_buffer.size())
            {
                release_ele->release();
            }
        }
    }

private:
    void relloc_sensor_index_buffer()
    {
        EASY_FUNCTION(profiler::colors::Green)
        uint32_t old_size = m_element_index_size;
        uint32_t begin_sensor_index_size = old_size;
        uint32_t end_sensor_index_size = old_size * 2;
        m_element_index_size = old_size * 2;
        for(int i = begin_sensor_index_size;  i < end_sensor_index_size; i++)
        {
            m_buffer_unusing_queue.enqueue(i);
        }
        m_element_index_buffer.resize(m_element_index_size);
    }

    BUFFERINFO_INDEX_TYPE get_sensor_index()
    {
        EASY_FUNCTION(profiler::colors::Green)
        auto qsz = this->m_buffer_unusing_queue.size_approx();
        if (qsz == 0)
        {
            relloc_sensor_index_buffer();
        }
        BUFFERINFO_INDEX_TYPE new_buffer_index;
        this->m_buffer_unusing_queue.try_dequeue(new_buffer_index);
        return new_buffer_index;
    }
private:
    std::vector<BufferInfoElement<T>> m_element_index_buffer;
    moodycamel::ConcurrentQueue<BUFFERINFO_INDEX_TYPE> m_buffer_unusing_queue;
    uint32_t m_element_index_size;
};


class SensorInfoManager
{
public:
    SensorInfoManager();
    ~SensorInfoManager();

    void reset();
    BufferInfoElement<transdata_sensorposinfo> * alloc_sensorinfo_element(transdata_sensorposinfo &&_sensorinfo);
    void release_sensorinfo_element(BufferInfoElement<transdata_sensorposinfo>* release_ele);
private:
    InfoManager<transdata_sensorposinfo> m_sensor_info_buffer_manager;
};



class EntityInfoManager
{
public:
    EntityInfoManager();
    ~EntityInfoManager();

    void reset();
    BufferInfoElement<tagEntityPosInfo> * alloc_entityinfo_element(tagEntityPosInfo &&_entityinfo);
    void release_entityinfo_element(BufferInfoElement<tagEntityPosInfo>* release_ele);
private:
    InfoManager<tagEntityPosInfo> m_entity_info_buffer_manager;
};


DECLARE_TYPEDEF_TBB_HASH_MAP(TYPE_ULID, tagHexindexSensorInfo*, SENSORINFOS_CONCURRENT_HASHMAP)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class  GaeactorProcessor
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    explicit GaeactorProcessor();
    virtual ~GaeactorProcessor();
    void init();
    void cleardata();
    void clearSensorInfo(const TYPE_ULID & sensorulid, const TYPE_ULID &sensingmediaid);
    void clearEntityInfo(const TYPE_ULID & entityid);
    gaeactorenvironment_ex::H3IndexBufferManager& getBuffer();

    void update_hexindex_entity(const TYPE_ULID& uildsrc,
                                const H3INDEX& h3, const FLOAT64 &hgt,
                                const transdata_entityposinfo& _entityinfo,
                                IDENTIFI_EVENT_INFO& eventinfo);

    void update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                const HEXIDX_HGT_ARRAY  &_hexidxs,
                                transdata_sensorposinfo&& _sensorinfo,
                                POLYGON_LIST &&_polygon,
                                IDENTIFI_EVENT_INFO& eventinfo);


    void reset();

    void force_cover_deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslist, const UINT32 &_silent_time);

    void force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                            const HEXIDX_HGT_ARRAY  &_hexidxs,
                                            transdata_sensorposinfo&& _sensorinfo,
                                            POLYGON_LIST &&_polygon);

    void refresh_silent_timeout();

    void refresh_event(IDENTIFI_EVENT_INFO& eventinfo);

    void refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo &eninfo, IDENTIFI_EVENT_INFO& identifi_event_info);
    void refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret, IDENTIFI_EVENT_INFO& eventinfo);



    void trigger_refresh_event_by_entity_update(const TYPE_ULID& entityid,
                                                const tbb::concurrent_hash_map<UINT8,std::tuple<H3INDEX, H3INDEX>>& _hexidx_array_need_replace,
                                                IDENTIFI_EVENT_INFO& eventinfo);

    void trigger_refresh_event_by_sensor(const TYPE_ULID &sensorid,
                                         const TYPE_ULID &sensingmedia_id,
                                         const transdata_sensorposinfo &_sensorinfo,
                                         const bool &bNeedClear,
                                         const HEXIDX_HGT_ARRAY &remove_hexidxslist,
                                         const HEXIDX_HGT_ARRAY &reserve_hexidxslist,
                                         const HEXIDX_HGT_ARRAY &append_hexidxslist,
                                         IDENTIFI_EVENT_INFO &eventinfo);

    transdata_sensorposinfo gettransdata_sensorposinfo_by_sensingmedia(const SENSOR_KEY &sensor_key);
private:
    transdata_entityposinfo gettransdata_entityposinfo(const TYPE_ULID &ulid);
    bool isEntityHaveSensorProperty(const TYPE_ULID &entityulid);

    void refresh_event_type(IDENTIFI_EVENT_INFO& identifi_event_info, const TYPE_ULID &entityid, const TYPE_ULID &sensorid, const TYPE_ULID &sensingmedia_id, int itype);
private:    
    H3IndexBufferManager *m_pH3IndexBufferManager;
    ENTITYINFOS_CONCURRENT_HASHMAP *m_entityInfosMap;

    SENSORINFOS_CONCURRENT_HASHMAP *m_sensorInfosMap;

    QMutex m_exist_events_map_mutex;
    EXIST_EVENTS_CONCURRENT_HASHMAP *m_exist_events_map;


    DECLARE_TYPEDEF_TBB_HASH_MAP(EVENT_KEY_TYPE,bool, EVENTS_VALID_CONCURRENT_HASHMAP);
    DECLARE_TYPEDEF_TBB_HASH_MAP(TYPE_ULID,EVENTS_VALID_CONCURRENT_HASHMAP, ENTITY_EVENTS_CONCURRENT_HASHMAP);
    DECLARE_TYPEDEF_TBB_HASH_MAP(SENSOR_KEY,EVENTS_VALID_CONCURRENT_HASHMAP, SENSOR_EVENTS_CONCURRENT_HASHMAP);
    ENTITY_EVENTS_CONCURRENT_HASHMAP *m_entity_events_map;
    SENSOR_EVENTS_CONCURRENT_HASHMAP *m_sensor_events_map;


    SensorInfoManager *m_SensorInfoManager;

    EntityInfoManager *m_EntityInfoManager;

};
}

#endif // GAEACTOR_PROCESSOR_INTERFACE_H
