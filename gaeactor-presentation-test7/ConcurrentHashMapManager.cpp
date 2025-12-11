#include "ConcurrentHashMapManager.h"

#include <QCoreApplication>

#include <QJsonArray>
#ifdef USING_GUI_SHOW
#endif
#include "settingsconfig.h"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include <random>
#include "gaeactor_processor_interface.h"
#include "gaeactor_processor_interface_instance.h"

#include "LocationHelper.h"
#include "loghelper.h"
#include "easy/profiler.h"

#include "src/OriginalDateTime.h"
#include <sstream>
#include <sqlite_orm/sqlite_orm.h>
#include <sqlite3.h>
#include <src/OriginalThread.h>
#include <QJsonDocument>

//inline auto init_Storage(const std::string& path) {
//    using namespace sqlite_orm;
//    auto storage_agentpos_instance = make_storage(
//        path
//        );

//    storage_agentpos_instance.sync_schema();
//    return storage_agentpos_instance;
//}

#define USING_INPUT_CONCURRENT

//#define USING_SHOW_DETAIL
#ifndef USING_GUI_SHOW
#undef USING_SHOW_DETAIL
#endif

#define USING_DEAL_INPUT_JSARRAY_CONCURRENT

#define USING_GAEACTOR_NEW

#define CHECK_IS_OUT_RANGE(RANGE_START, RANGE_END, TARGET_START, TARGET_END) ((((TARGET_START) < (RANGE_START)) && ((TARGET_END) < (RANGE_START))) || (((TARGET_START) > (RANGE_END)) && ((TARGET_END) > (RANGE_END))))

#include <iostream>
#include <string>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"




ConcurrentHashMapManager::ConcurrentHashMapManager(QObject *parent)
    :QObject(parent)
    ,m_pGaeactorProcessorInterfaceInstanceAccumulate(nullptr)
    ,m_sqlite_mutex(new QMutex(QMutex::Recursive))
{
#ifndef USING_GAEACTOR_NEW
    gaeactorenvironment_ex::GaeactorProcessorInterface::getInstance().record_sensor_overlap(true);
#endif
    qRegisterMetaType<tagLineInfo>("tagLineInfo");
    qRegisterMetaType<UINT64>("UINT64");
    m_jsonmapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    m_jsonmapper->getSerializer()->getConfig()->useBeautifier = false;
    resetAccumulate();

    connect(this, &ConcurrentHashMapManager::deal_result_sig, this, &ConcurrentHashMapManager::deal_result_slot);

    int nPriority = 0;
#ifdef _MSC_VER
    nPriority = THREAD_PRIORITY_NORMAL;
#else
    nPriority = 2;
#endif

    m_hSqlite_data_DealThread = new stdutils::OriThread(std::bind(&ConcurrentHashMapManager::data_deal_savedb_thread_func,this,std::placeholders::_1),\
                                                        nullptr,\
                                                        nPriority);
    m_hSqlite_data_DealThread->start();



    loadShp();

    auto append_building_resolution =[&](int _resolution)
    {
        tagTaskInfo _all_lines;
        _all_lines._task_id = _resolution;
        _all_lines._resolution = _resolution;

        analysisBuildPolygon(_all_lines);
        m_RES_buildings.insert(std::make_pair(_resolution, std::move(_all_lines)));
    };

    append_building_resolution(6);
    append_building_resolution(7);
    append_building_resolution(8);
    append_building_resolution(9);
    append_building_resolution(10);
    append_building_resolution(11);
    append_building_resolution(12);
    append_building_resolution(13);
    append_building_resolution(14);
    append_building_resolution(15);
}

ConcurrentHashMapManager::~ConcurrentHashMapManager()
{
    if(m_pGaeactorProcessorInterfaceInstanceAccumulate)
    {
        delete m_pGaeactorProcessorInterfaceInstanceAccumulate;
    }
    if(m_hSqlite_data_DealThread)
    {
        delete m_hSqlite_data_DealThread;
    }
    if(m_sqlite_mutex)
    {
        delete m_sqlite_mutex;
    }
}

void ConcurrentHashMapManager::reset_data()
{
    m_hexidxslisttmp_concurrent_hash_map.clear();
    m_hexidxslisttmp3d_concurrent_hash_map.clear();
}

void ConcurrentHashMapManager::reset_processor(GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance)
{
    if(_pGaeactorProcessorInterfaceInstance)
    {
#ifndef USING_GAEACTOR_NEW
        _pGaeactorProcessorInterfaceInstance->reset();
#else
        delete _pGaeactorProcessorInterfaceInstance;
//        _pGaeactorProcessorInterfaceInstance->deleteLater();
#endif
    }
}

tagLineInfoEx &ConcurrentHashMapManager::get_data_or_create(tagTaskInfo& _tagTaskInfo,const EVENT_KEY_TYPE &key, const std::string &code)
{
    LINES_CONCURRENT_HASHMAP_Accessor _accessor;
    if(!_tagTaskInfo.m_lines.find(_accessor, key))
    {
        _tagTaskInfo.m_lines.insert(_accessor, key);
        _accessor->second = tagLineInfoEx();
        return _accessor->second;
    }
    return _accessor->second;
}

tagTaskInfo &ConcurrentHashMapManager::get_task_or_create(UINT64 task_id)
{
    TASK_LINES_CONCURRENT_HASHMAP_Accessor _accessor;
    if(!m_task_all_lines.find(_accessor, task_id))
    {
        m_task_all_lines.insert(_accessor, task_id);
        tagTaskInfo _tagTaskInfo;
        _tagTaskInfo._task_id = task_id;
        _accessor->second = std::move(_tagTaskInfo);
        return _accessor->second;
    }
    return _accessor->second;
}

tagTaskInfo *ConcurrentHashMapManager::get_task(UINT64 task_id)
{
    TASK_LINES_CONCURRENT_HASHMAP_Accessor _accessor;
    if(!m_task_all_lines.find(_accessor, task_id))
    {
        return nullptr;
    }
    return &_accessor->second;
}

bool ConcurrentHashMapManager::remove_task(UINT64 task_id)
{
    TASK_LINES_CONCURRENT_HASHMAP_Accessor _accessor;
    if(m_task_all_lines.find(_accessor, task_id))
    {
        m_task_all_lines.erase(_accessor);
        return true;
    }
    return false;
}

tagLineInfoEx * ConcurrentHashMapManager::get_data(const tagTaskInfo &_tagTaskInfo, const EVENT_KEY_TYPE &key)
{
    LINES_CONCURRENT_HASHMAP_Accessor _caccessor;
    if(!_tagTaskInfo.m_lines.find(_caccessor, key))
    {
        return nullptr;
    }
    return &_caccessor->second;
}

tagLineInfoEx *ConcurrentHashMapManager::get_data_by_code(tagTaskInfo &_tagTaskInfo, const std::string &code)
{
    tbb::concurrent_hash_map<std::string, tagLineInfoEx*>::accessor _caccessor;
    if(!_tagTaskInfo.m_codeline.find(_caccessor, code))
    {
        return nullptr;
    }
    return _caccessor->second;
}

void ConcurrentHashMapManager::appenditem_hex_color(const H3INDEX &h3Index, const transdata_param_seq_hexidx &hgt, const QColor &cl)
{
    HEXIDX_HGT_COLOR_CONCURRENT_HASHMAP_Accessor _accessor;
    if(!m_hexidxslisttmp_concurrent_hash_map.find(_accessor, h3Index))
    {
        m_hexidxslisttmp_concurrent_hash_map.insert(_accessor, h3Index);
        HEXIDX_HGT_COLOR _HEXIDX_HGT_COLOR = std::make_tuple(std::move(hgt), std::move(cl));
        _accessor->second = std::move(_HEXIDX_HGT_COLOR);
    }
    else
    {
        HEXIDX_HGT_COLOR &_HEXIDX_HGT_COLOR  = _accessor->second;
        std::get<0>(_HEXIDX_HGT_COLOR) = hgt;
        std::get<1>(_HEXIDX_HGT_COLOR) = cl;
    }
}

void ConcurrentHashMapManager::appenditem(const TYPE_ULID &uildsrc, const H3INDEX &h3Index, const transdata_param_seq_hexidx_hgt &hgt, const QColor &cl)
{
    HEXIDX_HGT_ARRAY_COLORS_CONCURRENT_HASHMAP_Accessor _accessor;
    if(!m_hexidxslisttmp3d_concurrent_hash_map.find(_accessor, uildsrc))
    {
        m_hexidxslisttmp3d_concurrent_hash_map.insert(_accessor, uildsrc);
        HEXIDX_HGT_ARRAY hexidxslist;
        std::vector<QColor> vcl;
        hexidxslist.push_back(transdata_param_seq_hexidx{h3Index, hgt});
        vcl.push_back(cl);

        HEXIDX_HGT_ARRAY_COLORS _HEXIDX_HGT_ARRAY_COLORS = std::make_tuple(std::move(hexidxslist), std::move(vcl));
        _accessor->second =std::move(_HEXIDX_HGT_ARRAY_COLORS);
    }
    else
    {
        HEXIDX_HGT_ARRAY_COLORS &_HEXIDX_HGT_ARRAY_COLORS  = _accessor->second;
        HEXIDX_HGT_ARRAY &hexidxslist = std::get<0>(_HEXIDX_HGT_ARRAY_COLORS);
        std::vector<QColor> &vcl = std::get<1>(_HEXIDX_HGT_ARRAY_COLORS);
        hexidxslist.push_back(transdata_param_seq_hexidx{h3Index, hgt});
        vcl.push_back(cl);
    }
}


oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::inputCheckRoutePath(const oatpp::Object<GeoJson_linestring_Dto> &dto)
{
    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    EASY_FUNCTION(profiler::colors::Pink)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject linegeojson  = ConcurrentHashMapManager::string_to_json_object(QString::fromStdString(result));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    resetDisplay();
    bool bconflictbuilding = dto->conflictbuilding;
    INT32 _resolution = dto->resolution;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(0, 0, 0)); // 设置时间为零点
    QDateTime endOfDay = now.addDays(1);
    UINT32 cur_daty_dt_startTime = now.toTime_t(); // 转换为时间戳
    UINT32 cur_daty_dt_endTime = endOfDay.toTime_t() - 1; // 减去一秒得到当天结束的时间戳

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 task_id = ConcurrentHashMapManager::generate_random_positive_uint64();
    if(linegeojson.contains("taskid") && linegeojson.value("taskid").isString() && !linegeojson.value("taskid").toString().isEmpty())
    {
        task_id = linegeojson.value("taskid").toString().toULongLong();
    }
    tagTaskInfo &_all_lines = get_task_or_create(task_id);
    _all_lines._resolution = _resolution;
    GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance =  nullptr;
#ifndef USING_GAEACTOR_NEW
    _pGaeactorProcessorInterfaceInstance = gaeactorenvironment_ex::GaeactorProcessorInterface::getInstance().pGaeactorProcessorInterfaceInstance();
#else
    _pGaeactorProcessorInterfaceInstance = new gaeactorenvironment_ex::GaeactorProcessorInterfaceInstance();
#endif
    if(!_pGaeactorProcessorInterfaceInstance)
    {
        return _GeoJson_Return_Dto;
    }
    _pGaeactorProcessorInterfaceInstance->record_sensor_overlap(true);


    double _heightlimitup = m_heightlimitup.load();
    double _heightlimitdown = m_heightlimitdown.load();
    auto beginTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    auto linefeatures  = linegeojson.value("features").toArray();
    std::atomic<UINT32> totalhexcount = 0;
    if(!linefeatures.isEmpty())
    {
        std::stringstream ss;
        ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input resolution "<<_resolution<<" input lines "<<linefeatures.size()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss);
        std::cout<<ss.str()<<std::endl;
#ifndef USING_DEAL_INPUT_JSARRAY_CONCURRENT
        for(int index = 0;index < linefeatures.size(); index ++)
        {
            auto featuresitemobj = linefeatures.at(index).toObject();
            auto geometry = featuresitemobj.value("geometry").toObject();
            auto type = geometry.value("type").toString();
            if(type == "LineString")
            {
                totalhexcount+=deal_linestring(cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
            }
            else if(type == "Polygon")
            {
                totalhexcount+=deal_polygon(cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
            }
        }
#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, linefeatures.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  auto featuresitemobj = linefeatures.at(j).toObject();
                                  auto geometry = featuresitemobj.value("geometry").toObject();
                                  auto type = geometry.value("type").toString();
                                  if(type == "LineString")
                                  {
                                      totalhexcount+=deal_linestring(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                  }
                                  else if(type == "Polygon")
                                  {
                                      totalhexcount+=deal_polygon(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                  }
                              }
                          });
#endif
    }
    UINT32 buildinghexcount=append_building(bconflictbuilding, _resolution, _all_lines, _pGaeactorProcessorInterfaceInstance);

    std::stringstream ss2;
    ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input hex count "<<totalhexcount.load()<<" "<<buildinghexcount<<" \n";
    //TRACE_LOG_PRINT_EX2(ss2);
    std::cout<<ss2.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    deal_outputCheckRoutePathResult(_all_lines, beginTimestamp,_GeoJson_Return_Dto,_pGaeactorProcessorInterfaceInstance);

    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::inputCheckRoutePathEx(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto)
{
    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject linegeojson  = ConcurrentHashMapManager::string_to_json_object(QString::fromStdString(result));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    resetDisplay();
    bool bconflictbuilding = dto->conflictbuilding;
    INT32 _resolution = dto->resolution;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(0, 0, 0)); // 设置时间为零点
    QDateTime endOfDay = now.addDays(1);
    UINT32 cur_daty_dt_startTime = now.toTime_t(); // 转换为时间戳
    UINT32 cur_daty_dt_endTime = endOfDay.toTime_t() - 1; // 减去一秒得到当天结束的时间戳

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 task_id = ConcurrentHashMapManager::generate_random_positive_uint64();
    if(linegeojson.contains("taskid") && linegeojson.value("taskid").isString() && !linegeojson.value("taskid").toString().isEmpty())
    {
        task_id = linegeojson.value("taskid").toString().toULongLong();
    }
    tagTaskInfo &_all_lines = get_task_or_create(task_id);
    _all_lines._resolution = _resolution;
    GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance =  nullptr;
#ifndef USING_GAEACTOR_NEW
    _pGaeactorProcessorInterfaceInstance = gaeactorenvironment_ex::GaeactorProcessorInterface::getInstance().pGaeactorProcessorInterfaceInstance();
#else
    _pGaeactorProcessorInterfaceInstance = new gaeactorenvironment_ex::GaeactorProcessorInterfaceInstance();
#endif
    if(!_pGaeactorProcessorInterfaceInstance)
    {
        return _GeoJson_Return_Dto;
    }

    double _heightlimitup = m_heightlimitup.load();
    double _heightlimitdown = m_heightlimitdown.load();
    _pGaeactorProcessorInterfaceInstance->record_sensor_overlap(true);
    auto beginTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::atomic<UINT32> totalhexcount = 0;
    auto featurecollections  = linegeojson.value("featurecollections").toArray();
    if(!featurecollections.isEmpty())
    {
        std::stringstream ss;
        ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input resolution "<<_resolution<<" input lines "<<featurecollections.size()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss);
        std::cout<<ss.str()<<std::endl;
#ifndef USING_DEAL_INPUT_JSARRAY_CONCURRENT

        for(int index = 0;index < featurecollections.size(); index ++)
        {
            auto featurecollectionobj = featurecollections.at(index).toObject();
            auto featurecollectionobjpro = featurecollectionobj.value("properties").toObject();
            auto linefeatures  = featurecollectionobj.value("features").toArray();
            if(!linefeatures.isEmpty())
            {
                for(int routeid = 0; routeid < linefeatures.size(); routeid++)
                {
                    auto featuresitem = linefeatures.at(routeid);
                    auto featuresitemobj = featuresitem.toObject();
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    auto properties = featuresitemobj.value("properties").toObject();
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    QJsonObject agentrouteid;
                    auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                    agentrouteid.insert("agentid",agentid);
                    agentrouteid.insert("routeid",routeid);
                    properties.insert("agentrouteid",agentrouteid);
                    properties.insert("featurecollectionpros",featurecollectionobjpro);
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    featuresitemobj.insert("properties",properties);
                    auto geometry = featuresitemobj.value("geometry").toObject();
                    auto type = geometry.value("type").toString();
                    if(type == "LineString")
                    {
                        totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime, _all_lines,index, featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                    }
                    else if(type == "Polygon")
                    {
                        totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime, _all_lines,index, featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                    }
                }
            }
        }
#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, featurecollections.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  auto featurecollectionobj = featurecollections.at(j).toObject();
                                  auto featurecollectionobjpro = featurecollectionobj.value("properties").toObject();
                                  auto linefeatures  = featurecollectionobj.value("features").toArray();
                                  if(!linefeatures.isEmpty())
                                  {
#if 1
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, linefeatures.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 routeid = q.begin(); routeid != q.end(); routeid++)
                                                            {
                                                                auto featuresitemobj = linefeatures.at(routeid).toObject();
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                auto properties = featuresitemobj.value("properties").toObject();
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                QJsonObject agentrouteid;
                                                                auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                                                                agentrouteid.insert("agentid",agentid);
                                                                agentrouteid.insert("routeid",(INT32)routeid);
                                                                properties.insert("agentrouteid",agentrouteid);
                                                                properties.insert("featurecollectionpros",featurecollectionobjpro);
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                featuresitemobj.insert("properties",properties);
                                                                auto geometry = featuresitemobj.value("geometry").toObject();
                                                                auto type = geometry.value("type").toString();
                                                                if(type == "LineString")
                                                                {
                                                                    totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                                                }
                                                                else if(type == "Polygon")
                                                                {
                                                                    totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                                                }
                                                            }
                                                        });
#else
                                      for(int routeid = 0; routeid < linefeatures.size(); routeid++)
                                      {
                                          auto featuresitem = linefeatures.at(routeid);
                                          auto featuresitemobj = featuresitem.toObject();
                                          ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                          auto properties = featuresitemobj.value("properties").toObject();
                                          ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                          QJsonObject agentrouteid;
                                          auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                                          agentrouteid.insert("agentid",agentid);
                                          agentrouteid.insert("routeid",routeid);
                                          properties.insert("agentrouteid",agentrouteid);
                                          properties.insert("featurecollectionpros",featurecollectionobjpro);
                                          ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                          featuresitemobj.insert("properties",properties);
                                          auto geometry = featuresitemobj.value("geometry").toObject();
                                          auto type = geometry.value("type").toString();
                                          if(type == "LineString")
                                          {
                                              totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                          }
                                          else if(type == "Polygon")
                                          {
                                              totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                          }
                                      }
#endif
                                  }
                              }
                          });
#endif
    }
    UINT32 buildinghexcount=append_building(bconflictbuilding, _resolution, _all_lines, _pGaeactorProcessorInterfaceInstance);

    std::stringstream ss2;
    ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input hex count "<<totalhexcount.load()<<" "<<buildinghexcount<<" \n";
    //TRACE_LOG_PRINT_EX2(ss2);
    std::cout<<ss2.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    deal_outputCheckRoutePathResult(_all_lines, beginTimestamp,_GeoJson_Return_Dto,_pGaeactorProcessorInterfaceInstance);
    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::inputCheckRoutePathAccumulate(const oatpp::Object<GeoJson_linestring_Dto> &dto)
{
    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    EASY_FUNCTION(profiler::colors::Pink)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject linegeojson  = ConcurrentHashMapManager::string_to_json_object(QString::fromStdString(result));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    resetDisplay();
    bool bconflictbuilding = dto->conflictbuilding;
    INT32 _resolution = dto->resolution;


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(0, 0, 0)); // 设置时间为零点
    QDateTime endOfDay = now.addDays(1);
    UINT32 cur_daty_dt_startTime = now.toTime_t(); // 转换为时间戳
    UINT32 cur_daty_dt_endTime = endOfDay.toTime_t() - 1; // 减去一秒得到当天结束的时间戳

    /////////////////////////////////////////////////////////////////////////////////////////////////////////    
    UINT64 task_id = 0;
    tagTaskInfo &_all_lines = get_task_or_create(task_id);
    _all_lines._resolution = _resolution;
    GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance =  m_pGaeactorProcessorInterfaceInstanceAccumulate;
    if(!_pGaeactorProcessorInterfaceInstance)
    {
        return _GeoJson_Return_Dto;
    }
    double _heightlimitup = m_heightlimitup.load();
    double _heightlimitdown = m_heightlimitdown.load();
    auto beginTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    auto linefeatures  = linegeojson.value("features").toArray();
    std::atomic<UINT32> totalhexcount = 0;
    if(!linefeatures.isEmpty())
    {
        std::stringstream ss;
        ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" Accumulate input resolution "<<_resolution<<" input lines "<<linefeatures.size()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss);
        std::cout<<ss.str()<<std::endl;
#ifndef USING_DEAL_INPUT_JSARRAY_CONCURRENT
        for(int index = 0;index < linefeatures.size(); index ++)
        {
            auto featuresitemobj = linefeatures.at(index).toObject();
            auto geometry = featuresitemobj.value("geometry").toObject();
            auto type = geometry.value("type").toString();
            if(type == "LineString")
            {
                totalhexcount+=deal_linestring(cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
            }
            else if(type == "Polygon")
            {
                totalhexcount+=deal_polygon(cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
            }
        }
#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, linefeatures.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  auto featuresitemobj = linefeatures.at(j).toObject();
                                  auto geometry = featuresitemobj.value("geometry").toObject();
                                  auto type = geometry.value("type").toString();
                                  if(type == "LineString")
                                  {
                                      totalhexcount+=deal_linestring(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                  }
                                  else if(type == "Polygon")
                                  {
                                      totalhexcount+=deal_polygon(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                  }
                              }
                          });
#endif
    }
    UINT32 buildinghexcount=append_building(bconflictbuilding, _resolution, _all_lines, _pGaeactorProcessorInterfaceInstance);

    std::stringstream ss2;
    ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input hex count "<<totalhexcount.load()<<" "<<buildinghexcount<<" \n";
    //TRACE_LOG_PRINT_EX2(ss2);
    std::cout<<ss2.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    deal_outputCheckRoutePathResult(_all_lines, beginTimestamp,_GeoJson_Return_Dto,_pGaeactorProcessorInterfaceInstance);

    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::inputCheckRoutePathExAccumulate(const oatpp::Object<GeoJson_FeatureCollections_Dto> &dto)
{
    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject linegeojson  = ConcurrentHashMapManager::string_to_json_object(QString::fromStdString(result));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    resetDisplay();
    bool bconflictbuilding = dto->conflictbuilding;
    INT32 _resolution = dto->resolution;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    QDateTime now = QDateTime::currentDateTime();
    now.setTime(QTime(0, 0, 0)); // 设置时间为零点
    QDateTime endOfDay = now.addDays(1);
    UINT32 cur_daty_dt_startTime = now.toTime_t(); // 转换为时间戳
    UINT32 cur_daty_dt_endTime = endOfDay.toTime_t() - 1; // 减去一秒得到当天结束的时间戳

    double _heightlimitup = m_heightlimitup.load();
    double _heightlimitdown = m_heightlimitdown.load();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    UINT64 task_id = 0;
    tagTaskInfo &_all_lines = get_task_or_create(task_id);
    _all_lines._resolution = _resolution;
    GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance =  m_pGaeactorProcessorInterfaceInstanceAccumulate;
    if(!_pGaeactorProcessorInterfaceInstance)
    {
        return _GeoJson_Return_Dto;
    }
    auto beginTimestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::atomic<UINT32> totalhexcount = 0;
    auto featurecollections  = linegeojson.value("featurecollections").toArray();
    if(!featurecollections.isEmpty())
    {
        std::stringstream ss;
        ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" Accumulate input resolution "<<_resolution<<" input lines "<<featurecollections.size()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss);
        std::cout<<ss.str()<<std::endl;
#ifndef USING_DEAL_INPUT_JSARRAY_CONCURRENT

        for(int index = 0;index < featurecollections.size(); index ++)
        {
            auto featurecollectionobj = featurecollections.at(index).toObject();
            auto featurecollectionobjpro = featurecollectionobj.value("properties").toObject();
            auto linefeatures  = featurecollectionobj.value("features").toArray();
            if(!linefeatures.isEmpty())
            {
                for(int routeid = 0; routeid < linefeatures.size(); routeid++)
                {
                    auto featuresitem = linefeatures.at(routeid);
                    auto featuresitemobj = featuresitem.toObject();
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    auto properties = featuresitemobj.value("properties").toObject();
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    QJsonObject agentrouteid;
                    auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                    agentrouteid.insert("agentid",agentid);
                    agentrouteid.insert("routeid",routeid);
                    properties.insert("agentrouteid",agentrouteid);
                    properties.insert("featurecollectionpros",featurecollectionobjpro);
                    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                    featuresitemobj.insert("properties",properties);
                    auto geometry = featuresitemobj.value("geometry").toObject();
                    auto type = geometry.value("type").toString();
                    if(type == "LineString")
                    {
                        totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime, _all_lines,index, featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                    }
                    else if(type == "Polygon")
                    {
                        totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime, _all_lines,index, featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                    }
                }
            }
        }
#else
        tbb::parallel_for(tbb::blocked_range<size_t>(0, featurecollections.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  auto featurecollectionobj = featurecollections.at(j).toObject();
                                  auto featurecollectionobjpro = featurecollectionobj.value("properties").toObject();
                                  auto linefeatures  = featurecollectionobj.value("features").toArray();
                                  if(!linefeatures.isEmpty())
                                  {
#if 1
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, linefeatures.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 routeid = q.begin(); routeid != q.end(); routeid++)
                                                            {
                                                                auto featuresitemobj = linefeatures.at(routeid).toObject();
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                auto properties = featuresitemobj.value("properties").toObject();
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                QJsonObject agentrouteid;
                                                                auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                                                                agentrouteid.insert("agentid",agentid);
                                                                agentrouteid.insert("routeid",(INT32)routeid);
                                                                properties.insert("agentrouteid",agentrouteid);
                                                                properties.insert("featurecollectionpros",featurecollectionobjpro);
                                                                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                featuresitemobj.insert("properties",properties);
                                                                auto geometry = featuresitemobj.value("geometry").toObject();
                                                                auto type = geometry.value("type").toString();
                                                                if(type == "LineString")
                                                                {
                                                                    totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                                                }
                                                                else if(type == "Polygon")
                                                                {
                                                                    totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,j,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                                                                }
                                                            }
                                                        });
#else
                        for(int routeid = 0; routeid < linefeatures.size(); routeid++)
                        {
                            auto featuresitem = linefeatures.at(routeid);
                            auto featuresitemobj = featuresitem.toObject();
                            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            auto properties = featuresitemobj.value("properties").toObject();
                            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            QJsonObject agentrouteid;
                            auto agentid = QString::number(ConcurrentHashMapManager::generate_random_positive_uint64());
                            agentrouteid.insert("agentid",agentid);
                            agentrouteid.insert("routeid",routeid);
                            properties.insert("agentrouteid",agentrouteid);
                            properties.insert("featurecollectionpros",featurecollectionobjpro);
                            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            featuresitemobj.insert("properties",properties);
                            auto geometry = featuresitemobj.value("geometry").toObject();
                            auto type = geometry.value("type").toString();
                            if(type == "LineString")
                            {
                                totalhexcount+=deal_linestring(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                            }
                            else if(type == "Polygon")
                            {
                                totalhexcount+=deal_polygon(cur_daty_dt_startTime,cur_daty_dt_endTime,_all_lines,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                            }
                        }
#endif
                                  }
                              }
                          });
#endif
    }
    UINT32 buildinghexcount=append_building(bconflictbuilding, _resolution, _all_lines, _pGaeactorProcessorInterfaceInstance);

    std::stringstream ss2;
    ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input hex count "<<totalhexcount.load()<<" "<<buildinghexcount<<" \n";
    //TRACE_LOG_PRINT_EX2(ss2);
    std::cout<<ss2.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    deal_outputCheckRoutePathResult(_all_lines, beginTimestamp,_GeoJson_Return_Dto,_pGaeactorProcessorInterfaceInstance);
    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_linestring_opearte_Dto> ConcurrentHashMapManager::inputCheckRoutePathSettings(const oatpp::Object<GeoJson_linestring_opearte_Dto> &dto)
{
    auto GeoJson_linestring_opearte_Dto_return =  GeoJson_linestring_opearte_Dto::createShared();
    if(dto->issettype)
    {
        if(dto->paramname == "EchoRoutes")
        {
            m_bechoroutes = dto->paramval;
        }
        else if(dto->paramname == "CheckTime")
        {
            m_bchecktime = dto->paramval;
        }
        else if(dto->paramname == "ShowHex3d")
        {
            m_bshowhex3d = dto->paramval;
        }
        else if(dto->paramname == "ReturnDirect")
        {
            m_breturndirect = dto->paramval;
        }
        else if(dto->paramname == "LineResolution")
        {
            int resolution = dto->paramval2;

            resolution = resolution < 1 ? 1 : resolution;
            resolution = resolution > 15 ? 15 : resolution;
            m_bresolution = resolution;
        }
        else if(dto->paramname == "AccumulateReset")
        {
            emit deal_result_sig("AccumulateReset", m_pGaeactorProcessorInterfaceInstanceAccumulate);
        }

        else if(dto->paramname == "DefaultHeightlimitUp")
        {
            m_heightlimitup = GeoJson_linestring_opearte_Dto_return->paramval3;
        }
        else if(dto->paramname == "DefaultHeightlimitDown")
        {
            m_heightlimitdown = GeoJson_linestring_opearte_Dto_return->paramval3;
        }
    }
    else
    {
        if(dto->paramname == "EchoRoutes")
        {
            GeoJson_linestring_opearte_Dto_return->paramval = m_bechoroutes;
        }
        else if(dto->paramname == "CheckTime")
        {
            GeoJson_linestring_opearte_Dto_return->paramval = m_bchecktime;
        }
        else if(dto->paramname == "ShowHex3d")
        {
            GeoJson_linestring_opearte_Dto_return->paramval = m_bshowhex3d;
        }
        else if(dto->paramname == "ReturnDirect")
        {
            GeoJson_linestring_opearte_Dto_return->paramval = m_breturndirect;
        }
        else if(dto->paramname == "LineResolution")
        {
            GeoJson_linestring_opearte_Dto_return->paramval2 = m_bresolution;
        }
        else if(dto->paramname == "DefaultHeightlimitUp")
        {
            GeoJson_linestring_opearte_Dto_return->paramval3 = m_heightlimitup;
        }
        else if(dto->paramname == "DefaultHeightlimitDown")
        {
            GeoJson_linestring_opearte_Dto_return->paramval3 = m_heightlimitdown;
        }
    }
    GeoJson_linestring_opearte_Dto_return->issettype = dto->issettype;
    GeoJson_linestring_opearte_Dto_return->paramname = dto->paramname;
    return GeoJson_linestring_opearte_Dto_return;
}

void ConcurrentHashMapManager::show_building()
{
//    resetDisplay();

    emit deal_result_sig("show_building",nullptr);
    auto itor = m_RES_buildings.at(8).m_lines.begin();
    while(itor != m_RES_buildings.at(8).m_lines.end())
    {
        emit draw_linestring_sig(itor->second.m_line);
        itor++;
    }
}
oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::getLineConflictsInfo(const oatpp::Object<GeoJson_query_line_id_Dto> &dto)
{

    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    auto generate_return=[&](tagTaskInfo* ptagTaskInfo, tagLineInfoEx * _tagLineInfo)
    {
        if(_tagLineInfo)
        {
            _GeoJson_Return_Dto->count = _tagLineInfo->m_conflictpts.size();
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            QJsonArray linesfeatures;

            QJsonObject featuresitemobj = _tagLineInfo->m_line.featuresitemobj;
            auto properties = featuresitemobj.value("properties").toObject();
            QJsonArray linehex;
            for(auto hexindex:_tagLineInfo->m_line.m_hexidxs)
            {
                linehex.append(QString::number(hexindex));
            }
            properties.insert("linehex",linehex);
            featuresitemobj.insert("properties",properties);
            linesfeatures.append(featuresitemobj);
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            std::vector<UINT32> keys;
            {
                keys.clear();
                keys.reserve(_tagLineInfo->m_conflictpts.size());
                tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo*>::const_iterator _exist_events_map_itor = _tagLineInfo->m_conflictpts.begin();
                while (_exist_events_map_itor != _tagLineInfo->m_conflictpts.end())
                {
                    keys.push_back(_exist_events_map_itor->first);
                    _exist_events_map_itor++;
                }
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            _GeoJson_Return_Dto->points = GeoJson_point_Dto::createShared();
            _GeoJson_Return_Dto->points->type = "FeatureCollection";
            _GeoJson_Return_Dto->points->resolution = ptagTaskInfo->_resolution;

            _GeoJson_Return_Dto->points->features = oatpp::Vector<oatpp::Object<GeoJson_features_point_Dto>>::createShared();
            tbb::concurrent_hash_map<UINT64, QJsonObject> _linesfeactures;
            GEOJSON_POINTS_CONCURRENT_HASHMAP _GEOJSON_POINTS_CONCURRENT_HASHMAP;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, keys.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 m = r.begin(); m != r.end(); ++m)
                                  {
                                      auto ptindex = keys.at(m);
                                      tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo*>::const_accessor _accessor;
                                      if(_tagLineInfo->m_conflictpts.find(_accessor, ptindex))
                                      {
                                          tagTaskResultPtInfo* ptagTaskResultPtInfo = _accessor->second;
                                          if(ptagTaskResultPtInfo)
                                          {
                                              auto pointfeature = GeoJson_features_point_Dto::createShared();
                                              pointfeature->type = "Feature";

                                              /////////////////////////////////////////////////////////////////////
                                              pointfeature->properties = GeoJson_point_properties_Dto::createShared();
                                              pointfeature->properties->pointid = ptindex;

                                              QString hexColor = QString("#%1%2%3")
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'));
                                              pointfeature->properties->color = hexColor.toStdString();

                                              /////////////////////////////////////////////////////////////////////
                                              pointfeature->geometry = GeoJson_geometry_point_Dto::createShared();
                                              pointfeature->geometry->type = "Point";
                                              pointfeature->geometry->coordinates = oatpp::Vector<oatpp::Float64>::createShared();
                                              pointfeature->geometry->coordinates->push_back(ptagTaskResultPtInfo->_latlng.lng);
                                              pointfeature->geometry->coordinates->push_back(ptagTaskResultPtInfo->_latlng.lat);
                                              /////////////////////////////////////////////////////////////////////
                                              pointfeature->properties->pointlinestrings = oatpp::Vector<oatpp::Object<GeoJson_points_linestring_id_Dto>>::createShared();

                                              for(int j = 0; j < ptagTaskResultPtInfo->m_ptconflictlines.size(); j++)
                                              {
                                                  auto _GeoJson_points_linestring_id_Dto = ptagTaskResultPtInfo->getlineInfo(j);
                                                  tagLineInfoEx * cur_tagLineInfo = std::get<0>(*_GeoJson_points_linestring_id_Dto);
                                                  if(cur_tagLineInfo != _tagLineInfo)
                                                  {
                                                      tbb::concurrent_hash_map<UINT64, QJsonObject> ::accessor _linesfeactures_accessor;
                                                      if(!_linesfeactures.find(_linesfeactures_accessor, cur_tagLineInfo->m_line._id))
                                                      {
                                                          _linesfeactures.insert(_linesfeactures_accessor,cur_tagLineInfo->m_line._id);
                                                          _linesfeactures_accessor->second = cur_tagLineInfo->m_line.featuresitemobj;
                                                      }
                                                  }
                                                  pointfeature->properties->pointlinestrings->push_back(std::get<1>(*_GeoJson_points_linestring_id_Dto));
                                              }

                                              {
                                                  GEOJSON_POINTS_CONCURRENT_HASHMAP_Accessor _ret_tmp_accessor;
                                                  if(!_GEOJSON_POINTS_CONCURRENT_HASHMAP.find(_ret_tmp_accessor, ptindex))
                                                  {
                                                      _GEOJSON_POINTS_CONCURRENT_HASHMAP.insert(_ret_tmp_accessor,ptindex);
                                                      _ret_tmp_accessor->second = std::move(pointfeature);
                                                  }
                                              }
                                          }
                                      }
                                  }
                              });
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                GEOJSON_POINTS_CONCURRENT_HASHMAP_ConstIterator _ret_tmp_itor = _GEOJSON_POINTS_CONCURRENT_HASHMAP.begin();
                while (_ret_tmp_itor != _GEOJSON_POINTS_CONCURRENT_HASHMAP.end())
                {
                    _GeoJson_Return_Dto->points->features->push_back(std::move(_ret_tmp_itor->second));
                    _ret_tmp_itor++;
                }
            }

            {
                tbb::concurrent_hash_map<UINT64, QJsonObject>::const_iterator _ret_tmp_itor = _linesfeactures.begin();
                while (_ret_tmp_itor != _linesfeactures.end())
                {
                    linesfeatures.append(std::move(_ret_tmp_itor->second));
                    _ret_tmp_itor++;
                }
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            QJsonObject routesobj;
            routesobj.insert("type","FeatureCollection");
            routesobj.insert("resolution",ptagTaskInfo->_resolution);
            routesobj.insert("features",linesfeatures);
            oatpp::String str = ConcurrentHashMapManager::json_object_to_string(routesobj).toStdString();
            _GeoJson_Return_Dto->routes = m_jsonmapper->readFromString<oatpp::Object<GeoJson_linestring_Dto>>(str);

        }
    };
    if(!dto->taskid->empty())
    {
        auto taskid = QString::fromStdString(dto->taskid).toULongLong();
        _GeoJson_Return_Dto->taskid = dto->taskid;

        tagTaskInfo* ptagTaskInfo = get_task(taskid);
        if(ptagTaskInfo)
        {
            if(dto->code->empty() && !dto->agentid->empty() && dto->routeid != 0)
            {
                auto agentid = QString::fromStdString(dto->agentid).toULongLong();
                auto routeid = dto->routeid;
                EVENT_KEY_TYPE key = EVENT_KEY_TYPE{agentid, 0, (TYPE_ULID)routeid};
                tagLineInfoEx * _tagLineInfo = this->get_data(*ptagTaskInfo,key);
                generate_return(ptagTaskInfo, _tagLineInfo);
            }
            else if(!dto->code->empty() && dto->agentid->empty() && dto->routeid == 0)
            {
                tagLineInfoEx * _tagLineInfo = this->get_data_by_code(*ptagTaskInfo,dto->code);
                generate_return(ptagTaskInfo, _tagLineInfo);
            }
        }
    }
    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::getConflictsPtsInfo(const oatpp::Object<GeoJson_query_pt_id_Dto> &dto)
{
    auto _GeoJson_Return_Dto = GeoJson_Return_Dto::createShared();
    if(!dto->taskid->empty())
    {
        auto taskid = QString::fromStdString(dto->taskid).toULongLong();
        _GeoJson_Return_Dto->taskid = dto->taskid;
        tagTaskInfo* ptagTaskInfo = get_task(taskid);
        if(ptagTaskInfo &&
            dto->beginindex >= 0 &&
            dto->beginindex < ptagTaskInfo->m_taskresultpts.size())
        {
            _GeoJson_Return_Dto->points = GeoJson_point_Dto::createShared();
            _GeoJson_Return_Dto->points->type = "FeatureCollection";
            _GeoJson_Return_Dto->points->resolution = ptagTaskInfo->_resolution;

            _GeoJson_Return_Dto->points->features = oatpp::Vector<oatpp::Object<GeoJson_features_point_Dto>>::createShared();

            GEOJSON_POINTS_CONCURRENT_HASHMAP _GEOJSON_POINTS_CONCURRENT_HASHMAP;

            std::vector<UINT32> keys;
            {
                keys.clear();
                keys.reserve(ptagTaskInfo->m_taskresultpts.size());
                tbb::concurrent_hash_map<UINT32, tagTaskResultPtInfo>::const_iterator _exist_events_map_itor = ptagTaskInfo->m_taskresultpts.begin();
                while (_exist_events_map_itor != ptagTaskInfo->m_taskresultpts.end())
                {
                    keys.push_back(_exist_events_map_itor->first);
                    _exist_events_map_itor++;
                }
            }
            UINT32 begin_ = dto->beginindex;
            UINT32 end_ = dto->beginindex+dto->count > keys.size() ? keys.size():dto->beginindex+dto->count;

            tbb::parallel_for(tbb::blocked_range<size_t>(begin_, end_),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 j = r.begin(); j != r.end(); ++j)
                                  {
                                      UINT32 ptindex = keys.at(j);
                                      tagTaskResultPtInfo* ptagTaskResultPtInfo = ptagTaskInfo->getPtInfo(ptindex);
                                      if(ptagTaskResultPtInfo)
                                      {
                                          auto pointfeature = GeoJson_features_point_Dto::createShared();
                                          pointfeature->type = "Feature";

                                          /////////////////////////////////////////////////////////////////////
                                          pointfeature->properties = GeoJson_point_properties_Dto::createShared();
                                          pointfeature->properties->pointid = ptindex;

                                          QString hexColor = QString("#%1%2%3")
                                                                 .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                 .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                 .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'));
                                          pointfeature->properties->color = hexColor.toStdString();

                                          /////////////////////////////////////////////////////////////////////
                                          pointfeature->geometry = GeoJson_geometry_point_Dto::createShared();
                                          pointfeature->geometry->type = "Point";
                                          pointfeature->geometry->coordinates = oatpp::Vector<oatpp::Float64>::createShared();
                                          pointfeature->geometry->coordinates->push_back(ptagTaskResultPtInfo->_latlng.lng);
                                          pointfeature->geometry->coordinates->push_back(ptagTaskResultPtInfo->_latlng.lat);
                                          /////////////////////////////////////////////////////////////////////
                                          pointfeature->properties->pointlinestrings = oatpp::Vector<oatpp::Object<GeoJson_points_linestring_id_Dto>>::createShared();

                                          for(int j = 0; j < ptagTaskResultPtInfo->m_ptconflictlines.size(); j++)
                                          {
                                              auto _GeoJson_points_linestring_id_Dto = ptagTaskResultPtInfo->getlineInfo(j);
                                              pointfeature->properties->pointlinestrings->push_back(std::get<1>(*_GeoJson_points_linestring_id_Dto));
                                          }
                                          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                                          {
                                              GEOJSON_POINTS_CONCURRENT_HASHMAP_Accessor _ret_tmp_accessor;
                                              if(!_GEOJSON_POINTS_CONCURRENT_HASHMAP.find(_ret_tmp_accessor, ptindex))
                                              {
                                                  _GEOJSON_POINTS_CONCURRENT_HASHMAP.insert(_ret_tmp_accessor,ptindex);
                                                  _ret_tmp_accessor->second = std::move(pointfeature);
                                              }
                                          }
                                          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                      }
                                  }
                              });
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            {
                GEOJSON_POINTS_CONCURRENT_HASHMAP_ConstIterator _ret_tmp_itor = _GEOJSON_POINTS_CONCURRENT_HASHMAP.begin();
                while (_ret_tmp_itor != _GEOJSON_POINTS_CONCURRENT_HASHMAP.end())
                {
                    _GeoJson_Return_Dto->points->features->push_back(std::move(_ret_tmp_itor->second));
                    _ret_tmp_itor++;
                }
            }
            _GeoJson_Return_Dto->count = _GeoJson_Return_Dto->points->features->size();

            std::stringstream ss;
            ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" query ConflictsPtsInfo "<<ptagTaskInfo->m_taskresultpts.size()<<" "<<_GeoJson_Return_Dto->count <<"\n";
            //TRACE_LOG_PRINT_EX2(ss);
            std::cout<<ss.str()<<std::endl;
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
    }
    return _GeoJson_Return_Dto;
}

oatpp::Object<GeoJson_clear_pt_id_Dto> ConcurrentHashMapManager::clearConflictsPtsInfo(const oatpp::Object<GeoJson_clear_pt_id_Dto> &dto)
{
    if(!dto->taskid->empty())
    {
        QString jsobj;
        if(dto->taskid == "clearall")
        {
            jsobj = "cleartask_all";
        }
        else
        {
            jsobj = "cleartask_"+QString::fromStdString(dto->taskid);
        }
        emit deal_result_sig(jsobj, nullptr);
    }
    return dto;
}

oatpp::Object<GeoJson_tasks_Dto> ConcurrentHashMapManager::getTaskInfo()
{
    auto _GeoJson_tasks_Dto = GeoJson_tasks_Dto::createShared();

    _GeoJson_tasks_Dto->tasks = oatpp::Vector<oatpp::Object<GeoJson_task_item_Dto>>::createShared();

    std::vector<UINT64> keys;
    {
        keys.clear();
        keys.reserve(m_task_all_lines.size());
        TASK_LINES_CONCURRENT_HASHMAP_ConstIterator _exist_events_map_itor = m_task_all_lines.begin();
        while (_exist_events_map_itor != m_task_all_lines.end())
        {
            keys.push_back(_exist_events_map_itor->first);
            _exist_events_map_itor++;
        }
    }
    for(int i = 0; i < keys.size(); i++)
    {
        auto task_id = keys.at(i);
        TASK_LINES_CONCURRENT_HASHMAP_ConstAccessor caccessor;
        if(m_task_all_lines.find(caccessor, task_id))
        {
            auto _GeoJson_task_item_Dto = GeoJson_task_item_Dto::createShared();

            _GeoJson_task_item_Dto->taskid = QString::number(task_id).toStdString();
            _GeoJson_task_item_Dto->linescount = caccessor->second.m_lines.size();
            _GeoJson_task_item_Dto->conflictscount = caccessor->second.m_taskresultpts.size();
            _GeoJson_tasks_Dto->tasks->push_back(std::move(_GeoJson_task_item_Dto));
        }
    }

    return _GeoJson_tasks_Dto;
}

oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> ConcurrentHashMapManager::LatLngToHex(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto)
{
    H3INDEX ret = 0;
    if(dto->resolution>0 && dto->resolution <= 15)
    {
        LocationHelper::doCoords(ret, dto->LAT, dto->LNG, dto->resolution);
    }
    auto GeoJson_LATLNG_TO_HEX_Dto_return = GeoJson_LATLNG_TO_HEX_Dto::createShared();

    GeoJson_LATLNG_TO_HEX_Dto_return->LAT = dto->LAT;
    GeoJson_LATLNG_TO_HEX_Dto_return->LNG = dto->LNG;
    GeoJson_LATLNG_TO_HEX_Dto_return->resolution = dto->resolution;
    GeoJson_LATLNG_TO_HEX_Dto_return->HEX = QString::number(ret).toStdString();
    return GeoJson_LATLNG_TO_HEX_Dto_return;
}

oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> ConcurrentHashMapManager::HexToLatLng(const oatpp::Object<GeoJson_LATLNG_TO_HEX_Dto> &dto)
{
    auto GeoJson_LATLNG_TO_HEX_Dto_return = GeoJson_LATLNG_TO_HEX_Dto::createShared();
    if(dto->resolution>0 && dto->resolution <= 15)
    {
        H3INDEX hexindex = QString::fromStdString(dto->HEX).toULongLong();
        LatLng ret;
        LocationHelper::doCell(ret, hexindex);

        GeoJson_LATLNG_TO_HEX_Dto_return->LAT = LocationHelper::radianToDegree(ret.lat);
        GeoJson_LATLNG_TO_HEX_Dto_return->LNG = LocationHelper::radianToDegree(ret.lng);
        GeoJson_LATLNG_TO_HEX_Dto_return->resolution = LocationHelper::getresolution(hexindex);
    }

    GeoJson_LATLNG_TO_HEX_Dto_return->HEX = dto->HEX;
    return GeoJson_LATLNG_TO_HEX_Dto_return;
}

#ifdef USING_GUI_SHOW
oatpp::Object<GeoJson_Return_Dto> ConcurrentHashMapManager::inputDisplayCheckRoutePathResult(const oatpp::Object<GeoJson_Return_Dto> &dto)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject resultobj  = ConcurrentHashMapManager::string_to_json_object(QString::fromStdString(result));
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    resetDisplay();

    auto linegeojson = resultobj.value("routes").toObject();

    if(!linegeojson.isEmpty())
    {
        INT32 _resolution  = linegeojson.value("resolution").toInt();

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        QDateTime now = QDateTime::currentDateTime();
        now.setTime(QTime(0, 0, 0)); // 设置时间为零点
        QDateTime endOfDay = now.addDays(1);
        UINT32 cur_daty_dt_startTime = now.toTime_t(); // 转换为时间戳
        UINT32 cur_daty_dt_endTime = endOfDay.toTime_t() - 1; // 减去一秒得到当天结束的时间戳

        double _heightlimitup = m_heightlimitup.load();
        double _heightlimitdown = m_heightlimitdown.load();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        tagTaskInfo _all_lines;
        _all_lines._task_id = 0;
        _all_lines._resolution = _resolution;
        auto linefeatures  = linegeojson.value("features").toArray();
        std::atomic<UINT32> totalhexcount = 0;
        if(!linefeatures.isEmpty())
        {
            std::stringstream ss;
            ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input resolution "<<_resolution<<" input lines "<<linefeatures.size()<<" \n";
            //TRACE_LOG_PRINT_EX2(ss);
            std::cout<<ss.str()<<std::endl;
#ifndef USING_DEAL_INPUT_JSARRAY_CONCURRENT
            for(int index = 0;index < linefeatures.size(); index ++)
            {
                auto featuresitemobj = linefeatures.at(index).toObject();
                auto geometry = featuresitemobj.value("geometry").toObject();
                auto type = geometry.value("type").toString();
                if(type == "LineString")
                {
                    totalhexcount+=deal_linestring(_resolution, cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                }
                else if(type == "Polygon")
                {
                    totalhexcount+=deal_polygon(_resolution, cur_daty_dt_startTime, cur_daty_dt_endTime, _all_lines, index,featuresitemobj,_pGaeactorProcessorInterfaceInstance,_heightlimitup,_heightlimitdown);
                }
            }
#else
            tbb::parallel_for(tbb::blocked_range<size_t>(0, linefeatures.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 j = r.begin(); j != r.end(); j++)
                                  {
                                      auto featuresitemobj = linefeatures.at(j).toObject();
                                      auto geometry = featuresitemobj.value("geometry").toObject();
                                      auto type = geometry.value("type").toString();
                                      if(type == "LineString")
                                      {
                                          totalhexcount+=deal_linestring(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,nullptr,_heightlimitup,_heightlimitdown);
                                      }
                                      else if(type == "Polygon")
                                      {
                                          totalhexcount+=deal_polygon(cur_daty_dt_startTime, cur_daty_dt_endTime,_all_lines, j,featuresitemobj,nullptr,_heightlimitup,_heightlimitdown);
                                      }
                                  }
                              });
#endif
        }

        std::stringstream ss2;
        ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input hex count "<<totalhexcount.load()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss2);
        std::cout<<ss2.str()<<std::endl;
        /////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USING_SHOW_RESULT
        auto itor = _all_lines.m_lines.begin();
        while(itor != _all_lines.m_lines.end())
        {
            emit draw_linestring_sig(itor->second.m_line);
            itor++;
        }
#endif
    }

    auto points = resultobj.value("points").toObject();

    if(!points.isEmpty())
    {
        INT32 _resolution  = linegeojson.value("resolution").toInt();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        auto pointfeatures  = points.value("features").toArray();
        if(!pointfeatures.isEmpty())
        {
            std::stringstream ss;
            ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input resolution "<<_resolution<<" input points "<<pointfeatures.size()<<" \n";
            //TRACE_LOG_PRINT_EX2(ss);
            std::cout<<ss.str()<<std::endl;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, pointfeatures.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 j = r.begin(); j != r.end(); j++)
                                  {
                                      auto featuresitemobj = pointfeatures.at(j).toObject();
                                      auto properties = featuresitemobj.value("properties").toObject();
                                      auto pointlinestrings = properties.value("pointlinestrings").toArray();
                                      auto geometry = featuresitemobj.value("geometry").toObject();
                                      auto coordinates = geometry.value("coordinates").toArray();
                                      double lng = coordinates.at(0).toDouble();
                                      double lat = coordinates.at(1).toDouble();


                                      H3INDEX _h3Index;
                                      LocationHelper::doCoords(_h3Index, lat, lng, _resolution);

                                      TYPE_ULID agentid = ConcurrentHashMapManager::generate_random_positive_uint64();
                                      TYPE_ULID sensingmediaid = ConcurrentHashMapManager::generate_random_positive_uint64();
                                      TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(agentid) ^ (std::hash<TYPE_ULID>()(sensingmediaid) << 1);
                                      QColor cl = QColor(255, 0, 0, 128);
                                      for(auto pointlinestringsitem:pointlinestrings)
                                      {
                                          auto pointlinestringsitemobj = pointlinestringsitem.toObject();

                                          transdata_param_seq_hexidx_hgt hgt;
                                          hgt.PARAM_seq_hexidx_hgt = pointlinestringsitemobj.value("height").toDouble();
                                          hgt.PARAM_seq_hexidx_hgt0 = pointlinestringsitemobj.value("heightlimitdown").toDouble();
                                          hgt.PARAM_seq_hexidx_hgtn = pointlinestringsitemobj.value("heightlimitup").toDouble();
                                          this->appenditem(uildsrc, _h3Index, hgt, cl);
                                      }
                                      appenditem_hex_color(_h3Index, transdata_param_seq_hexidx(), cl);
                                  }
                              });
        }
    }


    QString jsobj = "showresult_0";
    emit deal_result_sig("show_result", nullptr);
    return dto;
}
#endif

void ConcurrentHashMapManager::outputCheckRoutePathRoutesResult(tagTaskInfo &_taskinfo, oatpp::Object<GeoJson_linestring_Dto>& routes)
{
    EASY_FUNCTION(profiler::colors::Lime)
    QJsonObject routesobj;
    routesobj.insert("type","FeatureCollection");
    routesobj.insert("resolution",_taskinfo._resolution);
    QJsonArray features;
    auto itor = _taskinfo.m_lines.begin();
    while(itor != _taskinfo.m_lines.end())
    {
        features.append(itor->second.m_line.featuresitemobj);
        itor++;
    }
    routesobj.insert("features",features);
    oatpp::String str = ConcurrentHashMapManager::json_object_to_string(routesobj).toStdString();
    routes = m_jsonmapper->readFromString<oatpp::Object<GeoJson_linestring_Dto>>(str);
}


int ConcurrentHashMapManager::generateRandomNumber()
{
    // 创建随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());

    // 创建一个在0到255之间的均匀分布
    std::uniform_int_distribution<> dis(0, 255);

    // 生成随机数
    return dis(gen);
}

double ConcurrentHashMapManager::calc_dist(const double &lat1, const double &lon1, const double &lat2, const double &lon2)
{
    double radLat1 = toRadians(lat1);
    double radLat2 = toRadians(lat2);
    double a = radLat1 - radLat2;
    double b = toRadians(lon1) - toRadians(lon2);
    double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
    s = s * (6371007.180918475L);
    s = round(s * 10000) / 10000;
    return s;
}

double ConcurrentHashMapManager::toRadians(double degree)
{
    return degree * M_PI / 180.0;
}

QString ConcurrentHashMapManager::json_object_to_string(QJsonObject qjo, bool compact)
{
    QJsonDocument doc(qjo);
    if (compact) {
        return QString(doc.toJson(QJsonDocument::Compact));
    }
    else {
        return QString(doc.toJson(QJsonDocument::Indented));
    }
}

QJsonObject ConcurrentHashMapManager::string_to_json_object(QString in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if (!doc.isNull())
    {
        if (doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            return obj;
        }
    }
    else
    {
        return obj;
    }
    return obj;
}


std::default_random_engine rd_generator_(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

QByteArray ConcurrentHashMapManager::generate_random_64bit_id()
{
    const static QByteArray candidates = "0123456789abcdef";
    const quint32 nbits = 16;
    static std::uniform_int_distribution<quint32> u(0, nbits - 1); // uniform generation for index [0-15]
    static QByteArray buffer(nbits, 'f'); // 4bits each, 16x4bits = 64bits
    quint32 index = 0;
    for (auto j = 0; j < nbits; ++j) {
        index = u(rd_generator_); // index within 0-15
        memcpy(buffer.data() + j, candidates.data() + index, 1);
    }
    QByteArray data = QByteArray::fromHex(buffer);
    return data;
}

QReadWriteLock ConcurrentHashMapManager::random_uint64_contents_mutex;
QHash<quint64, bool> ConcurrentHashMapManager::random_uint64_contents_;

quint64 ConcurrentHashMapManager::generate_random_positive_uint64()
{
    //  const auto data = generate_random_64bit_id();
    //  quint64 result = 0;
    //  memcpy(&result, data.data(), data.size());
    //  return std::move(result);

    //////////////////////////////////////////////////////////
    auto get_generate_random_64bit_id=[]()->quint64{
        const auto data = generate_random_64bit_id();
        quint64 result = 0;
        memcpy(&result, data.data(), data.size());
        return std::move(result);
    };
    //////////////////////////////////////////////////////////
    quint64 context_script_id_ = 0;
    do
    {
        context_script_id_ = get_generate_random_64bit_id();
    }while(ConcurrentHashMapManager::has_random_positive_uint64(context_script_id_));
    //////////////////////////////////////////////////////////
    ConcurrentHashMapManager::init_random_positive_uint64(context_script_id_);
    return context_script_id_;
}

bool ConcurrentHashMapManager::has_random_positive_uint64(const quint64& random_id)
{
    QReadLocker locker(&random_uint64_contents_mutex);
    return random_uint64_contents_.contains(random_id);
}

void ConcurrentHashMapManager::init_random_positive_uint64(uint64_t random_id)
{
    QWriteLocker locker(&random_uint64_contents_mutex);
    random_uint64_contents_.insert(random_id, true);
}

void ConcurrentHashMapManager::deal_result_slot(const QString &jsobj, void *ptr)
{
    if(jsobj == "AccumulateReset")
    {
        resetAccumulate();
    }
    else if(jsobj.startsWith("cleartask"))
    {
        if(jsobj == "cleartask_all")
        {
            std::vector<UINT64> keys;
            {
                keys.clear();
                keys.reserve(m_task_all_lines.size());
                TASK_LINES_CONCURRENT_HASHMAP_ConstIterator _exist_events_map_itor = m_task_all_lines.begin();
                while (_exist_events_map_itor != m_task_all_lines.end())
                {
                    keys.push_back(_exist_events_map_itor->first);
                    _exist_events_map_itor++;
                }
            }
            for(int i = 0; i < keys.size(); i++)
            {
                auto task_id = keys.at(i);
                TASK_LINES_CONCURRENT_HASHMAP_Accessor caccessor;
                if(!m_task_all_lines.find(caccessor, task_id))
                {
                    m_task_all_lines.erase(caccessor);
                }
            }
        }
        else
        {
            QStringList taskids = jsobj.split("_");
            if(taskids.size() == 2)
            {
                remove_task(taskids.at(1).toULongLong());
            }
        }
    }
}

UINT32 ConcurrentHashMapManager::outputCheckRoutePathPointsResult(tagTaskInfo &_taskinfo,
                                                                oatpp::Object<GeoJson_point_Dto> &points,
                                                                GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance)
{
    EASY_FUNCTION(profiler::colors::Orange)
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USING_GUI_SHOW
#ifdef USING_SHOW_RESULT
    this->reset_data();
#endif
#endif

    points = GeoJson_point_Dto::createShared();
    points->type = "FeatureCollection";
    points->resolution = _taskinfo._resolution;

    points->features = oatpp::Vector<oatpp::Object<GeoJson_features_point_Dto>>::createShared();

    std::vector<HEXINDEX_SENSORS_ENTITYS_INFO> buggrt;
    if(_pGaeactorProcessorInterfaceInstance)
    {
        buggrt = _pGaeactorProcessorInterfaceInstance->getCellbuffersSensorInfo(false);
    }

#ifdef USING_GENERATE_GEOPOINTS_CONCURRENT
    GEOJSON_POINTS_CONCURRENT_HASHMAP _GEOJSON_POINTS_CONCURRENT_HASHMAP;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, buggrt.size()),
                      [&](const tbb::blocked_range<size_t>& r) {
                          for (UINT32 index = r.begin(); index != r.end(); ++index)
                          {
                              auto hexinfo = buggrt.at(index);
#ifdef USING_GUI_SHOW
#ifdef USING_SHOW_RESULT
#ifndef USING_GENERATE_SHOWRESULT_INDEPENDENT
                              prepare_deal_3d(hexinfo);
                              prepare_deal_2d(hexinfo);
#endif
#endif
#endif
                              H3INDEX _h3Index = std::get<0>(hexinfo);
                              bool _bValid = std::get<1>(hexinfo);
                              uint32_t _eHexidexStatus = std::get<2>(hexinfo);
                              auto sensorlists = std::get<3>(hexinfo);
                              auto entitylists = std::get<4>(hexinfo);
                              if (_bValid && _eHexidexStatus == E_HEXINDEX_STATUS_SENSOR)
                              {
                                  if (sensorlists.size() > 1)
                                  {
                                      std::vector<tagSensorConflictInfo> _tagSensorlistInfos;
                                      std::vector<tagSensorConflictInfo> _tagLinesSensorConflictInfos;
                                      _tagSensorlistInfos.reserve(sensorlists.size());
                                      for(int j = 0; j < sensorlists.size(); j++)
                                      {
                                          const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                                          const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                                          const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                                          EVENT_KEY_TYPE linekey = EVENT_KEY_TYPE{agentid, 0, sensingmediaid};
                                          tagLineInfoEx * _tagLineInfo = this->get_data(_taskinfo,linekey);
                                          if(!_tagLineInfo)
                                          {
                                              continue;
                                          }
                                          const bool &bBuiling = _tagLineInfo->m_line.m_tagLinePros.bBuiling;
                                          tagSensorConflictInfo _tagSensorConflictInfo{std::make_tuple(agentid, sensingmediaid), hgt, _tagLineInfo};

                                          if(_tagSensorlistInfos.empty())
                                          {
                                              _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                          }
                                          else
                                          {
                                              bool bHeightConflict = false;
                                              bool bTimeConflict = false;
                                              for(int m = 0; m < _tagSensorlistInfos.size(); m++)
                                              {
                                                  const tagSensorConflictInfo& item = _tagSensorlistInfos.at(m);
                                                  //当前是建筑
                                                  if(bBuiling)
                                                  {
                                                      //只和非建筑比对高度
                                                      if(!item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                                      {
                                                          if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                                          {
                                                              bHeightConflict = true;
                                                          }
                                                      }
                                                      //高度存在冲突
                                                      if(bHeightConflict)
                                                      {
                                                          _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                                          break;
                                                      }
                                                  }
                                                  else
                                                  {
                                                      //非建筑航线需要比对所有的高度
                                                      if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                                      {
                                                          bHeightConflict = true;
                                                      }
                                                      if(!m_bchecktime)
                                                      {
                                                          bTimeConflict = true;
                                                      }
                                                      else
                                                      {
                                                          if(item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                                          {
                                                              bTimeConflict = true;
                                                          }
                                                          else
                                                          {
                                                              if(bHeightConflict &&
                                                                  _tagLineInfo &&
                                                                  item.m_ptagLineInfos &&
                                                                  (!CHECK_IS_OUT_RANGE(item.m_ptagLineInfos->m_line.m_tagLinePros.dt_startTime,
                                                                                       item.m_ptagLineInfos->m_line.m_tagLinePros.dt_endTime,
                                                                                       _tagLineInfo->m_line.m_tagLinePros.dt_startTime,
                                                                                       _tagLineInfo->m_line.m_tagLinePros.dt_endTime)))
                                                              {
                                                                  bTimeConflict = true;
                                                              }
                                                          }
                                                      }

                                                      if(bHeightConflict && bTimeConflict)
                                                      {
                                                          _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                                          break;
                                                      }
                                                  }
                                              }
                                          }
                                          _tagSensorlistInfos.push_back(std::move(_tagSensorConflictInfo));
                                      }

                                      /////////////////////////////////////////////////////////////////////

                                      if(_tagLinesSensorConflictInfos.size() > 1)
                                      {
                                          /////////////////////////////////////////////////////////////////////
                                          LatLng ret;
                                          LocationHelper::doCell(ret, _h3Index);

                                          LatLng location;
                                          location.lng = LocationHelper::radianToDegree(ret.lng);
                                          location.lat = LocationHelper::radianToDegree(ret.lat);

                                          /////////////////////////////////////////////////////////////////////
                                          if(m_breturndirect)
                                          {
                                              auto pointfeature = GeoJson_features_point_Dto::createShared();
                                              pointfeature->type = "Feature";

                                              /////////////////////////////////////////////////////////////////////
                                              pointfeature->properties = GeoJson_point_properties_Dto::createShared();
                                              pointfeature->properties->pointid = index;

                                              QString hexColor = QString("#%1%2%3")
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                                                     .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'));
                                              pointfeature->properties->color = hexColor.toStdString();

                                              /////////////////////////////////////////////////////////////////////
                                              pointfeature->geometry = GeoJson_geometry_point_Dto::createShared();
                                              pointfeature->geometry->type = "Point";
                                              pointfeature->geometry->coordinates = oatpp::Vector<oatpp::Float64>::createShared();
                                              pointfeature->geometry->coordinates->push_back(location.lng);
                                              pointfeature->geometry->coordinates->push_back(location.lat);
                                              /////////////////////////////////////////////////////////////////////

                                              pointfeature->properties->pointlinestrings = oatpp::Vector<oatpp::Object<GeoJson_points_linestring_id_Dto>>::createShared();
                                              for(int j = 0; j < _tagLinesSensorConflictInfos.size(); j++)
                                              {
                                                  const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagLinesSensorConflictInfos.at(j).m_sensorid;
                                                  const transdata_param_seq_hexidx_hgt& hgt = _tagLinesSensorConflictInfos.at(j).m_hgtrange;

                                                  auto _GeoJson_points_linestring_id_Dto = GeoJson_points_linestring_id_Dto::createShared();
                                                  const tagLineInfoEx * _tagLineInfo = _tagLinesSensorConflictInfos.at(j).m_ptagLineInfos;
                                                  if(nullptr != _tagLinesSensorConflictInfos.at(j).m_ptagLineInfos)
                                                  {
                                                      oatpp::String str = ConcurrentHashMapManager::json_object_to_string(_tagLineInfo->m_line.featurecollectionpros).toStdString();
                                                      _GeoJson_points_linestring_id_Dto->featurecollectionpros = m_jsonmapper->readFromString<oatpp::Object<GeoJson_FeatureCollection_properties_Dto>>(str);

                                                      auto _hex_hgt_itor = _tagLineInfo->m_line._hex_hgt.find(_h3Index);
                                                      if(_hex_hgt_itor != _tagLineInfo->m_line._hex_hgt.end())
                                                      {
                                                          _GeoJson_points_linestring_id_Dto->height = _hex_hgt_itor->second;
                                                      }
                                                  }
                                                  else
                                                  {
                                                      _GeoJson_points_linestring_id_Dto->height = 0.0;
                                                  }
                                                  _GeoJson_points_linestring_id_Dto->heightlimitdown = hgt.PARAM_seq_hexidx_hgt0;
                                                  _GeoJson_points_linestring_id_Dto->heightlimitup = hgt.PARAM_seq_hexidx_hgtn;

                                                  _GeoJson_points_linestring_id_Dto->agentrouteids = GeoJson_linestring_id_Dto::createShared();
                                                  _GeoJson_points_linestring_id_Dto->agentrouteids->agentid = QString::number(std::get<0>(exist_sensorid)).toStdString();
                                                  _GeoJson_points_linestring_id_Dto->agentrouteids->routeid = (int32_t)(std::get<1>(exist_sensorid));

                                                  pointfeature->properties->pointlinestrings->push_back(_GeoJson_points_linestring_id_Dto);
                                              }

                                              {
                                                  GEOJSON_POINTS_CONCURRENT_HASHMAP_Accessor _ret_tmp_accessor;
                                                  if(!_GEOJSON_POINTS_CONCURRENT_HASHMAP.find(_ret_tmp_accessor, index))
                                                  {
                                                      _GEOJSON_POINTS_CONCURRENT_HASHMAP.insert(_ret_tmp_accessor,index);
                                                      _ret_tmp_accessor->second = std::move(pointfeature);
                                                  }
                                              }
                                          }

                                          /////////////////////////////////////////////////////////////////////

                                          {
                                              _taskinfo.appendpt(index, location, _h3Index);

                                              tagTaskResultPtInfo* _tagTaskResultInfo = _taskinfo.getPtInfo(index);
                                              if(_tagTaskResultInfo)
                                              {
                                                  for(int j = 0; j < _tagLinesSensorConflictInfos.size(); j++)
                                                  {
                                                      tagLineInfoEx * _tagLineInfo = _tagLinesSensorConflictInfos.at(j).m_ptagLineInfos;

                                                      const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagLinesSensorConflictInfos.at(j).m_sensorid;
                                                      const transdata_param_seq_hexidx_hgt& hgt = _tagLinesSensorConflictInfos.at(j).m_hgtrange;

                                                      auto _GeoJson_points_linestring_id_Dto = GeoJson_points_linestring_id_Dto::createShared();
                                                      if(nullptr != _tagLinesSensorConflictInfos.at(j).m_ptagLineInfos)
                                                      {
                                                          oatpp::String str = ConcurrentHashMapManager::json_object_to_string(_tagLineInfo->m_line.featurecollectionpros).toStdString();
                                                          _GeoJson_points_linestring_id_Dto->featurecollectionpros = m_jsonmapper->readFromString<oatpp::Object<GeoJson_FeatureCollection_properties_Dto>>(str);

                                                          auto _hex_hgt_itor = _tagLineInfo->m_line._hex_hgt.find(_h3Index);
                                                          if(_hex_hgt_itor != _tagLineInfo->m_line._hex_hgt.end())
                                                          {
                                                              _GeoJson_points_linestring_id_Dto->height = _hex_hgt_itor->second;
                                                          }
                                                      }
                                                      else
                                                      {
                                                          _GeoJson_points_linestring_id_Dto->height = 0.0;
                                                      }
                                                      _GeoJson_points_linestring_id_Dto->heightlimitdown = hgt.PARAM_seq_hexidx_hgt0;
                                                      _GeoJson_points_linestring_id_Dto->heightlimitup = hgt.PARAM_seq_hexidx_hgtn;

                                                      _GeoJson_points_linestring_id_Dto->agentrouteids = GeoJson_linestring_id_Dto::createShared();
                                                      _GeoJson_points_linestring_id_Dto->agentrouteids->agentid = QString::number(std::get<0>(exist_sensorid)).toStdString();
                                                      _GeoJson_points_linestring_id_Dto->agentrouteids->routeid = (int32_t)(std::get<1>(exist_sensorid));

                                                      _tagLineInfo->appendTaskPt(index, _tagTaskResultInfo);
                                                      _tagTaskResultInfo->appendline(j,_tagLineInfo, std::move(_GeoJson_points_linestring_id_Dto));
                                                  }
                                              }
                                          }
                                      }
                                  }
                              }
                          }
                      });

    {
        GEOJSON_POINTS_CONCURRENT_HASHMAP_ConstIterator _ret_tmp_itor = _GEOJSON_POINTS_CONCURRENT_HASHMAP.begin();
        while (_ret_tmp_itor != _GEOJSON_POINTS_CONCURRENT_HASHMAP.end())
        {
            points->features->push_back(std::move(_ret_tmp_itor->second));
            _ret_tmp_itor++;
        }
    }
#else
    int point_id = 0;
    for(int index = 0;index < buggrt.size();index++)
    {
        auto hexinfo = buggrt.at(index);

#ifdef USING_SHOW_RESULT
        prepare_deal_3d(hexinfo);
        prepare_deal_2d(hexinfo);
#endif
        H3INDEX _h3Index = std::get<0>(hexinfo);
        bool _bValid = std::get<1>(hexinfo);
        uint32_t _eHexidexStatus = std::get<2>(hexinfo);
        auto sensorlists = std::get<3>(hexinfo);
        auto entitylists = std::get<4>(hexinfo);
        if (_bValid && _eHexidexStatus == E_HEXINDEX_STATUS_SENSOR)
        {
            if (sensorlists.size() > 1)
            {
                /////////////////////////////////////////////////////////////////////
                std::vector<tagSensorConflictInfo> _tagSensorlistInfos;

                std::vector<tagSensorConflictInfo> _tagSensorConflictInfos;
                _tagSensorlistInfos.reserve(sensorlists.size());
                for(int j = 0; j < sensorlists.size(); j++)
                {
                    const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                    const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                    const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                    tagSensorConflictInfo _tagSensorConflictInfo;
                    _tagSensorConflictInfo.m_sensorid = std::make_tuple(agentid, sensingmediaid);
                    _tagSensorConflictInfo.m_hgtrange = hgt;

                    if(_tagSensorlistInfos.empty())
                    {
                        _tagSensorConflictInfos.push_back(_tagSensorConflictInfo);
                    }
                    else
                    {
                        for(int m = 0; m < _tagSensorlistInfos.size(); m++)
                        {
                            const transdata_param_seq_hexidx_hgt& exist_hgt = _tagSensorlistInfos.at(m).m_hgtrange;
                            if(!((hgt.PARAM_seq_hexidx_hgt0 < exist_hgt.PARAM_seq_hexidx_hgt0 && hgt.PARAM_seq_hexidx_hgtn < exist_hgt.PARAM_seq_hexidx_hgt0) ||
                                  (hgt.PARAM_seq_hexidx_hgt0 > exist_hgt.PARAM_seq_hexidx_hgtn && hgt.PARAM_seq_hexidx_hgtn > exist_hgt.PARAM_seq_hexidx_hgtn)))
                            {
                                _tagSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                break;
                            }
                        }
                    }
                    _tagSensorlistInfos.push_back(std::move(_tagSensorConflictInfo));
                }

                /////////////////////////////////////////////////////////////////////

                if(_tagSensorConflictInfos.size() > 1)
                {
                    auto pointfeature = GeoJson_features_point_Dto::createShared();
                    pointfeature->type = "Feature";

                    /////////////////////////////////////////////////////////////////////
                    pointfeature->properties = GeoJson_point_properties_Dto::createShared();
                    point_id++;
                    pointfeature->properties->pointid = point_id;

                    QString hexColor = QString("#%1%2%3")
                                           .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                           .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'))
                                           .arg(ConcurrentHashMapManager::generateRandomNumber(), 2, 16, QLatin1Char('0'));
                    pointfeature->properties->color = hexColor.toStdString();

                    /////////////////////////////////////////////////////////////////////

                    pointfeature->properties->pointlinestrings = oatpp::Vector<oatpp::Object<GeoJson_points_linestring_id_Dto>>::createShared();
                    for(int j = 0; j < _tagSensorConflictInfos.size(); j++)
                    {
                        const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(j).m_sensorid;
                        const transdata_param_seq_hexidx_hgt& hgt = _tagSensorConflictInfos.at(j).m_hgtrange;

                        auto _GeoJson_points_linestring_id_Dto = GeoJson_points_linestring_id_Dto::createShared();
                        {
                            EVENT_KEY_TYPE linekey = EVENT_KEY_TYPE{std::get<0>(exist_sensorid), 0, (TYPE_ULID)std::get<1>(exist_sensorid)};
                            if(m_all_lines.find(linekey) != m_all_lines.end())
                            {
                                tagLineInfo & _tagLineInfo = m_all_lines.at(linekey);

                                oatpp::String str = ConcurrentHashMapManager::json_object_to_string(_tagLineInfo.featurecollectionpros).toStdString();
                                _GeoJson_points_linestring_id_Dto->featurecollectionpros = m_jsonmapper->readFromString<oatpp::Object<GeoJson_FeatureCollection_properties_Dto>>(str);

                                auto _hex_hgt_itor = _tagLineInfo._hex_hgt.find(_h3Index);
                                if(_hex_hgt_itor != _tagLineInfo._hex_hgt.end())
                                {
                                    _GeoJson_points_linestring_id_Dto->height = _hex_hgt_itor->second;
                                }
                            }
                            else
                            {
                                _GeoJson_points_linestring_id_Dto->height = 0.0;
                            }
                        }
                        _GeoJson_points_linestring_id_Dto->heightlimitdown = hgt.PARAM_seq_hexidx_hgt0;
                        _GeoJson_points_linestring_id_Dto->heightlimitup = hgt.PARAM_seq_hexidx_hgtn;

                        _GeoJson_points_linestring_id_Dto->agentrouteids = GeoJson_linestring_id_Dto::createShared();
                        _GeoJson_points_linestring_id_Dto->agentrouteids->agentid = QString::number(std::get<0>(exist_sensorid)).toStdString();
                        _GeoJson_points_linestring_id_Dto->agentrouteids->routeid = (int32_t)(std::get<1>(exist_sensorid));

                        pointfeature->properties->pointlinestrings->push_back(_GeoJson_points_linestring_id_Dto);
                    }

                    /////////////////////////////////////////////////////////////////////

                    LatLng ret;
                    LocationHelper::doCell(ret, _h3Index);

                    LatLng location;
                    location.lng = LocationHelper::radianToDegree(ret.lng);
                    location.lat = LocationHelper::radianToDegree(ret.lat);
                    pointfeature->geometry = GeoJson_geometry_point_Dto::createShared();
                    pointfeature->geometry->type = "Point";
                    pointfeature->geometry->coordinates = oatpp::Vector<oatpp::Float64>::createShared();
                    pointfeature->geometry->coordinates->push_back(location.lng);
                    pointfeature->geometry->coordinates->push_back(location.lat);
                    points->features->push_back(std::move(pointfeature));
                }
            }
        }
    }
#endif
    return _taskinfo.m_taskresultpts.size();
}

DECLARE_TYPEDEF_TBB_HASH_MAP(H3INDEX,transdata_param_seq_hexidx, HEXINDEX_HGT_CONCURRENT_HASHMAP)

UINT32 ConcurrentHashMapManager::deal_linestring(const UINT32& cur_daty_dt_startTime,
                                                 const UINT32& cur_daty_dt_endTime,
                                                 tagTaskInfo &_taskinfo,
                                                 const UINT32 &lineindex,
                                                 QJsonObject &featuresitemobj,
                                                 GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance,
                                                 const double& _heightlimitup,
                                                 const double& _heightlimitdown)
{
    UINT32 hexcount =  0;
    EASY_FUNCTION(profiler::colors::Blue)
    auto deal_line = [&](const TYPE_ULID& agentid,const TYPE_ULID& routeid, tagLineInfo & _tagLineInfo,const double& heightlimitup, const double& heightlimitdown)->UINT32
    {
        EASY_FUNCTION(profiler::colors::DeepPurple)

        HEXIDX_HGT_ARRAY  _hexidxs;
        HEXINDEX_HGT_CONCURRENT_HASHMAP _hexindex_hgt_concurrent_hash_map;
        tbb::concurrent_hash_map<H3INDEX,bool> _hexindex_hgt_concurrent_hash_map2;

        std::vector<tagPtLatLngHgtInfo> &_line_latlnghgts = _tagLineInfo._line_latlnghgts;
        tbb::parallel_for(tbb::blocked_range<size_t>(0, _line_latlnghgts.size()-1),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  std::vector<LatLng> _line_section;
                                  _line_section.push_back(_line_latlnghgts.at(j)._latlng);
                                  _line_section.push_back(_line_latlnghgts.at(j + 1)._latlng);
                                  FLOAT64 &hgt_start = _line_latlnghgts.at(j)._hgt;
                                  FLOAT64 &hgt_end = _line_latlnghgts.at(j+1)._hgt;

                                  FLOAT64 &hgt_up_start = _line_latlnghgts.at(j)._hgt_up;
                                  FLOAT64 &hgt_up_end = _line_latlnghgts.at(j+1)._hgt_up;

                                  FLOAT64 &hgt_down_start = _line_latlnghgts.at(j)._hgt_down;
                                  FLOAT64 &hgt_down_end = _line_latlnghgts.at(j+1)._hgt_down;

                                  FLOAT64 hgt_diff = hgt_end-hgt_start;
                                  FLOAT64 hgt_diff_up = hgt_up_end-hgt_up_start;
                                  FLOAT64 hgt_diff_down = hgt_down_end-hgt_down_start;

                                  HEXIDX_ARRAY hexidxslist;
                                  LocationHelper::getPathIndex(hexidxslist, _line_section, _taskinfo._resolution);
                                  if(hexidxslist.size() > 0)
                                  {
                                      double distance = ConcurrentHashMapManager::calc_dist(_line_section.at(0).lat, _line_section.at(0).lng, _line_section.at(1).lat, _line_section.at(1).lng);
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 m = q.begin(); m != q.end(); m++)
                                                            {
                                                                const H3INDEX& _h3Index = hexidxslist[m];
                                                                LatLng ret;
                                                                LocationHelper::doCell(ret, _h3Index);

                                                                LatLng location;
                                                                location.lng = LocationHelper::radianToDegree(ret.lng);
                                                                location.lat = LocationHelper::radianToDegree(ret.lat);

                                                                double cur_distance = ConcurrentHashMapManager::calc_dist(location.lat, location.lng, _line_section.at(0).lat, _line_section.at(0).lng);
                                                                FLOAT64 hgt_ = hgt_start + hgt_diff * (cur_distance / distance);
                                                                transdata_param_seq_hexidx _transdata_param_seq_hexidx;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_element = _h3Index;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt = hgt_;
                                                                FLOAT64 hgt_up = hgt_up_start + hgt_diff_up * (cur_distance / distance);
                                                                FLOAT64 hgt_down = hgt_down_start + hgt_diff_down * (cur_distance / distance);
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt0 = hgt_down;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgtn = hgt_up;

                                                                {
                                                                    HEXINDEX_HGT_CONCURRENT_HASHMAP_Accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                    if(!_hexindex_hgt_concurrent_hash_map.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                    {
                                                                        _hexindex_hgt_concurrent_hash_map.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                        _hexindex_hgt_concurrent_hash_map_accessot->second = std::move(_transdata_param_seq_hexidx);
                                                                    }
                                                                }
                                                            }
                                                        });
                                  }

                                  /////////////////////////////////////////////////////////////////////////////////////////////////////////
                                  HEXIDX_ARRAY hexidxslist2;
                                  LocationHelper::getPathIndex(hexidxslist2, _line_section, m_bresolution);
                                  if(hexidxslist2.size() > 0)
                                  {
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist2.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 m = q.begin(); m != q.end(); m++)
                                                            {
                                                                const H3INDEX& _h3Index = hexidxslist2[m];
                                                                tbb::concurrent_hash_map<H3INDEX,bool>::accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                if(!_hexindex_hgt_concurrent_hash_map2.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                {
                                                                    _hexindex_hgt_concurrent_hash_map2.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                    _hexindex_hgt_concurrent_hash_map_accessot->second = true;
                                                                }
                                                            }
                                                        });
                                  }
                                  /////////////////////////////////////////////////////////////////////////////////////////////////////////
                              }
                          });


        _hexidxs.reserve(_hexindex_hgt_concurrent_hash_map.size());
        {
            HEXINDEX_HGT_CONCURRENT_HASHMAP_Iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map.begin();
            while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map.end())
            {
                const H3INDEX& _h3Index = _ret_tmp_itor->first;
                transdata_param_seq_hexidx &_transdata_param_seq_hexidx = _ret_tmp_itor->second;
                auto _hex_hgt_itor = _tagLineInfo._hex_hgt.find(_h3Index);
                if(_hex_hgt_itor == _tagLineInfo._hex_hgt.end())
                {
                    _tagLineInfo._hex_hgt.insert(std::make_pair(_h3Index, _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt));
                }
                else
                {
                    _hex_hgt_itor->second = _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt;
                }
                _hexidxs.push_back(std::move(_transdata_param_seq_hexidx));
                _ret_tmp_itor++;
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        _tagLineInfo.m_hexidxs.reserve(_hexindex_hgt_concurrent_hash_map2.size());
        {
            tbb::concurrent_hash_map<H3INDEX,bool>::const_iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map2.begin();
            while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map2.end())
            {
                const H3INDEX& _h3Index = _ret_tmp_itor->first;
                _tagLineInfo.m_hexidxs.push_back(std::move(_h3Index));
                _ret_tmp_itor++;
            }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        IDENTIFI_EVENT_INFO eventinfo;
        if(_pGaeactorProcessorInterfaceInstance)
        {
            _pGaeactorProcessorInterfaceInstance->update_hexindex_sensor(agentid, routeid, _hexidxs, transdata_sensorposinfo(), POLYGON_LIST(), eventinfo);
        }
        return _hexidxs.size();
    };
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto geometry = featuresitemobj.value("geometry").toObject();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto properties = featuresitemobj.value("properties").toObject();
    auto featurecollectionpros = properties.value("featurecollectionpros").toObject();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    tagLinePros _tagLinePros;
    {
        _tagLinePros.code = featurecollectionpros.value("code").toString();
        _tagLinePros.startTime = featurecollectionpros.value("startTime").toString();
        _tagLinePros.endTime = featurecollectionpros.value("endTime").toString();
        _tagLinePros.createTime = featurecollectionpros.value("createTime").toString();
        _tagLinePros.Priority = featurecollectionpros.value("Priority").toString();
        _tagLinePros.prase();
    }
    if(!isRouteDeal(cur_daty_dt_startTime,cur_daty_dt_endTime, _tagLinePros))
    {
        return hexcount;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto heightlimitup = properties.value("heightlimitup").toDouble(_heightlimitup);
    auto heightlimitdown = properties.value("heightlimitdown").toDouble(_heightlimitdown);
    auto heights = properties.value("heights").toArray();
    auto agentrouteid = properties.value("agentrouteid").toObject();
    auto agentid = agentrouteid.value("agentid").toString().toULongLong();
    auto routeid = agentrouteid.value("routeid").toInt();
    EVENT_KEY_TYPE key = EVENT_KEY_TYPE{agentid, 0, (TYPE_ULID)routeid};
    tagLineInfoEx & _tagLineInfo = get_data_or_create(_taskinfo,key,_tagLinePros.code.toStdString());


    {
        tbb::concurrent_hash_map<std::string, tagLineInfoEx*>::accessor _codeaccessor;
        if(!_taskinfo.m_codeline.find(_codeaccessor, _tagLinePros.code.toStdString()))
        {
            _taskinfo.m_codeline.insert(_codeaccessor, _tagLinePros.code.toStdString());
            _codeaccessor->second = &_tagLineInfo;
        }
    }
//    _tagLineInfo.m_line._id = ConcurrentHashMapManager::generate_random_positive_uint64();
    _tagLineInfo.m_line._id = lineindex;
    _tagLineInfo.m_line.featurecollectionpros = std::move(featurecollectionpros);
    _tagLineInfo.m_line.m_tagLinePros = std::move(_tagLinePros);

    auto coordinates = geometry.value("coordinates").toArray();
    if(!coordinates.isEmpty())
    {
        _tagLineInfo.m_line._line_latlnghgts.reserve(coordinates.size());
        for(int index = 0; index < coordinates.size(); index++)
        {
            auto pt = coordinates.at(index).toArray();
            tagPtLatLngHgtInfo _tagPtLatLngHgtInfo;
            bool bAppend = false;
            if(pt.size() == 2)
            {
                double _hgt = 0;
                if(heights.size() == coordinates.size())
                {
                    _hgt = heights.at(index).toDouble();
                }
                _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                _tagPtLatLngHgtInfo._hgt = _hgt;
                bAppend = true;
            }
            else if(pt.size() == 3)
            {
                _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                _tagPtLatLngHgtInfo._hgt = pt[2].toDouble();
                bAppend = true;
            }

            if(bAppend)
            {
                if(!_tagLineInfo.m_line._line_latlnghgts.empty())
                {
                    _tagLineInfo.m_line.m_distance += ConcurrentHashMapManager::calc_dist(_tagLineInfo.m_line._line_latlnghgts.back()._latlng.lat, _tagLineInfo.m_line._line_latlnghgts.back()._latlng.lng, _tagPtLatLngHgtInfo._latlng.lat, _tagPtLatLngHgtInfo._latlng.lng);
                }
                else
                {
                    _tagLineInfo.m_line.m_distance = 0.0;
                }
                {
                    _tagPtLatLngHgtInfo._hgt_up = (_tagPtLatLngHgtInfo._hgt + heightlimitup);
                    _tagPtLatLngHgtInfo._hgt_down = (_tagPtLatLngHgtInfo._hgt - heightlimitdown);

                    _tagLineInfo.m_line._line_latlnghgts.push_back(std::move(_tagPtLatLngHgtInfo));
                }
            }
        }
    }
    hexcount = deal_line(agentid,routeid, _tagLineInfo.m_line,heightlimitup, heightlimitdown);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto newproperties = featuresitemobj.value("properties").toObject();
    QJsonArray linehex;
    for(auto hexindex:_tagLineInfo.m_line.m_hexidxs)
    {
        linehex.append(QString::number(hexindex));
    }
    newproperties.insert("linehex",linehex);
    featuresitemobj.insert("properties",newproperties);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    _tagLineInfo.m_line.featuresitemobj = std::move(featuresitemobj);
    return hexcount;
}

UINT32 ConcurrentHashMapManager::deal_polygon(const UINT32 &cur_daty_dt_startTime, const UINT32 &cur_daty_dt_endTime, tagTaskInfo &_taskinfo, const UINT32 &lineindex, QJsonObject &featuresitemobj, GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance, const double &_heightlimitup, const double &_heightlimitdown)
{
    UINT32 hexcount =  0;
    EASY_FUNCTION(profiler::colors::Blue)
    auto deal_ploy = [&](const TYPE_ULID& agentid,const TYPE_ULID& routeid, tagLineInfo & _tagLineInfo,const double& heightlimitup, const double& heightlimitdown)->UINT32
    {
        EASY_FUNCTION(profiler::colors::DeepPurple)

        HEXIDX_HGT_ARRAY  _hexidxs;
        HEXINDEX_HGT_CONCURRENT_HASHMAP _hexindex_hgt_concurrent_hash_map;
        tbb::concurrent_hash_map<H3INDEX,bool> _hexindex_hgt_concurrent_hash_map2;
        tbb::parallel_for(tbb::blocked_range<size_t>(0, _tagLineInfo._polygon_latlnghgts.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  const std::vector<tagPtLatLngHgtInfo> & _polygon_latlng = _tagLineInfo._polygon_latlnghgts.at(j);
                                  FLOAT64 hgt_start = 0;
                                  FLOAT64 hgt_up_start = 0;
                                  FLOAT64 hgt_down_start = 0;

                                  std::vector<LatLng> _polygon_latlng_rads;
                                  _polygon_latlng_rads.resize(_polygon_latlng.size());
                                  for(int m = 0; m < _polygon_latlng.size(); m++)
                                  {
                                      hgt_start = _polygon_latlng.at(m)._hgt;
                                      hgt_up_start = _polygon_latlng.at(m)._hgt_up;
                                      hgt_down_start = _polygon_latlng.at(m)._hgt_down;

                                      _polygon_latlng_rads[m].lat = LocationHelper::degreeToRadian(_polygon_latlng[m]._latlng.lat);
                                      _polygon_latlng_rads[m].lng = LocationHelper::degreeToRadian(_polygon_latlng[m]._latlng.lng);
                                  }

                                  HEXIDX_ARRAY hexidxslist;
                                  LocationHelper::getPolygonResulutionIndex(hexidxslist, _polygon_latlng_rads, _taskinfo._resolution);
                                  if(hexidxslist.size() > 0)
                                  {
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 m = q.begin(); m != q.end(); m++)
                                                            {
                                                                const H3INDEX& _h3Index = hexidxslist[m];
                                                                transdata_param_seq_hexidx _transdata_param_seq_hexidx;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_element = _h3Index;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt = hgt_start;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt0 = hgt_down_start;
                                                                _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgtn = hgt_up_start;
                                                                {
                                                                    HEXINDEX_HGT_CONCURRENT_HASHMAP_Accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                    if(!_hexindex_hgt_concurrent_hash_map.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                    {
                                                                        _hexindex_hgt_concurrent_hash_map.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                        _hexindex_hgt_concurrent_hash_map_accessot->second = std::move(_transdata_param_seq_hexidx);
                                                                    }
                                                                }
                                                            }
                                                        });
                                  }

                                  /////////////////////////////////////////////////////////////////////////////////////////////////////////
                                  HEXIDX_ARRAY hexidxslist2;
                                  LocationHelper::getPolygonResulutionIndex(hexidxslist2, _polygon_latlng_rads, m_bresolution);
                                  if(hexidxslist2.size() > 0)
                                  {
                                      tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist2.size()),
                                                        [&](const tbb::blocked_range<size_t>& q) {
                                                            for (UINT32 m = q.begin(); m != q.end(); m++)
                                                            {
                                                                const H3INDEX& _h3Index = hexidxslist2[m];
                                                                tbb::concurrent_hash_map<H3INDEX,bool>::accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                if(!_hexindex_hgt_concurrent_hash_map2.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                {
                                                                    _hexindex_hgt_concurrent_hash_map2.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                    _hexindex_hgt_concurrent_hash_map_accessot->second = true;
                                                                }
                                                            }
                                                        });
                                  }
                                  /////////////////////////////////////////////////////////////////////////////////////////////////////////
                              }
                          });


        _hexidxs.reserve(_hexindex_hgt_concurrent_hash_map.size());
        {
            HEXINDEX_HGT_CONCURRENT_HASHMAP_Iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map.begin();
            while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map.end())
            {
                const H3INDEX& _h3Index = _ret_tmp_itor->first;
                transdata_param_seq_hexidx &_transdata_param_seq_hexidx = _ret_tmp_itor->second;
                auto _hex_hgt_itor = _tagLineInfo._hex_hgt.find(_h3Index);
                if(_hex_hgt_itor == _tagLineInfo._hex_hgt.end())
                {
                    _tagLineInfo._hex_hgt.insert(std::make_pair(_h3Index, _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt));
                }
                else
                {
                    _hex_hgt_itor->second = _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt;
                }
                _hexidxs.push_back(std::move(_transdata_param_seq_hexidx));
                _ret_tmp_itor++;
            }
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        _tagLineInfo.m_hexidxs.reserve(_hexindex_hgt_concurrent_hash_map2.size());
        {
            tbb::concurrent_hash_map<H3INDEX,bool>::const_iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map2.begin();
            while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map2.end())
            {
                const H3INDEX& _h3Index = _ret_tmp_itor->first;
                _tagLineInfo.m_hexidxs.push_back(std::move(_h3Index));
                _ret_tmp_itor++;
            }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        IDENTIFI_EVENT_INFO eventinfo;
        if(_pGaeactorProcessorInterfaceInstance)
        {
            _pGaeactorProcessorInterfaceInstance->update_hexindex_sensor(agentid, routeid, _hexidxs, transdata_sensorposinfo(), POLYGON_LIST(), eventinfo);
        }
        return _hexidxs.size();
    };
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto geometry = featuresitemobj.value("geometry").toObject();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto properties = featuresitemobj.value("properties").toObject();
    auto featurecollectionpros = properties.value("featurecollectionpros").toObject();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    tagLinePros _tagLinePros;
    {
        _tagLinePros.code = featurecollectionpros.value("code").toString();
        _tagLinePros.startTime = featurecollectionpros.value("startTime").toString();
        _tagLinePros.endTime = featurecollectionpros.value("endTime").toString();
        _tagLinePros.createTime = featurecollectionpros.value("createTime").toString();
        _tagLinePros.Priority = featurecollectionpros.value("Priority").toString();
        _tagLinePros.prase();
    }
    if(!isRouteDeal(cur_daty_dt_startTime,cur_daty_dt_endTime, _tagLinePros))
    {
        return hexcount;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto heightlimitup = properties.value("heightlimitup").toDouble(_heightlimitup);
    auto heightlimitdown = properties.value("heightlimitdown").toDouble(_heightlimitdown);
    auto heights = properties.value("heights").toArray();
    auto agentrouteid = properties.value("agentrouteid").toObject();
    auto agentid = agentrouteid.value("agentid").toString().toULongLong();
    auto routeid = agentrouteid.value("routeid").toInt();
    EVENT_KEY_TYPE key = EVENT_KEY_TYPE{agentid, 0, (TYPE_ULID)routeid};
    tagLineInfoEx & _tagLineInfo = get_data_or_create(_taskinfo,key,_tagLinePros.code.toStdString());


    {
        tbb::concurrent_hash_map<std::string, tagLineInfoEx*>::accessor _codeaccessor;
        if(!_taskinfo.m_codeline.find(_codeaccessor, _tagLinePros.code.toStdString()))
        {
            _taskinfo.m_codeline.insert(_codeaccessor, _tagLinePros.code.toStdString());
            _codeaccessor->second = &_tagLineInfo;
        }
    }
    //    _tagLineInfo.m_line._id = ConcurrentHashMapManager::generate_random_positive_uint64();
    _tagLineInfo.m_line._id = lineindex;
    _tagLineInfo.m_line.featurecollectionpros = std::move(featurecollectionpros);
    _tagLineInfo.m_line.m_tagLinePros = std::move(_tagLinePros);

    auto polygoncoordinates = geometry.value("coordinates_polygon").toArray();
    if(!polygoncoordinates.isEmpty())
    {
        _tagLineInfo.m_line._polygon_latlnghgts.reserve(polygoncoordinates.size());
        for(int j = 0; j< polygoncoordinates.size();j++)
        {
            std::vector<tagPtLatLngHgtInfo> _subpolygon_latlnghgts;
            double _hgt = 0;
            if(heights.size() == polygoncoordinates.size())
            {
                _hgt = heights.at(j).toDouble();
            }
            auto coordinates =  polygoncoordinates.at(j).toArray();
            _subpolygon_latlnghgts.reserve(coordinates.size());
            for(int index = 0; index < coordinates.size(); index++)
            {
                auto pt = coordinates.at(index).toArray();
                tagPtLatLngHgtInfo _tagPtLatLngHgtInfo;
                bool bAppend = false;
                if(pt.size() == 2)
                {
                    _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                    _tagPtLatLngHgtInfo._hgt = _hgt;
                    bAppend = true;
                }
                else if(pt.size() == 3)
                {
                    _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                    _tagPtLatLngHgtInfo._hgt = pt[2].toDouble();
                    bAppend = true;
                }

                if(bAppend)
                {
//                    _tagPtLatLngHgtInfo._hgt_up = (_tagPtLatLngHgtInfo._hgt + heightlimitup);
//                    _tagPtLatLngHgtInfo._hgt_down = (_tagPtLatLngHgtInfo._hgt - heightlimitdown);

                    _tagPtLatLngHgtInfo._hgt_up = _tagPtLatLngHgtInfo._hgt;
                    _tagPtLatLngHgtInfo._hgt_down = 0;
                    _subpolygon_latlnghgts.push_back(std::move(_tagPtLatLngHgtInfo));
                }
            }
            if(!_subpolygon_latlnghgts.empty())
            {
                _tagLineInfo.m_line._polygon_latlnghgts.push_back(std::move(_subpolygon_latlnghgts));
            }
        }
    }
    hexcount = deal_ploy(agentid,routeid, _tagLineInfo.m_line, heightlimitup, heightlimitdown);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto newproperties = featuresitemobj.value("properties").toObject();
    QJsonArray linehex;
    for(auto hexindex:_tagLineInfo.m_line.m_hexidxs)
    {
        linehex.append(QString::number(hexindex));
    }
    newproperties.insert("linehex",linehex);
    featuresitemobj.insert("properties",newproperties);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    _tagLineInfo.m_line.featuresitemobj = std::move(featuresitemobj);
    return hexcount;
}

void ConcurrentHashMapManager::deal_outputCheckRoutePathResult(tagTaskInfo &_taskinfo,
                                                               UINT64 beginTimestamp,
                                                               oatpp::Object<GeoJson_Return_Dto> &_GeoJson_Return_Dto,
                                                               GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance)
{
    auto inputfinishedTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::stringstream ss2;
    ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input finished cost "<<inputfinishedTimeStamp - beginTimestamp<<" ms \n";
    //TRACE_LOG_PRINT_EX2(ss2);
    std::cout<<ss2.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(m_bechoroutes)
    {
        outputCheckRoutePathRoutesResult(_taskinfo, _GeoJson_Return_Dto->routes );
    }
    _GeoJson_Return_Dto->taskid = QString::number(_taskinfo._task_id).toStdString();
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    _GeoJson_Return_Dto->count = outputCheckRoutePathPointsResult(_taskinfo, _GeoJson_Return_Dto->points,_pGaeactorProcessorInterfaceInstance);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    auto outputfinishedTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::stringstream ss3;
    ss3<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" output finished cost "<<outputfinishedTimeStamp - inputfinishedTimeStamp<<" ms \noutput conflicts count "<<_GeoJson_Return_Dto->count<<"\n";
//    ss3<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" output finished cost "<<outputfinishedTimeStamp - inputfinishedTimeStamp<<" ms \n";
    //TRACE_LOG_PRINT_EX2(ss3);
    std::cout<<ss3.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USING_GUI_SHOW
#ifdef USING_SHOW_RESULT
    auto itor = _taskinfo.m_lines.begin();
    while(itor != _taskinfo.m_lines.end())
    {
        if(!itor->second.m_line.m_tagLinePros.bBuiling)
        {
            emit draw_linestring_sig(itor->second.m_line);
        }
        itor++;
    }
#endif
#endif

    if(_pGaeactorProcessorInterfaceInstance == m_pGaeactorProcessorInterfaceInstanceAccumulate)
    {
        QString jsobj = "showresult_"+QString::number(_taskinfo._task_id);
        emit deal_result_sig(jsobj, _pGaeactorProcessorInterfaceInstance);
    }
    else
    {
        QString jsobj = "showclearresult_"+QString::number(_taskinfo._task_id);
        emit deal_result_sig(jsobj, _pGaeactorProcessorInterfaceInstance);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto endfinishedTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    std::stringstream ss4;
    ss4<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" reset finished cost "<<endfinishedTimeStamp - outputfinishedTimeStamp<<" ms \ntotal cost "<<endfinishedTimeStamp-beginTimestamp<<" ms \n";
    //TRACE_LOG_PRINT_EX2(ss4);
    std::cout<<ss4.str()<<std::endl;
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
}


bool ConcurrentHashMapManager::isRouteDeal(const UINT32& cur_daty_dt_startTime,
                                           const UINT32& cur_daty_dt_endTime,
                                           const tagLinePros &_tagLinePros)
{
    bool bDeal = true;

    if(!m_bchecktime)
    {
        return bDeal;
    }
    //航线的起止时间 都在当天之前 或者 航线的起止时间当天之后
    if(!_tagLinePros.bBuiling && CHECK_IS_OUT_RANGE(cur_daty_dt_startTime, cur_daty_dt_endTime, _tagLinePros.dt_startTime,_tagLinePros.endTime))
    {
        std::stringstream ss4;
        ss4<<_tagLinePros.code.toStdString()<<" time not in today !!! startTime: "<<_tagLinePros.startTime.toStdString()<<" endTime: "<<_tagLinePros.endTime.toStdString()<<" \n";
        //TRACE_LOG_PRINT_EX2(ss4);
        std::cout<<ss4.str()<<std::endl;
        bDeal = false;
    }
    return bDeal;
}

void ConcurrentHashMapManager::resetAccumulate()
{
    if(nullptr != m_pGaeactorProcessorInterfaceInstanceAccumulate)
    {
        resetDisplay();
        auto beginTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
        std::stringstream ss2;
        ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" reset begin \n";
        //TRACE_LOG_PRINT_EX2(ss2);
        std::cout<<ss2.str()<<std::endl;
        m_pGaeactorProcessorInterfaceInstanceAccumulate->reset();
        UINT64 task_id = 0;
        tagTaskInfo &_all_lines = get_task_or_create(task_id);
        _all_lines.clear();

        auto endTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
        std::stringstream ss3;
        ss3<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" reset end cost "<<endTimeStamp - beginTimeStamp<<" ms \n";
        //TRACE_LOG_PRINT_EX2(ss3);
        std::cout<<ss3.str()<<std::endl;
    }
    else
    {
        m_pGaeactorProcessorInterfaceInstanceAccumulate = new gaeactorenvironment_ex::GaeactorProcessorInterfaceInstance();
        m_pGaeactorProcessorInterfaceInstanceAccumulate->record_sensor_overlap(true);
    }
}

void ConcurrentHashMapManager::resetDisplay()
{
#ifdef USING_SHOW_RESULT
    emit deal_result_sig("resetDisplay",nullptr);
#endif
}

void ConcurrentHashMapManager::save_ooda_to_db(tagLineItem &&_tagOODAItem)
{
    m_psaveinstance.append_data(std::move(_tagOODAItem));
    m_deal_data_fullCond.wakeAll();
}

void ConcurrentHashMapManager::sqlite_mutex_lock()
{
    m_sqlite_mutex->lock();
}

void ConcurrentHashMapManager::sqlite_mutex_unlock()
{
    m_sqlite_mutex->unlock();
}

#include <QDir>
void ConcurrentHashMapManager::loadShp()
{
    m_buildingfeaturesjsarray.clear();

    std::function<void (const QString &)> findShpFolders=[&](const QString &path)
    {
        QDir dir(path);

        // 检查路径是否存在且是一个目录
        if (!dir.exists()) {
            qDebug() << "提供的路径不是一个有效的目录";
            return;
        }

        // 获取目录中的所有条目
        QFileInfoList entries ;
        entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries)
        {
            if (entry.isDir())
            {
                findShpFolders(entry.absoluteFilePath());
            }
            else
            {
                if(entry.suffix() == "shp")
                {
                    readPolygonShp(entry.absoluteFilePath().toStdString(),m_buildingfeaturesjsarray);
                }
            }
        }
    };

    QString shppath = QCoreApplication::applicationDirPath() + "/data/gzsczt_fwjz_pro_cgcs2000/gz";
    findShpFolders(shppath);
//    QString shppath = QCoreApplication::applicationDirPath() + "/data/gzsczt_fwjz_pro_cgcs2000/gz.shp";
//    readPolygonShp(shppath.toStdString(),m_buildingfeaturesjsarray);
}

void ConcurrentHashMapManager::data_deal_savedb_thread_func(void *pParam)
{
//    bool bEmpty = true;
//    {
//        stdutils::OriMutexLocker locker(&m_psaveinstance.m_data_mutex);
//        bEmpty &= m_psaveinstance.m_data.empty();
//    }

//    if(bEmpty)
//    {
//        m_deal_data_fullCond.wait(&m_deal_data_mutex);
//    }

//    using namespace sqlite_orm;

//    {
//        QMutexLocker locker(m_sqlite_mutex);
//        auto storage_instance = init_Storage(SettingsConfig::getInstance().get_log_dir(SettingsConfig::E_LOG_DIR_TYPE_DEBUGINFO_CFG).toStdString()+"/Deconflictor_data.db");
//        storage_instance.begin_transaction();
//        {
//            stdutils::OriMutexLocker locker(&m_psaveinstance.m_data_mutex);
//            for(auto itor  = m_psaveinstance.m_data.begin(); itor != m_psaveinstance.m_data.end(); itor++)
//            {
//                auto &val = *itor;
//                storage_instance.insert(std::move(val));
//            }
//            m_psaveinstance.m_data.clear();
//        }
//        storage_instance.commit();
//    }
}

int ConcurrentHashMapManager::callback(void *paramobj, int argc, char **argv, char **azColName)
{
    tagSqlExecItem *psqlitem = reinterpret_cast<tagSqlExecItem *>(paramobj);
    if(psqlitem)
    {
        std::unordered_map<std::string,std::string> val;
        for (int i = 0; i < argc; i++)
        {
            if(argv[i] != nullptr)
            {
                val.insert(std::make_pair(azColName[i], argv[i]));
            }
            else
            {
                std::cout<<azColName[i]<<" "<<"empty \n";
                val.insert(std::make_pair(azColName[i], ""));
            }
        }
        //        psqlitem->m_pDataBaseSaveManager->deal_sql_result(psqlitem, val);
        if(!val.empty())
        {
            psqlitem->ret_val.push_back(std::move(val));
        }
    }
    return 0;
}

void ConcurrentHashMapManager::exec_item(tagSqlExecItem &sqlitem)
{

//    using namespace sqlite_orm;
//#if 0
//    std::string path = "D:/project/WorkProject_20240402001152/bin/win64/202412070657-00000/default/debugdata/Deconflictor_data.db";
//#else
//    std::string path = SettingsConfig::getInstance().get_log_dir(SettingsConfig::E_LOG_DIR_TYPE_DEBUGINFO_CFG).toStdString()+"/Deconflictor_data.db";
//#endif
//    {
//        QMutexLocker locker(m_sqlite_mutex);

//        auto storage_instance = init_Storage(path);
//        auto selectStatement = storage_instance.prepare(select(count<tagLineItem>()));

//        sqlite3 *db = selectStatement.con.get();
//        char *zErrMsg = nullptr;

//        //    // 打开数据库
//        //    int rc = sqlite3_open(path.c_str(), &db);
//        //    if (rc) {
//        //        fprintf(stderr, "can not open db: %s\n", sqlite3_errmsg(db));
//        //    } else {
//        //        fprintf(stdout, "db open succeed\n");
//        //    }

//        //    std::cout<<" --------------------------------------------------------------------------- \n";
//        // 执行SQL语句
//        if(db)
//        {
//            int rc = sqlite3_exec(db, sqlitem.sql.c_str(), &ConcurrentHashMapManager::callback, (void*)&sqlitem, &zErrMsg);
//            if (rc != SQLITE_OK)
//            {
//                fprintf(stderr, "SQL error: %s\n", zErrMsg);
//                sqlite3_free(zErrMsg);
//            }
//        }
//    }
}

void ConcurrentHashMapManager::sql_exec(tagSqlExecItem &sqlitem)
{
    sqlitem.m_pDataBaseSaveManager = this;
    exec_item(sqlitem);
}

void ConcurrentHashMapManager::deal_sql_result(const tagSqlExecItem *psqlitem, const std::unordered_map<std::string, std::string> &val)
{
    if(psqlitem == nullptr)
    {
        return;
    }

    std::cout<<" exec sql "<<psqlitem->sql<<"\n";
    auto itor = val.begin();
    while(itor != val.end())
    {
        std::cout<<itor->first<<" "<<itor->second<<"\n";
        itor++;
    }
}

void ConcurrentHashMapManager::prepare_deal_2d(const tagTaskInfo &_taskinfo, const std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>> & hexinfo)
{
    H3INDEX _h3Index = std::get<0>(hexinfo);
    bool _bValid = std::get<1>(hexinfo);
    uint32_t _eHexidexStatus = std::get<2>(hexinfo);
    auto sensorlists = std::get<3>(hexinfo);
    auto entitylists = std::get<4>(hexinfo);
    if (_bValid)
    {
        QColor cl = QColor(128, 128, 128, 128);
        switch (_eHexidexStatus)
        {
        case E_HEXINDEX_STATUS_FREE:
        {
            return;
        }break;
        case E_HEXINDEX_STATUS_ENTITY:
        {
            return;
        }break;
        case E_HEXINDEX_STATUS_SENSOR:
        {
            cl = QColor(0, 255, 0, 128);
            if (sensorlists.size() > 1)
            {
                cl = QColor(0, 0, 255, 128);
#if 1
                std::vector<tagSensorConflictInfo> _tagSensorlistInfos;
                std::vector<tagSensorConflictInfo> _tagLinesSensorConflictInfos;
                _tagSensorlistInfos.reserve(sensorlists.size());
                for(int j = 0; j < sensorlists.size(); j++)
                {
                    const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                    const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                    const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                    EVENT_KEY_TYPE linekey = EVENT_KEY_TYPE{agentid, 0, sensingmediaid};
                    tagLineInfoEx * _tagLineInfo = this->get_data(_taskinfo,linekey);
                    if(!_tagLineInfo)
                    {
                        continue;
                    }
                    const bool &bBuiling = _tagLineInfo->m_line.m_tagLinePros.bBuiling;
                    tagSensorConflictInfo _tagSensorConflictInfo{std::make_tuple(agentid, sensingmediaid), hgt, _tagLineInfo};

                    if(_tagSensorlistInfos.empty())
                    {
                        _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                    }
                    else
                    {
                        bool bHeightConflict = false;
                        bool bTimeConflict = false;
                        for(int m = 0; m < _tagSensorlistInfos.size(); m++)
                        {
                            const tagSensorConflictInfo& item = _tagSensorlistInfos.at(m);
                            //当前是建筑
                            if(bBuiling)
                            {
                                //只和非建筑比对高度
                                if(!item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                {
                                    if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                    {
                                        bHeightConflict = true;
                                    }
                                }
                                //高度存在冲突
                                if(bHeightConflict)
                                {
                                    _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                    break;
                                }
                            }
                            else
                            {
                                if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                {
                                    bHeightConflict = true;
                                }
                                if(!m_bchecktime)
                                {
                                    bTimeConflict = true;
                                }
                                else
                                {
                                    if(item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                    {
                                        bTimeConflict = true;
                                    }
                                    else
                                    {
                                        if(bHeightConflict &&
                                            _tagLineInfo &&
                                            item.m_ptagLineInfos &&
                                            (!CHECK_IS_OUT_RANGE(item.m_ptagLineInfos->m_line.m_tagLinePros.dt_startTime,
                                                                 item.m_ptagLineInfos->m_line.m_tagLinePros.dt_endTime,
                                                                 _tagLineInfo->m_line.m_tagLinePros.dt_startTime,
                                                                 _tagLineInfo->m_line.m_tagLinePros.dt_endTime)))
                                        {
                                            bTimeConflict = true;
                                        }
                                    }
                                }

                                if(bHeightConflict && bTimeConflict)
                                {
                                    _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                    break;
                                }
                            }
                        }
                    }
                    _tagSensorlistInfos.push_back(std::move(_tagSensorConflictInfo));
                }
                if(_tagLinesSensorConflictInfos.size() > 1)
                {
                    cl = QColor(255, 0, 0, 128);
                }
                else
                {
                    cl = QColor(0, 0, 255, 128);
                }
#else
                struct tagSensorConflictInfo_2d{
                    std::tuple<TYPE_ULID,TYPE_ULID> m_sensorid;
                    std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> m_sensorlist;
                    transdata_param_seq_hexidx_hgt m_hgtrange;                    
                    tagLineInfoEx* m_ptagLineInfos;
                };
                std::vector<tagSensorConflictInfo_2d> _tagSensorConflictInfos;
                _tagSensorConflictInfos.reserve(sensorlists.size());
                for(int j = 0; j < sensorlists.size(); j++)
                {
                    const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                    const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                    const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                    tagSensorConflictInfo_2d _tagSensorConflictInfo;
                    _tagSensorConflictInfo.m_sensorid = std::make_tuple(agentid, sensingmediaid);
                    _tagSensorConflictInfo.m_sensorlist.push_back(std::make_tuple(agentid, sensingmediaid));
                    _tagSensorConflictInfo.m_hgtrange = hgt;

                    for(int m = 0; m < _tagSensorConflictInfos.size(); m++)
                    {
                        const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(m).m_sensorid;
                        std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> &exist_sensorlist = _tagSensorConflictInfos.at(m).m_sensorlist;
                        const transdata_param_seq_hexidx_hgt& exist_hgt = _tagSensorConflictInfos.at(m).m_hgtrange;
                        if(!((hgt.PARAM_seq_hexidx_hgt0 < exist_hgt.PARAM_seq_hexidx_hgt0 && hgt.PARAM_seq_hexidx_hgtn < exist_hgt.PARAM_seq_hexidx_hgt0) ||
                              (hgt.PARAM_seq_hexidx_hgt0 > exist_hgt.PARAM_seq_hexidx_hgtn && hgt.PARAM_seq_hexidx_hgtn > exist_hgt.PARAM_seq_hexidx_hgtn)))
                        {
                            /////////////////////////////////////////////////////////////////////
                            exist_sensorlist.push_back(_tagSensorConflictInfo.m_sensorid);
                            _tagSensorConflictInfo.m_sensorlist.push_back(exist_sensorid);
                            /////////////////////////////////////////////////////////////////////
                            cl = QColor(255, 0, 0, 128);
                            /////////////////////////////////////////////////////////////////////
                        }
                    }
                    _tagSensorConflictInfos.push_back(std::move(_tagSensorConflictInfo));
                }
#endif
            }
#ifndef USING_SHOW_DETAIL
            else
            {
                return;
            }
#endif
        }
        break;
        case E_HEXINDEX_STATUS_ALL:
        {
            cl = QColor(255, 255, 0, 128);
        }break;
        default:
            break;
        }
        this->appenditem_hex_color(_h3Index, transdata_param_seq_hexidx{_h3Index, transdata_param_seq_hexidx_hgt{0,0, 0, 0}}, cl);
    }
}


void ConcurrentHashMapManager::prepare_deal_3d(const tagTaskInfo &_taskinfo,const std::tuple<H3INDEX, bool, uint32_t, std::vector<std::tuple<TYPE_ULID, TYPE_ULID,transdata_param_seq_hexidx_hgt>>,std::vector<std::tuple<TYPE_ULID,FLOAT64>>> & hexinfo)
{
    H3INDEX _h3Index = std::get<0>(hexinfo);
    bool _bValid = std::get<1>(hexinfo);
    uint32_t _eHexidexStatus = std::get<2>(hexinfo);
    auto sensorlists = std::get<3>(hexinfo);
    auto entitylists = std::get<4>(hexinfo);
    if (_bValid)
    {
        QColor cl = QColor(128, 128, 128, 128);
        switch (_eHexidexStatus)
        {
        case E_HEXINDEX_STATUS_FREE:
        {
            return;
        }break;
        case E_HEXINDEX_STATUS_ENTITY:
        {
            return;
        }break;
        case E_HEXINDEX_STATUS_SENSOR:
        {
            cl = QColor(0, 255, 0, 128);
            if (sensorlists.size() > 1)
            {
                cl = QColor(0, 0, 255, 128);
            }
#ifndef USING_SHOW_DETAIL
            else
            {
                return;
            }
#endif
            if(sensorlists.size() == 1)
            {
                const TYPE_ULID& agentid = std::get<0>(sensorlists.at(0));
                const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(0));
                const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(0));
                TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(agentid) ^ (std::hash<TYPE_ULID>()(sensingmediaid) << 1);
                this->appenditem(uildsrc, _h3Index, hgt, cl);
            }
            else
            {
#if 1
                std::vector<tagSensorConflictInfo> _tagSensorlistInfos;
                std::vector<tagSensorConflictInfo> _tagLinesSensorConflictInfos;
                _tagSensorlistInfos.reserve(sensorlists.size());
                for(int j = 0; j < sensorlists.size(); j++)
                {
                    const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                    const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                    const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                    EVENT_KEY_TYPE linekey = EVENT_KEY_TYPE{agentid, 0, sensingmediaid};
                    tagLineInfoEx * _tagLineInfo = this->get_data(_taskinfo,linekey);
                    if(!_tagLineInfo)
                    {
                        continue;
                    }
                    const bool &bBuiling = _tagLineInfo->m_line.m_tagLinePros.bBuiling;
                    tagSensorConflictInfo _tagSensorConflictInfo{std::make_tuple(agentid, sensingmediaid), hgt, _tagLineInfo};

                    if(_tagSensorlistInfos.empty())
                    {
                        _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                    }
                    else
                    {
                        bool bHeightConflict = false;
                        bool bTimeConflict = false;
                        for(int m = 0; m < _tagSensorlistInfos.size(); m++)
                        {
                            const tagSensorConflictInfo& item = _tagSensorlistInfos.at(m);
                            //当前是建筑
                            if(bBuiling)
                            {
                                //只和非建筑比对高度
                                if(!item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                {
                                    if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                    {
                                        bHeightConflict = true;
                                    }
                                }
                                //高度存在冲突
                                if(bHeightConflict)
                                {
                                    _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                    break;
                                }
                            }
                            else
                            {
                                if(!CHECK_IS_OUT_RANGE(item.m_hgtrange.PARAM_seq_hexidx_hgt0, item.m_hgtrange.PARAM_seq_hexidx_hgtn, hgt.PARAM_seq_hexidx_hgt0, hgt.PARAM_seq_hexidx_hgtn))
                                {
                                    bHeightConflict = true;
                                }
                                if(!m_bchecktime)
                                {
                                    bTimeConflict = true;
                                }
                                else
                                {
                                    if(item.m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                                    {
                                        bTimeConflict = true;
                                    }
                                    else
                                    {
                                        if(bHeightConflict &&
                                            _tagLineInfo &&
                                            item.m_ptagLineInfos &&
                                            (!CHECK_IS_OUT_RANGE(item.m_ptagLineInfos->m_line.m_tagLinePros.dt_startTime,
                                                                 item.m_ptagLineInfos->m_line.m_tagLinePros.dt_endTime,
                                                                 _tagLineInfo->m_line.m_tagLinePros.dt_startTime,
                                                                 _tagLineInfo->m_line.m_tagLinePros.dt_endTime)))
                                        {
                                            bTimeConflict = true;
                                        }
                                    }
                                }

                                if(bHeightConflict && bTimeConflict)
                                {
                                    _tagLinesSensorConflictInfos.push_back(_tagSensorConflictInfo);
                                    break;
                                }
                            }
                        }
                    }
                    _tagSensorlistInfos.push_back(std::move(_tagSensorConflictInfo));
                }
                if(_tagLinesSensorConflictInfos.size() > 1)
                {
                    for(int j = 0; j < _tagLinesSensorConflictInfos.size(); j++)
                    {
                        const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagLinesSensorConflictInfos.at(j).m_sensorid;
                        QColor cl = QColor(255, 0, 0, 128);
                        const transdata_param_seq_hexidx_hgt& hgt = _tagLinesSensorConflictInfos.at(j).m_hgtrange;
                        TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(std::get<0>(exist_sensorid)) ^ (std::hash<TYPE_ULID>()(std::get<1>(exist_sensorid)) << 1);
                        this->appenditem(uildsrc, _h3Index, hgt, cl);

                        if(_tagLinesSensorConflictInfos.at(j).m_ptagLineInfos &&
                            _tagLinesSensorConflictInfos.at(j).m_ptagLineInfos->m_line.m_tagLinePros.bBuiling)
                        {
                            emit draw_linestring_sig(_tagLinesSensorConflictInfos.at(j).m_ptagLineInfos->m_line);
                        }
                    }
                }
//                else
//                {
//                    for(int j = 0; j < _tagSensorlistInfos.size(); j++)
//                    {
//                        const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorlistInfos.at(j).m_sensorid;
//                        QColor cl = QColor(0, 0, 255, 128);
//                        const transdata_param_seq_hexidx_hgt& hgt = _tagSensorlistInfos.at(j).m_hgtrange;
//                        TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(std::get<0>(exist_sensorid)) ^ (std::hash<TYPE_ULID>()(std::get<1>(exist_sensorid)) << 1);
//                        this->appenditem(uildsrc, _h3Index, hgt, cl);
//                    }
//                }
#else
                struct tagSensorConflictInfo_3d{
                    std::tuple<TYPE_ULID,TYPE_ULID> m_sensorid;
                    std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> m_sensorlist;
                    transdata_param_seq_hexidx_hgt m_hgtrange;
                    QColor m_cl;
                };
                std::vector<tagSensorConflictInfo_3d> _tagSensorConflictInfos;
                _tagSensorConflictInfos.reserve(sensorlists.size());
                for(int j = 0; j < sensorlists.size(); j++)
                {
                    const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                    const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                    const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                    tagSensorConflictInfo_3d _tagSensorConflictInfo;
                    _tagSensorConflictInfo.m_sensorid = std::make_tuple(agentid, sensingmediaid);
                    _tagSensorConflictInfo.m_sensorlist.push_back(std::make_tuple(agentid, sensingmediaid));
                    _tagSensorConflictInfo.m_hgtrange = hgt;
                    _tagSensorConflictInfo.m_cl = QColor(0, 0, 255, 128);

                    for(int m = 0; m < _tagSensorConflictInfos.size(); m++)
                    {
                        const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(m).m_sensorid;
                        QColor & exist_cl = _tagSensorConflictInfos.at(m).m_cl;
                        std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> &exist_sensorlist = _tagSensorConflictInfos.at(m).m_sensorlist;
                        const transdata_param_seq_hexidx_hgt& exist_hgt = _tagSensorConflictInfos.at(m).m_hgtrange;
                        if(!((hgt.PARAM_seq_hexidx_hgt0 < exist_hgt.PARAM_seq_hexidx_hgt0 && hgt.PARAM_seq_hexidx_hgtn < exist_hgt.PARAM_seq_hexidx_hgt0) ||
                              (hgt.PARAM_seq_hexidx_hgt0 > exist_hgt.PARAM_seq_hexidx_hgtn && hgt.PARAM_seq_hexidx_hgtn > exist_hgt.PARAM_seq_hexidx_hgtn)))
                        {
                            /////////////////////////////////////////////////////////////////////
                            exist_sensorlist.push_back(_tagSensorConflictInfo.m_sensorid);
                            _tagSensorConflictInfo.m_sensorlist.push_back(exist_sensorid);
                            /////////////////////////////////////////////////////////////////////
                            exist_cl = QColor(255, 0, 0, 128);
                            _tagSensorConflictInfo.m_cl = exist_cl;
                            cl = QColor(255, 0, 0, 128);
                            /////////////////////////////////////////////////////////////////////
                        }
                    }
                    _tagSensorConflictInfos.push_back(std::move(_tagSensorConflictInfo));
                }

                for(int j = 0; j < _tagSensorConflictInfos.size(); j++)
                {
                    const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(j).m_sensorid;
                    const std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> &exist_sensorlist = _tagSensorConflictInfos.at(j).m_sensorlist;
                    if(exist_sensorlist.size() > 1)
                    {
                        QColor & cl = _tagSensorConflictInfos.at(j).m_cl;
                        const transdata_param_seq_hexidx_hgt& hgt = _tagSensorConflictInfos.at(j).m_hgtrange;
                        TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(std::get<0>(exist_sensorid)) ^ (std::hash<TYPE_ULID>()(std::get<1>(exist_sensorid)) << 1);
                        this->appenditem(uildsrc, _h3Index, hgt, cl);
                    }
                }
#endif
            }

        }
        break;
        case E_HEXINDEX_STATUS_ALL:
        {
            return;
        }break;
        default:
            break;
        }
    }
}



bool ConcurrentHashMapManager::readPolygonShp(const std::string &filename, std::vector<QJsonObject> &featuresjsarray)
{
    GDALAllRegister();
    GDALDataset   *poDS;
    CPLSetConfigOption("SHAPE_ENCODING","");  //解决中文乱码问题
    //读取shp文件
    poDS = (GDALDataset*) GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );

    if( poDS == NULL )
    {
        std::cout<<" Open failed. "<<filename<<std::endl;
        return false;
    }

    OGRLayer  *poLayer;
    poLayer = poDS->GetLayer(0); //读取层
    OGRFeature *poFeature;

    poLayer->ResetReading();

    featuresjsarray.reserve(featuresjsarray.size()+poLayer->GetFeatureCount());
    int i=0;
    while( (poFeature = poLayer->GetNextFeature()) != NULL )
    {
        /////////////////////////////////////////////////////////////////////////////////////////
        OGRGeometry *poGeometry = poFeature->GetGeometryRef();
        OGRwkbGeometryType geotype;
        geotype = poGeometry->getGeometryType();
        /////////////////////////////////////////////////////////////////////////////////////////

        if(wkbPolygon == geotype)
        {
            /////////////////////////////////////////////////////////////////////////////////////////
            i++;
            /////////////////////////////////////////////////////////////////////////////////////////

            double objectid = poFeature->GetFieldAsDouble("OBJECTID");
            const char *fwpcbh = poFeature->GetFieldAsString("fwpcbh");
            const char *bsm = poFeature->GetFieldAsString("bsm");
            const char *fwjzdm = poFeature->GetFieldAsString("fwjzdm");
            const char *jzmc = poFeature->GetFieldAsString("jzmc");
            const char *xxdz = poFeature->GetFieldAsString("xxdz");
            double jzgd = poFeature->GetFieldAsDouble("jzgd");

//            std::cout<<i<<" objectid "<<objectid<<" jzgd "<<jzgd<<" \n";
            /////////////////////////////////////////////////////////////////////////////////////////
            OGRPolygon *poPolygon = (OGRPolygon*)poGeometry;
            OGRLinearRing* poExteriorRing = poPolygon->getExteriorRing();
            if(poExteriorRing != nullptr)
            {
                QJsonArray coordinates_polygon_sub;
                int nPoints = poExteriorRing->getNumPoints();
//                for(int i = 0; i < nPoints; i++)
                for(int i = nPoints-1; i >= 0; i--)
                {
                    OGRPoint poPoint;
                    poExteriorRing->getPoint(i, &poPoint);
                    double x = poPoint.getX();
                    double y = poPoint.getY();
                    // 处理点 x, y
                    //                    std::cout<<x<<","<<y<<" \n";
                    QJsonArray coordinates_polygon_sub_item;
                    coordinates_polygon_sub_item.append(x);
                    coordinates_polygon_sub_item.append(y);
                    coordinates_polygon_sub.append(coordinates_polygon_sub_item);
                }
                if(!coordinates_polygon_sub.empty())
                {
                    QJsonObject properties;
                    /////////////////////////////////////////////////////////////////////////////////////////
                    QJsonObject agentrouteid;
                    agentrouteid.insert("agentid","7777777777");
                    agentrouteid.insert("routeid",i);
                    properties.insert("agentrouteid",agentrouteid);
                    /////////////////////////////////////////////////////////////////////////////////////////

                    QJsonObject featurecollectionpros;
                    featurecollectionpros.insert("code",QString::number((INT32)objectid));
                    featurecollectionpros.insert("startTime",QString(fwpcbh));
                    featurecollectionpros.insert("endTime",QString(bsm));
                    featurecollectionpros.insert("createTime",QString(fwjzdm));
                    featurecollectionpros.insert("Priority",QString(jzmc)+"_"+QString(xxdz));
                    properties.insert("featurecollectionpros",featurecollectionpros);
//                    qInfo()<<featurecollectionpros;

                    /////////////////////////////////////////////////////////////////////////////////////////
                    QJsonArray heights;
                    heights.append(jzgd);
                    properties.insert("heights",heights);

                    /////////////////////////////////////////////////////////////////////////////////////////
                    properties.insert("heightlimitup",0.0);
                    properties.insert("heightlimitdown",0.0);

                    /////////////////////////////////////////////////////////////////////////////////////////
                    QJsonObject feature;
                    feature.insert("type","Feature");
                    feature.insert("properties",properties);

                    QJsonObject geometry;

                    QJsonArray coordinates_polygon;

                    coordinates_polygon.append(coordinates_polygon_sub);
                    geometry.insert("coordinates_polygon",coordinates_polygon);
                    geometry.insert("type","Polygon");
                    feature.insert("geometry",geometry);

                    featuresjsarray.push_back(std::move(feature));
                }
                else
                {
                    std::cout<<i<<" objectid "<<objectid<<" jzgd "<<jzgd<<" nPoints "<<nPoints<<" \n";
                }
            }
        }
        OGRFeature::DestroyFeature( poFeature );
    }
    GDALClose( poDS );
    return true;
}

void ConcurrentHashMapManager::analysisBuildPolygon(tagTaskInfo &_all_lines)
{
    auto deal_build_polygon=[&](tagTaskInfo &_taskinfo, const UINT32 &lineindex, QJsonObject &featuresitemobj, const double &_heightlimitup, const double &_heightlimitdown)->UINT32
    {
        UINT32 hexcount =  0;
        EASY_FUNCTION(profiler::colors::Blue)
        auto deal_ploy = [&](const TYPE_ULID& agentid,const TYPE_ULID& routeid, tagLineInfo & _tagLineInfo,const double& heightlimitup, const double& heightlimitdown)->UINT32
        {
            EASY_FUNCTION(profiler::colors::DeepPurple)

            HEXIDX_HGT_ARRAY  &_hexidxs = _tagLineInfo._hexidxs;
            HEXINDEX_HGT_CONCURRENT_HASHMAP _hexindex_hgt_concurrent_hash_map;
            tbb::concurrent_hash_map<H3INDEX,bool> _hexindex_hgt_concurrent_hash_map2;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, _tagLineInfo._polygon_latlnghgts.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 j = r.begin(); j != r.end(); j++)
                                  {
                                      const std::vector<tagPtLatLngHgtInfo> & _polygon_latlng = _tagLineInfo._polygon_latlnghgts.at(j);
                                      FLOAT64 hgt_start = 0;
                                      FLOAT64 hgt_up_start = 0;
                                      FLOAT64 hgt_down_start = 0;

                                      std::vector<LatLng> _polygon_latlng_rads;
                                      _polygon_latlng_rads.resize(_polygon_latlng.size());
                                      for(int m = 0; m < _polygon_latlng.size(); m++)
                                      {
                                          hgt_start = _polygon_latlng.at(m)._hgt;
                                          hgt_up_start = _polygon_latlng.at(m)._hgt_up;
                                          hgt_down_start = _polygon_latlng.at(m)._hgt_down;

                                          _polygon_latlng_rads[m].lat = LocationHelper::degreeToRadian(_polygon_latlng[m]._latlng.lat);
                                          _polygon_latlng_rads[m].lng = LocationHelper::degreeToRadian(_polygon_latlng[m]._latlng.lng);
                                      }

                                      HEXIDX_ARRAY hexidxslist;
                                      LocationHelper::getPolygonResulutionIndex(hexidxslist, _polygon_latlng_rads, _taskinfo._resolution);
                                      if(hexidxslist.size() > 0)
                                      {
                                          tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist.size()),
                                                            [&](const tbb::blocked_range<size_t>& q) {
                                                                for (UINT32 m = q.begin(); m != q.end(); m++)
                                                                {
                                                                    const H3INDEX& _h3Index = hexidxslist[m];
                                                                    transdata_param_seq_hexidx _transdata_param_seq_hexidx;
                                                                    _transdata_param_seq_hexidx.PARAM_seq_hexidx_element = _h3Index;
                                                                    _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt = hgt_start;
                                                                    _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt0 = hgt_down_start;
                                                                    _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgtn = hgt_up_start;
                                                                    {
                                                                        HEXINDEX_HGT_CONCURRENT_HASHMAP_Accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                        if(!_hexindex_hgt_concurrent_hash_map.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                        {
                                                                            _hexindex_hgt_concurrent_hash_map.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                            _hexindex_hgt_concurrent_hash_map_accessot->second = std::move(_transdata_param_seq_hexidx);
                                                                        }
                                                                    }
                                                                }
                                                            });
                                      }

                                      /////////////////////////////////////////////////////////////////////////////////////////////////////////
                                      HEXIDX_ARRAY hexidxslist2;
                                      LocationHelper::getPolygonResulutionIndex(hexidxslist2, _polygon_latlng_rads, m_bresolution);
                                      if(hexidxslist2.size() > 0)
                                      {
                                          tbb::parallel_for(tbb::blocked_range<size_t>(0, hexidxslist2.size()),
                                                            [&](const tbb::blocked_range<size_t>& q) {
                                                                for (UINT32 m = q.begin(); m != q.end(); m++)
                                                                {
                                                                    const H3INDEX& _h3Index = hexidxslist2[m];
                                                                    tbb::concurrent_hash_map<H3INDEX,bool>::accessor _hexindex_hgt_concurrent_hash_map_accessot;
                                                                    if(!_hexindex_hgt_concurrent_hash_map2.find(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index))
                                                                    {
                                                                        _hexindex_hgt_concurrent_hash_map2.insert(_hexindex_hgt_concurrent_hash_map_accessot, _h3Index);
                                                                        _hexindex_hgt_concurrent_hash_map_accessot->second = true;
                                                                    }
                                                                }
                                                            });
                                      }
                                      /////////////////////////////////////////////////////////////////////////////////////////////////////////
                                  }
                              });


            _hexidxs.reserve(_hexindex_hgt_concurrent_hash_map.size());
            {
                HEXINDEX_HGT_CONCURRENT_HASHMAP_Iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map.begin();
                while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map.end())
                {
                    const H3INDEX& _h3Index = _ret_tmp_itor->first;
                    transdata_param_seq_hexidx &_transdata_param_seq_hexidx = _ret_tmp_itor->second;
                    auto _hex_hgt_itor = _tagLineInfo._hex_hgt.find(_h3Index);
                    if(_hex_hgt_itor == _tagLineInfo._hex_hgt.end())
                    {
                        _tagLineInfo._hex_hgt.insert(std::make_pair(_h3Index, _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt));
                    }
                    else
                    {
                        _hex_hgt_itor->second = _transdata_param_seq_hexidx.PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt;
                    }
                    _hexidxs.push_back(std::move(_transdata_param_seq_hexidx));
                    _ret_tmp_itor++;
                }
            }

            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            _tagLineInfo.m_hexidxs.reserve(_hexindex_hgt_concurrent_hash_map2.size());
            {
                tbb::concurrent_hash_map<H3INDEX,bool>::const_iterator _ret_tmp_itor = _hexindex_hgt_concurrent_hash_map2.begin();
                while (_ret_tmp_itor != _hexindex_hgt_concurrent_hash_map2.end())
                {
                    const H3INDEX& _h3Index = _ret_tmp_itor->first;
                    _tagLineInfo.m_hexidxs.push_back(std::move(_h3Index));
                    _ret_tmp_itor++;
                }
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            return _hexidxs.size();
        };
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        auto geometry = featuresitemobj.value("geometry").toObject();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        auto properties = featuresitemobj.value("properties").toObject();
        auto featurecollectionpros = properties.value("featurecollectionpros").toObject();
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        tagLinePros _tagLinePros;
        {
            _tagLinePros.code = featurecollectionpros.value("code").toString();
            _tagLinePros.startTime = featurecollectionpros.value("startTime").toString();
            _tagLinePros.endTime = featurecollectionpros.value("endTime").toString();
            _tagLinePros.createTime = featurecollectionpros.value("createTime").toString();
            _tagLinePros.Priority = featurecollectionpros.value("Priority").toString();
            _tagLinePros.bBuiling = true;
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        auto heightlimitup = properties.value("heightlimitup").toDouble(_heightlimitup);
        auto heightlimitdown = properties.value("heightlimitdown").toDouble(_heightlimitdown);
        auto heights = properties.value("heights").toArray();
        auto agentrouteid = properties.value("agentrouteid").toObject();
        auto agentid = agentrouteid.value("agentid").toString().toULongLong();
        auto routeid = agentrouteid.value("routeid").toInt();
        EVENT_KEY_TYPE key = EVENT_KEY_TYPE{agentid, 0, (TYPE_ULID)routeid};
        tagLineInfoEx & _tagLineInfo = get_data_or_create(_taskinfo,key,_tagLinePros.code.toStdString());

        {
            tbb::concurrent_hash_map<std::string, tagLineInfoEx*>::accessor _codeaccessor;
            if(!_taskinfo.m_codeline.find(_codeaccessor, _tagLinePros.code.toStdString()))
            {
                _taskinfo.m_codeline.insert(_codeaccessor, _tagLinePros.code.toStdString());
                _codeaccessor->second = &_tagLineInfo;
            }
        }
        _tagLineInfo.m_line._id = ConcurrentHashMapManager::generate_random_positive_uint64();
//        _tagLineInfo.m_line._id = lineindex;
        _tagLineInfo.m_line.featurecollectionpros = std::move(featurecollectionpros);
        _tagLineInfo.m_line.m_tagLinePros = std::move(_tagLinePros);

        auto polygoncoordinates = geometry.value("coordinates_polygon").toArray();
        if(!polygoncoordinates.isEmpty())
        {
            _tagLineInfo.m_line._polygon_latlnghgts.reserve(polygoncoordinates.size());
            for(int j = 0; j< polygoncoordinates.size();j++)
            {
                std::vector<tagPtLatLngHgtInfo> _subpolygon_latlnghgts;
                double _hgt = 0;
                if(heights.size() == polygoncoordinates.size())
                {
                    _hgt = heights.at(j).toDouble();
                }
                auto coordinates =  polygoncoordinates.at(j).toArray();
                _subpolygon_latlnghgts.reserve(coordinates.size());
                for(int index = 0; index < coordinates.size(); index++)
                {
                    auto pt = coordinates.at(index).toArray();
                    tagPtLatLngHgtInfo _tagPtLatLngHgtInfo;
                    bool bAppend = false;
                    if(pt.size() == 2)
                    {
                        _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                        _tagPtLatLngHgtInfo._hgt = _hgt;
                        bAppend = true;
                    }
                    else if(pt.size() == 3)
                    {
                        _tagPtLatLngHgtInfo._latlng = LatLng{pt[1].toDouble(), pt[0].toDouble()};
                        _tagPtLatLngHgtInfo._hgt = pt[2].toDouble();
                        bAppend = true;
                    }

                    if(bAppend)
                    {
                        _tagPtLatLngHgtInfo._hgt_up = _tagPtLatLngHgtInfo._hgt;
                        _tagPtLatLngHgtInfo._hgt_down = 0;
                        _subpolygon_latlnghgts.push_back(std::move(_tagPtLatLngHgtInfo));
                    }
                }
                if(!_subpolygon_latlnghgts.empty())
                {
                    _tagLineInfo.m_line._polygon_latlnghgts.push_back(std::move(_subpolygon_latlnghgts));
                }
            }
        }
        hexcount = deal_ploy(agentid,routeid, _tagLineInfo.m_line, heightlimitup, heightlimitdown);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        auto newproperties = featuresitemobj.value("properties").toObject();
        QJsonArray linehex;
        for(auto hexindex:_tagLineInfo.m_line.m_hexidxs)
        {
            linehex.append(QString::number(hexindex));
        }
        newproperties.insert("linehex",linehex);
        featuresitemobj.insert("properties",newproperties);
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        _tagLineInfo.m_line.featuresitemobj = featuresitemobj;
        return hexcount;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////


    double _heightlimitup = m_heightlimitup.load();
    double _heightlimitdown = m_heightlimitdown.load();

    std::atomic<UINT32> totalhexcount = 0;
    if(!m_buildingfeaturesjsarray.empty())
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_buildingfeaturesjsarray.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 j = r.begin(); j != r.end(); j++)
                              {
                                  QJsonObject featuresitemobj = m_buildingfeaturesjsarray.at(j);
                                  auto geometry = featuresitemobj.value("geometry").toObject();
                                  auto type = geometry.value("type").toString();
                                  if(type == "Polygon")
                                  {
                                      totalhexcount+=deal_build_polygon(_all_lines, j,featuresitemobj,_heightlimitup,_heightlimitdown);
                                  }
                              }
                          });
    }

    std::stringstream ss;
    ss<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" input resolution "<<_all_lines._resolution<<" building count "<<_all_lines.m_lines.size()<<" hex count "<<totalhexcount.load()<<" \n";
    //TRACE_LOG_PRINT_EX2(ss);
    std::cout<<ss.str()<<std::endl;
}

UINT32 ConcurrentHashMapManager::append_building(const bool& bconflictbuilding,
                                                 const INT32& _resolution,
                                                 tagTaskInfo &_all_lines,
                                                 GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance)
{
    auto deal_building_polygon=[&](tagTaskInfo &_taskinfo,const EVENT_KEY_TYPE&key, tagLineInfoEx val, const UINT32 &lineindex, GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance)->UINT32
    {
        UINT32 hexcount =  0;
        EASY_FUNCTION(profiler::colors::Blue)
        auto deal_ploy = [&](const TYPE_ULID& agentid,const TYPE_ULID& routeid, tagLineInfo & _tagLineInfo)->UINT32
        {
            EASY_FUNCTION(profiler::colors::DeepPurple)

            HEXIDX_HGT_ARRAY  &_hexidxs = _tagLineInfo._hexidxs;
            /////////////////////////////////////////////////////////////////////////////////////////////////////////
            IDENTIFI_EVENT_INFO eventinfo;
            if(_pGaeactorProcessorInterfaceInstance)
            {
                _pGaeactorProcessorInterfaceInstance->update_hexindex_sensor(agentid, routeid, _hexidxs, transdata_sensorposinfo(), POLYGON_LIST(), eventinfo);
            }
            return _hexidxs.size();
        };

        tagLineInfoEx * _tagLineInfo = nullptr;

//        val.m_line._id = lineindex;
        {
            LINES_CONCURRENT_HASHMAP_Accessor _accessor;
            if(!_taskinfo.m_lines.find(_accessor, key))
            {
                _taskinfo.m_lines.insert(_accessor, key);
                _accessor->second = std::move(val);
                _tagLineInfo = &_accessor->second;
            }
        }

        if(_tagLineInfo)
        {
            {
                tbb::concurrent_hash_map<std::string, tagLineInfoEx*>::accessor _codeaccessor;
                if(!_taskinfo.m_codeline.find(_codeaccessor, val.m_line.m_tagLinePros.code.toStdString()))
                {
                    _taskinfo.m_codeline.insert(_codeaccessor, val.m_line.m_tagLinePros.code.toStdString());
                    _codeaccessor->second = _tagLineInfo;
                }
            }
        }
        hexcount = deal_ploy(key.sensorid,key.sensingmediaid, _tagLineInfo->m_line);
        return hexcount;
    };
    std::atomic<UINT32> totalhexcount = 0;
    UINT32 isize = _all_lines.m_lines.size();
    if(bconflictbuilding)
    {
        auto _RES_buildings_itor = std::find_if(m_RES_buildings.begin(),
                                                m_RES_buildings.end(),[&](const std::unordered_map<INT32,tagTaskInfo>::value_type &vt){
                                                    return vt.first == _resolution;
                                                });
        if(_RES_buildings_itor != m_RES_buildings.end())
        {
            tagTaskInfo & pBuilding = _RES_buildings_itor->second;
            std::stringstream ss2;
            ss2<<stdutils::OriDateTime::currentDateTimeTimestamptoString()<<" select builing count "<<pBuilding.m_lines.size()<<" res "<<_resolution<<" \n";
            //TRACE_LOG_PRINT_EX2(ss2);
            std::cout<<ss2.str()<<std::endl;

            std::vector<EVENT_KEY_TYPE> keys;

            {
                keys.clear();
                keys.reserve(pBuilding.m_lines.size());
                LINES_CONCURRENT_HASHMAP::const_iterator _exist_events_map_itor = pBuilding.m_lines.begin();
                while (_exist_events_map_itor != pBuilding.m_lines.end())
                {
                    keys.push_back(_exist_events_map_itor->first);
                    _exist_events_map_itor++;
                }
            }
#if 1
            tbb::parallel_for(tbb::blocked_range<size_t>(0, keys.size()),
                              [&](const tbb::blocked_range<size_t>& r) {
                                  for (UINT32 j = r.begin(); j != r.end(); j++)
                                  {
                                      const auto &key = keys.at(j);
                                      LINES_CONCURRENT_HASHMAP::const_accessor _c_accessor;
                                      if(pBuilding.m_lines.find(_c_accessor, key))
                                      {
                                          totalhexcount += deal_building_polygon(_all_lines, key, _c_accessor->second, isize+j, _pGaeactorProcessorInterfaceInstance);
                                      }
                                  }
                              });
#else
            int j = 0;
            LINES_CONCURRENT_HASHMAP::const_iterator _c_accessor = pBuilding.m_lines.begin();
            while(_c_accessor != pBuilding.m_lines.end())
            {
                totalhexcount += deal_building_polygon(_all_lines, _c_accessor->first, _c_accessor->second, isize+j, _pGaeactorProcessorInterfaceInstance);
                _c_accessor++;
                j++;
            }
#endif
        }
    }
    return totalhexcount;
}
