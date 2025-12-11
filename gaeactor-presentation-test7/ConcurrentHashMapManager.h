#ifndef CONCURRENTHASHMAPMANAGER_H
#define CONCURRENTHASHMAPMANAGER_H
#include "gaeactor_environment_tbb_datahelper.h"

#include "params_define.h"
#include <QColor>
#include "./httpserver/httpserver/dto/AgentDto.hpp"
#include <optional>
#include <QObject>
#include "src/OriginalMutex.h"
#include <QMutex>
#include <QReadWriteLock>

#define USING_GENERATE_GEOPOINTS_CONCURRENT

#define USING_GENERATE_SHOWRESULT_INDEPENDENT

#define USING_SHOW_RESULT


#ifndef USING_GUI_SHOW
#undef USING_SHOW_RESULT
#endif


struct tagLineInfoEx;
struct tagTaskResultPtInfo{
    UINT64 _pt_id;
    LatLng _latlng;
    H3INDEX _h3Index;
    tbb::concurrent_hash_map<UINT32, std::tuple<tagLineInfoEx*, oatpp::Object<GeoJson_points_linestring_id_Dto>>> m_ptconflictlines;


    void appendline(const UINT32& ptindex, tagLineInfoEx*& lines, oatpp::Object<GeoJson_points_linestring_id_Dto> && dto)
    {
        tbb::concurrent_hash_map<UINT32, std::tuple<tagLineInfoEx*, oatpp::Object<GeoJson_points_linestring_id_Dto>>>::accessor _accessor;
        if(!m_ptconflictlines.find(_accessor, ptindex))
        {
            m_ptconflictlines.insert(_accessor, ptindex);
            auto val = std::make_tuple(lines,std::move(dto));
            _accessor->second = std::move(val);
        }
    }

    const std::tuple<tagLineInfoEx*, oatpp::Object<GeoJson_points_linestring_id_Dto>>* getlineInfo(const UINT32& ptindex)
    {
        tbb::concurrent_hash_map<UINT32, std::tuple<tagLineInfoEx*, oatpp::Object<GeoJson_points_linestring_id_Dto>>>::const_accessor _accessor;
        if(m_ptconflictlines.find(_accessor, ptindex))
        {
            return &_accessor->second;
        }
        return nullptr;
    }
};

struct tagLineInfoEx{
    tagLineInfo m_line;
    tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo*> m_conflictpts;
    void appendTaskPt(const UINT32& ptindex, tagTaskResultPtInfo*& pt)
    {
        tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo*>::accessor _accessor;
        if(!m_conflictpts.find(_accessor, ptindex))
        {
            m_conflictpts.insert(_accessor, ptindex);
            _accessor->second = std::move(pt);
        }
    }
};



DECLARE_TYPEDEF_TBB_HASH_MAP(UINT64, oatpp::Object<GeoJson_features_point_Dto>, GEOJSON_POINTS_CONCURRENT_HASHMAP)
typedef std::tuple<HEXIDX_HGT_ARRAY,std::vector<QColor>> HEXIDX_HGT_ARRAY_COLORS;
DECLARE_TYPEDEF_TBB_HASH_MAP(UINT64, HEXIDX_HGT_ARRAY_COLORS, HEXIDX_HGT_ARRAY_COLORS_CONCURRENT_HASHMAP)
typedef std::tuple<transdata_param_seq_hexidx, QColor> HEXIDX_HGT_COLOR;
DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX, HEXIDX_HGT_COLOR, HEXIDX_HGT_COLOR_CONCURRENT_HASHMAP)

DECLARE_TYPEDEF_TBB_HASH_MAP(EVENT_KEY_TYPE, tagLineInfoEx, LINES_CONCURRENT_HASHMAP)

struct tagTaskInfo{
    UINT64 _task_id;
    INT32 _resolution;
    LINES_CONCURRENT_HASHMAP m_lines;
    tbb::concurrent_hash_map<std::string, tagLineInfoEx*> m_codeline;
    tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo> m_taskresultpts;

    void clear()
    {
        _task_id = 0;
        _resolution = 0;
        m_lines.clear();
        m_taskresultpts.clear();
    }

    void appendpt(const UINT32& ptindex, LatLng& location,H3INDEX &_h3Index)
    {
        tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo>::accessor _accessor;
        if(!m_taskresultpts.find(_accessor, ptindex))
        {
            m_taskresultpts.insert(_accessor, ptindex);
            tagTaskResultPtInfo _tagTaskResultInfo;
            _tagTaskResultInfo._pt_id = ptindex;
            _tagTaskResultInfo._latlng = location;
            _accessor->second = std::move(_tagTaskResultInfo);
        }
    }

    tagTaskResultPtInfo* getPtInfo(const UINT32& ptindex)
    {
        tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo>::accessor _accessor;
        if(m_taskresultpts.find(_accessor, ptindex))
        {
            return &_accessor->second;
        }
        return nullptr;
    }
};



DECLARE_TYPEDEF_TBB_HASH_MAP(UINT64, tagTaskInfo, TASK_LINES_CONCURRENT_HASHMAP)


namespace oatpp {
namespace parser {
namespace json {
namespace mapping {

class ObjectMapper;
}
}
}
}

namespace stdutils {
class OriThread;
}

enum E_HEXINDEX_STATUS:BYTE
{
    E_HEXINDEX_STATUS_FREE = 0x00,
    E_HEXINDEX_STATUS_ENTITY = 0x01,
    E_HEXINDEX_STATUS_SENSOR = 0x02,
    E_HEXINDEX_STATUS_ALL = E_HEXINDEX_STATUS_ENTITY | E_HEXINDEX_STATUS_SENSOR
};


struct tagLineItem
{
//    INT32 id;
//    UINT64 agentid;
//    UINT64 routeid;
//    std::string code;
//    std::string startTime;
//    std::string endTime;
//    std::string createTime;
//    std::string Priority;
//    UINT64 startTime_dt;
//    UINT64 endTime_dt;
//    UINT64 createTime_dt;
//    UINT32 iPriority;

//    UINT64 tasksignature;

//    std::list<UINT64> conflictpoints;
};

template<class T>
class SqlSaveInstance
{
public:
    SqlSaveInstance()
        {

        };
    ~SqlSaveInstance()
        {
        };
    void append_data(T && _data)
    {
        stdutils::OriMutexLocker locker(&m_data_mutex);
        m_data.push_back(std::move(_data));
    };
    stdutils::OriMutexLock m_data_mutex;
    std::list<T> m_data;

};
class ConcurrentHashMapManager;
struct tagSqlExecItem
{
    std::string sql;
    ConcurrentHashMapManager * m_pDataBaseSaveManager;
    std::list<std::unordered_map<std::string,std::string>> ret_val;
};
class ConcurrentHashMapManager: public QObject
{
    Q_OBJECT
public:
    ConcurrentHashMapManager(QObject *parent=nullptr);
    virtual ~ConcurrentHashMapManager();

    void reset_data();

    void reset_processor(GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance);

    tagLineInfoEx & get_data_or_create(tagTaskInfo &_tagTaskInfo, const EVENT_KEY_TYPE& key, const std::string &code);



    tagTaskInfo& get_task_or_create(UINT64 task_id);

    tagTaskInfo* get_task(UINT64 task_id);

    bool remove_task(UINT64 task_id);

    tagLineInfoEx * get_data(const tagTaskInfo &_tagTaskInfo, const EVENT_KEY_TYPE& key);

    tagLineInfoEx * get_data_by_code(tagTaskInfo &_tagTaskInfo, const std::string &code);

    void appenditem_hex_color(const H3INDEX& h3Index, const transdata_param_seq_hexidx& hgt, const QColor& cl);

    void appenditem(const TYPE_ULID& uildsrc,const H3INDEX& h3Index, const transdata_param_seq_hexidx_hgt& hgt, const QColor& cl);


    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePath(const oatpp::Object<GeoJson_linestring_Dto>& dto);
    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathEx(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto);

    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathAccumulate(const oatpp::Object<GeoJson_linestring_Dto>& dto);
    oatpp::Object<GeoJson_Return_Dto> inputCheckRoutePathExAccumulate(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto);
    oatpp::Object<GeoJson_linestring_opearte_Dto> inputCheckRoutePathSettings(const oatpp::Object<GeoJson_linestring_opearte_Dto> &dto);

    oatpp::Object<GeoJson_Return_Dto> getLineConflictsInfo(const oatpp::Object<GeoJson_query_line_id_Dto> &dto);
    oatpp::Object<GeoJson_Return_Dto> getConflictsPtsInfo(const oatpp::Object<GeoJson_query_pt_id_Dto> &dto);
    oatpp::Object<GeoJson_clear_pt_id_Dto> clearConflictsPtsInfo(const oatpp::Object<GeoJson_clear_pt_id_Dto> &dto);
    oatpp::Object<GeoJson_tasks_Dto> getTaskInfo();
    oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> LatLngToHex(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto);
    oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> HexToLatLng(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto);

#ifdef USING_GUI_SHOW
    oatpp::Object<GeoJson_Return_Dto> inputDisplayCheckRoutePathResult(const oatpp::Object<GeoJson_Return_Dto> &dto);
#endif

    void prepare_deal_2d(const tagTaskInfo &_taskinfo,const std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>> & hexinfo);

    void prepare_deal_3d(const tagTaskInfo &_taskinfo, const std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>> & hexinfo);

    static int generateRandomNumber();


    static double calc_dist(const double &lat1, const double &lon1, const double &lat2, const double &lon2);
    static double toRadians(double degree);
    static QString json_object_to_string(QJsonObject qjo, bool compact = true);

    static QJsonObject string_to_json_object(QString in);

    static QByteArray generate_random_64bit_id();

    static quint64 generate_random_positive_uint64();


    static bool has_random_positive_uint64(const quint64& random_id);

    static void init_random_positive_uint64(uint64_t random_id);



    void show_building();

signals:
    void draw_linestring_sig(tagLineInfo jsobj);
    void deal_result_sig(const QString &jsobj,void * pGaeactorProcessorInterfaceInstance);
private slots:
    void deal_result_slot(const QString &jsobj,void * ptr);
private:
    void outputCheckRoutePathRoutesResult(tagTaskInfo &_all_lines,
                                          oatpp::Object<GeoJson_linestring_Dto>& routes);
    UINT32 outputCheckRoutePathPointsResult(tagTaskInfo &_all_lines,
                                          oatpp::Object<GeoJson_point_Dto>& points,
                                          GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance);

    UINT32 deal_linestring(const UINT32& cur_daty_dt_startTime,
                           const UINT32& cur_daty_dt_endTime,
                           tagTaskInfo &_taskinfo,
                           const UINT32 &lineindex,
                           QJsonObject &featuresitemobj,
                           GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance,
                           const double& _heightlimitup,
                           const double& _heightlimitdown);

    UINT32 deal_polygon(const UINT32& cur_daty_dt_startTime,
                        const UINT32& cur_daty_dt_endTime,
                        tagTaskInfo &_taskinfo,
                        const UINT32 &lineindex,
                        QJsonObject &featuresitemobj,
                        GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance,
                        const double& _heightlimitup,
                        const double& _heightlimitdown);

    void deal_outputCheckRoutePathResult(tagTaskInfo &_all_lines,
                                         UINT64 beginTimestamp,
                                         oatpp::Object<GeoJson_Return_Dto> &_GeoJson_Return_Dto,
                                         GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance);

    bool isRouteDeal(const UINT32 &cur_daty_dt_startTime, const UINT32 &cur_daty_dt_endTime, const tagLinePros &_tagLinePros);

    void resetAccumulate();

    void resetDisplay();

    void save_ooda_to_db(tagLineItem&& _tagOODAItem);
    void sqlite_mutex_lock();
    void sqlite_mutex_unlock();

    void loadShp();
    bool readPolygonShp(const std::string &filename, std::vector<QJsonObject>  &featuresjsarray);

    void analysisBuildPolygon(tagTaskInfo &_all_lines);

    UINT32 append_building(const bool& bconflictbuilding,
                           const INT32& _resolution,
                           tagTaskInfo &_all_lines,
                           GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance);
public:
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> m_jsonmapper;

    std::atomic_bool m_bechoroutes = false;
    std::atomic_bool m_bchecktime = false;
    std::atomic_bool m_bshowhex3d = false;
    std::atomic_bool m_breturndirect = false;

    std::atomic_uint32_t m_bresolution = 6;
    std::atomic<double> m_heightlimitup = 50.0;
    std::atomic<double> m_heightlimitdown = 50.0;


//    std::atomic_bool m_bchecktime = true;
    HEXIDX_HGT_COLOR_CONCURRENT_HASHMAP m_hexidxslisttmp_concurrent_hash_map;
    HEXIDX_HGT_ARRAY_COLORS_CONCURRENT_HASHMAP m_hexidxslisttmp3d_concurrent_hash_map;


    TASK_LINES_CONCURRENT_HASHMAP m_task_all_lines;
    GAEAPROCESSORINTERFACEINSTANCE_PTR m_pGaeactorProcessorInterfaceInstanceAccumulate =  nullptr;

private:
    void data_deal_savedb_thread_func(void *pParam);

    static int callback(void *NotUsed, int argc, char **argv, char **azColName);

    void exec_item(tagSqlExecItem &sqlitem);


    void sql_exec(tagSqlExecItem &sqlitem);

    void deal_sql_result(const tagSqlExecItem *psqlitem, const std::unordered_map<std::string,std::string>& val);

private:
    stdutils::OriThread* m_hSqlite_data_DealThread;
public:
    stdutils::OriMutexLock m_deal_data_mutex;
    stdutils::OriWaitCondition m_deal_data_fullCond;

    SqlSaveInstance<tagLineItem> m_psaveinstance;
    QMutex *m_sqlite_mutex;


    static QReadWriteLock random_uint64_contents_mutex;
    static QHash<quint64, bool> random_uint64_contents_;


    std::vector<QJsonObject> m_buildingfeaturesjsarray;
    std::unordered_map<INT32,tagTaskInfo> m_RES_buildings;

};
#endif // CONCURRENTHASHMAPMANAGER_H
