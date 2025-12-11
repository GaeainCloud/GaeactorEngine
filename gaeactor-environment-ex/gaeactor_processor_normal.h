#ifndef GAEACTOR_PROCESSOR_NORMAL_INTERFACE_H
#define GAEACTOR_PROCESSOR_NORMAL_INTERFACE_H
#include <QObject>
#include "base_define.h"
#include "head_define.h"
#include "internal_transformdata_define.h"
#include "transformdata_define.h"
#include <QReadWriteLock>
#include <QMutex>
#include <unordered_map>
class QTimer;
namespace gaeactorenvironment_ex_normal {

enum E_HEXINDEX_STATUS:BYTE
{
    E_HEXINDEX_STATUS_FREE = 0x00,
    E_HEXINDEX_STATUS_ENTITY = 0x01,
    E_HEXINDEX_STATUS_SENSOR = 0x02,
    E_HEXINDEX_STATUS_ALL = E_HEXINDEX_STATUS_ENTITY | E_HEXINDEX_STATUS_SENSOR
};
//	Q_DECLARE_FLAGS(E_HEXINDEX_STATUS, E_HEXINDEX_STATUS)
struct H3CellInfo
{
    UINT32 basecell;
    UINT32 resolution;
//    UINT32 resolution_basecell;
//    UINT64 resolution_basecell_digit_origin;
    UINT64 resolution_basecell_digit_valid;
//    UINT64 digit_origin;
    UINT64 digit_valid;
};
class H3IndexBufferManager;
struct HexIdexInfo
{
    HexIdexInfo();
    void init(const H3INDEX& h3, H3CellInfo &&h3cellinfo,H3IndexBufferManager *_pH3IndexBufferManager);

    void updateData(const TYPE_ULID& uildsrc, const H3INDEX& h3, const UINT32& basecell, const UINT32& resolution, const UINT64& digit_origin, E_DISPLAY_MODE eDdisplayMode, bool bRemove);

    void updateData(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove);


    E_HEXINDEX_STATUS getHexidexStatus();
    bool isValid();

    void updateIntersect();
public:
    std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> sensorlist();

    std::unordered_map<TYPE_ULID, bool> entitylist();

public:
    H3IndexBufferManager* m_pH3IndexBufferManager;
    H3INDEX m_h3Index;
    H3CellInfo m_h3CellInfo;
    std::atomic_bool m_bValid;
    E_HEXINDEX_STATUS m_eHexidexStatus;
private:
    QReadWriteLock m_sensorlist_mutex;
    std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> m_sensorlist;
    QReadWriteLock m_entitylist_mutex;
    std::unordered_map<TYPE_ULID, bool> m_entitylist;
};

typedef std::unordered_map<UINT64, HexIdexInfo*> CELLBUFFERMAP;
typedef std::unordered_map<UINT64, CELLBUFFERMAP> RESOLUTION_BASECELL_CELLBUFFERMAP;



struct tagIntersectInfo
{
    std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, bool> sensorlist;
    std::unordered_map<TYPE_ULID, bool> entitylist;
};

typedef std::unordered_map<H3INDEX, tagIntersectInfo> INTERSECTINFOMAP;
class GaeactorProcessor;
class H3IndexBufferManager
{
public:

    H3IndexBufferManager();
    ~H3IndexBufferManager();
    void setGaeactorProcessor(GaeactorProcessor* pGaeactorProcessor);

    H3CellInfo getCellInfo(const H3INDEX& h3);

    std::vector<HexIdexInfo*> getCellBuffersByEntityHex(const H3INDEX& h3);

    HexIdexInfo* getCellBufferByHex(const H3INDEX& h3);

    void lockIntersect();
    void unlockIntersect();

    void appendIntersect(const H3INDEX& h3, tagIntersectInfo&& info);
    void removeIntersect(const H3INDEX& h3);

    CELLBUFFERMAP getCellbuffers();

    std::unordered_map<H3INDEX, tagIntersectInfo> getIntersectInfos();

    std::unordered_map<H3INDEX, tagIntersectInfo> getIntersectInfos_by_cores(const TYPE_ULID &entityid);

    std::unordered_map<H3INDEX, tagIntersectInfo> getIntersectInfos_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid);

    void deal_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove);

    void deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove);
    QReadWriteLock* IntersectInfoMap_mutex();

    QMutex* IntersectInfoMap_update_mutex();

    INTERSECTINFOMAP &IntersectInfoMap();

private:
    GaeactorProcessor* m_pGaeactorProcessor;
    QMutex m_h3CellBuffer_mutex;
    CELLBUFFERMAP m_Cellbuffer;

    QMutex m_IntersectInfoMap_update_mutex;

    QReadWriteLock m_IntersectInfoMap_mutex;
    INTERSECTINFOMAP m_IntersectInfoMap;
};

class GaeactorProcessor : public QObject
{
    Q_OBJECT
public:

    ///////////////////////////////////////////////////////////////////////////////////

    struct tagHexindexEntityInfo
    {
        H3INDEX m_h3Index;
        bool m_bValid;
        transdata_entityposinfo m_entityInfo;
        bool m_isSensor;
    };

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

    typedef std::unordered_map<H3INDEX, HEXIDX_TIME_STAMP_INFO> HEXIDX_TIME_STAMP_INFO_HASHMAP;

    struct tagHexindexlistSensorInfo {
        void deal_silent_time_out(HEXIDX_ARRAY &clearhexidxslist, UINT64 currentTimeStamp);
        void deal_clear_all_hexindexs(HEXIDX_ARRAY &clearhexidxslist);
        void deal_clear_hexindexs_by_h3list(HEXIDX_ARRAY &clearhexidxslist, const HEXIDX_ARRAY& hexidxslist);

        void deal_invalid_hexindexs_by_h3list(HEXIDX_ARRAY &invalidhexidxslist, const HEXIDX_ARRAY& hexidxslist);
        void deal_invalid_hexindexs_by_h3list(HEXIDX_ARRAY &invalidhexidxslist,const HEXIDX_HGT_ARRAY& _hexidxs);
        void deal_newappend_hexindexs_by_h3list(HEXIDX_ARRAY &newapendhexidxslist,const HEXIDX_ARRAY& hexidxslist,const UINT64& currentTimeStamp);
        void deal_newappend_hexindexs_by_h3list(HEXIDX_ARRAY &newapendhexidxslist,const HEXIDX_HGT_ARRAY& _hexidxs,const UINT64& currentTimeStamp);
        void append_hexindexinfo(const H3INDEX& h3, const UINT64& currentTimeStamp, const transdata_param_seq_hexidx & _hexidxs_item = transdata_param_seq_hexidx());

    private:
        QReadWriteLock m_hexIndexslist_mutex;
        HEXIDX_TIME_STAMP_INFO_HASHMAP m_hexIndexslist;
    public:
        transdata_sensorposinfo m_sensorInfo;
        POLYGON_LIST m_polygon;
        bool m_bValid;

        UINT32 m_silent_time;
        UINT64 m_lastUpdateTimestamp;
    };
    typedef std::unordered_map<TYPE_ULID, tagHexindexlistSensorInfo*> SENSOR_HEXIDX_INFO_HASHMAP;

    struct tagHexindexSensorInfo
    {
        tagHexindexSensorInfo(H3IndexBufferManager *_pH3IndexBufferManager);
        ~tagHexindexSensorInfo();
        void append_HexindexlistSensorInfo(const TYPE_ULID &sensingmediaid, tagHexindexlistSensorInfo *info);
        void clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid);
        void clear_hexindex_sensor_by_hexidexlist(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY &hexidxslist);
        transdata_sensorposinfo gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid);

        tagHexindexlistSensorInfo* getHexindexlistSensorInfo(const TYPE_ULID &sensingmediaid);
        void refresh_silent_timeout(const TYPE_ULID &sensorulid, UINT64 currentTimeStamp);
    private:
        QReadWriteLock m_sensor_hexidx_info_map_mutex;
        SENSOR_HEXIDX_INFO_HASHMAP m_sensor_hexidx_info_map;
    public:
        bool m_bValid;
        transdata_sensorposinfo m_sensorinfo;
        H3IndexBufferManager *m_pH3IndexBufferManager;
    };

    ///////////////////////////////////////////////////////////////////////////////////
    explicit GaeactorProcessor(QObject* parent = nullptr);
    virtual ~GaeactorProcessor();
    void cleardata();
    gaeactorenvironment_ex_normal::H3IndexBufferManager& getBuffer();

    void deal_hexindex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const H3INDEX& h3, E_DISPLAY_MODE eDdisplayMode, bool bRemove);

    void clear_hexindex_entity(const TYPE_ULID& uildsrc);
    void clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid);

    void clear_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3);
    void clear_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY& hexidxslist);

    void deal_hexindex_entity(const TYPE_ULID& uildsrc, const H3INDEX& h3);
    void deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY& hexidxslist, const UINT32& _silent_time = 255);

    void update_hexindex_entity(const TYPE_ULID& uildsrc,
                                const H3INDEX& h3,
                                const FLOAT64 &hgt,
                                const transdata_entityposinfo& _entityinfo);

    void update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                const HEXIDX_HGT_ARRAY  &_hexidxs,
                                transdata_sensorposinfo&& _sensorinfo,
                                POLYGON_LIST &&_polygon);


    void force_cover_deal_hexindex_sensor(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY& hexidxslist, const UINT32 &_silent_time);

    void force_cover_update_hexindex_sensor(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,
                                const HEXIDX_HGT_ARRAY  &_hexidxs,
                                transdata_sensorposinfo&& _sensorinfo,
                                POLYGON_LIST &&_polygon);

    void refresh_silent_timeout();

    void refresh_event(IDENTIFI_EVENT_INFO& eventinfo);

    void refresh_events_by_cores(const TYPE_ULID &entityid, const H3INDEX &h3Indexsrc, const FLOAT64 &hgt, const transdata_entityposinfo &eninfo, IDENTIFI_EVENT_INFO& identifi_event_info);
    void refresh_events_by_sensors(const TYPE_ULID &sensorid, const TYPE_ULID &sensingmediaid, const HEXIDX_HGT_ARRAY &hexidxslistret, IDENTIFI_EVENT_INFO& eventinfo);

    transdata_sensorposinfo gettransdata_sensorposinfo_by_sensingmedia(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid);
    transdata_sensorposinfo gettransdata_sensorposinfo(const TYPE_ULID &ulid);
private:
    transdata_entityposinfo gettransdata_entityposinfo(const TYPE_ULID &ulid);
    bool isEntityHaveSensorProperty(const TYPE_ULID &entityulid);

    void force_cover_update_hexindex_sensor_h3list_ex(const TYPE_ULID &sensorulid, const TYPE_ULID &sensingmediaid, const HEXIDX_ARRAY  &hexidxslist, GaeactorProcessor::tagHexindexlistSensorInfo * ptagHexindexlistSensorInfo);
    void force_cover_update_hexindex_sensor_ex(const TYPE_ULID &sensorulid,const TYPE_ULID &sensingmediaid,const HEXIDX_HGT_ARRAY  &_hexidxs,GaeactorProcessor::tagHexindexlistSensorInfo * ptagHexindexlistSensorInfo);
private:
    H3IndexBufferManager m_H3IndexBufferManager;
    QTimer * m_pUpdateTimer;

    QReadWriteLock m_entityInfosMap_mutex;
    std::unordered_map<TYPE_ULID, tagHexindexEntityInfo> m_entityInfosMap;

    QReadWriteLock m_sensorInfosMap_mutex;
    std::unordered_map<TYPE_ULID, tagHexindexSensorInfo*> m_sensorInfosMap;

    QReadWriteLock m_exist_events_map_mutex;
    std::unordered_map<EVENT_KEY_TYPE, QPair<EVENT_INFO,bool>> m_exist_events_map;
};
}

#endif // GAEACTOR_PROCESSOR_INTERFACE_H
