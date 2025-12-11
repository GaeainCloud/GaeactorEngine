#pragma execution_character_set("utf-8")
#include "datamanager.hpp"
#include <QJsonObject>
#include <string>
#include <QJsonArray>

#include <QDir>

#include <iostream>
#include <QCoreApplication>
#include "../components/function.h"

#include "src/algorithm/Dijkstra.h"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include "KlusterWebSocketServer.h"
#include "../components/configmanager.h"
#include "settingsconfig.h"

#include "loghelper.h"

#include "src/storage/OriginalDataInputManager.h"

#include <sqlite_orm/sqlite_orm.h>
#include "./src/OriginalDateTime.h"
#include "./src/OriginalThread.h"

#include <QtXlsx/QtXlsx>
#include <QTimeZone>

#include "runningmodeconfig.h"


#if 0
#define POINT_EXTEND_METRE	(15.0f)
#define GRAPH_MAX_VEX_DIFF	(3.0f)
#else
#define POINT_EXTEND_METRE	(30.0f)
#define GRAPH_MAX_VEX_DIFF	(1.5f)
#endif
#define EXTEND_INDEX (3)

//#define DEP_END_INDEX_DIRECT
#define ARR_END_INDEX_EXINCLUDE_PBNPOINT
#define DEAL_ANGLE (20.0f)

#define TARGET_HEIGHT_METER (300.0f)
#define HOLDPOINT_EXTEND_METER (25)
#define HOLDPOINT_EXTEND_AREA_METER (150)
#define TAKEOFF_METER (1750)

#define LANDINGPOINT_METER (15)

//#define CALC_PATH

using namespace sqlite_orm;


inline auto initStorage(const std::string& path) {
    using namespace sqlite_orm;
    auto storage_instance = make_storage(
        path,
        make_table("simrange",
                   make_column("id", &SiminfoLog::id,sqlite_orm::primary_key().autoincrement()),
                   make_column("simname", &SiminfoLog::siname),
                   make_column("min", &SiminfoLog::min),
                   make_column("max", &SiminfoLog::max),
                   make_column("step_interval", &SiminfoLog::step_interval),
                   make_column("step_dt", &SiminfoLog::step_dt),
                   make_column("step_freq", &SiminfoLog::step_freq),
                   make_column("one_second_sim_step_second", &SiminfoLog::one_second_sim_step_second),
                   make_column("timestamp", &SiminfoLog::timestamp)),

        make_table("simlog",
                   make_column("id", &infoLog::id,sqlite_orm::primary_key().autoincrement()),
                   make_column("title", &infoLog::title),
                   make_column("log", &infoLog::log),
                   make_column("reason", &infoLog::reason),
                   make_column("bTrans", &infoLog::bTrans),
                   make_column("timestampstr", &infoLog::timestampstr),
                   make_column("timestamp", &infoLog::timestamp)),

        make_table("simdata",
                   make_column("id", &FlightPlanConfLog::id,sqlite_orm::primary_key().autoincrement()),
                   make_column("flightinstanceid", &FlightPlanConfLog::flightinstanceid),
                   make_column("m_Date", &FlightPlanConfLog::m_Date),
                   make_column("m_FilghtNumber", &FlightPlanConfLog::m_FilghtNumber),
                   make_column("m_DepArrFlag", &FlightPlanConfLog::m_DepArrFlag),
                   make_column("m_flight_dep_arr_type", &FlightPlanConfLog::m_flight_dep_arr_type),
                   make_column("m_PlaneNum", &FlightPlanConfLog::m_PlaneNum),
                   make_column("m_PlaneType", &FlightPlanConfLog::m_PlaneType),
                   make_column("m_FlightClass", &FlightPlanConfLog::m_FlightClass),
                   make_column("m_FlightLeg", &FlightPlanConfLog::m_FlightLeg),
                   make_column("m_FlightStartPlace", &FlightPlanConfLog::m_FlightStartPlace),
                   make_column("m_FlightEndPlace", &FlightPlanConfLog::m_FlightEndPlace),
                   make_column("m_aheadtimelen", &FlightPlanConfLog::m_aheadtimelen),
                   make_column("m_PlanDateTimeTakeOff_ahead_timestamp", &FlightPlanConfLog::m_PlanDateTimeTakeOff_ahead_timestamp),
                   make_column("m_PlanDateTimeTakeOff", &FlightPlanConfLog::m_PlanDateTimeTakeOff),
                   make_column("m_PlanDateTimeTakeOff_timestamp", &FlightPlanConfLog::m_PlanDateTimeTakeOff_timestamp),
                   make_column("m_ExpectedDateTimeTakeOff", &FlightPlanConfLog::m_ExpectedDateTimeTakeOff),
                   make_column("m_RealityDateTimeTakeOff", &FlightPlanConfLog::m_RealityDateTimeTakeOff),
                   make_column("m_PlanDateTimeLanding_ahead_timestamp", &FlightPlanConfLog::m_PlanDateTimeLanding_ahead_timestamp),
                   make_column("m_PlanDateTimeLanding", &FlightPlanConfLog::m_PlanDateTimeLanding),
                   make_column("m_PlanDateTimeLanding_timestamp", &FlightPlanConfLog::m_PlanDateTimeLanding_timestamp),
                   make_column("m_ExpectedDateTimeLanding", &FlightPlanConfLog::m_ExpectedDateTimeLanding),
                   make_column("m_RealityDateTimeLanding", &FlightPlanConfLog::m_RealityDateTimeLanding),
                   make_column("m_PlanDateTimeLanding_behind_timestamp", &FlightPlanConfLog::m_PlanDateTimeLanding_behind_timestamp),
                   make_column("m_behindtimelen", &FlightPlanConfLog::m_behindtimelen),
                   make_column("m_Delay", &FlightPlanConfLog::m_Delay),
                   make_column("m_Seat", &FlightPlanConfLog::m_Seat),
                   make_column("m_Terminal", &FlightPlanConfLog::m_Terminal),
                   make_column("m_Runway", &FlightPlanConfLog::m_Runway),
                   make_column("flightid", &FlightPlanConfLog::flightid),
                   make_column("target_parkingpoint", &FlightPlanConfLog::target_parkingpoint),
                   make_column("target_runway", &FlightPlanConfLog::target_runway),
                   make_column("alloc_parkingpoint", &FlightPlanConfLog::alloc_parkingpoint),
                   make_column("alloc_runway", &FlightPlanConfLog::alloc_runway))
        );

    storage_instance.sync_schema();
    return storage_instance;
}





DataManager& DataManager::getInstance()
{
	static DataManager configmanager;
	return configmanager;
}

void DataManager::setCurrentAirport(const QString& airport_code)
{
	m_currentAirport = airport_code;
}

QString DataManager::getCurrentAirport()
{
	return m_currentAirport;
}

DataManager::DataManager()
	:m_pHttpClient(nullptr),
	mCache(nullptr),
	networkManager(nullptr),
	m_pThreadPool(nullptr),
    m_pDijkstra(nullptr),
    m_environment_init_succeed_callback(nullptr),
    m_logWbs(nullptr),
    m_binary_websocket_server_ex(nullptr)
{

    QDateTime datetime = QDateTime::currentDateTime().toUTC();
    QString dateStr = datetime.toString("yyyyMMddhhmmsszzz");

    updatereviewsrecorddir("TierAirlineAirPortSimulation_custom_"+dateStr);

	initHttp();

	QString LocalImageLayerDataSourceDir = QCoreApplication::applicationDirPath();
	init(LocalImageLayerDataSourceDir);

#ifdef _MSC_VER
    m_hSqliteRecordDataDealThread = new stdutils::OriThread(std::bind(&DataManager::data_deal_thread_func,this,std::placeholders::_1),\
                                                nullptr,\
                                                THREAD_PRIORITY_NORMAL);
#else
    m_hSqliteRecordDataDealThread = new stdutils::OriThread(std::bind(&DataManager::data_deal_thread_func,this,std::placeholders::_1),\
                                                nullptr,\
                                                2);
#endif
    m_hSqliteRecordDataDealThread->start();
}

#include <QDir>

void DataManager::init(const QString& path)
{
	m_deal_path = path + "/data/airport/";

	readAirportInfoPath(m_deal_path);

	auto jsonobjs = FunctionAssistant::read_json_file_object(m_deal_path + "/airport.json");

	auto airports = jsonobjs.value("airports").toArray();
	for (auto airportsitem : airports)
	{
		auto airportsobj = airportsitem.toObject();
		auto airportcode = airportsobj.value("code").toString();
		auto airportname = airportsobj.value("name").toString();
		auto airportnamei18n = airportsobj.value("namei18n").toString();
		m_airporttitles.insert(std::make_pair(airportcode, std::make_tuple(airportname, airportnamei18n)));
	}
}


void DataManager::updateDijkstraMap(const LAT_LNG& latlng, const LAT_LNG& lstlatlng, bool bBegin)
{
	if (bBegin)
	{
		UINT64 item_id;
		appendLinestringPoint(item_id, latlng, E_POINT_TYPE_OSM);
	}
	else
	{
		UINT64 lstitem_id;
		bool bExist = getLinestringPoint(lstitem_id, lstlatlng);
		if (bExist)
		{
			appendDijkstraMap(latlng, lstlatlng, lstitem_id, E_POINT_TYPE_OSM_EXTEND, E_POINT_TYPE_OSM);
		}
	}
}

bool DataManager::getLinestringPointId(UINT64&item_id, const LAT_LNG& latlng)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return false;
	}

	auto _linestringpoits_itor = std::find_if(m_pCurrentAirPortInfo->m_linestringpoints.begin(),
		m_pCurrentAirPortInfo->m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		item_id = _linestringpoits_itor->first;
		return true;
	}
	return false;
}

bool DataManager::appendLinestringPoint(UINT64&item_id, const LAT_LNG& latlng, E_POINT_TYPE ePointType)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return false;
	}
	bool bNewAppend = false;
	item_id = 0;
	auto _linestringpoits_itor = std::find_if(m_pCurrentAirPortInfo->m_linestringpoints.begin(),
		m_pCurrentAirPortInfo->m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (_linestringpoits_itor == m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		item_id = FunctionAssistant::generate_random_positive_uint64();
		m_pCurrentAirPortInfo->m_linestringpoints.insert(std::make_pair(item_id, std::make_tuple(latlng, ePointType)));
		if (m_pDijkstra)
		{
			m_pDijkstra->appendNode(item_id, latlng);
		}
		bNewAppend = true;
	}
	else
	{
		item_id = _linestringpoits_itor->first;
		switch (ePointType)
		{
		case E_POINT_TYPE_OSM:
		case E_POINT_TYPE_OSM_EXTEND:
		{
			E_POINT_TYPE &oldePointType = std::get<1>(_linestringpoits_itor->second);
			if (oldePointType != ePointType)
			{
				LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
				if (m_pDijkstra)
				{
					m_pDijkstra->appendNode(item_id, latlng);
				}
				if (oldePointType != E_POINT_TYPE_OSM && ePointType == E_POINT_TYPE_OSM)
				{
					oldePointType = ePointType;
				}
				//oldePointType = ePointType;
			}
		}
		break;
		default:
		{
		}
		break;
		}

		bNewAppend = false;
	}
	return bNewAppend;
}

UINT64 DataManager::appendDijkstraMap(const LAT_LNG& latlng, const LAT_LNG& lstlatlng, UINT64 lstid, E_POINT_TYPE ePointExtendType, E_POINT_TYPE ePointTypeSrc)
{
	UINT64 currentid;
	double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
	if (dis > POINT_EXTEND_METRE)
	{
		int step = dis / POINT_EXTEND_METRE;
		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
		LAT_LNG lstextendpt = lstlatlng;
		UINT64 lstextenditem_id = lstid;
		for (int i = 1; i < step + 1; i++)
		{
			LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, POINT_EXTEND_METRE);
			appendLinestringPoint(currentid, currentextendpt, ePointExtendType);
			if (m_pDijkstra)
			{
				m_pDijkstra->updateEdge(lstextenditem_id, currentid, POINT_EXTEND_METRE);
			}
			lstextendpt = currentextendpt;
			lstextenditem_id = currentid;
		}
		double rdis = dis - POINT_EXTEND_METRE * step;

		appendLinestringPoint(currentid, latlng, ePointTypeSrc);
		if (m_pDijkstra)
		{
			m_pDijkstra->updateEdge(lstextenditem_id, currentid, rdis);
		}
	}
	else
	{
		appendLinestringPoint(currentid, latlng, ePointTypeSrc);
		if (m_pDijkstra)
		{
			m_pDijkstra->updateEdge(lstid, currentid, dis);
		}
	}
	return currentid;
}

bool DataManager::getLinestringPoint(UINT64&item_id, const LAT_LNG& latlng)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return false;
	}
	bool bExist = false;
	auto lastlng_linestringpoits_itor = std::find_if(m_pCurrentAirPortInfo->m_linestringpoints.begin(),
		m_pCurrentAirPortInfo->m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (lastlng_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		item_id = lastlng_linestringpoits_itor->first;
		bExist = true;
	}
	return bExist;
}





std::vector<LAT_LNG> DataManager::getpath(const LAT_LNG& a, const LAT_LNG& b)
{
	std::vector<LAT_LNG> ret;
	if (m_pDijkstra)
	{
		UINT64 lstitem_id1;
		UINT64 lstitem_id2;
		bool bExist1 = DataManager::getInstance().getLinestringPoint(lstitem_id1, a);
		bool bExist2 = DataManager::getInstance().getLinestringPoint(lstitem_id2, b);
		tagPathInfo pathInfo = m_pDijkstra->getPathThreadSafe(lstitem_id1, lstitem_id2);

		if (!pathInfo.m_path.empty())
		{
			ret.reserve(pathInfo.m_path.size());
			for (int j = 0; j < pathInfo.m_path.size(); j++)
			{
				ret.push_back(pathInfo.m_path.at(j).m_pos);
			}
		}
	}
	return ret;
}

void DataManager::findNextPoiDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, const LAT_LNG& latlng, const std::vector<std::tuple<LAT_LNG, UINT64>>& pts)
{
	calibrate_id = 0;
	calibrate_latlng = LAT_LNG{ 0,0 };
	E_POINT_TYPE ePointType;
	float dismin = std::numeric_limits<float>::max();
	auto _linestringpoits_itor = pts.begin();
	while (_linestringpoits_itor != pts.end())
	{
		double dis = FunctionAssistant::calc_dist(std::get<0>(*_linestringpoits_itor), latlng);
		if (dis < dismin)
		{
			dismin = dis;
			calibrate_id = std::get<1>(*_linestringpoits_itor);
			calibrate_latlng = std::get<0>(*_linestringpoits_itor);
		}
		_linestringpoits_itor++;
	}
}

void DataManager::findDistanceMinPoint(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	calibrate_id = 0;
	calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_pCurrentAirPortInfo->m_linestringpoints.begin();
	while (_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		double dis = FunctionAssistant::calc_dist(std::get<0>(_linestringpoits_itor->second), latlng);
		if (dis < dismin &&
			(std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM || std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM_EXTEND))
		{
			dismin = dis;
			calibrate_id = _linestringpoits_itor->first;
			calibrate_latlng = std::get<0>(_linestringpoits_itor->second);
			ePointType = std::get<1>(_linestringpoits_itor->second);
		}
		_linestringpoits_itor++;
	}
}

void DataManager::findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const UINT64& excludeIDs)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	calibrate_id = 0;
	calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_pCurrentAirPortInfo->m_linestringpoints.begin();
	while (_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		double dis = FunctionAssistant::calc_dist(std::get<0>(_linestringpoits_itor->second), latlng);
		if (dis < dismin &&
			(std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM || std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM_EXTEND) &&
			excludeIDs != _linestringpoits_itor->first)
		{
			dismin = dis;
			calibrate_id = _linestringpoits_itor->first;
			calibrate_latlng = std::get<0>(_linestringpoits_itor->second);
			ePointType = std::get<1>(_linestringpoits_itor->second);
		}
		_linestringpoits_itor++;
	}
}

void DataManager::findDistanceMinPointExclude(UINT64& calibrate_id, LAT_LNG& calibrate_latlng, E_POINT_TYPE& ePointType, const LAT_LNG& latlng, const LAT_LNG& excludelatlng)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	calibrate_id = 0;
	calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_pCurrentAirPortInfo->m_linestringpoints.begin();
	while (_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
	{
		double dis = FunctionAssistant::calc_dist(std::get<0>(_linestringpoits_itor->second), latlng);
		if (dis < dismin &&
			(std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM || std::get<1>(_linestringpoits_itor->second) == E_POINT_TYPE_OSM_EXTEND) &&
			std::get<0>(_linestringpoits_itor->second) != excludelatlng)
		{
			dismin = dis;
			calibrate_id = _linestringpoits_itor->first;
			calibrate_latlng = std::get<0>(_linestringpoits_itor->second);
			ePointType = std::get<1>(_linestringpoits_itor->second);
		}
		_linestringpoits_itor++;
	}
}

void DataManager::findMindisWithMidPointBetweenPoints(std::tuple<LAT_LNG, UINT64>& midpoint,
	const std::tuple<LAT_LNG, UINT64>& pointa,
	const std::tuple<LAT_LNG, UINT64>& pointb)
{
	LAT_LNG midpttmp = FunctionAssistant::calculate_intermediate_coordinate(std::get<0>(pointa), std::get<0>(pointb));

    E_POINT_TYPE ePointType;
    findDistanceMinPoint(std::get<1>(midpoint), std::get<0>(midpoint), ePointType, midpttmp);
}




bool clearDir(QString path)
{
    if (path.isEmpty())
    {
        return false;
    }

    QDir dir(path);
    if (!dir.exists())
    {
        return false;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();
    for (auto file : fileList)
    {
        if (file.isFile())
        {
            file.dir().remove(file.fileName());
        }
        else
        {
            clearDir(file.absoluteFilePath());
            file.dir().rmdir(file.absoluteFilePath());
        }
    }

    return true;
}



bool DataManager::getDirFilesInfo(int index,const QString& titledir,const QString &srcparetndir, const QString &srcdir, bool bRecv)
{
    QString prefixstr = (bRecv) ? "Recv" : "Send";

    //QString srcdir = QDir::currentPath() + srcobj.value("RecordFilePath" + prefixstr).toString();
    tagReplayItemInfo item;
    item.m_index = index;
    QString datetimestr = srcparetndir.mid(srcparetndir.lastIndexOf("_")+1, srcparetndir.length() - srcparetndir.lastIndexOf("_"));
    QDateTime datetime = QDateTime::fromString(datetimestr, "yyyyMMddhhmmsszzz");

    datetime.setTimeSpec(Qt::UTC);
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
    QDateTime localTime = datetime;
#else /* UNIX */
    //    // 获取本地时区
    //    //QTimeZone localTimeZone = QTimeZone::systemTimeZone();
    //    // 设置时区偏移量为+8:00
    //    QTimeZone localTimeZone = QTimeZone("Asia/Shanghai");

    //    // 将UTC时间转换为本地时间
    //    QDateTime localTime = datetime.toTimeZone(localTimeZone);
    QDateTime localTime = datetime.toOffsetFromUtc(8 * 60 * 60);
#endif

    item.m_date = localTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    item.m_absoluteFilePath = srcdir;
    item.m_titlename = srcparetndir;
	item.m_titledir = titledir;
    //item.m_startPos = srcobj.value("RecordCountDataSize" + prefixstr).toString().toULongLong();
    //item.m_startFrames = srcobj.value("RecordCountFrameCount" + prefixstr).toString().toULongLong();

    QDir recv_dir(srcdir);
    recv_dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    QFileInfoList fileEntryListrecv = recv_dir.entryInfoList();
    bool bexist = false;
    for (auto fileinforecv : fileEntryListrecv)
    {
        if (fileinforecv.isFile() && fileinforecv.fileName().endsWith(FILE_SUFFIX) && fileinforecv.fileName().startsWith(prefixstr.toLower()))
        {
            item.m_size += fileinforecv.size();
            originaldatastoragestd::tagOriginalDataFileHeadRecordInfo recordInfo;
            if (originaldatastoragestd::OriginalDataInputManager::getFileTimeStamp(fileinforecv.absoluteFilePath().toStdString().c_str(), recordInfo))
            {
                item.m_iValidDataTotalLen += recordInfo.getCurrentFileValidDataMaxLen();
                item.m_iFrames += recordInfo.getCurrentFileFrames();
                if (0 == item.m_iTimeStampStart)
                {
                    item.m_iTimeStampStart = recordInfo.m_iallTimeStampStart;
                }
                item.m_iTimeStampEnd = recordInfo.m_iallTimeStampEnd;
                bexist = true;
            }
        }
    }
    if (bexist)
    {
        m_reviewdata.insert(std::make_pair(item.m_titlename,std::move(item)));
    }
    return bexist;
}

QJsonObject DataManager::outputSimulationRange()
{
	QDateTime dt_max = QDateTime::fromMSecsSinceEpoch((DataManager::getInstance().m_play_max) * 1000);
	QString dt_st_max = dt_max.toString("yyyy-MM-dd hh:mm:ss");

	QDateTime dt_min = QDateTime::fromMSecsSinceEpoch((DataManager::getInstance().m_play_min) * 1000);
    QString dt_st_min = dt_min.toString("yyyy-MM-dd hh:mm:ss");


	QDateTime dt_cur = QDateTime::fromMSecsSinceEpoch((DataManager::getInstance().m_play_cur) * 1000);
	QString dt_st_cur = dt_cur.toString("yyyy-MM-dd hh:mm:ss");

	QJsonObject resultObj;
	resultObj.insert("timestamp_min", QString::number(DataManager::getInstance().m_play_min));
	resultObj.insert("timestamp_cur", QString::number(DataManager::getInstance().m_play_cur));
	resultObj.insert("timestamp_max", QString::number(DataManager::getInstance().m_play_max));
	resultObj.insert("time_min", dt_st_min);
	resultObj.insert("time_cur", dt_st_cur);
	resultObj.insert("time_max", dt_st_max);
	resultObj.insert("simname", DataManager::getInstance().m_simname);
	return resultObj;
}

QJsonObject DataManager::outputSimulationReviewRange()
{
	QDateTime dt_max = QDateTime::fromMSecsSinceEpoch((DataManager::getInstance().m_reviewsimifo.max) * 1000);
	QString dt_st_max = dt_max.toString("yyyy-MM-dd hh:mm:ss");

	QDateTime dt_min = QDateTime::fromMSecsSinceEpoch((DataManager::getInstance().m_reviewsimifo.min) * 1000);
    QString dt_st_min = dt_min.toString("yyyy-MM-dd hh:mm:ss");

    QDateTime review_dt_cur = QDateTime::fromMSecsSinceEpoch((m_review_cur));
    QDateTime review_dt_min = QDateTime::fromMSecsSinceEpoch((m_review_min));
    QDateTime review_dt_max = QDateTime::fromMSecsSinceEpoch((m_review_max));
    review_dt_cur.setTimeSpec(Qt::UTC);
    review_dt_min.setTimeSpec(Qt::UTC);
    review_dt_max.setTimeSpec(Qt::UTC);
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
    QDateTime localTime_cur = review_dt_cur;
    QDateTime localTime_min = review_dt_min;
    QDateTime localTime_max = review_dt_max;
#else /* UNIX */
    //    // 获取本地时区
    //    //QTimeZone localTimeZone = QTimeZone::systemTimeZone();
    //    // 设置时区偏移量为+8:00
    //    QTimeZone localTimeZone = QTimeZone("Asia/Shanghai");
    //    QDateTime localTime_cur = review_dt_cur.toTimeZone(localTimeZone);
    //    QDateTime localTime_min = review_dt_min.toTimeZone(localTimeZone);
    //    QDateTime localTime_max = review_dt_max.toTimeZone(localTimeZone);

    QDateTime localTime_cur = review_dt_cur.toOffsetFromUtc(8 * 60 * 60);
    QDateTime localTime_min = review_dt_min.toOffsetFromUtc(8 * 60 * 60);
    QDateTime localTime_max = review_dt_max.toOffsetFromUtc(8 * 60 * 60);

#endif

    QString review_dt_st_cur = localTime_cur.toString("yyyy-MM-dd hh:mm:ss");
    QString review_dt_st_min = localTime_min.toString("yyyy-MM-dd hh:mm:ss");
    QString review_dt_st_max = localTime_max.toString("yyyy-MM-dd hh:mm:ss");
	QJsonObject resultObj;
	resultObj.insert("timestamp_min", QString::number(DataManager::getInstance().m_reviewsimifo.min));
	resultObj.insert("timestamp_max", QString::number(DataManager::getInstance().m_reviewsimifo.max));
	resultObj.insert("time_min", dt_st_min);
	resultObj.insert("time_max", dt_st_max);

	resultObj.insert("timestamp_review_min", review_dt_st_min);
	resultObj.insert("timestamp_review_max", review_dt_st_max);
	resultObj.insert("timestamp_review_cur", review_dt_st_cur);
	resultObj.insert("time_review_min", QString::number(DataManager::getInstance().m_review_min));
	resultObj.insert("time_review_max", QString::number(DataManager::getInstance().m_review_max));
	resultObj.insert("time_review_cur", QString::number(DataManager::getInstance().m_review_cur));

	resultObj.insert("simname", QString::fromStdString(DataManager::getInstance().m_reviewsimifo.siname));
	return resultObj;
}

void DataManager::reset_src()
{
	m_parkingpointuseinfo.clear();
	m_flights.clear();
	m_runwayuseinfo.clear();
}

void DataManager::data_deal_thread_func(void *pParam)
{
    bool bEmpty = true;
    {
        stdutils::OriMutexLocker locker(&m_saveFlightPlanConfLogsmutex);
        bEmpty &= m_saveFlightPlanConfLogs.empty();
    }

    {
        stdutils::OriMutexLocker locker(&m_saveSiminfoLogsmutex);
        bEmpty &= m_saveSiminfoLogs.empty();
    }


    {
        stdutils::OriMutexLocker locker(&m_saveinfoLogsmutex);
        bEmpty &= m_saveinfoLogs.empty();
    }

    if(bEmpty)
    {
        m_dealfullCond.wait(&m_dealmutex);
    }

    using namespace sqlite_orm;
    
    auto storage_instance = initStorage(m_log_dir_simname.toStdString()+"/data.db");
    {
        stdutils::OriMutexLocker locker(&m_saveFlightPlanConfLogsmutex);
        for(auto itor  = m_saveFlightPlanConfLogs.begin(); itor != m_saveFlightPlanConfLogs.end(); itor++)
        {
            auto &_FlightPlanConfLog = *itor;
#if 1
            auto scids = storage_instance.get_all<FlightPlanConfLog>(where(c(&FlightPlanConfLog::flightid) == _FlightPlanConfLog.flightid));
            if (scids.size() == 0)
#else

            auto count = storage_instance.count<FlightPlanConfLog>(where(c(&FlightPlanConfLog::flightid) == _FlightPlanConfLog.flightid));
            if (count == 0)
#endif
            {
                // make the insertion
                storage_instance.begin_transaction();
                storage_instance.insert(_FlightPlanConfLog);
                storage_instance.commit();
            }
        }
        m_saveFlightPlanConfLogs.clear();
    }

    {
        stdutils::OriMutexLocker locker(&m_saveSiminfoLogsmutex);
        for(auto itor  = m_saveSiminfoLogs.begin(); itor != m_saveSiminfoLogs.end(); itor++)
        {
            auto &SiminfoLog = *itor;
            // make the insertion
            storage_instance.begin_transaction();
            storage_instance.insert(SiminfoLog);
            storage_instance.commit();
        }
        m_saveSiminfoLogs.clear();
    }


    {
        stdutils::OriMutexLocker locker(&m_saveinfoLogsmutex);
        for(auto itor  = m_saveinfoLogs.begin(); itor != m_saveinfoLogs.end(); itor++)
        {
            auto &infoLog = *itor;
            // make the insertion
            storage_instance.begin_transaction();
            storage_instance.insert(infoLog);
            storage_instance.commit();
        }
        m_saveinfoLogs.clear();
    }

}

void DataManager::loadreviews()
{
    m_reviewdata.clear();

    QString savedir = QCoreApplication::applicationDirPath() + "/OriginalData";


    bool bsend = false;
    QDir timedir(savedir);
    timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    QFileInfoList fileEntryListtimedate = timedir.entryInfoList();
    QStringList ss = timedir.entryList();
    int index = 0;
    for (auto fileinfotimedate : fileEntryListtimedate)
    {
        if (fileinfotimedate.isDir())
        {
            QString  reocrdsavedir = fileinfotimedate.absoluteFilePath();

            QDir rootdir(reocrdsavedir);
            rootdir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
            QFileInfoList fileEntryList = rootdir.entryInfoList();
            int index = 0;
            for (auto fileinfo : fileEntryList)
            {
                if (fileinfo.isDir())
                {
					QString titledir = fileinfo.absoluteFilePath();
                    QDir timedir(titledir);
                    timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                    QFileInfoList fileEntryListtime = timedir.entryInfoList();
                    QStringList ss = timedir.entryList();
                    for (auto fileinfotime : fileEntryListtime)
                    {
                        if (fileinfotime.isDir())
                        {
                            QString recordname = fileinfo.fileName() + "_" + fileinfotime.fileName();
                            QString dir_send_str = fileinfotime.absoluteFilePath();
                            QDir dir_send(dir_send_str);
                            if (!dir_send.exists())
                            {
                                fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                                continue;
                            }

                            if (dir_send.exists())
                            {
                                bsend = getDirFilesInfo(++index, titledir,fileinfo.fileName(), dir_send_str, false);
                            }
                            if (!bsend)
                            {
                                clearDir(fileinfotime.absoluteFilePath());
                                fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                            }
                            else
                            {
                                index++;
                            }
                        }
                    }
                }
            }
        }
    }
}


void DataManager::exportexcel()
{
    {
        stdutils::OriMutexLocker locker(&m_recordruntimelogsmutex);
        QDateTime datetime = QDateTime::currentDateTime().toUTC();
        QString dateStr = datetime.toString("yyyyMMddhhmmsszzz");

        QString fileName = m_log_dir_simname+"/simlogs_"+dateStr+".xlsx";
        QXlsx::Document _Xlsx(fileName);
        _Xlsx.write("A" + QString::number(1), "编号");
        _Xlsx.write("B" + QString::number(1), "类型");
        _Xlsx.write("C" + QString::number(1), "内容");
        _Xlsx.write("D" + QString::number(1), "原因");
        _Xlsx.write("E" + QString::number(1), "时间");
        int icount = 1;
        for (auto itor = m_recordruntimelogs.begin(); itor != m_recordruntimelogs.end(); itor++)
        {
            infoLog& _infoLog = *itor;
            _Xlsx.write("A" + QString::number(icount+1), QString::number(_infoLog.id));
            _Xlsx.write("B" + QString::number(icount+1), QString::fromStdString(_infoLog.title));
            _Xlsx.write("C" + QString::number(icount+1), QString::fromStdString(_infoLog.log));
            _Xlsx.write("D" + QString::number(icount+1), QString::fromStdString(_infoLog.reason));
            _Xlsx.write("E" + QString::number(icount+1), QString::fromStdString(_infoLog.timestampstr));
            icount++;
        };
        _Xlsx.save();

        m_recordruntimelogs.clear();
    }
}

void DataManager::setCurrentReviewItem(const tagReplayItemInfo &_currentreview)
{
    m_currentreview = _currentreview;
	updatereviewsiminfo();
    m_review_min = _currentreview.m_iTimeStampStart;
    m_review_max = _currentreview.m_iTimeStampEnd;
    m_review_range  = _currentreview.getTotalMSecs();
    m_review_cur = _currentreview.m_iTimeStampStart;
}

void DataManager::update_flightInfo_to_db(uint64_t id, const flightinstance &_flightinstance)
{
    QDir dir(m_log_dir_simname);
    if(dir.exists())
    {
        FlightPlanConfLog _FlightPlanConfLog{id,
            _flightinstance.pflightdata->m_Date.toStdString(),
            _flightinstance.pflightdata->m_FilghtNumber.toStdString(),
            _flightinstance.pflightdata->m_DepArrFlag.toStdString(),
            _flightinstance.pflightdata->m_flight_dep_arr_type,
            _flightinstance.pflightdata->m_PlaneNum.toStdString(),
            _flightinstance.pflightdata->m_PlaneType.toStdString(),
            _flightinstance.pflightdata->m_FlightClass.toStdString(),
            _flightinstance.pflightdata->m_FlightLeg.toStdString(),
            _flightinstance.pflightdata->m_FlightStartPlace.toStdString(),
            _flightinstance.pflightdata->m_FlightEndPlace.toStdString(),
            _flightinstance.pflightdata->m_aheadtimelen,
            _flightinstance.pflightdata->m_PlanDateTimeTakeOff_ahead_timestamp,
            _flightinstance.pflightdata->m_PlanDateTimeTakeOff.toStdString(),
            _flightinstance.pflightdata->m_PlanDateTimeTakeOff_timestamp,
            _flightinstance.pflightdata->m_ExpectedDateTimeTakeOff.toStdString(),
            _flightinstance.pflightdata->m_RealityDateTimeTakeOff.toStdString(),

            _flightinstance.pflightdata->m_PlanDateTimeLanding_ahead_timestamp,
            _flightinstance.pflightdata->m_PlanDateTimeLanding.toStdString(),
            _flightinstance.pflightdata->m_PlanDateTimeLanding_timestamp,
            _flightinstance.pflightdata->m_ExpectedDateTimeLanding.toStdString(),
            _flightinstance.pflightdata->m_RealityDateTimeLanding.toStdString(),
            _flightinstance.pflightdata->m_PlanDateTimeLanding_behind_timestamp,
            _flightinstance.pflightdata->m_behindtimelen,
            _flightinstance.pflightdata->m_Delay.toStdString(),
            _flightinstance.pflightdata->m_Seat.toStdString(),
            _flightinstance.pflightdata->m_Terminal.toStdString(),
            _flightinstance.pflightdata->m_Runway.toStdString(),

            _flightinstance.pflightdata->flightid.toStdString(),
            _flightinstance.pathplanvalidinfo.target_parkingpoint.toStdString(),
            _flightinstance.pathplanvalidinfo.target_runway.toStdString(),
            _flightinstance.pathplanvalidinfo.alloc_parkingpoint.toStdString(),
            _flightinstance.pathplanvalidinfo.alloc_runway.toStdString(),
            _flightinstance.flightinstanceid
        };

        stdutils::OriMutexLocker locker(&m_saveFlightPlanConfLogsmutex);
        m_saveFlightPlanConfLogs.push_back(_FlightPlanConfLog);
        m_dealfullCond.wakeAll();
            }
}


FlightPlanConfLog DataManager::get_flightInfo_from_db(const UINT64 &flightid)
{
    FlightPlanConfLog ret;
#if 0
    using namespace sqlite_orm;
    auto storage_instance = initStorage(m_currentreview.m_titledir.toStdString()+"/data.db");
    auto count = storage_instance.count<FlightPlanConfLog>(
        where(c(&FlightPlanConfLog::flightid) == QString::number(flightid).toStdString()));
    if (count == 0) {
        return ret;
    } else {
        auto scids = storage_instance.get_all<FlightPlanConfLog>(
            where(c(&FlightPlanConfLog::flightid) == QString::number(flightid).toStdString()));
        if (scids.size() > 0) {
			auto sc = scids.at(0);
			ret = sc;

        }
    }
#else
    auto itor = m_recordflights.find(flightid);
    if(itor != m_recordflights.end())
    {
        ret = itor->second;
    }
#endif
    return ret;
}

void DataManager::update_SiminfoLog_to_db(const SiminfoLog &_flightinstance)
{
    QDir dir(m_log_dir_simname);
    if(dir.exists())
    {
        stdutils::OriMutexLocker locker(&m_saveSiminfoLogsmutex);
        m_saveSiminfoLogs.push_back(_flightinstance);
        m_dealfullCond.wakeAll();
            }
}

void DataManager::getTimestampRange_reviewdata(const UINT64 &timestamp_start, const UINT64 &timestamp_end)
{
    auto siminfos = DataManager::getInstance().get_SiminfoLog_from_dbs(timestamp_start, timestamp_end);

    for(auto itor = siminfos.rbegin(); itor != siminfos.rend(); itor++)
    {
        DataManager::getInstance().m_reviewsimifo = itor->second;
        ConfigManager::getInstance().review_step_interval = DataManager::getInstance().m_reviewsimifo.step_interval;
        ConfigManager::getInstance().review_step_dt = DataManager::getInstance().m_reviewsimifo.step_dt;
        ConfigManager::getInstance().review_step_freq = DataManager::getInstance().m_reviewsimifo.step_freq;
        ConfigManager::getInstance().review_one_second_sim_step_second = DataManager::getInstance().m_reviewsimifo.one_second_sim_step_second;
        break;
    }
    auto loginfos = DataManager::getInstance().get_infoLog_from_db(timestamp_start, timestamp_end);

    for(auto itor = loginfos.begin(); itor != loginfos.end(); itor++)
    {
        auto& item = itor->second;
        DataManager::getInstance().trans_review_log(item.title, item.log, item.reason, item.timestampstr, item.bTrans);
    }
}



std::map<UINT64, SiminfoLog> DataManager::get_SiminfoLog_from_dbs(const UINT64 &timestamp_start, const UINT64 &timestamp_end)
{
    std::map<UINT64, SiminfoLog> ret;
#if 0
    using namespace sqlite_orm;
    auto storage_instance = initStorage(m_currentreview.m_titledir.toStdString()+"/data.db");

    auto count = storage_instance.count<SiminfoLog>(where(c(&SiminfoLog::timestamp) >= timestamp_start
                                                       and c(&SiminfoLog::timestamp) <= timestamp_end));
    if (count != 0)
    {
        auto scids = storage_instance.get_all<SiminfoLog>(where(c(&SiminfoLog::timestamp) >= timestamp_start
                                                             and c(&SiminfoLog::timestamp) <= timestamp_end));
        if (scids.size() >= 0)
        {
            for(int index = 0; index < scids.size(); index++)
            {
                auto sc = scids.at(index);
                ret.push_back(sc);
            }
        }
    }
#else

    auto itor = m_recordsims.begin();
    while(itor != m_recordsims.end())
    {
        if(itor->second.timestamp >= timestamp_start && itor->second.timestamp <= timestamp_end)
        {
            ret.insert(std::make_pair(itor->first, itor->second));
            itor = m_recordsims.erase(itor);
            continue;
        }

        itor++;
    }

#endif
    return ret;
}

void DataManager::update_infoLog_to_db(infoLog &&_flightinstance)
{
    {
        stdutils::OriMutexLocker locker(&m_recordruntimelogsmutex);
        m_recordruntimelogs.push_back(_flightinstance);
    }
    QDir dir(m_log_dir_simname);
    if(dir.exists())
    {
        infoLog _infoLog = std::move(_flightinstance);
        _infoLog.timestamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
        stdutils::OriMutexLocker locker(&m_saveinfoLogsmutex);
        m_saveinfoLogs.push_back(std::move(_infoLog));
        m_dealfullCond.wakeAll();
            }
}

std::map<UINT64,infoLog> DataManager::get_infoLog_from_db(const UINT64 &timestamp_start, const UINT64 &timestamp_end)
{
    std::map<UINT64,infoLog> ret;
#if 0
    using namespace sqlite_orm;
    auto storage_instance = initStorage(m_currentreview.m_titledir.toStdString()+"/data.db");

    auto count = storage_instance.count<infoLog>(where(c(&infoLog::timestamp) >= timestamp_start
                                                       and c(&infoLog::timestamp) <= timestamp_end));
    if (count != 0)
    {
        auto scids = storage_instance.get_all<infoLog>(where(c(&infoLog::timestamp) >= timestamp_start
                                                                and c(&infoLog::timestamp) <= timestamp_end));
        if (scids.size() >= 0)
        {
            for(int index = 0; index < scids.size(); index++)
            {
                auto sc = scids.at(index);
                ret.push_back(sc);
            }
        }
    }
#else

    auto itor = m_recordlogs.begin();
    while(itor != m_recordlogs.end())
    {
        if(itor->second.timestamp >= timestamp_start && itor->second.timestamp <= timestamp_end)
        {
            ret.insert(std::make_pair(itor->first, itor->second));
            itor = m_recordlogs.erase(itor);
            continue;
        }
        itor++;
    }
#endif
    return ret;
}


void DataManager::updatereviewsiminfo()
{
    m_recordflights.clear();
    m_recordsims.clear();
    m_recordlogs.clear();

    using namespace sqlite_orm;
    auto storage_instance = initStorage(m_currentreview.m_titledir.toStdString()+"/data.db");

    auto _recordflights = storage_instance.get_all<FlightPlanConfLog>();
    if (_recordflights.size() > 0)
    {
        for(int index = 0; index < _recordflights.size(); index++)
        {
            auto sc = _recordflights.at(index);
            m_recordflights.insert(std::make_pair(QString::fromStdString(sc.flightid).toULongLong(), sc));
        }
    }

    auto _recordsimss = storage_instance.get_all<SiminfoLog>();
    if (_recordsimss.size() > 0)
    {
        for(int index = 0; index < _recordsimss.size(); index++)
        {
            auto sc = _recordsimss.at(index);
            m_recordsims.insert(std::make_pair(sc.timestamp, sc));
        }
    }

    auto _recordlogs = storage_instance.get_all<infoLog>();
    if (_recordlogs.size() > 0)
    {
        for(int index = 0; index < _recordlogs.size(); index++)
        {
            auto sc = _recordlogs.at(index);
            m_recordlogs.insert(std::make_pair(sc.timestamp, sc));
        }
    }

    for(auto itor = m_recordsims.begin(); itor != m_recordsims.end(); itor++)
    {
        DataManager::getInstance().m_reviewsimifo = itor->second;
        ConfigManager::getInstance().review_step_interval = DataManager::getInstance().m_reviewsimifo.step_interval;
        ConfigManager::getInstance().review_step_dt = DataManager::getInstance().m_reviewsimifo.step_dt;
        ConfigManager::getInstance().review_step_freq = DataManager::getInstance().m_reviewsimifo.step_freq;
        ConfigManager::getInstance().review_one_second_sim_step_second = DataManager::getInstance().m_reviewsimifo.one_second_sim_step_second;
        break;
    }
}

void DataManager::updatereviewsrecorddir(const QString &dirs)
{
    if(m_log_dir_simname.contains("TierAirlineAirPortSimulation_custom_"))
    {
        rmdirs(m_log_dir_simname);
    }
    m_simname = dirs;
    QString savedir = QDir::currentPath()+"/OriginalData";
    m_log_dir_simname = QString("%1/%2/%3").arg(savedir).arg(QDateTime::currentDateTime().toUTC().toString("yyyyMMdd")).arg(m_simname);
    QDir dir(m_log_dir_simname);
    if(!dir.exists())
    {
        if(!dir.mkpath(m_log_dir_simname))
        {
        }
    }
}

void DataManager::rmdirs(const QString &dirs)
{
    QDir olddir(dirs);
    if (olddir.exists())
    {
		std::cout << " rem dirs " << dirs.toStdString() << std::endl;
        clearDir(dirs);
        olddir.rmdir(dirs);
    }
}

tagFlightEventTime *DataManager::find_target_event(uint64_t timestamp)
{
    tagFlightEventTime *pflightevent = nullptr;

    std::map<uint64_t, tagFlightEventTime> &day_flight_events = DataManager::getInstance().total_flightEventTimedata;

    auto day_flight_events_itor = day_flight_events.begin();
    while (day_flight_events_itor != day_flight_events.end())
    {
        if (day_flight_events_itor->first <= timestamp)
        {
            tagFlightEventTime &flightevent = day_flight_events_itor->second;
            pflightevent = &flightevent;
        }
        else
        {
            break;
        }
        day_flight_events_itor++;
    }
    return pflightevent;

}

void DataManager::setEnvironment_init_succeed_callback(environment_init_succeed_callback newEnvironment_init_succeed_callback)
{
    m_environment_init_succeed_callback = std::move(newEnvironment_init_succeed_callback);
}

void DataManager::deal_receive_data_callback(const COMM_CHANNEL_INFO &channelinfo, const std::string &pdata)
{
    QString val = QString::fromStdString(pdata);
    QJsonObject jsonobj = FunctionAssistant::string_to_json_object(val);
    auto infotype = jsonobj.value("infotype").toString();
    auto operatetype = jsonobj.value("operatetype").toBool();
    if(channelinfo.m_topic == SettingsConfig::getInstance().getTopicChannelFormat("gaeactorhub_sets_changed"))
    {
        if(infotype == "gaeactorhub_starting")
        {
            auto msg = jsonobj.value("msg").toString();
            if(msg == "environment_init" && !m_bGaeactor_connected)
            {
                m_bGaeactor_connected = true;
                std::cout<<" gaeactor_hub connecting "<<pdata<<"\n";

                {
                    QJsonObject json;
                    json.insert("title", "environment");
                    json.insert("flight",msg);
                    json.insert("reson", msg);
                    json.insert("type", "runtime");
                    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                    if (m_logWbs)
                    {
                        m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
                    }

                }
                msg+="_succeed";
                {
                    QJsonObject json;
                    json.insert("title", "environment");
                    json.insert("flight",msg);
                    json.insert("reson", msg);
                    json.insert("type", "runtime");
                    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                    if (m_logWbs)
                    {
                        m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
                    }
                }
                if(m_environment_init_succeed_callback)
                {
                    m_environment_init_succeed_callback();
                }
                gaeactorcomm::GaeactorComm::getInstance().sendData(m_publish_process_status, val.toStdString());
            }
        }
    }
}

void DataManager::appendFlight(quint64 flightid, const flightinstance & data)
{
	QWriteLocker locker(&m_flights_mutex);
    m_flights.insert(std::make_pair(flightid, data));
    update_flightInfo_to_db(m_flights.size(),data);
}

flightinstance DataManager::getflightInfo(quint64 flightid)
{
	flightinstance ret;
	ret.flightinstanceid = 0;
	ret.pflightdata = nullptr;
	ret._tagPath_Plan = nullptr;
	QReadLocker locker(&m_flights_mutex);
	if (DataManager::getInstance().m_flights.find(flightid) != DataManager::getInstance().m_flights.end())
	{
		ret = DataManager::getInstance().m_flights.at(flightid);
    }
    return ret;
}

EventDriver *DataManager::peventDriver() const
{
    return m_peventDriver;
}

void DataManager::setPeventDriver(EventDriver *newPeventDriver)
{
    m_peventDriver = newPeventDriver;
}

#define ARR_EXTEND_LEN (4500)
#define DEP_EXTEND_LEN (4500)

#include <iostream>
#include <sstream>

void DataManager::initHttp()
{
	if (m_pHttpClient == nullptr)
	{
		m_pHttpClient = new HttpClient(nullptr);
		m_pHttpClient->setNetworkManager(networkManager);
    }

    QString localip = SettingsConfig::getInstance().lavicdesktop_localip();
    if (m_logWbs == nullptr)
	{
        m_logWbs = new KlusterWebSocketServer("log server", localip.toStdString().c_str(), SettingsConfig::getInstance().lavicdesktop_websocket_log_port_port(), nullptr);
        m_logWbs->set_recive_data_callback_func(std::bind(&DataManager::websocket_receive_callback,this,std::placeholders::_1,std::placeholders::_2));

        m_logWbs->listen();
	}

    m_publish_process_status = gaeactorcomm::GaeactorComm::getInstance().allocPublishTopicChannel(SettingsConfig::getInstance().getTopicChannelFormat("gaector_process_status_changed").c_str(), E_COMM_CHANNEL_DATE_TYPE_STRING);

    m_subscribe_gaeactorhub_status = gaeactorcomm::GaeactorComm::getInstance().allocSubscribeTopicChannel(SettingsConfig::getInstance().getTopicChannelFormat("gaeactorhub_sets_changed").c_str(), E_COMM_CHANNEL_DATE_TYPE_STRING);
    gaeactorcomm::GaeactorComm::getInstance().set_string_data_callback(m_subscribe_gaeactorhub_status,std::bind(&DataManager::deal_receive_data_callback,this,std::placeholders::_1,std::placeholders::_2));

    if(m_binary_websocket_server_ex == nullptr)
    {
        m_binary_websocket_server_ex = new KlusterWebSocketServer("review server",localip.toStdString().c_str(), SettingsConfig::getInstance().lavicdesktop_websocket_review_port_port(),nullptr);
        m_binary_websocket_server_ex->listen();
    }
}



void DataManager::readAirportInfoPath(const QString &path)
{
	m_currentAirport = "CAN";
	QDir dir(m_deal_path);
	std::cout << " decode dir *********************begin***********************"<< std::endl;
	dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); i++)
	{
		QString airport_dir = list.at(i).filePath();

		QFileInfo fileInfo(airport_dir);
		if (fileInfo.isDir())
		{
			std::cout << " decode dir " << airport_dir.toStdString() << std::endl;
			QDir dir(airport_dir);
			QStringList folders = dir.dirName().split('/'); // 使用 '/' 分割文件夹路径
			QString lastFolderName = folders.last();

			auto m_airports_itor = m_airports.find(lastFolderName);
			if (m_airports_itor == m_airports.end())
			{
				m_airports.insert(std::make_pair(lastFolderName, tagAirPortInfo()));
			}

			m_pCurrentAirPortInfo = &m_airports.at(lastFolderName);

			if (m_pCurrentAirPortInfo)
			{
				m_pCurrentAirPortInfo->data_reset();

				readGeoDirPath(airport_dir + "/aeroway");

				m_pCurrentAirPortInfo->getArea();

				QString wypointsPath = airport_dir + "/waypoints/airportwaypoints.geojson";
				readWaypointsGeoJsonFile(wypointsPath.toStdString());


				QString runwaypath = airport_dir + "/aeroway/runway.geojson";
				readWpsRunwayFile(runwaypath.toStdString());

				readPathDir(airport_dir + "/path");
			}
		}
	}
	std::cout << " decode dir *********************end***********************" << std::endl;

}

QString DataManager::getEntityIcon(const TYPE_ULID &uildsrc)
{
	QReadLocker locker(&m_entityicon_mutex);
	if (m_entityicon.find(uildsrc) == m_entityicon.end())
	{
		return QString();
	}
	return m_entityicon.at(uildsrc).symbolName;
}

void DataManager::requestEntityIcon(const TYPE_ULID &uildsrc)
{
	//QJsonObject jsobj;
	//m_pHttpClient->request_agentid_agentdata(uildsrc, jsobj);
	//setEntityIcon(uildsrc, jsobj);
}

void DataManager::setEntityIcon(const TYPE_ULID &uildsrc, const QJsonObject &entityAgentObj)
{
	QWriteLocker locker(&m_entityicon_mutex);
	if (m_entityicon.find(uildsrc) != m_entityicon.end())
	{
		m_entityicon.at(uildsrc).entityAgentObj = entityAgentObj;
		if (entityAgentObj.contains("modelUrlSymbols") && entityAgentObj.value("modelUrlSymbols").isArray())
		{
			auto modelUrlSymbols = entityAgentObj.value("modelUrlSymbols").toArray();
			for (auto modelUrlSymbolsitem : modelUrlSymbols)
			{
				auto modelUrlSymbolsitemobj = modelUrlSymbolsitem.toObject();
				m_entityicon.at(uildsrc).symbolName = modelUrlSymbolsitemobj.value("symbolName").toString();
			}
		}
	}
	else
	{
		entityAgentItem item;
		item.entityAgentObj = entityAgentObj;
		if (entityAgentObj.contains("modelUrlSymbols") && entityAgentObj.value("modelUrlSymbols").isArray())
		{
			auto modelUrlSymbols = entityAgentObj.value("modelUrlSymbols").toArray();
			for (auto modelUrlSymbolsitem : modelUrlSymbols)
			{
				auto modelUrlSymbolsitemobj = modelUrlSymbolsitem.toObject();
				item.symbolName = modelUrlSymbolsitemobj.value("symbolName").toString();
			}
		}
		m_entityicon.insert(std::make_pair(uildsrc, std::move(item)));
	}
}

void DataManager::setEntityIconName(const TYPE_ULID &uildsrc, const QString &entityAgentObj)
{
	QWriteLocker locker(&m_entityicon_mutex);
	if (m_entityicon.find(uildsrc) != m_entityicon.end())
	{
		m_entityicon.at(uildsrc).symbolName = entityAgentObj;
	}
	else
	{
		entityAgentItem item;
		item.entityAgentObj = QJsonObject();
		item.symbolName = entityAgentObj;
		m_entityicon.insert(std::make_pair(uildsrc, std::move(item)));
	}
}


void DataManager::trans_log(const QString & title, const std::stringstream& log, const std::stringstream& reson, bool bTrans)
{
    QString trimestampstr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    if (bTrans)
    {
        QJsonObject json;
        json.insert("title", title);
        json.insert("flight", QString::fromStdString(log.str()));
        json.insert("reson", QString::fromStdString(reson.str()));
        json.insert("type", "runtime");
        json.insert("timestamp", trimestampstr);
        if (m_logWbs)
        {
            m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
        }
    }

    std::stringstream ss;
    ss <<trimestampstr.toStdString()<<" "<< title.toStdString() << log.str() << "【" << reson.str() << "】";
    //    TRACE_LOG_PRINT_EX2(ss)
    std::cout<<ss.str()<<std::endl;

    infoLog _info{0,title.toStdString(), log.str(), reson.str(),bTrans, trimestampstr.toStdString(),0};
    update_infoLog_to_db(std::move(_info));

}

void DataManager::trans_review_log(const std::string &title, const std::string &log, const std::string &reson, const std::string &trimestampstr, bool bTrans)
{
    if (bTrans)
    {
        QJsonObject json;
        json.insert("title", QString::fromStdString(title));
        json.insert("flight", QString::fromStdString(log));
        json.insert("reson", QString::fromStdString(reson));
        json.insert("type", "review");
        json.insert("timestamp", QString::fromStdString(trimestampstr));
        if (m_logWbs)
        {
            m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
        }
    }

    std::stringstream ss;
    ss <<trimestampstr<<" "<< title << log << "【" << reson << "】";
    //    TRACE_LOG_PRINT_EX2(ss)
    std::cout<<ss.str()<<std::endl;
}


void DataManager::updateConnecting(bool bstatus)
{
    m_bGaeactor_connected = bstatus;
}


void DataManager::deal_review(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp)
{
    m_review_cur = iDataSendTimeStamp;
    trans_review(pData, iDataLen);
}

void DataManager::trans_review(BYTE *pData, UINT32 iDataLen)
{
    if(m_binary_websocket_server_ex)
    {
        m_binary_websocket_server_ex->SendBinaryMessage((const char *)pData, iDataLen);
    }
}

void DataManager::trans_sim_data_ok()
{
    QJsonObject json;
    json.insert("title", "sim_data");
    json.insert("flight","analysis_succeed_finished");
    json.insert("reson", "analysis_succeed_finished");
    json.insert("type", "runtime");
    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if (m_logWbs)
    {
        m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
    }
}

void DataManager::trans_sim_runtime_end()
{
    QJsonObject json;
    json.insert("title", "sim_runtime_process");
    json.insert("flight","sim_runtime_process_finished");
    json.insert("reson", "sim_runtime_process_finished");
    json.insert("type", "runtime");
    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if (m_logWbs)
    {
        m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
    }
}

void DataManager::trans_sim_review_end()
{
    QJsonObject json;
    json.insert("title", "sim_review_process");
    json.insert("flight","sim_review_process_finished");
    json.insert("reson", "sim_review_process_finished");
    json.insert("type", "review");
    json.insert("timestamp", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    if (m_logWbs)
    {
        m_logWbs->SendTextMessage(FunctionAssistant::json_object_to_string(json));
    }
}

void DataManager::websocket_receive_callback(const BYTE *pOriginData, const UINT32 &userPayloadSize)
{
}

std::unordered_map<QString, AgentKeyItemInfo> &DataManager::agentKeyMaps()
{
	return m_agentKeyMaps;
}

void DataManager::setAgentKeyMaps(const std::unordered_map<QString, AgentKeyItemInfo> &newAgentKeyMaps)
{
	m_agentKeyMaps = newAgentKeyMaps;
}

std::unordered_map<QString, AgentInstanceInfo>& DataManager::agentInstances()
{
	return m_agentInstances;
}

void DataManager::setAgentInstances(const std::unordered_map<QString, AgentInstanceInfo> & newAgentInstances)
{
	m_agentInstances = newAgentInstances;
}

std::unordered_map<QString, std::unordered_map<QString, QString> >& DataManager::InstagentInstance()
{
	return m_InstagentInstance;
}

void DataManager::setInstagentInstance(const std::unordered_map<QString, std::unordered_map<QString, QString> > &newInstagentInstance)
{
	m_InstagentInstance = newInstagentInstance;
}

HttpClient *DataManager::pHttpClient() const
{
	return m_pHttpClient;
}



DataManager::~DataManager()
{
	if (m_pHttpClient != nullptr)
	{
		m_pHttpClient->deleteLater();
	}

	if (mCache != nullptr)
	{
		mCache->deleteLater();
	}

	if (networkManager != nullptr)
	{
		networkManager->deleteLater();
	}
	if (m_pThreadPool != nullptr)
	{
		delete m_pThreadPool;
	}

	if (m_pDijkstra != nullptr)
	{
		delete m_pDijkstra;
	}

    if (m_logWbs)
    {
        m_logWbs->deleteLater();
	}

    if(m_hSqliteRecordDataDealThread)
    {
        delete m_hSqliteRecordDataDealThread;
    }

    gaeactorcomm::GaeactorComm::getInstance().removeTopicChannel(m_publish_process_status);
    gaeactorcomm::GaeactorComm::getInstance().removeTopicChannel(m_subscribe_gaeactorhub_status);


}

void DataManager::readGeoDirPath(const QString &path)
{
	QFileInfo fileInfo(path);
	if (fileInfo.isDir())
	{
		QDir dir(path);
		dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		QFileInfoList list = dir.entryInfoList();
		for (int i = 0; i < list.size(); i++)
		{
			readGeoDirPath(list.at(i).filePath());
		}
	}
	else if (fileInfo.isFile()) // �ҵ���Ƭ
	{
		QString newPath = path;
		//		readGeoJsonFile(newPath.toStdString());
		readAerowayFile(newPath.toStdString());
	}
}

void DataManager::readGeoJsonFile(const std::string & filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	QFileInfo fileinfo(filename.c_str());
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
		GeoJsonInfos geoinfos;
		auto featuresarray = jsonobj.value("features").toArray();
		for (auto featuresarrayitem : featuresarray)
		{
			tagGeoJsonInfo geoinfoitem;
			auto properties = featuresarrayitem.toObject().value("properties").toObject();

			geoinfoitem.properties = properties;
			geoinfoitem.name = properties.value("name").toString();

			QString osm_id = properties.value("osm_id").toString();
			if (osm_id.isEmpty())
			{
				osm_id = properties.value("osm_way_id").toString();
				if (osm_id.isEmpty())
				{
					osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());
				}
			}

			auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

			auto coordinatestype = geometry.value("type").toString().toLower();

			if (coordinatestype == "multipolygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					auto subarray = coordinatesitem.toArray();
					for (auto subarrayitem : subarray)
					{
						auto subsubarray = subarrayitem.toArray();
						LATLNGS_VECTOR sublatlnglist;
						sublatlnglist.reserve(subarray.size());
						for (auto subsubarrayitem : subsubarray)
						{
							PARSE_LATLNG_FROME_JSON(sublatlnglist, subsubarrayitem);
						}
						geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
					}
				}
				geoinfoitem.type = E_GEOTYPE_MULITPOLYGON;
				geoinfos.type = E_GEOTYPE_MULITPOLYGON;
			}
			else if (coordinatestype == "polygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					auto subarray = coordinatesitem.toArray();
					LATLNGS_VECTOR sublatlnglist;
					sublatlnglist.reserve(subarray.size());
					for (auto subarrayitem : subarray)
					{
						PARSE_LATLNG_FROME_JSON(sublatlnglist, subarrayitem);
					}
					geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				}
				geoinfoitem.type = E_GEOTYPE_POLYGON;
				geoinfos.type = E_GEOTYPE_POLYGON;
			}
			else if (coordinatestype == "linestring")
			{
				QString other_tags = properties.value("other_tags").toString();
				QStringList other_tags_list = other_tags.split(",");
				for (auto item : other_tags_list)
				{
					item.replace("\"", "");
					QStringList pairlist = item.split("=>");
					if (pairlist.size() == 2)
					{
						geoinfoitem.m_tags.insert(pairlist.at(0).trimmed(), pairlist.at(1));
					}
				}
				geoinfoitem.z_order = properties.value("z_order").toInt();

				auto coordinatesarray = geometry.value("coordinates").toArray();
				LATLNGS_VECTOR sublatlnglist;
				sublatlnglist.reserve(coordinatesarray.size());
				for (auto coordinatesitem : coordinatesarray)
				{
					LAT_LNG lstlatlng;
					bool bBegin = true;
					if (sublatlnglist.empty())
					{
					}
					else
					{
						lstlatlng = sublatlnglist.back();
						bBegin = false;
					}
					PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);
					LAT_LNG latlng = sublatlnglist.back();
					updateDijkstraMap(latlng, lstlatlng, bBegin);
				}
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				geoinfoitem.type = E_GEOTYPE_LINE;
				geoinfos.type = E_GEOTYPE_LINE;
			}
			else if (coordinatestype == "point")
			{
				LATLNGS_VECTOR sublatlnglist;
				PARSE_LATLNG_FROME_JSON(sublatlnglist, geometry.value("coordinates"));
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));

				geoinfoitem.type = E_GEOTYPE_POINT;
				geoinfos.type = E_GEOTYPE_POINT;
			}

			geoinfoitem.dealInfo();
			geoinfos.subItem.insert(osm_id, std::move(geoinfoitem));
		}
		m_pCurrentAirPortInfo->m_GeoJsonInfos.insert(namefile, geoinfos);
	}
}


void DataManager::readWpsRunwayPath(const QString& path)
{
	QFileInfo fileInfo(path);
	if (fileInfo.isDir())
	{
		QDir dir(path);
		dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		QFileInfoList list = dir.entryInfoList();
		for (int i = 0; i < list.size(); i++)
		{
			readWpsRunwayPath(list.at(i).filePath());
		}
	}
	else if (fileInfo.isFile()) // �ҵ���Ƭ
	{
		QString newPath = path;
		readWpsRunwayFile(newPath.toStdString());
	}
}

void DataManager::readWpsRunwayFile(const std::string& filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	QFileInfo fileinfo(filename.c_str());
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
		GeoJsonInfos geoinfos;
		auto featuresarray = jsonobj.value("features").toArray();
		for (auto featuresarrayitem : featuresarray)
		{
			tagGeoJsonInfo geoinfoitem;
			auto properties = featuresarrayitem.toObject().value("properties").toObject();

			geoinfoitem.properties = properties;
			geoinfoitem.name = properties.value("name").toString();

			QString osm_id = properties.value("osm_id").toString();
			if (osm_id.isEmpty())
			{
				osm_id = properties.value("osm_way_id").toString();
				if (osm_id.isEmpty())
				{
					osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());
				}
			}

			auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

			auto coordinatestype = geometry.value("type").toString().toLower();

			if (coordinatestype == "multipolygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					auto subarray = coordinatesitem.toArray();
					for (auto subarrayitem : subarray)
					{
						LATLNGS_VECTOR sublatlnglist;

						auto subsubarray = subarrayitem.toArray();
						for (auto subsubarrayitem : subsubarray)
						{
							PARSE_LATLNG_FROME_JSON(sublatlnglist, subsubarrayitem);
						}
						geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
					}
				}
				geoinfoitem.type = E_GEOTYPE_MULITPOLYGON;
				geoinfos.type = E_GEOTYPE_MULITPOLYGON;
			}
			else if (coordinatestype == "polygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					LATLNGS_VECTOR sublatlnglist;
					auto subarray = coordinatesitem.toArray();
					for (auto subarrayitem : subarray)
					{
						PARSE_LATLNG_FROME_JSON(sublatlnglist, subarrayitem);
					}
					geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				}
				geoinfoitem.type = E_GEOTYPE_POLYGON;
				geoinfos.type = E_GEOTYPE_POLYGON;
			}
			else if (coordinatestype == "linestring")
			{
				QString other_tags = properties.value("other_tags").toString();
				QStringList other_tags_list = other_tags.split(",");
				for (auto item : other_tags_list)
				{
					item.replace("\"", "");
					QStringList pairlist = item.split("=>");
					if (pairlist.size() == 2)
					{
						geoinfoitem.m_tags.insert(pairlist.at(0).trimmed(), pairlist.at(1));
					}
				}
				geoinfoitem.z_order = properties.value("z_order").toInt();

				auto coordinatesarray = geometry.value("coordinates").toArray();
				LATLNGS_VECTOR sublatlnglist;
				for (auto coordinatesitem : coordinatesarray)
				{
					PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);
				}
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				geoinfoitem.type = E_GEOTYPE_LINE;
				geoinfos.type = E_GEOTYPE_LINE;
			}
			else if (coordinatestype == "point")
			{
				LATLNGS_VECTOR sublatlnglist;
				PARSE_LATLNG_FROME_JSON(sublatlnglist, geometry.value("coordinates"));
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));

				geoinfoitem.type = E_GEOTYPE_POINT;
				geoinfos.type = E_GEOTYPE_POINT;
			}

			geoinfoitem.dealInfo();
			geoinfos.subItem.insert(osm_id, std::move(geoinfoitem));
		}
		m_pCurrentAirPortInfo->m_WPSRunwayInfos.insert(namefile, geoinfos);
		///////////////////////////////////////////////////////////////////
		//song
		for (auto subcoordinates : geoinfos.subItem)
		{
			switch (geoinfos.type)
			{
			case E_GEOTYPE_LINE:
			{
				bool bShowArea = false;


				if (subcoordinates.m_tags.contains("aeroway"))
				{
					if (subcoordinates.m_tags.value("aeroway") == "runway")
					{
						bShowArea = true;
					}
					else if (subcoordinates.m_tags.value("aeroway") == "taxiway")
					{
					}
					else if (subcoordinates.m_tags.value("aeroway") == "parking_position")
					{
					}
				}
				if (bShowArea)
				{
					auto generateExtendSamplingEx = [](const LAT_LNG& startLatLng, const LAT_LNG& endLatLng, LATLNGS_VECTOR &retlist)
					{
						retlist.push_back(startLatLng);
						double dis = FunctionAssistant::calc_dist(startLatLng, endLatLng);
						if (dis > POINT_EXTEND_METRE)
						{
							int step = dis / POINT_EXTEND_METRE;
							glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(startLatLng, endLatLng);
							LAT_LNG lstextendpt = startLatLng;
							for (int i = 1; i < step + 1; i++)
							{
								LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, POINT_EXTEND_METRE);
								retlist.push_back(currentextendpt);

								lstextendpt = currentextendpt;
							}
						}
						retlist.push_back(endLatLng);
					};

					auto generateExtendSampling = [](const LAT_LNG& latlng, const LAT_LNG& lstlatlng, LATLNGS_VECTOR &retlist)
					{
						double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
						if (dis > POINT_EXTEND_METRE)
						{
							int step = dis / POINT_EXTEND_METRE;
							glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
							LAT_LNG lstextendpt = lstlatlng;
							for (int i = 1; i < step + 1; i++)
							{
								LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, POINT_EXTEND_METRE);
								retlist.push_back(currentextendpt);

								lstextendpt = currentextendpt;
							}
							double rdis = dis - POINT_EXTEND_METRE * step;

							retlist.push_back(latlng);
						}
						else
						{
							retlist.push_back(latlng);
						}
					};


					auto printLatLng = [](const LAT_LNG &latlng)
					{
						std::cout << "[" << latlng.lng << "," << latlng.lat << "]," << std::endl;
					};

					if (subcoordinates.coordinates.size() != 0)
					{
						for (auto coordinatesitem : subcoordinates.coordinates)
						{

#if 1
							LATLNGS_VECTOR &runway_pts = coordinatesitem;
#else
							auto runway_start = coordinatesitem.front();
							auto runway_end = coordinatesitem.back();

							auto runway_extend1 = FunctionAssistant::generateExtendRunWay(runway_end, runway_start, 5 * 1000);
							auto runway_extend2 = FunctionAssistant::generateExtendRunWay(runway_start, runway_end, 5 * 1000);

							//同向的
							//生成降落航路
							//生成起飞航路

							LATLNGS_VECTOR runway_pts;

							runway_pts.push_back(runway_extend1);
							runway_pts.push_back(runway_start);
							runway_pts.insert(runway_pts.end(), coordinatesitem.begin(), coordinatesitem.end());
							runway_pts.push_back(runway_end);
							runway_pts.push_back(runway_extend2);
#endif



							LATLNGS_VECTOR runway_retlist_1;
#if 0
							generateExtendSamplingEx(runway_pts.front(), runway_pts.back(), runway_retlist_1);
#else
							runway_retlist_1.push_back(runway_pts.at(0));
							for (int i = 1; i < runway_pts.size(); i++)
							{
								generateExtendSampling(runway_pts.at(i), runway_pts.at(i - 1), runway_retlist_1);
							}
#endif
							if (subcoordinates.m_tags.contains("ref"))
							{
								QString name = subcoordinates.m_tags.value("ref");
								QStringList namelist = name.split("/");
								for (auto poiKeywordTmp : namelist)
								{
									poiKeywordTmp.remove(QRegExp("^0+"));
									m_pCurrentAirPortInfo->m_RunwayInfos.insert(poiKeywordTmp, runway_retlist_1);
								}
							}
						}
					}
				}
			}
			break;
			}
		}
	}
}


void DataManager::readAerowayFile(const std::string &filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	QFileInfo fileinfo(filename.c_str());
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
		GeoJsonInfos geoinfos;
		auto featuresarray = jsonobj.value("features").toArray();
		for (auto featuresarrayitem : featuresarray)
		{
			tagGeoJsonInfo geoinfoitem;
			auto properties = featuresarrayitem.toObject().value("properties").toObject();

			geoinfoitem.properties = properties;
			geoinfoitem.name = properties.value("name").toString();

			QString osm_id = properties.value("osm_id").toString();
			if (osm_id.isEmpty())
			{
				osm_id = properties.value("osm_way_id").toString();
				if (osm_id.isEmpty())
				{
					osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());
				}
			}

			auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

			auto coordinatestype = geometry.value("type").toString().toLower();
			if (coordinatestype == "linestring")
			{
				QString other_tags = properties.value("other_tags").toString();
				QStringList other_tags_list = other_tags.split(",");
				for (auto item : other_tags_list)
				{
					item.replace("\"", "");
					QStringList pairlist = item.split("=>");
					if (pairlist.size() == 2)
					{
						geoinfoitem.m_tags.insert(pairlist.at(0).trimmed(), pairlist.at(1));
					}
				}
				geoinfoitem.z_order = properties.value("z_order").toInt();

				auto coordinatesarray = geometry.value("coordinates").toArray();
				LATLNGS_VECTOR sublatlnglist;
				sublatlnglist.reserve(coordinatesarray.size());
				for (auto coordinatesitem : coordinatesarray)
				{

					PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);

					LAT_LNG latlng = sublatlnglist.back();
					auto _m_aerowayinfo_itor = m_pCurrentAirPortInfo->m_aerowayinfo.find(latlng);
					if (_m_aerowayinfo_itor != m_pCurrentAirPortInfo->m_aerowayinfo.end())
					{
						if (_m_aerowayinfo_itor->second != "runway")
						{
							_m_aerowayinfo_itor->second = geoinfoitem.m_tags.value("aeroway");
						}
					}
					else
					{
						m_pCurrentAirPortInfo->m_aerowayinfo.insert(std::make_pair(latlng, geoinfoitem.m_tags.value("aeroway")));
					}
				}
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				geoinfoitem.type = E_GEOTYPE_LINE;
				geoinfos.type = E_GEOTYPE_LINE;
			}
		}
	}
}


void DataManager::readWaypointsGeoJsonFile(const std::string & filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	QFileInfo fileinfo(filename.c_str());
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
		GeoJsonInfos geoinfos;
		auto featuresarray = jsonobj.value("features").toArray();
		for (auto featuresarrayitem : featuresarray)
		{
			tagGeoJsonInfo geoinfoitem;
			auto properties = featuresarrayitem.toObject().value("properties").toObject();

			geoinfoitem.properties = properties;

			QString osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());

			auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

			auto coordinatestype = geometry.value("type").toString().toLower();

			if (coordinatestype == "multilinestring")
			{
				auto parentcoordinatesarray = geometry.value("coordinates").toArray();
				QString aeroway;
				for (auto parentcoordinatesarrayitem : parentcoordinatesarray)
				{
					auto coordinatesarray = parentcoordinatesarrayitem.toArray();
					LATLNGS_VECTOR sublatlnglist;
					sublatlnglist.reserve(coordinatesarray.size());
					for (auto coordinatesitem : coordinatesarray)
					{
						LAT_LNG lstlatlng;
						bool bBegin = true;
						if (sublatlnglist.empty())
						{
						}
						else
						{
							lstlatlng = sublatlnglist.back();
							bBegin = false;
						}
						PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);


						LAT_LNG latlng = sublatlnglist.back();
						if (aeroway.isEmpty() || aeroway != "runway")
						{
							auto _m_aerowayinfo_itor = m_pCurrentAirPortInfo->m_aerowayinfo.find(latlng);
							if (_m_aerowayinfo_itor != m_pCurrentAirPortInfo->m_aerowayinfo.end())
							{
								aeroway = _m_aerowayinfo_itor->second;
							}
							else
							{
								float dismin = DIJKSTRA_MAX_VALUE;
								auto _linestringpoits_itor = m_pCurrentAirPortInfo->m_aerowayinfo.begin();
								while (_linestringpoits_itor != m_pCurrentAirPortInfo->m_aerowayinfo.end())
								{
									double dis = FunctionAssistant::calc_dist(_linestringpoits_itor->first, latlng);
									if (dis < dismin)
									{
										dismin = dis;
										aeroway = _linestringpoits_itor->second;
									}
									_linestringpoits_itor++;
								}
							}
						}

						updateDijkstraMap(latlng, lstlatlng, bBegin);
					}
					geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				}
				geoinfoitem.m_tags.insert("aeroway", aeroway);
				geoinfoitem.type = E_GEOTYPE_LINE;
				geoinfos.type = E_GEOTYPE_LINE;
			}

			geoinfoitem.dealInfo();
			geoinfos.subItem.insert(osm_id, std::move(geoinfoitem));
		}
		m_pCurrentAirPortInfo->m_GeoJsonInfos.insert(namefile, geoinfos);
	}
}

bool DataManager::readGeoJsonData(const std::string & filename, GeoJsonInfos& geoinfos)
{
	bool bRet = false;
	QFileInfo fileinfo(filename.c_str());
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
		auto featuresarray = jsonobj.value("features").toArray();
		for (auto featuresarrayitem : featuresarray)
		{
			tagGeoJsonInfo geoinfoitem;
			auto properties = featuresarrayitem.toObject().value("properties").toObject();

			geoinfoitem.properties = properties;
			geoinfoitem.name = properties.value("name").toString();

			QString osm_id = properties.value("osm_id").toString();
			if (osm_id.isEmpty())
			{
				osm_id = properties.value("osm_way_id").toString();
				if (osm_id.isEmpty())
				{
					osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());
				}
			}

			auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

			auto coordinatestype = geometry.value("type").toString().toLower();

			if (coordinatestype == "multipolygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					auto subarray = coordinatesitem.toArray();
					for (auto subarrayitem : subarray)
					{
						LATLNGS_VECTOR sublatlnglist;

						auto subsubarray = subarrayitem.toArray();
						for (auto subsubarrayitem : subsubarray)
						{
							PARSE_LATLNG_FROME_JSON(sublatlnglist, subsubarrayitem);
						}
						geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
					}
				}
				geoinfoitem.type = E_GEOTYPE_MULITPOLYGON;
				geoinfos.type = E_GEOTYPE_MULITPOLYGON;
			}
			else if (coordinatestype == "polygon")
			{
				auto coordinatesarray = geometry.value("coordinates").toArray();
				for (auto coordinatesitem : coordinatesarray)
				{
					LATLNGS_VECTOR sublatlnglist;
					auto subarray = coordinatesitem.toArray();
					for (auto subarrayitem : subarray)
					{
						PARSE_LATLNG_FROME_JSON(sublatlnglist, subarrayitem);
					}
					geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				}
				geoinfoitem.type = E_GEOTYPE_POLYGON;
				geoinfos.type = E_GEOTYPE_POLYGON;
			}
			else if (coordinatestype == "linestring")
			{
				QString other_tags = properties.value("other_tags").toString();
				QStringList other_tags_list = other_tags.split(",");
				for (auto item : other_tags_list)
				{
					item.replace("\"", "");
					QStringList pairlist = item.split("=>");
					if (pairlist.size() == 2)
					{
						geoinfoitem.m_tags.insert(pairlist.at(0).trimmed(), pairlist.at(1));
					}
				}
				geoinfoitem.z_order = properties.value("z_order").toInt();

				auto coordinatesarray = geometry.value("coordinates").toArray();
				LATLNGS_VECTOR sublatlnglist;
				for (auto coordinatesitem : coordinatesarray)
				{
					PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);
				}
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
				geoinfoitem.type = E_GEOTYPE_LINE;
				geoinfos.type = E_GEOTYPE_LINE;
			}
			else if (coordinatestype == "point")
			{
				LATLNGS_VECTOR sublatlnglist;
				PARSE_LATLNG_FROME_JSON(sublatlnglist, geometry.value("coordinates"));
				geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));

				geoinfoitem.type = E_GEOTYPE_POINT;
				geoinfos.type = E_GEOTYPE_POINT;
			}

			geoinfoitem.dealInfo();
			geoinfos.subItem.insert(osm_id, std::move(geoinfoitem));
		}
		bRet = true;
	}
	return bRet;
}



QStringList DataManager::getAirPortList()
{
	QStringList ret;

	for (auto itor = m_airports.begin(); itor != m_airports.end(); itor++)
	{
		ret.push_back(itor->first);
	}
	return ret;
}

std::unordered_map<QString, std::tuple<QString, QString>>& DataManager::getAirPortNameList()
{
	return m_airporttitles;
}

tagAirPortInfo * DataManager::getAirportInfo(const QString & airport_code)
{
	if (m_airports.find(airport_code) != m_airports.end())
	{
		return &m_airports.at(airport_code);
	}
	else
	{
		return nullptr;
	}
}

tagAirPortInfo * DataManager::getCurrentAirportInfo()
{
	if (m_airports.find(m_currentAirport) != m_airports.end())
	{
		return &m_airports.at(m_currentAirport);
	}
	else
	{
		return nullptr;
	}
}

void DataManager::readPathDir(const QString &path)
{
	QString pathinfo = path + "/PathInfo.json";

	QDir fileInfo(pathinfo);
	if (fileInfo.exists(pathinfo))
	{
		readPoisJsonFile(path + "/Agent.json");
		readPathInfoFile(pathinfo);
	}
}

void DataManager::readPoisJsonFile(const QString & filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	auto appendPoiMapItem = [&](tagPoiItem& poiitemnew) {
#if 0

		UINT64 disminid = 0;
		LAT_LNG disminLatlng;
		E_POINT_TYPE ePointType;
		findDistanceMinPoint(disminid, disminLatlng, ePointType, poiitemnew.poipoint);

		poiitemnew.m_calibrate_osm_path_info = std::make_tuple(disminid, disminLatlng);

		m_poiitemsmap.insert(std::make_pair(poiitemnew.poiKeyword, std::move(poiitemnew)));
#else
		UINT64 disminid = 0;
		LAT_LNG disminLatlng;
		E_POINT_TYPE ePointType;
		findDistanceMinPoint(disminid, disminLatlng, ePointType, poiitemnew.poipoint);

		UINT64 item_id = 0;
		if (getLinestringPointId(item_id, poiitemnew.poipoint))
		{
			poiitemnew.m_calibrate_osm_path_info = std::make_tuple(disminid, disminLatlng);
		}
		else
		{
			E_POINT_TYPE eExtendPointType = E_POINT_TYPE_POI;
			switch (ePointType)
			{
			case E_POINT_TYPE_OSM: eExtendPointType = E_POINT_TYPE_POI_EXTEND_OSM; break;
			case E_POINT_TYPE_OSM_EXTEND: eExtendPointType = E_POINT_TYPE_POI_EXTEND_OSM_EXTEND; break;
			case E_POINT_TYPE_POI: eExtendPointType = E_POINT_TYPE_POI_EXTEND_POI; break;
			case E_POINT_TYPE_POI_EXTEND_OSM:
			case E_POINT_TYPE_POI_EXTEND_OSM_EXTEND:
			case E_POINT_TYPE_POI_EXTEND_POI:
			case E_POINT_TYPE_POI_EXTEND_POI_EXTEND:
				eExtendPointType = E_POINT_TYPE_POI_EXTEND_POI_EXTEND; break;
				break;
			default:
				break;
			}
			item_id = appendDijkstraMap(poiitemnew.poipoint, disminLatlng, disminid, eExtendPointType, E_POINT_TYPE_POI);
			poiitemnew.m_calibrate_osm_path_info = std::make_tuple(disminid, disminLatlng);
		}
#endif

		m_pCurrentAirPortInfo->m_poiitemsmap.insert(std::make_pair(poiitemnew.poiKeyword, std::move(poiitemnew)));
	};
	QFileInfo fileinfo(filename);
	if (fileinfo.exists())
	{
		QString namefile = fileinfo.baseName();
		auto jsonobj = FunctionAssistant::read_json_file_object(filename);
		auto poisarray = jsonobj.value("pois").toArray();
		for (auto poisarrayitem : poisarray)
		{
			auto poisarrayitemobj = poisarrayitem.toObject();
			tagPoiItem poiitem;
			poiitem.poiKey = poisarrayitemobj.value("poiKey").toString();
			poiitem.poiKeyword = poisarrayitemobj.value("poiKeyword").toString();
			poiitem.poiFrame = poisarrayitemobj.value("poiFrame").toInt();
			poiitem.poiDirection = poisarrayitemobj.value("poiDirection").toInt();
			poiitem.poiName = poisarrayitemobj.value("poiName").toString();
			poiitem.poiNameI18n = poisarrayitemobj.value("poiNameI18n").toString();


			auto poipointarray = poisarrayitemobj.value("poiPoint").toArray();

			if (poipointarray.size() == 3)
			{
				poiitem.poipoint = LAT_LNG{ poipointarray.at(1).toDouble(),poipointarray.at(0).toDouble() };
				poiitem.alt = poipointarray.at(2).toDouble();
			}
			if (poiitem.poiKeyword.startsWith("PB#") && poiitem.poiKeyword.contains("-"))
			{
				QString cc = poiitem.poiKeyword.right(poiitem.poiKeyword.size() - QString("PB#").length());
				QStringList ptlist = cc.split("-");
				if (!ptlist.isEmpty())
				{
					for (auto item : ptlist)
					{
						tagPoiItem poiitemnew = poiitem;
						poiitemnew.poiKeyword = QString("PBN%1").arg(item.mid(1, item.size() - 1));

						poiitemnew.poiName = poiitemnew.poiKeyword;
						poiitemnew.poiNameI18n = poiitemnew.poiKeyword;

						appendPoiMapItem(poiitemnew);
					}
				}
				else
				{
					appendPoiMapItem(poiitem);
				}
			}
			else
			{
				appendPoiMapItem(poiitem);
			}
		}
	}
	return;
}


//#define  REBUILD_EXTEND_WPS

void DataManager::readPathInfoFile(const QString & filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}

	QMap<QString, bool> _poiinvalids;
	QJsonObject jsonobj = FunctionAssistant::read_json_file_object(filename);
	QJsonArray pathinfoarr = jsonobj.value("pathInfos").toArray();

	bool bSave = false;
	for (auto pathinfoarritem : pathinfoarr)
	{
		auto pathinfoarritemobj = pathinfoarritem.toObject();

		tagPath_Plan path_plan;
		path_plan.fromJson(pathinfoarritemobj);

		if (!m_pCurrentAirPortInfo->m_runways.contains(path_plan.m_runway))
		{
			m_pCurrentAirPortInfo->m_runways.push_back(path_plan.m_runway);
		}

		E_FLIGHT_DEP_ARR_TYPE flight_dep_arr_type = path_plan.m_flight_dep_arr_type;
		QString parkingpoint = path_plan.m_parkingpoint;
		QString runway = path_plan.m_runway;
		auto _Path_Plans_itor = m_pCurrentAirPortInfo->m_Path_Plans.find(path_plan.m_parkingpoint);
		if (_Path_Plans_itor != m_pCurrentAirPortInfo->m_Path_Plans.end())
		{
			ARR_DEP_RUNWAY_PATH &arr_dep_runway_path = _Path_Plans_itor->second;
			auto arr_dep_runway_path_itor = arr_dep_runway_path.find(flight_dep_arr_type);
			if (arr_dep_runway_path_itor != arr_dep_runway_path.end())
			{
				RUNWAY_PATH &runway_path = arr_dep_runway_path_itor->second;
				auto runway_path_itor = runway_path.find(runway);
				if (runway_path_itor != runway_path.end())
				{
					runway_path_itor->second.pathindex = std::move(path_plan);
				}
				else
				{
					tagPathPlanInfo pathinfo;
					pathinfo.pathindex = std::move(path_plan);
					runway_path.insert(std::make_pair(runway, std::move(pathinfo)));
				}
			}
			else
			{
				tagPathPlanInfo pathinfo;
				pathinfo.pathindex = std::move(path_plan);
				RUNWAY_PATH runway_path;
				runway_path.insert(std::make_pair(runway, std::move(pathinfo)));
				arr_dep_runway_path.insert(std::make_pair(flight_dep_arr_type, std::move(runway_path)));
			}
		}
		else
		{
			tagPathPlanInfo pathinfo;
			pathinfo.pathindex = std::move(path_plan);
			RUNWAY_PATH runway_path;
			runway_path.insert(std::make_pair(runway, std::move(pathinfo)));
			ARR_DEP_RUNWAY_PATH arr_dep_runway_path;
			arr_dep_runway_path.insert(std::make_pair(flight_dep_arr_type, std::move(runway_path)));

			m_pCurrentAirPortInfo->m_Path_Plans.insert(std::make_pair(parkingpoint, std::move(arr_dep_runway_path)));
		}

		tagPath_Plan *ppath = &m_pCurrentAirPortInfo->m_Path_Plans.at(parkingpoint).at(flight_dep_arr_type).at(runway).pathindex;

		if (ppath)
		{
			if (ppath->m_pathPoints.size() > 2)
			{
				if (std::get<1>(ppath->m_pathPoints.at(0)) &&
					std::get<1>(ppath->m_pathPoints.at(ppath->m_pathPoints.size() - 1)) &&
					!ppath->m_extendwpslatlng.empty())
				{
					ppath->m_bValid = true;
				}
				else
				{
					ppath->m_bValid = false;
				}
			}
			else
			{
				ppath->m_bValid = false;
			}

			switch (flight_dep_arr_type)
			{
			case E_FLIGHT_DEP_ARR_TYPE_DEP:
			{
				auto itt = std::find_if(m_pCurrentAirPortInfo->m_deppaths.begin(),
					m_pCurrentAirPortInfo->m_deppaths.end(), [&](const std::list<tagPath_Plan*>::value_type& vt) {
					return vt == ppath;
				});
				if (itt == m_pCurrentAirPortInfo->m_deppaths.end() &&
					!ppath->m_parkingpoint.startsWith("FX"))
				{
					m_pCurrentAirPortInfo->m_deppaths.push_back(ppath);				
				}
			}
			break;
			case E_FLIGHT_DEP_ARR_TYPE_ARR:
			{
				auto itt = std::find_if(m_pCurrentAirPortInfo->m_arrpaths.begin(),
					m_pCurrentAirPortInfo->m_arrpaths.end(), [&](const std::list<tagPath_Plan*>::value_type& vt) {
					return vt == ppath;
				});

				if (itt == m_pCurrentAirPortInfo->m_arrpaths.end() &&
					!ppath->m_parkingpoint.startsWith("FX"))
				{
					m_pCurrentAirPortInfo->m_arrpaths.push_back(ppath);
				}
			}
			break;
			default:
				break;
			}

			for (int i = 0; i < ppath->m_pathPoints.size(); i++)
			{
				if (!std::get<1>(ppath->m_pathPoints.at(i)))
				{
					_poiinvalids.insert(std::get<0>(ppath->m_pathPoints.at(i)), false);
				}
			}
		}

		//if (ppath->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP && 
		//	!ppath->m_extendwpslatlng.empty() && 
		//	!ppath->m_extendwpslatlng_simple.empty() && 
		//	!ppath->m_runwayextendwpslatlng.empty())
		//{
		//	UINT64 pbn_id = -1;
		//	LAT_LNG _pbn_pt;
		//	for (int i = 0; i < ppath->m_pathPoints.size(); i++)
		//	{
		//		auto & pbn_name = std::get<0>(ppath->m_pathPoints.at(i));
		//		auto&bvalid = std::get<1>(ppath->m_pathPoints.at(i));

		//		if (bvalid && pbn_name.contains("BN"))
		//		{
		//			auto _poiitemsmap_itor = m_pCurrentAirPortInfo->m_poiitemsmap.find(pbn_name);
		//			if (_poiitemsmap_itor != m_pCurrentAirPortInfo->m_poiitemsmap.end())
		//			{
		//				pbn_id = std::get<0>(_poiitemsmap_itor->second.m_calibrate_osm_path_info);
		//				_pbn_pt = std::get<1>(_poiitemsmap_itor->second.m_calibrate_osm_path_info);
		//			}
		//			break;
		//		}
		//	}
		//	int start_index = 0;
		//	if (pbn_id != -1)
		//	{
		//		float dismin = DIJKSTRA_MAX_VALUE;
		//		for (int j = 0; j < ppath->m_extendwpslatlng.size(); j++)
		//		{
		//			auto & pt_ = ppath->m_extendwpslatlng.at(j);
		//			double dis = FunctionAssistant::calc_dist(pt_, _pbn_pt);
		//			if (dis < dismin)
		//			{
		//				dismin = dis;
		//				start_index = j;
		//			}
		//			else
		//			{
		//				break;
		//			}
		//		}
		//	}

		//	
		//	for (int j = 0; j < start_index; j++)
		//	{

		//		LAT_LNG& latlng = ppath->m_extendwpslatlng.at(j);
		//		auto _linestringpoits_itor = std::find_if(m_pCurrentAirPortInfo->m_linestringpoints.begin(),
		//			m_pCurrentAirPortInfo->m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE>>::value_type &vt) {
		//			if (std::get<0>(vt.second) == latlng)
		//			{
		//				return true;
		//			}
		//			return false;
		//		});
		//		if (_linestringpoits_itor != m_pCurrentAirPortInfo->m_linestringpoints.end())
		//		{
		//			LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
		//			auto & ePointType = std::get<1>(_linestringpoits_itor->second);
		//			if ((ePointType == E_POINT_TYPE_OSM) || (ePointType == E_POINT_TYPE_OSM_EXTEND))
		//			{
		//				ppath->m_extendwpslatlng_start_simple.push_back(latlng);
		//			}
		//		}
		//	}
		//	LATLNGS_VECTOR m_runway_total_simple_tmp;
		//	m_runway_total_simple_tmp.insert(m_runway_total_simple_tmp.end(), ppath->m_extendwpslatlng_start_simple.begin(), ppath->m_extendwpslatlng_start_simple.end());
		//	m_runway_total_simple_tmp.insert(m_runway_total_simple_tmp.end(), ppath->m_extendwpslatlng_simple.begin(), ppath->m_extendwpslatlng_simple.end());
		//	m_runway_total_simple_tmp.push_back(ppath->m_runwayextendwpslatlng.front());
		//	m_runway_total_simple_tmp.push_back(ppath->m_runwayextendwpslatlng.back());

		//	ppath->m_runway_total_simple = std::move(m_runway_total_simple_tmp);
		//	bSave = true;
		//}
	}
	std::cout << "************************" << std::endl;
	auto itor = _poiinvalids.begin();
	while (itor != _poiinvalids.end())
	{
		std::cout << itor.key().toStdString() << std::endl;
		itor++;
	}
	std::cout << "************************" << std::endl;

	if (bSave)
	{
		QFileInfo fileinfo(filename);
		
		QString pathinfo = fileinfo.absolutePath() + "/PathInfo_.json";
		QDir fileInfo(pathinfo);
		if (!fileInfo.exists(pathinfo))
		{
			writePathInfoFile(pathinfo);
			qDebug() << "****************save path_plan_extends";
		}
	}
}

void DataManager::writePathInfoFile(const QString & filename)
{
	if (nullptr == m_pCurrentAirPortInfo)
	{
		return;
	}
	QJsonArray pathinfoarr;
	auto pathplans_itor = m_pCurrentAirPortInfo->m_Path_Plans.begin();
	while (pathplans_itor != m_pCurrentAirPortInfo->m_Path_Plans.end())
	{
		const QString& parkingpoint = pathplans_itor->first;
		const ARR_DEP_RUNWAY_PATH & arr_dep_runway = pathplans_itor->second;

		auto arr_dep_runway_itor = arr_dep_runway.begin();
		while (arr_dep_runway_itor != arr_dep_runway.end())
		{
			QString arr_dep_runway_str;
			if (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_DEP)
			{
				arr_dep_runway_str = "出港";
			}
			else if (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_ARR)
			{
				arr_dep_runway_str = "进港";
			}

			const RUNWAY_PATH & runway_path = arr_dep_runway_itor->second;
			auto runway_path_itor = runway_path.begin();
			while (runway_path_itor != runway_path.end())
			{
				const QString& runway = runway_path_itor->first;
				const tagPathPlanInfo & pathplaninfo = runway_path_itor->second;


				QJsonObject pathinfoarritem = pathplaninfo.pathindex.toJson();
				pathinfoarr.push_back(pathinfoarritem);
				runway_path_itor++;
			}
			arr_dep_runway_itor++;
		}

		pathplans_itor++;
	}

	QJsonObject jsonobj;
	jsonobj.insert("pathInfos", pathinfoarr);

	FunctionAssistant::write_json_file_object(filename, jsonobj);
}




void tagPath_Plan::resize(int wpssize)
{
	m_pathPoints.reserve(wpssize);
	m_trackinglatlng.reserve(wpssize);
	m_tracking_osm_path_info.reserve(wpssize);
	m_tracking_osm_path_info_calibrate.reserve(wpssize);
};

void tagPath_Plan::analysis(const std::unordered_map<QString, tagStandard_Taxiing_Path>& _Standard_Taxiing_Paths, const std::unordered_map<QString, tagPoiItem>& _poiitemsmap)
{
	auto appendPoint = [&](const QString& parkingpoint)
	{
		int index = m_tracking_osm_path_info.size();
		if (_poiitemsmap.find(parkingpoint) != _poiitemsmap.end())
		{
			m_trackinglatlng.push_back(_poiitemsmap.at(parkingpoint).poipoint);
			m_tracking_osm_path_info.push_back(_poiitemsmap.at(parkingpoint).m_calibrate_osm_path_info);
			m_tracking_osm_path_info_calibrate.push_back(_poiitemsmap.at(parkingpoint).m_calibrate_osm_path_info);
			m_pathPoints.push_back(std::make_tuple(parkingpoint, true, index));
		}
		else
		{
			m_pathPoints.push_back(std::make_tuple(parkingpoint, false, index));
		}
	};
	///////////////////////////////////////////////////////////////////////////
	m_pathPoints.clear();
	m_trackinglatlng.clear();

	m_tracking_osm_path_info.clear();
	m_tracking_osm_path_info_calibrate.clear();
	///////////////////////////////////////////////////////////////////////////
	if (!m_path.isEmpty())
	{
		QStringList pathPoints = m_path.split(",");
		QString externpath;
		QString externpath2;

		switch (m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			externpath = pathPoints.back();
			externpath2 = pathPoints.at(pathPoints.size() - 2);
		}
		break;
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			externpath = pathPoints.front();
			externpath2 = pathPoints.at(1);
		}
		break;
		default:
			break;
		}

        m_trackingcl = FunctionAssistant::randColor(255);


        auto getParkpointstart=[&](const QString& parkingpoint)->QString
        {

            QString _parkingpoint = parkingpoint;


            QString _parkingpointtmp = parkingpoint;
            ////_parkingpointtmp = "132L";
            //_parkingpointtmp = "GY01";

            QRegularExpression regex("^[^0-9]\\S*");

            QRegularExpressionMatch match = regex.match(_parkingpointtmp);

            QString matchstr;
            if (match.hasMatch())
            {
                matchstr = match.captured(0);
            }
            if(matchstr.isEmpty() )
            {
                _parkingpoint.insert(1, "PBN");
            }
            if(matchstr.startsWith("P"))
            {
                _parkingpoint.insert(1, "BN");
            }
            else
            {
                _parkingpoint.insert(0, "PBN");
            }
            return _parkingpoint;
        };

		auto _Standard_Taxiing_Paths_itor = _Standard_Taxiing_Paths.find(externpath);
		if (_Standard_Taxiing_Paths_itor != _Standard_Taxiing_Paths.end())
		{
			const tagStandard_Taxiing_Path& standard_Taxiing_Path = _Standard_Taxiing_Paths_itor->second;
			if (standard_Taxiing_Path.m_flight_dep_arr_type == m_flight_dep_arr_type &&
				standard_Taxiing_Path.m_runway.contains(m_runway))
			{
				switch (m_flight_dep_arr_type)
				{
				case E_FLIGHT_DEP_ARR_TYPE_DEP:
				{
					if (standard_Taxiing_Path.m_pathPoints.contains(externpath2))
					{
                        QString parkingpointtmp = m_parkingpoint;
                        parkingpointtmp = getParkpointstart(parkingpointtmp);
						int index = standard_Taxiing_Path.m_pathPoints.indexOf(externpath2);
						int wpssize = (standard_Taxiing_Path.m_pathPoints.size() - index - 1) + pathPoints.size() - 2 + 1 + 1;
						resize(wpssize);

						appendPoint(m_parkingpoint);
						appendPoint(parkingpointtmp);
						for (int j = 1; j < pathPoints.size() - 1; j++)
						{
							appendPoint(pathPoints.at(j));
						}
						for (int i = index + 1; i < standard_Taxiing_Path.m_pathPoints.size(); i++)
						{
							appendPoint(standard_Taxiing_Path.m_pathPoints.at(i));
						}
						if (wpssize != m_pathPoints.size())
						{
							std::cout << "un match" << std::endl;
						}
					}
					else
					{
                        QString parkingpointtmp = m_parkingpoint;
                        parkingpointtmp = getParkpointstart(parkingpointtmp);
						int wpssize = standard_Taxiing_Path.m_pathPoints.size() + pathPoints.size() - 2 + 1 + 1;
						resize(wpssize);

						appendPoint(m_parkingpoint);
						appendPoint(parkingpointtmp);
						for (int j = 1; j < pathPoints.size() - 1; j++)
						{
							appendPoint(pathPoints.at(j));
						}

						for (int i = 0; i < standard_Taxiing_Path.m_pathPoints.size(); i++)
						{
							appendPoint(standard_Taxiing_Path.m_pathPoints.at(i));
						}
						if (wpssize != m_pathPoints.size())
						{
							std::cout << "un match" << std::endl;
						}
					}
				}
				break;
				case E_FLIGHT_DEP_ARR_TYPE_ARR:
				{
					if (standard_Taxiing_Path.m_pathPoints.contains(externpath2))
					{
						int index = standard_Taxiing_Path.m_pathPoints.indexOf(externpath2);
						int wpssize = index + 1 + pathPoints.size() - 2 + 1;
						resize(wpssize);

						for (int i = 0; i < index + 1; i++)
						{
							appendPoint(standard_Taxiing_Path.m_pathPoints.at(i));
						}
						for (int j = 2; j < pathPoints.size(); j++)
						{
							appendPoint(pathPoints.at(j));
						}
						appendPoint(m_parkingpoint);
						if (wpssize != m_pathPoints.size())
						{
							std::cout << "un match" << std::endl;

						}
					}
					else
					{
						int wpssize = standard_Taxiing_Path.m_pathPoints.size() + pathPoints.size() - 1 + 1;
						resize(wpssize);

						for (int i = 0; i < standard_Taxiing_Path.m_pathPoints.size(); i++)
						{
							appendPoint(standard_Taxiing_Path.m_pathPoints.at(i));
						}
						for (int j = 1; j < pathPoints.size(); j++)
						{
							appendPoint(pathPoints.at(j));
						}
						appendPoint(m_parkingpoint);
						if (wpssize != m_pathPoints.size())
						{
							std::cout << "un match" << std::endl;
						}
					}
				}
				break;
				default:
					break;
				}
			}
			if (this->m_pathPoints.size() > 2)
			{
				if (std::get<1>(this->m_pathPoints.at(0)) &&
					std::get<1>(this->m_pathPoints.at(this->m_pathPoints.size() - 1)) &&
					!this->m_extendwpslatlng.empty())
				{
					this->m_bValid = true;
				}
				else
				{
					this->m_bValid = false;
				}
			}
			else
			{
				this->m_bValid = false;
			}
		}
		else
		{

			int wpssize = pathPoints.size() - 1 + 1;
			resize(wpssize);
			for (int j = 1; j < pathPoints.size(); j++)
			{
				appendPoint(pathPoints.at(j));
			}
			appendPoint(m_parkingpoint);
			if (wpssize != m_pathPoints.size())
			{
				std::cout << "un match" << std::endl;
			}

			if (this->m_pathPoints.size() > 2)
			{
				if (std::get<1>(this->m_pathPoints.at(0)) &&
					std::get<1>(this->m_pathPoints.at(this->m_pathPoints.size() - 1)) &&
					!this->m_extendwpslatlng.empty())
				{
					this->m_bValid = true;
				}
				else
				{
					this->m_bValid = false;
				}
			}
			else
			{
				this->m_bValid = false;
			}

		}
	}
}



QJsonObject tagPath_Plan::toJson() const
{
    QJsonArray pathPoints;
    QJsonArray pathPointsLatLng;
    QJsonArray pathPointsNearLatLng;
    QJsonArray pathPointsCalibrateLatLng;

    auto getLatLngJsonArray = [](const LAT_LNG& latlng)->QJsonArray {
        QJsonArray latlngarr;
        latlngarr.append(latlng.lng);
        latlngarr.append(latlng.lat);
        return latlngarr;
    };
    for (int i = 0; i < m_pathPoints.size(); i++)
    {
        QJsonObject pathPoint;
        pathPoint.insert("poiname", std::get<0>(m_pathPoints.at(i)));
        pathPoint.insert("poivalid", std::get<1>(m_pathPoints.at(i)));
        pathPoint.insert("index", std::get<2>(m_pathPoints.at(i)));
        pathPoints.append(pathPoint);
    }
    for (int i = 0; i < m_trackinglatlng.size(); i++)
    {
        pathPointsLatLng.append(getLatLngJsonArray(m_trackinglatlng.at(i)));
    }
    for (int i = 0; i < m_tracking_osm_path_info.size(); i++)
    {
        pathPointsNearLatLng.append(getLatLngJsonArray(std::get<1>(m_tracking_osm_path_info.at(i))));
    }
    for (int i = 0; i < m_tracking_osm_path_info_calibrate.size(); i++)
    {
        pathPointsCalibrateLatLng.append(getLatLngJsonArray(std::get<1>(m_tracking_osm_path_info_calibrate.at(i))));
    }


    QJsonArray extendwps;
    for (int i = 0; i < m_extendwpslatlng.size(); i++)
    {
        extendwps.append(getLatLngJsonArray(m_extendwpslatlng.at(i)));
    }

    QJsonArray extendwps_simple;
    for (int i = 0; i < m_extendwpslatlng_simple.size(); i++)
    {
        extendwps_simple.append(getLatLngJsonArray(m_extendwpslatlng_simple.at(i)));
    }

    QJsonArray extendwps_start_simple;
    for (int i = 0; i < m_extendwpslatlng_start_simple.size(); i++)
    {
        extendwps_start_simple.append(getLatLngJsonArray(m_extendwpslatlng_start_simple.at(i)));
    }

    QJsonArray extendwps_runway;
    for (int i = 0; i < m_runwayextendwpslatlng.size(); i++)
    {
        extendwps_runway.append(getLatLngJsonArray(m_runwayextendwpslatlng.at(i)));
    }

    QJsonArray extendwps_total;
    for (int i = 0; i < m_runway_total.size(); i++)
    {
        extendwps_total.append(getLatLngJsonArray(m_runway_total.at(i)));
    }

    QJsonArray extendwps_total_simple;
    for (int i = 0; i < m_runway_total_simple.size(); i++)
    {
        extendwps_total_simple.append(getLatLngJsonArray(m_runway_total_simple.at(i)));
    }

    QJsonObject jsonobj;
    jsonobj.insert("runway", m_runway);
    jsonobj.insert("arrdeptype", (INT32)m_flight_dep_arr_type);
    jsonobj.insert("parkingpoint", m_parkingpoint);
    jsonobj.insert("airportcode", m_airportcode);
    jsonobj.insert("path", m_path);
    jsonobj.insert("pathPoints", pathPoints);
    jsonobj.insert("pathPointsLatLng", pathPointsLatLng);
    jsonobj.insert("pathPointsNearLatLng", pathPointsNearLatLng);
    jsonobj.insert("pathPointsCalibrateLatLng", pathPointsCalibrateLatLng);
    jsonobj.insert("extendwps", extendwps);
    jsonobj.insert("extendwps_simple", extendwps_simple);
    jsonobj.insert("extendwps_start_simple", extendwps_start_simple);
    jsonobj.insert("extendwps_runway", extendwps_runway);
    jsonobj.insert("extendwps_total", extendwps_total);
    jsonobj.insert("extendwps_total_simple", extendwps_total_simple);
    jsonobj.insert("valid", m_bValid);

    return jsonobj;
}

void tagPath_Plan::fromJson(const QJsonObject& jsonobj)
{
    m_runway = jsonobj.value("runway").toString();
    m_flight_dep_arr_type = (E_FLIGHT_DEP_ARR_TYPE)(jsonobj.value("arrdeptype").toInt());
    m_parkingpoint = jsonobj.value("parkingpoint").toString();
    m_airportcode = jsonobj.value("airportcode").toString();
    m_path = jsonobj.value("path").toString();
    m_bValid = jsonobj.value("valid").toBool();

    QJsonArray pathPoints = jsonobj.value("pathPoints").toArray();
    QJsonArray pathPointsLatLng = jsonobj.value("pathPointsLatLng").toArray();
    QJsonArray pathPointsNearLatLng = jsonobj.value("pathPointsNearLatLng").toArray();
    QJsonArray pathPointsCalibrateLatLng = jsonobj.value("pathPointsCalibrateLatLng").toArray();
    QJsonArray extendwps = jsonobj.value("extendwps").toArray();
    QJsonArray extendwps_start_simple = jsonobj.value("extendwps_start_simple").toArray();
    QJsonArray extendwps_simple = jsonobj.value("extendwps_simple").toArray();

    QJsonArray extendwps_runway = jsonobj.value("extendwps_runway").toArray();
    QJsonArray extendwps_total = jsonobj.value("extendwps_total").toArray();

    QJsonArray extendwps_total_simple = jsonobj.value("extendwps_total_simple").toArray();

    m_pathPoints.reserve(pathPoints.size());
    for (int i = 0; i < pathPoints.size(); i++)
    {
        QJsonObject pathPoint = pathPoints.at(i).toObject();
        m_pathPoints.push_back(std::make_tuple(pathPoint.value("poiname").toString(), pathPoint.value("poivalid").toBool(), pathPoint.value("index").toInt()));
    }

    m_trackinglatlng.reserve(pathPointsLatLng.size());
    for (int i = 0; i < pathPointsLatLng.size(); i++)
    {
        if (pathPointsLatLng.at(i).isArray() && pathPointsLatLng.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = pathPointsLatLng.at(i).toArray();
            m_trackinglatlng.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_tracking_osm_path_info.reserve(pathPointsNearLatLng.size());
    for (int i = 0; i < pathPointsNearLatLng.size(); i++)
    {
        if (pathPointsNearLatLng.at(i).isArray() && pathPointsNearLatLng.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = pathPointsNearLatLng.at(i).toArray();
            m_tracking_osm_path_info.push_back(std::make_tuple(i, LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() }));
        }
    }

    m_tracking_osm_path_info.reserve(pathPointsCalibrateLatLng.size());
    for (int i = 0; i < pathPointsCalibrateLatLng.size(); i++)
    {
        if (pathPointsCalibrateLatLng.at(i).isArray() && pathPointsCalibrateLatLng.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = pathPointsCalibrateLatLng.at(i).toArray();
            m_tracking_osm_path_info_calibrate.push_back(std::make_tuple(i, LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() }));
        }
    }

    m_extendwpslatlng.reserve(extendwps.size());
    for (int i = 0; i < extendwps.size(); i++)
    {
        if (extendwps.at(i).isArray() && extendwps.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps.at(i).toArray();
            m_extendwpslatlng.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_extendwpslatlng_simple.reserve(extendwps_simple.size());
    for (int i = 0; i < extendwps_simple.size(); i++)
    {
        if (extendwps_simple.at(i).isArray() && extendwps_simple.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps_simple.at(i).toArray();
            m_extendwpslatlng_simple.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_extendwpslatlng_start_simple.reserve(extendwps_start_simple.size());
    for (int i = 0; i < extendwps_start_simple.size(); i++)
    {
        if (extendwps_start_simple.at(i).isArray() && extendwps_start_simple.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps_start_simple.at(i).toArray();
            m_extendwpslatlng_start_simple.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_runwayextendwpslatlng.reserve(extendwps_runway.size());
    for (int i = 0; i < extendwps_runway.size(); i++)
    {
        if (extendwps_runway.at(i).isArray() && extendwps_runway.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps_runway.at(i).toArray();
            m_runwayextendwpslatlng.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_runway_total.reserve(extendwps_total.size());
    for (int i = 0; i < extendwps_total.size(); i++)
    {
        if (extendwps_total.at(i).isArray() && extendwps_total.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps_total.at(i).toArray();
            m_runway_total.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }

    m_runway_total_simple.reserve(extendwps_total_simple.size());
    for (int i = 0; i < extendwps_total_simple.size(); i++)
    {
        if (extendwps_total_simple.at(i).isArray() && extendwps_total_simple.at(i).toArray().size() >= 2)
        {
            QJsonArray latlngarr = extendwps_total_simple.at(i).toArray();
            m_runway_total_simple.push_back(LAT_LNG{ latlngarr.at(1).toDouble(),latlngarr.at(0).toDouble() });
        }
    }
}


QJsonObject tagPath_Plan::outputgeojson()
{
	QJsonObject geojson;
	geojson.insert("type", "FeatureCollection");
	QJsonArray features;

	QJsonObject properties;
	if (!m_runway_total.empty())
	{
		properties.insert("name", "path_total");
		properties.insert("stroke-color", "#00ff00");
		properties.insert("stroke-width", 1);
		properties.insert("stroke-opacity", 0.5);
		FunctionAssistant::appendfeatureline(properties, features, this->m_runway_total);
	}

	if (!m_extendwpslatlng.empty())
	{
		properties.insert("name", "path_taxi");
		properties.insert("stroke-color", "#ff0000");
		properties.insert("stroke-width", 2);
		properties.insert("stroke-opacity", 1);
		FunctionAssistant::appendfeatureline(properties, features, this->m_extendwpslatlng);
	}

	if (!m_runwayextendwpslatlng.empty())
	{
		properties.insert("name", "path_runway");
		properties.insert("stroke-color", "#0000ff");
		properties.insert("stroke-width", 2);
		properties.insert("stroke-opacity", 1);
		FunctionAssistant::appendfeatureline(properties, features, this->m_runwayextendwpslatlng);
	}

	for (int i = 0; i < m_pathPoints.size(); i++)
	{
		QString name = std::get<0>(m_pathPoints.at(i));
		bool bvalid = std::get<1>(m_pathPoints.at(i));
		int index = std::get<2>(m_pathPoints.at(i));
		if (index < m_trackinglatlng.size())
		{
			const LAT_LNG& pos = m_trackinglatlng.at(index);
			QJsonObject properties;
			properties.insert("name", name + "_poi");
			properties.insert("valid", bvalid);
			properties.insert("marker-color", "#00ffff");
			properties.insert("marker-size", "medium");
			properties.insert("marker-symbol", "circle");
			FunctionAssistant::appendfeaturepoint(properties, features, pos);
		}

		if (index < m_tracking_osm_path_info_calibrate.size())
		{
			const LAT_LNG& pos = std::get<1>(m_tracking_osm_path_info_calibrate.at(index));

			QJsonObject properties;
			properties.insert("name", name + "_calibrate");
			properties.insert("valid", bvalid);
			if (bvalid)
			{
				properties.insert("marker-color", "#00ff00");
				properties.insert("marker-size", "medium");
				properties.insert("marker-symbol", "circle");
			}
			else
			{
				properties.insert("marker-color", "#ff0000");
				properties.insert("marker-size", "medium");
				properties.insert("marker-symbol", "circle-stroked");

			}
			FunctionAssistant::appendfeaturepoint(properties, features, pos);
		}
	}
	geojson.insert("features", features);
	//qDebug() << geojson;
	return geojson;
}

void tagStandard_Taxiing_Path::analysis()
{
	if (!m_pathdetail.isEmpty())
	{
		QStringList pathPoints = m_pathdetail.split(",");
		m_pathPoints.resize(pathPoints.size());
		for (int i = 0; i < pathPoints.size(); i++)
		{
			m_pathPoints[i] = pathPoints.at(i);
		}
	}
}

