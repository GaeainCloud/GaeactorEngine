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

#include <glm/vec3.hpp>
#include "ProjectionEPSG3857.h"
#include "LocationHelper.h"


#include "components/function.h"

#include "../components/httpclient/httpclient.hpp"
#include "transformdata_define.h"



#include "gaeactor_comm_interface.h"

#include "../uiwidget/playwidget.h"

namespace stdutils
{
class OriThread;
};
#include "./src/OriginalMutex.h"

extern bool operator==(const LAT_LNG& object1, const LAT_LNG& object2);
extern bool operator!=(const LAT_LNG& object1, const LAT_LNG& object2);

class EventDriver;
#include <sstream>
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

enum E_GEOTYPE
{
	E_GEOTYPE_POINT,
	E_GEOTYPE_LINE,
	E_GEOTYPE_POLYGON,
	E_GEOTYPE_MULITPOLYGON
};

struct SiminfoLog
{
    uint64_t id;
    uint64_t min;
    uint64_t max;
    uint32_t step_interval;
    double step_dt;
    double step_freq;
    double one_second_sim_step_second;
    std::string siname;
    uint64_t timestamp;
};

struct infoLog
{
    uint64_t id;
    std::string title;
    std::string log;
    std::string reason;
    bool bTrans;
    std::string timestampstr;
    uint64_t timestamp;
};

struct FlightPlanConfLog
{
    uint64_t id;
    std::string m_Date;
    std::string m_FilghtNumber;
    std::string m_DepArrFlag;
    UINT32 m_flight_dep_arr_type;
    std::string m_PlaneNum;
    std::string m_PlaneType;
    std::string m_FlightClass;
    std::string m_FlightLeg;
    std::string m_FlightStartPlace;
    std::string m_FlightEndPlace;
    uint64_t m_aheadtimelen;

    uint64_t m_PlanDateTimeTakeOff_ahead_timestamp;
    std::string m_PlanDateTimeTakeOff;
    uint64_t m_PlanDateTimeTakeOff_timestamp;
    std::string m_ExpectedDateTimeTakeOff;
    std::string m_RealityDateTimeTakeOff;

    uint64_t m_PlanDateTimeLanding_ahead_timestamp;
    std::string m_PlanDateTimeLanding;
    uint64_t m_PlanDateTimeLanding_timestamp;
    std::string m_ExpectedDateTimeLanding;
    std::string m_RealityDateTimeLanding;
    uint64_t m_PlanDateTimeLanding_behind_timestamp;
    uint64_t m_behindtimelen;

    std::string m_Delay;
    std::string m_Seat;
    std::string m_Terminal;
    std::string m_Runway;

    std::string flightid;

    std::string target_parkingpoint;
    std::string target_runway;
    std::string alloc_parkingpoint;
    std::string alloc_runway;

    quint64 flightinstanceid;
};

struct  tagGeoJsonInfo
{
	QString name;
	QColor m_color;
	E_GEOTYPE type;
	QJsonObject properties;
	std::vector<LATLNGS_VECTOR> coordinates;

	////////////////////////////////////////////////////////
	// lines info
	QMap<QString, QString> m_tags;
	int z_order;
	std::vector<std::tuple<LATLNGS_VECTOR, QColor>> coordinatesExtend;
	////////////////////////////////////////////////////////
	/// \brief dealPolygonInfo
	///
	void dealInfo()
	{
		if (type == E_GEOTYPE_LINE)
		{
			dealLineInfo();
		}
		else if (type == E_GEOTYPE_MULITPOLYGON || type == E_GEOTYPE_POLYGON)
		{
			dealPolygonInfo();
		}

		else if (type == E_GEOTYPE_POINT)
		{
			dealPointInfo();
		}
	}

	void dealPolygonInfo()
	{
		QColor cl = FunctionAssistant::randColor(64);
		if (!properties.value("aeroway").toString().isEmpty())
		{
			if (properties.value("aeroway").toString() == "taxiway")
			{
				//                cl = QColor(64, 64, 64, 128);
				cl = QColor(255, 255, 0, 128);
			}
			else if (properties.value("aeroway").toString() == "aerodrome")
			{
				cl = QColor(64, 64, 64, 64);
			}
			else if (properties.value("aeroway").toString() == "apron")
			{
				cl = QColor(64, 64, 64, 32);
			}
		}

		else if (!properties.value("landuse").toString().isEmpty())
		{
			if (properties.value("landuse").toString() == "grass")
			{
				cl = QColor(0, 192, 0, 192);
			}
			else if (properties.value("landuse").toString() == "farmland")
			{
				cl = QColor(128, 128, 255, 192);
			}
			else if (properties.value("landuse").toString() == "construction")
			{
				cl = QColor(128, 128, 0, 192);
			}
		}

		else if (!properties.value("natural").toString().isEmpty())
		{
			if (properties.value("natural").toString() == "water")
			{
				cl = QColor(0, 192, 192, 192);
			}
		}

		else if (!properties.value("amenity").toString().isEmpty())
		{
			if (properties.value("amenity").toString() == "fuel")
			{
				cl = QColor(255, 255, 0, 192);
			}
			else if (properties.value("amenity").toString() == "parking")
			{
				cl = QColor(255, 255, 0, 128);
			}
		}
		m_color = cl;
	}

	void dealLineInfo()
	{
		double width = 4.0f;
		width = m_tags.value("width", "4.0").toDouble();
		width = width / 2;

		QColor cl = cl = QColor(0, 0, 255, 255);

		if (!properties.value("highway").toString().isEmpty())
		{
			if (m_tags.value("service") == "parking_aisle")
			{
				cl = QColor(0, 0, 255, 128);
				width = 3;
			}
			else if (m_tags.value("service") == "private")
			{
				cl = QColor(255, 0, 0, 128);
				width = 3;
			}
			else if (m_tags.value("service") == "driveway")
			{
				cl = QColor(255, 255, 0, 128);
				width = 3;
			}

			if (m_tags.value("access") == "private")
			{
				cl = QColor(255, 0, 0, 192);
				width = 3;
			}

			QString highway = properties.value("highway").toString();

			if (highway == "footway")
			{
				cl = QColor(0, 64, 64, 128);
				width = 2;
			}
			else if (highway == "tertiary" || highway == "tertiary_link")
			{
				cl = QColor(0, 128, 128, 128);
				width = 3;
			}
			else if (highway == "secondary" || highway == "secondary_link")
			{
				cl = QColor(0, 192, 192, 128);
				width = 4;
			}
			else if (highway == "primary")
			{
				cl = QColor(0, 255, 255, 128);
				width = 5;
			}
			else if (highway == "trunk" || highway == "trunk_link")
			{
				cl = QColor(0, 255, 255, 192);
				width = 6;
			}
			else if (highway == "construction")
			{
				cl = QColor(0, 255, 255, 64);
				width = 5;
			}
			else if (highway == "residential")
			{
				cl = QColor(0, 255, 255, 32);
				width = 2;
			}
			else if (highway == "steps")
			{
				cl = QColor(0, 128, 0, 32);
				width = 2;
			}
			else if (highway == "unclassified")
			{
				cl = QColor(255, 255, 0, 32);
				width = 4;
			}
		}
		else if (m_tags.contains("aeroway"))
		{
			if (m_tags.value("aeroway") == "runway")
			{
				cl = QColor(255, 255, 0, 255);
				width = 30;
			}
			else if (m_tags.value("aeroway") == "taxiway")
			{
				cl = QColor(255, 255, 0, 192);
				width = 20;
			}
			else if (m_tags.value("aeroway") == "parking_position")
			{
				cl = QColor(255, 255, 0, 128);
				width = 10;
			}
			else if (m_tags.value("aeroway") == "stopbar")
			{
				cl = QColor(0, 255, 255, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "landingpoint")
			{
				cl = QColor(0, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "holdingpoint")
			{
				cl = QColor(255, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "transrunway")
			{
				cl = QColor(255, 0, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "tr_checkbar_stopbar")
			{
				cl = QColor(255, 0, 255, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "tr_checkbar")
			{
				cl = QColor(255, 255, 0, 128);
				width = 15;
			}
			else if (m_tags.value("aeroway") == "vacaterunway")
			{
				cl = QColor(0, 192, 0, 128);
				width = 5;
			}
			else if (m_tags.value("aeroway") == "tr_stopbar")
			{
				cl = QColor(0, 192, 192, 128);
				width = 15;
			}
		}
		else if (!properties.value("waterway").toString().isEmpty())
		{
			auto waterway = properties.value("waterway").toString();
			if (waterway == "drain")
			{
				cl = QColor(0, 255, 255, 128);
				width = 4.0f;
			}
			else if (waterway == "canal")
			{
				cl = QColor(0, 255, 255, 192);
				width = 6.0f;
			}
			else if (waterway == "river")
			{
				cl = QColor(0, 0, 255, 192);
				width = 10.0f;
			}
		}
		else if (!properties.value("barrier").toString().isEmpty())
		{
			cl = QColor(0, 255, 0, 192);
			width = 2.0f;
		}
		else if (!properties.value("man_made").toString().isEmpty())
		{
			cl = QColor(255, 0, 0, 192);
			width = 4.0f;
		}
		else if (!properties.value("railway").toString().isEmpty())
		{
			auto railway = properties.value("railway").toString();
			if (railway == "rail")
			{
				cl = QColor(0xe9, 0x99, 0x4a, 128);
				width = 4.0f;
			}
			else if (railway == "subway")
			{
				cl = QColor(0xe9, 0x99, 0x4a, 192);
				width = 4.0f;
			}
		}

		m_color = cl;
		coordinatesExtend.clear();
		for (auto latlnglist : coordinates)
		{
			LATLNGS_VECTOR extentmp = FunctionAssistant::extendLineToPolygonPlane(latlnglist, width);
			coordinatesExtend.emplace_back(std::make_tuple(std::move(extentmp), cl));
		}
	}

	void dealPointInfo()
	{
		QColor cl = FunctionAssistant::randColor(128);
		m_color = cl;
	}
};


#define PARSE_STRING_FROM_JSON(DST,SRC,ELE) DST.ELE = SRC.value(#ELE).toString().toStdString();

#define PARSE_DOUBLE_FROM_JSON(DST,SRC,ELE) DST.ELE = SRC.value(#ELE).toDouble();

#define PARSE_LATLNG_FROME_JSON(DST, SRC ) {\
auto array = SRC.toArray();\
    LAT_LNG latlng;\
    latlng.lng = array.at(0).toDouble();\
    latlng.lat = array.at(1).toDouble();\
    DST.emplace_back(std::move(latlng));\
}



struct GeoJsonInfos
{
	QString name;
	E_GEOTYPE type;
	QHash <QString, tagGeoJsonInfo> subItem;
	GeoJsonInfos()
	{
	}
};

#include <QJsonArray>

struct AgentKeyItemInfo
{
	QString agentKey;
	QString agentId;
	QString agentKeyword;
	QString agentName;
	QString agentNameI18n;
	QString agentType;
	QString modelUrlSlim;
	QString modelUrlFat;
	QString modelUrlMedium;
	QJsonArray modelUrlSymbols;

	void toJson(QJsonObject& jsitem)
	{
		jsitem.insert("agentKey", agentKey);
		jsitem.insert("agentId", agentId);
		jsitem.insert("agentKeyword", agentKeyword);
		jsitem.insert("agentName", agentName);
		jsitem.insert("agentNameI18n", agentNameI18n);
		jsitem.insert("agentType", agentType);
		jsitem.insert("modelUrlSlim", modelUrlSlim);
		jsitem.insert("modelUrlFat", modelUrlFat);
		jsitem.insert("modelUrlMedium", modelUrlMedium);
		jsitem.insert("modelUrlSymbols", modelUrlSymbols);
	}

	void fromJson(const QJsonObject& jsitem)
	{
		agentKey = jsitem.value("agentKey").toString();
		agentId = jsitem.value("agentId").toString();
		agentKeyword = jsitem.value("agentKeyword").toString();
		agentName = jsitem.value("agentName").toString();
		agentNameI18n = jsitem.value("agentNameI18n").toString();
		agentType = jsitem.value("agentType").toString();
		modelUrlSlim = jsitem.value("modelUrlSlim").toString();
		modelUrlFat = jsitem.value("modelUrlFat").toString();
		modelUrlMedium = jsitem.value("modelUrlMedium").toString();
		modelUrlSymbols = jsitem.value("modelUrlSymbols").toArray();
	}
};


struct AgentInstanceItemInfo
{
	QString agentInstId;
	QString agentOffsetKey;
	QString asmKey;

	AgentKeyItemInfo agentKeyItem;

	void toJson(QJsonObject& jsitem)
	{
		jsitem.insert("agentInstId", agentInstId);
		jsitem.insert("agentOffsetKey", agentOffsetKey);
		jsitem.insert("asmKey", asmKey);
		agentKeyItem.toJson(jsitem);
	}

	void fromJson(const QJsonObject& jsitem)
	{
		agentInstId = jsitem.value("agentInstId").toString();
		agentOffsetKey = jsitem.value("agentOffsetKey").toString();
		asmKey = jsitem.value("asmKey").toString();
		agentKeyItem.fromJson(jsitem);
	}
};

struct AgentInstanceInfo
{
	AgentInstanceItemInfo m_agentinfo;
	std::vector<AgentInstanceItemInfo> m_subagentinfo;

	void toJson(QJsonObject &jsitem)
	{
		m_agentinfo.toJson(jsitem);
	}

	void fromJson(const QJsonObject& jsitem)
	{
		m_agentinfo.fromJson(jsitem);
	}
};


enum E_FLIGHT_DEP_ARR_TYPE :UINT32
{
	E_FLIGHT_DEP_ARR_TYPE_DEP,
	E_FLIGHT_DEP_ARR_TYPE_ARR
};
#define TAKEOFF_AHEADTIME_MIN (30)
#define LANDING_AHEADTIME_MIN (10)
#define LANDING_BEHINDTIME_MIN (10)

struct FlightPlanConf
{
	QString m_Date;
	QString m_FilghtNumber;
	QString m_DepArrFlag;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
	QString m_PlaneNum;
	QString m_PlaneType;
	QString m_FlightClass;
	QString m_FlightLeg;
	QString m_FlightStartPlace;
	QString m_FlightEndPlace;
	uint64_t m_aheadtimelen;

	uint64_t m_PlanDateTimeTakeOff_ahead_timestamp;
	QString m_PlanDateTimeTakeOff;
	uint64_t m_PlanDateTimeTakeOff_timestamp;
	QString m_ExpectedDateTimeTakeOff;
	QString m_RealityDateTimeTakeOff;

	uint64_t m_PlanDateTimeLanding_ahead_timestamp;
	QString m_PlanDateTimeLanding;
	uint64_t m_PlanDateTimeLanding_timestamp;
	QString m_ExpectedDateTimeLanding;
	QString m_RealityDateTimeLanding;
	uint64_t m_PlanDateTimeLanding_behind_timestamp;
	uint64_t m_behindtimelen;

	QString m_Delay;
	QString m_Seat;
	QString m_Terminal;
	QString m_Runway;

    QString flightid;
    QString wps;
	FlightPlanConf()
	{

	}

	std::string printf()
	{
		std::stringstream ss;
		ss <<"["<< "计划日期：" << m_Date.toStdString()<<" "
			<< "航班号：" << m_FilghtNumber.toStdString() << " "
			<< "起降标识：" << m_DepArrFlag.toStdString() << " "
			<< "机号：" << m_PlaneNum.toStdString() << " "
			<< "机型：" << m_PlaneType.toStdString() << " "
			<< "航班分类：" << m_FlightClass.toStdString() << " "
			<< "始发地：" << m_FlightStartPlace.toStdString() << " "
			<< "到达地：" << m_FlightEndPlace.toStdString() << " "
			<< "计划起飞：" << m_PlanDateTimeTakeOff.toStdString() << " "
			<< "计划落地：" << m_PlanDateTimeLanding.toStdString() << " "
			<< "机位：" << m_Seat.toStdString() << " "
			<< "航站楼：" << m_Terminal.toStdString() << " "
            << "跑道：" << m_Runway.toStdString()<< " "
            << "航线：" << wps.toStdString()<<"]";
            return ss.str();
	}

	void updatePlanDateTimeTakeOff_timestamp(uint64_t _PlanDateTimeTakeOff_timestamp)
	{
		this->m_PlanDateTimeTakeOff_timestamp = _PlanDateTimeTakeOff_timestamp;
		this->m_aheadtimelen = TAKEOFF_AHEADTIME_MIN * 60;
		this->m_PlanDateTimeTakeOff_ahead_timestamp = this->m_PlanDateTimeTakeOff_timestamp - this->m_aheadtimelen;
	}
	void updatePlanDateTimeLanding_timestamp(uint64_t _PlanDateTimeLanding_timestamp)
	{
		this->m_PlanDateTimeLanding_timestamp = _PlanDateTimeLanding_timestamp;
		this->m_aheadtimelen = LANDING_AHEADTIME_MIN * 60;
		this->m_PlanDateTimeLanding_ahead_timestamp = this->m_PlanDateTimeLanding_timestamp - this->m_aheadtimelen;
		this->m_behindtimelen = LANDING_BEHINDTIME_MIN * 60;
		this->m_PlanDateTimeLanding_behind_timestamp = this->m_PlanDateTimeLanding_timestamp + this->m_behindtimelen;
	}
};


struct tagFlightEventTime
{
	uint64_t m_itime;
	uint64_t m_eventid;
	uint64_t m_day_senscod_offset_s;
	uint64_t m_prev_with_offset_s;
	uint64_t m_next_with_offset_s;

	uint64_t m_day_senscod_offset_ms;
	uint64_t m_prev_with_offset_ms;
	uint64_t m_next_with_offset_ms;


	std::list<FlightPlanConf*> m_flightCfgs;
	tagFlightEventTime * m_prev_;
	tagFlightEventTime * m_next_;
	tagFlightEventTime()
	{
		m_itime = 0;
		m_eventid = 0;
		m_day_senscod_offset_s = 0;
		m_prev_with_offset_s = 0;
		m_next_with_offset_s = 0;
		m_day_senscod_offset_ms = 0;
		m_prev_with_offset_ms = 0;
		m_next_with_offset_ms = 0;
		m_prev_ = nullptr;
		m_next_ = nullptr;
	}

	tagFlightEventTime(uint64_t itime, uint64_t eventid, uint64_t idayoffset)
	{
		m_itime = itime;
		m_eventid = eventid;
		m_day_senscod_offset_s = idayoffset;
		m_prev_with_offset_s = 0;
		m_next_with_offset_s = 0;
		m_day_senscod_offset_ms = idayoffset * 1000;
		m_prev_with_offset_ms = 0;
		m_next_with_offset_ms = 0;
		m_prev_ = nullptr;
		m_next_ = nullptr;
	}

	void setPrev(tagFlightEventTime * prev_)
	{
		m_prev_ = prev_;
		if (prev_)
		{
			m_prev_with_offset_s = m_itime - prev_->m_itime;
			m_prev_with_offset_ms = m_prev_with_offset_s * 1000;
		}
	}

	void setNext(tagFlightEventTime * next_)
	{
		m_next_ = next_;
		if (next_)
		{
			m_next_with_offset_s = next_->m_itime - m_itime;
			m_next_with_offset_ms = m_next_with_offset_s * 1000;

		}
	}

	void appendFlightCfg(FlightPlanConf* flightcfg)
	{
		m_flightCfgs.push_back(flightcfg);
	}
};

namespace std {
	template<> struct hash<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>> {
		size_t operator()(const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& p)const {
			return hash<QString>()(std::get<0>(p)) ^ \
				hash<uint32_t>()(std::get<1>(p)) ^ \
				hash<QString>()(std::get<2>(p));
		}
	};
	template <>
	struct equal_to<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>> {
		bool operator()(const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& left, const std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>& right) const {
			return left == right;
		}
	};
};

struct tagPoiItem
{
	QString poiKey;
	QString poiKeyword;
	UINT32 poiFrame;
	LAT_LNG poipoint;
	double alt;
	UINT32 poiDirection;
	QString poiName;
	QString poiNameI18n;
	std::tuple<UINT64, LAT_LNG> m_calibrate_osm_path_info;
	tagPoiItem()
	{
		m_calibrate_osm_path_info = std::make_tuple(0, LAT_LNG{ 0,0 });
	}
};

struct tagStandard_Taxiing_Path
{
	QString m_title;
	QString m_path;
	QString m_runway;
	QString m_parkingarea;
	QString m_pathdetail;
	QVector<QString> m_pathPoints;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
	void analysis();
};

class Dijkstra;
struct tagPath_Plan
{
	QString m_runway;
	QString m_parkingpoint;
	E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
	QString m_airportcode;
	QString m_path;
	QVector<std::tuple<QString, bool, int>> m_pathPoints;
	LATLNGS_VECTOR m_trackinglatlng;
	std::vector<std::tuple< UINT64, LAT_LNG>> m_tracking_osm_path_info;
	std::vector<std::tuple< UINT64, LAT_LNG>> m_tracking_osm_path_info_calibrate;

	LATLNGS_VECTOR m_extendwpslatlng;
	LATLNGS_VECTOR m_runwayextendwpslatlng;


	LATLNGS_VECTOR m_runway_total;

	LATLNGS_VECTOR m_extendwpslatlng_start_simple;
	LATLNGS_VECTOR m_extendwpslatlng_simple;
	LATLNGS_VECTOR m_runway_total_simple;

	QColor m_trackingcl;

	bool m_bValid;
	tagPath_Plan()
	{
		m_bValid = false;
	}

	void resize(int wpssize);
	void analysis(const std::unordered_map<QString, tagStandard_Taxiing_Path>& _Standard_Taxiing_Paths,
		const std::unordered_map<QString, tagPoiItem>& _poiitemsmap);

	QJsonObject toJson() const;
	void fromJson(const QJsonObject& jsonobj);


	QJsonObject outputgeojson();
};


Q_DECLARE_METATYPE(tagPath_Plan)

struct tagPathPlanInfo
{
	tagPath_Plan pathindex;
};

typedef std::unordered_map < QString, tagPathPlanInfo> RUNWAY_PATH;

typedef std::unordered_map < E_FLIGHT_DEP_ARR_TYPE, RUNWAY_PATH> ARR_DEP_RUNWAY_PATH;

struct PathPlanValidInfo
{
	enum E_PATH_TYPE
	{
		E_PATH_TYPE_UNKOWN = 0x00,
		E_PATH_TYPE_PLAN_PARKINGPOINT = 0x01,
		E_PATH_TYPE_PLAN_RUNWAY = 0x02,

		E_PATH_TYPE_CONFLICT_PARKINGPOINT = 0x04,
		E_PATH_TYPE_CONFLICT_RUNWAY = 0x08,

		E_PATH_TYPE_REALLOC_PARKINGPOINT = 0x10,
		E_PATH_TYPE_REALLOC_RUNWAY = 0x20,

		E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT = 0x40,
		E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY = 0x80,

		E_PATH_TYPE_REALLOC_PARKINGPOINT_UNUSE = 0x100,
		E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY_UNUSE = 0x200,
	};
	E_PATH_TYPE eStatus;

	QString target_parkingpoint;
	QString target_runway;
	QString alloc_parkingpoint;
	QString alloc_runway;
	PathPlanValidInfo()
	{
		eStatus = E_PATH_TYPE_UNKOWN;
	}
};

typedef std::tuple<tagPath_Plan*, FlightPlanConf*, PathPlanValidInfo> FLIGHTPLAN_PATH_INFO;


typedef std::function<void(tagPath_Plan *)> deal_data_func_callback;


template<typename T>
class ThreadTaskProcessor
{
public:
	ThreadTaskProcessor(int id,
		deal_data_func_callback _pCallbackfunc,
		const T& param1)
		:m_id(id)
		, m_pCallbackfunc(std::move(_pCallbackfunc))
		, m_param1(std::move(param1))
	{
		//m_processorNum++;
	}
	virtual ~ThreadTaskProcessor() {}
	void operator()()
	{
		if (m_pCallbackfunc)
		{
			m_pCallbackfunc(m_param1);
		}
	}

	static int getProcessorNum() {}
private:
	int m_id;
	deal_data_func_callback m_pCallbackfunc;
	T m_param1;
};




namespace TaskThreadPool
{

	class ThreadPool {
	public:
		ThreadPool(size_t);
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(Args...)>::type>;
		~ThreadPool();
	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// the task queue
		std::queue< std::function<void()> > tasks;

		// synchronization
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
	};


#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool(size_t threads)
		: stop(false)
	{
		for (size_t i = 0; i < threads; ++i)
		{
			workers.emplace_back([this] {
#ifdef WIN32
				SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#else
				// �����߳����ȼ�Ϊ���
				struct sched_param params;
				params.sched_priority = sched_get_priority_max(SCHED_FIFO);
				pthread_setschedparam(pthread_self(), SCHED_FIFO, &params);
#endif
				for (;;)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->queue_mutex);
						this->condition.wait(lock,
							[this] { return this->stop || !this->tasks.empty(); });
						if (this->stop && this->tasks.empty())
							return;
						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();
				}
			});
		}
	}

	// add new work item to the pool
	template<class F, class... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(queue_mutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread &worker : workers)
		{
			worker.join();
		}
	}
}

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


struct tagAirPortInfo {
	std::unordered_map<LAT_LNG, QString> m_aerowayinfo;

	QHash <QString, GeoJsonInfos> m_WPSRunwayInfos;

	QHash <QString, LATLNGS_VECTOR> m_RunwayInfos;

	QHash <QString, GeoJsonInfos> m_GeoJsonInfos;

	std::unordered_map<QString, tagPoiItem> m_poiitemsmap;

	std::map<QString, ARR_DEP_RUNWAY_PATH> m_Path_Plans;

	std::list<tagPath_Plan*> m_arrpaths;
	std::list<tagPath_Plan*> m_deppaths;

	QStringList m_runways;

	std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE>> m_linestringpoints;

	void data_reset()
	{
		m_aerowayinfo.clear();
		m_WPSRunwayInfos.clear();
		m_RunwayInfos.clear();
		m_GeoJsonInfos.clear();

		m_poiitemsmap.clear();

		m_Path_Plans.clear();

		m_arrpaths.clear();

		m_deppaths.clear();

		m_runways.clear();

		m_linestringpoints.clear();
	}

	LAT_LNG center;
	LAT_LNG topleft;
	LAT_LNG bottomright;
	void getArea()
	{
		double lat_total = 0.0;
		double lng_total = 0.0;
		for (auto itor = m_aerowayinfo.begin(); itor != m_aerowayinfo.end(); itor++)
		{
			lat_total += itor->first.lat;
			lng_total += itor->first.lng;
		}
		center.lat = lat_total / m_aerowayinfo.size();
		center.lng = lng_total / m_aerowayinfo.size();

		glm::dvec3 directionVectorleft = glm::dvec3(-1, 0, 0);
		auto left = FunctionAssistant::calculateDirectionExtendPoint(center, glm::dvec3(-1, 0, 0), 10000);
		auto right = FunctionAssistant::calculateDirectionExtendPoint(center, glm::dvec3(1, 0, 0), 10000);
		auto top = FunctionAssistant::calculateDirectionExtendPoint(center, glm::dvec3(0, -1, 0), 10000);
		auto bottom = FunctionAssistant::calculateDirectionExtendPoint(center, glm::dvec3(0, 1, 0), 10000);

		topleft = LAT_LNG{ top.lat, left.lng };
		bottomright = LAT_LNG{ bottom.lat, right.lng };
	}

};


struct flightinstance
{
	quint64 flightinstanceid;
	PathPlanValidInfo pathplanvalidinfo;
	FlightPlanConf * pflightdata;
	tagPath_Plan* _tagPath_Plan;
	std::string printf()
	{
		std::stringstream ss;
		ss << "[" << "计划停机位：" << pathplanvalidinfo.target_parkingpoint.toStdString() << " "
			<< "计划跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
			<< "使用停机位：" << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " "
			<< "使用跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
			<< "航班计划：" << pflightdata->printf() << "]";
		return ss.str();
	}
};


typedef std::function<void ()> environment_init_succeed_callback;

class QNetworkDiskCache;
class QNetworkAccessManager;
class KlusterWebSocketServer;
class DataManager
{
public:
	static DataManager& getInstance();
	virtual~DataManager();

	void setCurrentAirport(const QString& airport_code);
	QString getCurrentAirport();

	void init(const QString& path);

	bool readGeoJsonData(const std::string& filename, GeoJsonInfos& geoinfos);

	QStringList getAirPortList();

	std::unordered_map<QString, std::tuple<QString, QString>>& getAirPortNameList();

	tagAirPortInfo * getAirportInfo(const QString & airport_code);

	tagAirPortInfo * getCurrentAirportInfo();

	HttpClient *pHttpClient() const;

	std::unordered_map<QString, std::unordered_map<QString, QString> > &InstagentInstance();
	void setInstagentInstance(const std::unordered_map<QString, std::unordered_map<QString, QString> > &newInstagentInstance);

	std::unordered_map<QString, AgentInstanceInfo> &agentInstances();
	void setAgentInstances(const std::unordered_map<QString, AgentInstanceInfo> &newAgentInstances);

	std::unordered_map<QString, AgentKeyItemInfo>& agentKeyMaps();
	void setAgentKeyMaps(const std::unordered_map<QString, AgentKeyItemInfo> &newAgentKeyMaps);
	   
	QString  getEntityIcon(const TYPE_ULID &uildsrc);
	void requestEntityIcon(const TYPE_ULID &uildsrc);
	void setEntityIcon(const TYPE_ULID &uildsrc, const QJsonObject &entityAgentObj);
	void setEntityIconName(const TYPE_ULID &uildsrc, const QString &entityAgentObj);
	
    void trans_log(const QString& title, const std::stringstream& log, const std::stringstream& reson, bool bTrans = true);
    void trans_review_log(const std::string &title, const std::string &log, const std::string &reson, const std::string &trimestampstr, bool bTrans = true);
    void updateConnecting(bool bstatus);
    void deal_review(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp);
    void trans_review(BYTE *pData, UINT32 iDataLen);

    void trans_sim_data_ok();
    void trans_sim_runtime_end();
    void trans_sim_review_end();

    void websocket_receive_callback(const BYTE *pOriginData, const UINT32 &userPayloadSize);

    void loadreviews();

    void exportexcel();

    void setCurrentReviewItem(const tagReplayItemInfo& _currentreview);

    void update_flightInfo_to_db(uint64_t id, const flightinstance& _flightinstance);
    void update_SiminfoLog_to_db(const SiminfoLog& _flightinstance);
    void update_infoLog_to_db(infoLog &&_flightinstance);

    void getTimestampRange_reviewdata(const UINT64 &timestamp_start, const UINT64 &timestamp_end);


    FlightPlanConfLog get_flightInfo_from_db(const UINT64& flightid);
    std::map<UINT64,SiminfoLog> get_SiminfoLog_from_dbs(const UINT64 & timestamp_start,const UINT64 & timestamp_end);
    std::map<UINT64, infoLog> get_infoLog_from_db(const UINT64 & timestamp_start,const UINT64 & timestamp_end);

    void updatereviewsiminfo();
    void updatereviewsrecorddir(const QString& dirs);

    void rmdirs(const QString& dirs);
    QJsonObject outputSimulationRange();
    QJsonObject outputSimulationReviewRange();
    QString getModelPath(const UINT64& agentid);
public:

	bool getLinestringPoint(UINT64&item_id, const LAT_LNG& latlng);

	std::vector<LAT_LNG> getpath(const LAT_LNG& a, const LAT_LNG& b);

private:
	DataManager();
	void initHttp();


	void readAirportInfoPath(const QString &path);

	void readGeoDirPath(const QString &path);
	void readGeoJsonFile(const std::string & filename);
	void readWpsRunwayPath(const QString& path);
	void readWpsRunwayFile(const std::string& filename);


	void readAerowayFile(const std::string & filename);

	void readWaypointsGeoJsonFile(const std::string & filename);

	void readPathDir(const QString &path);

	void readPoisJsonFile(const QString & filename);

	void readPathInfoFile(const QString & filename);
	void writePathInfoFile(const QString & filename);

	void updateDijkstraMap(const LAT_LNG& latlng, const LAT_LNG& lstlatlng, bool bBegin);

	bool getLinestringPointId(UINT64&item_id, const LAT_LNG& latlng);

	bool appendLinestringPoint(UINT64&item_id, const LAT_LNG& latlng, E_POINT_TYPE ePointType);
	UINT64 appendDijkstraMap(const LAT_LNG& currentlatlng, const LAT_LNG& lstlatlng, UINT64 lstid, E_POINT_TYPE ePointExtendType, E_POINT_TYPE ePointTypeSrc);

	void findNextPoiDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, const LAT_LNG& latlng, const std::vector<std::tuple<LAT_LNG, UINT64>>& pts);

	void findDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng);
	void findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const UINT64& excludeIDs);
	void findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const LAT_LNG& excludelatlng);

	void findMindisWithMidPointBetweenPoints(std::tuple<LAT_LNG, UINT64>& midpoint,
		const std::tuple<LAT_LNG, UINT64>& pointa,
		const std::tuple<LAT_LNG, UINT64>& pointb);

    bool getDirFilesInfo(int index, const QString& titledir,const QString &srcparetndir, const QString &srcdir, bool bRecv);

    void data_deal_thread_func(void *pParam);

    void load_ive_model_path();
public:
    tagFlightEventTime * find_target_event(uint64_t timestamp);
	std::map<uint64_t, tagFlightEventTime> total_flightEventTimedata;

	std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> total_flightdata;
	std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>> m_flightdata;
    QString m_simname;
    QString m_log_dir_simname;

    tagReplayItemInfo m_currentreview;
    std::unordered_map<UINT64, FlightPlanConfLog> m_recordflights;
    std::unordered_map<UINT64, SiminfoLog> m_recordsims;
    std::unordered_map<UINT64, infoLog> m_recordlogs;
    stdutils::OriMutexLock m_recordruntimelogsmutex;
    std::list<infoLog> m_recordruntimelogs;

    std::unordered_map<QString, tagReplayItemInfo> m_reviewdata;


    std::unordered_map<QString, std::unordered_map<QString, QString>> m_InstagentInstance;

    environment_init_succeed_callback m_environment_init_succeed_callback;

	uint64_t m_play_min;
	uint64_t m_play_max;
	uint64_t m_play_range;
	uint64_t m_play_cur;
	uint64_t m_play_pause;

	double m_play_cur_d;


    uint64_t m_review_min;
    uint64_t m_review_max;
    uint64_t m_review_range;
    uint64_t m_review_cur;
	SiminfoLog m_reviewsimifo;

    double m_review_speed;
    QString m_review_status;

	std::unordered_map<QString, flightinstance> m_parkingpointuseinfo;

	void appendFlight(quint64 flightid,const flightinstance & data);
	flightinstance getflightInfo(quint64 flightid);
	QReadWriteLock m_flights_mutex;
	std::unordered_map<quint64, flightinstance> m_flights;

	std::unordered_map<QString, flightinstance> m_runwayuseinfo;

    EventDriver *m_peventDriver;

    EventDriver *peventDriver() const;
    void setPeventDriver(EventDriver *newPeventDriver);

    void setEnvironment_init_succeed_callback(environment_init_succeed_callback newEnvironment_init_succeed_callback);
    void deal_receive_data_callback(const COMM_CHANNEL_INFO &channelinfo, const std::string &pdata);
private:
	HttpClient* m_pHttpClient;


    std::map<QString,std::tuple<int,QList<QString>>> m_ive_paths;
    QMap<UINT64,QString> m_AGENT_ive_paths;

	std::unordered_map<QString, AgentInstanceInfo> m_agentInstances;
    std::unordered_map<QString, AgentKeyItemInfo> m_agentKeyMaps;


    /////////////////////////////////////////////////////////////////////////////////////////////////
    stdutils::OriMutexLock m_saveFlightPlanConfLogsmutex;
    std::list<FlightPlanConfLog> m_saveFlightPlanConfLogs;
    stdutils::OriMutexLock m_saveSiminfoLogsmutex;
    std::list<SiminfoLog> m_saveSiminfoLogs;
    stdutils::OriMutexLock m_saveinfoLogsmutex;
    std::list<infoLog> m_saveinfoLogs;


    stdutils::OriThread* m_hSqliteRecordDataDealThread;
    stdutils::OriMutexLock m_dealmutex;
    stdutils::OriWaitCondition m_dealfullCond;



	UINT32 m_total_path_num;
	std::atomic_uint32_t m_deal_path_num;
	TaskThreadPool::ThreadPool* m_pThreadPool;

	struct entityAgentItem
	{
		QJsonObject entityAgentObj;
		QString symbolName;
	};

	QReadWriteLock m_entityicon_mutex;
	std::unordered_map < TYPE_ULID, entityAgentItem> m_entityicon;

	QString m_deal_path;

	//机场相关
	/////////////////////////////////////////////////////////////////////////////////////////////////


	std::unordered_map<QString, std::tuple<QString, QString>> m_airporttitles;

	std::unordered_map<QString, tagAirPortInfo> m_airports;

	tagAirPortInfo* m_pCurrentAirPortInfo;

	QString m_currentAirport;


	Dijkstra* m_pDijkstra;

    KlusterWebSocketServer* m_logWbs;
    COMM_CHANNEL_INFO m_publish_process_status;
    COMM_CHANNEL_INFO m_subscribe_gaeactorhub_status;
    KlusterWebSocketServer* m_binary_websocket_server_ex;
    bool m_bGaeactor_connected = false;
};
#endif
