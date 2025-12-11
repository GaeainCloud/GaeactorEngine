#ifndef DATAMANAGER_H
#define DATAMANAGER_H
#include <QJsonDocument>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <unordered_map>
#include <QReadWriteLock>

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <QRunnable>

#include <glm/vec3.hpp>

#include "function.h"

#include "transformdata_define.h"
#include "taskthread.hpp"


typedef std::function<void(int total,int cur)> deal_data_process_func_callback;


namespace std {
template<> struct hash<LAT_LNG> {
    size_t operator()(const LAT_LNG& p)const {
        return hash<double>()((p.lat)) ^ \
               hash<double>()(p.lng);
    }
};
template <>
struct equal_to<LAT_LNG> {
    bool operator()(const LAT_LNG& left, const LAT_LNG& right) const {
        return left == right;
    }
};
};


enum E_POINT_TYPE
{
    E_POINT_TYPE_OSM,
    E_POINT_TYPE_OSM_EXTEND,
	E_POINT_TYPE_POI,
    E_POINT_TYPE_POI_EXTEND_OSM,
    E_POINT_TYPE_POI_EXTEND_OSM_EXTEND,
    E_POINT_TYPE_POI_EXTEND_POI,
    E_POINT_TYPE_POI_EXTEND_POI_EXTEND

};
class QNetworkDiskCache;
class QNetworkAccessManager;

class DataManager
{
public:
    static DataManager& getInstance();
    virtual~DataManager();

    void dealpath(const QString& path, double diff,double space);
    void init(const QString& path);

	bool readGeoJsonData(const std::string& filename, GeoJsonInfos& geoinfos);


	QHash<QString, GeoJsonInfos> & getGeoJsonInfos();

	QHash<QString, GeoJsonInfos>& getWpsInfos();
    QHash<QString, GeoJsonInfos>& getWpsRunwayInfos();

	std::unordered_map<QString, tagPoiItem> &getPoiitems();

    std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE,QString>>& getLinestringpoints();


	std::unordered_map<QString, std::unordered_map<QString, QString> > &InstagentInstance();
	void setInstagentInstance(const std::unordered_map<QString, std::unordered_map<QString, QString> > &newInstagentInstance);

	std::unordered_map<QString, AgentInstanceInfo> &agentInstances();
	void setAgentInstances(const std::unordered_map<QString, AgentInstanceInfo> &newAgentInstances);

	std::unordered_map<QString, AgentKeyItemInfo>& agentKeyMaps();
	void setAgentKeyMaps(const std::unordered_map<QString, AgentKeyItemInfo> &newAgentKeyMaps);

	std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> >& flightdata();
	void setFlightData(const std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> > &newData);

	void Parkingpointuseinfo_clear();
	void PlaneNumToFlightNum_clear();
	std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> >& PlaneNumToFlightNum();
	void setPlaneNumToFlightNum(const std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> > &newPlaneNumToFlightNum);

	std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> > &flighttimedata();
	void setFlighttimedata(const std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> > &newFlighttimedata);


	std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>>& flightEventTimedata();
	void setFlightEventTimedata(const std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>> &newFlightEventTimedata);

	std::map<QString, ARR_DEP_RUNWAY_PATH>& getPathPlans();

	QList<tagPath_Plan*> getArrpaths();
	QList<tagPath_Plan*> getDeppaths();

	QStringList& getRunways();


	void scheduleWpsprepare(const QString& agentId, FlightPlanConf *pflighltData);
	QJsonArray getScheduleWps(const QString& agentId, FlightPlanConf *pflighltData);

public:

    void getExtendWpsExName2Name(const QString& poi1, const QString& poi2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret);
    void getExtendWpsExID2Name(const UINT64& Poi_osm_path_id1, const QString& poi2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret);
    void getExtendWpsExID2ID(const UINT64& Poi_osm_path_id1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret);
    void getExtendWpsExID2ID_EX(const UINT64& Poi_osm_path_id1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret);
    void getExtendWpsExName2ID(const QString& poi1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret);

	void deal_generateExtendWps_callback(tagPath_Plan* path_plan);

	bool checkParkingPointUsingTimeConflict(FlightPlanConf *pflighltData, const QString&_parkingpoint);
	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> getParkingPointUsingTimeConflict(FlightPlanConf *pflighltData);


	bool checkRunwayUsingTimeConflict(FlightPlanConf *pflighltData,  const QString&_runway);
	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> getRunwayUsingTimeConflict(FlightPlanConf *pflighltData);

    void set_deal_data_process_func_callback(deal_data_process_func_callback newDeal_data_process_func_callback);

private:
    DataManager();



	void readGeoDirPath(const QString &path);
	void readGeoJsonFile(const std::string & filename);
	void readWpsRunwayPath(const QString& path);
	void readWpsRunwayFile(const std::string& filename);
	void readWpsGeoDirPath(const QString& path);
	void readWpsGeoJsonFile(const std::string& filename);

    void readAerowayFile(const std::string & filename);

    void readWaypointsGeoJsonFile(const std::string & filename);


	void readPathDir(const QString &path);

	void readPoisJsonFile(const QString & filename);

    void readPathInfoFileDir(const QString & dir);

    void readPathInfoFileInfo(const QString & filename);
    void readPathInfoFile(const QString & filename);
	void writePathInfoFile(const QString & filename);

	void decode_Standard_Taxiing_Path(const QString & filename);
	void decode_Path_Plan(const QString & filename);

    void updateDijkstraMap(const LAT_LNG& latlng, const LAT_LNG& lstlatlng, bool bBegin, const QString &aeroway);

    bool getLinestringPointId(UINT64&item_id, const LAT_LNG& latlng);

    bool appendLinestringPoint(UINT64&item_id, const LAT_LNG &latlng, E_POINT_TYPE ePointType, const QString &aeroway);
    UINT64 appendDijkstraMap(const LAT_LNG& currentlatlng, const LAT_LNG& lstlatlng, UINT64 lstid, E_POINT_TYPE ePointExtendType, E_POINT_TYPE ePointTypeSrc, const QString &aeroway);
    bool getLinestringPoint(UINT64&item_id, const LAT_LNG& latlng);


	bool getPoi_osm_path_id(UINT64&Poi_osm_path_id, const QString& poikeyword);
	tagPathInfo getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst);


    void findNextPoiDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, const LAT_LNG& latlng, const std::vector<std::tuple<LAT_LNG, UINT64>>& pts);

    void findDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng);
    void findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const UINT64& excludeIDs);
    void findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const LAT_LNG& excludelatlng);

    void findMindisWithMidPointBetweenPoints(std::tuple<LAT_LNG, UINT64>& midpoint,
        const std::tuple<LAT_LNG, UINT64>& pointa,
        const std::tuple<LAT_LNG, UINT64>& pointb);

	void generateExtendWps(tagPath_Plan& path_plan);

	void generateRunwayExtendWps(tagPath_Plan* path_plan);


	void generatePath_planExtendWps();

	void decode_stopbar();
	void decode_transrunwaybar();
private:
    QHash <QString, GeoJsonInfos> m_GeoJsonInfos;

	QHash <QString, GeoJsonInfos> m_WPSInfos;


	QHash <QString, LATLNGS_VECTOR> m_RunwayInfos;

	QHash <QString, GeoJsonInfos> m_WPSRunwayInfos;
    std::unordered_map<QString, AgentInstanceInfo> m_agentInstances;
    std::unordered_map<QString, AgentKeyItemInfo> m_agentKeyMaps;
    std::unordered_map<QString, std::unordered_map<QString, QString>> m_InstagentInstance;



#if 0
#else
    std::unordered_map<QString,std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>> m_flightdata;
    //std::map<uint64_t,std::unordered_map<QString, FlightPlanConf>> m_data;
#endif
	std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*>> m_flighttimedata;


	//������	һ���ڵĺ���ƫ��	
	std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>> m_flightEventTimedata;
    std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO>> m_PlaneNumToFlightNum;

	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> m_parkingpointuseinfo;

	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> m_runwayuseinfo;


	std::unordered_map<QString, tagPoiItem> m_poiitemsmap;

	std::unordered_map<QString, tagStandard_Taxiing_Path> m_Standard_Taxiing_Paths;

	std::map<QString, ARR_DEP_RUNWAY_PATH> m_Path_Plans;

	QList<tagPath_Plan*> m_arrpaths;
	QList<tagPath_Plan*> m_deppaths;

	QStringList m_runways;

	Dijkstra* m_pDijkstra;


    std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>> m_linestringpoints;


	UINT32 m_total_path_num;
	std::atomic_uint32_t m_deal_path_num;
	PathPlanExtendTaskManager * m_pPathPlanExtendTaskManager;
	TaskThreadPool::ThreadPool* m_pThreadPool;

    double m_point_extend_metres = 30.0f;
    double m_graph_max_vex_diff = 1.5f;

    deal_data_process_func_callback m_deal_data_process_func_callback;

    QString m_deal_path;

    std::unordered_map<LAT_LNG, QString> m_aerowayinfo;
};
#endif
