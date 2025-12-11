#pragma execution_character_set("utf-8")
#include "datamanager.hpp"
#include <QJsonObject>
#include <string>
#include <QJsonArray>

#include <QDir>

#include <QtXlsx/QtXlsx>
#include <iostream>
#include <QCoreApplication>
#include "function.h"

#include "src/algorithm/Dijkstra.h"
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <QNetworkAccessManager>
#include <QNetworkDiskCache>


//#if 0
//#define POINT_EXTEND_METRE	(15.0f)
//#define GRAPH_MAX_VEX_DIFF	(3.0f)
//#else
//#define POINT_EXTEND_METRE	(30.0f)
//#define GRAPH_MAX_VEX_DIFF	(1.5f)
//#endif
#define EXTEND_INDEX (3)

//#define DEP_END_INDEX_DIRECT
#define ARR_END_INDEX_EXINCLUDE_PBNPOINT
#define DEAL_ANGLE (20.0f)

#define TARGET_HEIGHT_METER (300.0f)
#define HOLDPOINT_EXTEND_METER (25)
#define HOLDPOINT_EXTEND_AREA_METER (150)
#define TAKEOFF_METER (1150)

#define LANDINGPOINT_METER (15)



DataManager& DataManager::getInstance()
{
	static DataManager configmanager;
	return configmanager;
}

DataManager::DataManager()
    :m_pDijkstra(nullptr),
	m_pPathPlanExtendTaskManager(nullptr),
    m_pThreadPool(nullptr),
    m_deal_data_process_func_callback(nullptr)
{
    m_pPathPlanExtendTaskManager = new PathPlanExtendTaskManager(nullptr);
}

void DataManager::dealpath(const QString& path, double diff,double space)
{
    m_deal_path = path;
    m_point_extend_metres = diff;
    m_graph_max_vex_diff = space;


    init(path);
}

void DataManager::init(const QString &path)
{

#ifdef CALC_PATH
    if(m_pDijkstra)
    {
        delete m_pDijkstra;
        m_pDijkstra = nullptr;
    }

    m_pDijkstra = new Dijkstra();
    m_pDijkstra->Create_graph(GRAPH_MAX_VEX * m_graph_max_vex_diff);
#endif // CALC_PATH

    //////////////////////////////////////////////////////////////////////////////
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

    //////////////////////////////////////////////////////////////////////////////

    readGeoDirPath(m_deal_path + "/aeroway");

    QString wypointsPath = m_deal_path + "/waypoints/airportwaypoints.geojson";
    readWaypointsGeoJsonFile(wypointsPath.toStdString());


    QString runwaypath = m_deal_path + "/aeroway/runway.geojson";
    readWpsRunwayFile(runwaypath.toStdString());

    readPathDir(m_deal_path + "/path");
}

std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> >& DataManager::flighttimedata()
{
	return m_flighttimedata;
}

void DataManager::setFlighttimedata(const std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> > &newFlighttimedata)
{
	m_flighttimedata = newFlighttimedata;
}

std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>>& DataManager::flightEventTimedata()
{
	return m_flightEventTimedata;
}

void DataManager::setFlightEventTimedata(const std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>> &newFlightEventTimedata)
{
	m_flightEventTimedata = newFlightEventTimedata;
}

std::map<QString, ARR_DEP_RUNWAY_PATH>& DataManager::getPathPlans()
{
	return m_Path_Plans;
}

QList<tagPath_Plan*> DataManager::getArrpaths()
{
	return m_arrpaths;
}

QList<tagPath_Plan*> DataManager::getDeppaths()
{
	return m_deppaths;
}

QStringList& DataManager::getRunways()
{
	return m_runways;
}


void DataManager::updateDijkstraMap(const LAT_LNG& latlng, const LAT_LNG& lstlatlng, bool bBegin,const QString& aeroway)
{
	if (bBegin)
	{
		UINT64 item_id;
        appendLinestringPoint(item_id, latlng, E_POINT_TYPE_OSM, aeroway);
	}
	else
	{
		UINT64 lstitem_id;
		bool bExist = getLinestringPoint(lstitem_id, lstlatlng);
		if (bExist)
		{
            appendDijkstraMap(latlng, lstlatlng, lstitem_id, E_POINT_TYPE_OSM_EXTEND, E_POINT_TYPE_OSM, aeroway);
		}
	}
}

bool DataManager::getLinestringPointId(UINT64&item_id, const LAT_LNG& latlng)
{
	auto _linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
        m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (_linestringpoits_itor != m_linestringpoints.end())
	{
		item_id = _linestringpoits_itor->first;
		return true;
	}
	return false;
}

bool DataManager::appendLinestringPoint(UINT64&item_id, const LAT_LNG& latlng, E_POINT_TYPE ePointType,const QString& aeroway)
{
	bool bNewAppend = false;
	item_id = 0;
	auto _linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
        m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (_linestringpoits_itor == m_linestringpoints.end())
	{
		item_id = FunctionAssistant::generate_random_positive_uint64();
        m_linestringpoints.insert(std::make_pair(item_id, std::make_tuple(latlng, ePointType,aeroway)));
		switch (ePointType)
		{
		case E_POINT_TYPE_OSM:
		case E_POINT_TYPE_OSM_EXTEND:
		{
			if (m_pDijkstra)
			{
				m_pDijkstra->appendNode(item_id, latlng);
			}
		}
		break;
		default:
			break;
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
            auto & eoldPointTypedes = std::get<2>(_linestringpoits_itor->second);

            if (oldePointType != ePointType)
			{
                LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
				if (m_pDijkstra)
				{
					m_pDijkstra->appendNode(item_id, latlng);
				}
                if(oldePointType != E_POINT_TYPE_OSM && ePointType == E_POINT_TYPE_OSM)
                {
                    oldePointType = ePointType;
                    eoldPointTypedes = aeroway;
                }

                if(oldePointType == E_POINT_TYPE_OSM && eoldPointTypedes != "runway" && aeroway== "runway")
                {
                    eoldPointTypedes = aeroway;
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

UINT64 DataManager::appendDijkstraMap(const LAT_LNG& latlng, const LAT_LNG &lstlatlng, UINT64 lstid, E_POINT_TYPE ePointExtendType, E_POINT_TYPE ePointTypeSrc, const QString& aeroway)
{
	UINT64 currentid;
    double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
    if (dis > m_point_extend_metres)
    {
        int step = dis / m_point_extend_metres;
		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
        LAT_LNG lstextendpt = lstlatlng;
		UINT64 lstextenditem_id = lstid;
		for (int i = 1; i < step + 1; i++)
        {
            LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, m_point_extend_metres);
            appendLinestringPoint(currentid, currentextendpt, ePointExtendType, aeroway);
			switch (ePointExtendType)
			{
			case E_POINT_TYPE_OSM:
			case E_POINT_TYPE_OSM_EXTEND:
			{
				if (m_pDijkstra)
                {
                    m_pDijkstra->updateEdge(lstextenditem_id, currentid, m_point_extend_metres);
				}
			}
			break;
			default:
				break;
			}
			lstextendpt = currentextendpt;
			lstextenditem_id = currentid;
        }
        double rdis = dis - m_point_extend_metres * step;

        appendLinestringPoint(currentid, latlng, ePointTypeSrc,aeroway);
		switch (ePointTypeSrc)
		{
		case E_POINT_TYPE_OSM:
		case E_POINT_TYPE_OSM_EXTEND:
		{
			if (m_pDijkstra)
			{
				m_pDijkstra->updateEdge(lstextenditem_id, currentid, rdis);
			}
		}
		break;
		default:
			break;
		}
	}
	else
	{
        appendLinestringPoint(currentid, latlng, ePointTypeSrc,aeroway);
		switch (ePointTypeSrc)
		{
		case E_POINT_TYPE_OSM:
		case E_POINT_TYPE_OSM_EXTEND:
		{
			if (m_pDijkstra)
			{
				m_pDijkstra->updateEdge(lstid, currentid, dis);
			}
		}
		break;
		default:
			break;
		}
	}
	return currentid;
}

bool DataManager::getLinestringPoint(UINT64&item_id, const LAT_LNG& latlng)
{
	bool bExist = false;
	auto lastlng_linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
        m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {

		if (std::get<0>(vt.second) == latlng)
		{
			return true;
		}
		return false;
	});
	if (lastlng_linestringpoits_itor != m_linestringpoints.end())
	{
		item_id = lastlng_linestringpoits_itor->first;
		bExist = true;
	}
	return bExist;
}

std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>& DataManager::getLinestringpoints()
{
	return m_linestringpoints;
}

bool DataManager::getPoi_osm_path_id(UINT64&Poi_osm_path_id, const QString& poikeyword)
{
	auto _poiitemsmap_itor = m_poiitemsmap.find(poikeyword);
	if (_poiitemsmap_itor != m_poiitemsmap.end())
	{
		Poi_osm_path_id = std::get<0>(_poiitemsmap_itor->second.m_calibrate_osm_path_info);
		return true;
	}
	return false;
}

tagPathInfo DataManager::getPath(const TYPE_ULID &uildsrc, const TYPE_ULID &dst)
{
	if (m_pDijkstra)
	{
		return m_pDijkstra->getPathThreadSafe(uildsrc, dst);
	}

	return tagPathInfo();
}

void DataManager::getExtendWpsExName2Name(const QString& poi1, const QString& poi2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret)
{
	ret.clear();
	UINT64 Poi_osm_path_id1;
	UINT64 Poi_osm_path_id2;
	if (this->getPoi_osm_path_id(Poi_osm_path_id1, poi1) &&
		this->getPoi_osm_path_id(Poi_osm_path_id2, poi2))
	{
		return getExtendWpsExID2ID_EX(Poi_osm_path_id1, Poi_osm_path_id2, ret);
	}
}

void DataManager::getExtendWpsExID2Name(const UINT64& Poi_osm_path_id1, const QString& poi2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret)
{
	ret.clear();
	UINT64 Poi_osm_path_id2;
	if (this->getPoi_osm_path_id(Poi_osm_path_id2, poi2))
	{
		return getExtendWpsExID2ID_EX(Poi_osm_path_id1, Poi_osm_path_id2, ret);
	}
}

void DataManager::getExtendWpsExID2ID(const UINT64& Poi_osm_path_id1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret)
{
	ret.clear();
	return getExtendWpsExID2ID_EX(Poi_osm_path_id1, Poi_osm_path_id2, ret);
}

void DataManager::getExtendWpsExID2ID_EX(const UINT64& Poi_osm_path_id1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret)
{
	if (Poi_osm_path_id1 != 0 && Poi_osm_path_id2 != 0 && Poi_osm_path_id1 != Poi_osm_path_id2)
	{
		auto pathInfo = this->getPath(Poi_osm_path_id1, Poi_osm_path_id2);

		if (!pathInfo.m_path.empty())
		{
			ret.reserve(pathInfo.m_path.size());
			for (int j = 0; j < pathInfo.m_path.size(); j++)
			{
				ret.push_back(std::make_tuple(pathInfo.m_path.at(j).m_pos, pathInfo.m_path.at(j).m_id));
			}
		}
	}
}

void DataManager::getExtendWpsExName2ID(const QString& poi1, const UINT64& Poi_osm_path_id2, std::vector<std::tuple<LAT_LNG, UINT64>>& ret)
{
	ret.clear();
	UINT64 Poi_osm_path_id1;
	if (this->getPoi_osm_path_id(Poi_osm_path_id1, poi1))
	{
		return getExtendWpsExID2ID_EX(Poi_osm_path_id1, Poi_osm_path_id2, ret);
	}
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
	calibrate_id = 0;
    calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_linestringpoints.begin();
	while (_linestringpoits_itor != m_linestringpoints.end())
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
	calibrate_id = 0;
    calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_linestringpoints.begin();
	while (_linestringpoits_itor != m_linestringpoints.end())
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
	calibrate_id = 0;
    calibrate_latlng = LAT_LNG{ 0,0 };

	float dismin = DIJKSTRA_MAX_VALUE;
	auto _linestringpoits_itor = m_linestringpoints.begin();
	while (_linestringpoits_itor != m_linestringpoints.end())
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

void DataManager::generateExtendWps(tagPath_Plan& path_plan)
{
	path_plan.m_extendwpslatlng.clear();
    path_plan.m_extendwpslatlng_simple.clear();
	///////////////////////////////////////////////////////////////////////////
	if (!path_plan.m_trackinglatlng.empty() &&
		!path_plan.m_tracking_osm_path_info.empty() &&
		!path_plan.m_tracking_osm_path_info_calibrate.empty() /*&&
		path_plan.m_pathPoints.size() == path_plan.m_tracking_osm_path_info.size() &&
		path_plan.m_pathPoints.size() == path_plan.m_trackinglatlng.size()*/)
	{
        std::list<std::tuple<LAT_LNG, UINT64, BYTE>> extendwpslatlngTmp;
		LATLNGS_VECTOR extendwpsEndlatlngTmp;
		UINT64 start_calibrate_id = std::get<0>(path_plan.m_tracking_osm_path_info.at(0));
        LAT_LNG start_calibrate_latlng = std::get<1>(path_plan.m_tracking_osm_path_info.at(0));
		//QString poiName_start = path_plan.m_pathPoints.at(0);

		int validSize = path_plan.m_trackinglatlng.size();
		for (int j = 0; j < validSize - 1; j++)
		{
			//const QString& poiName1 = path_plan.m_pathPoints.at(j);
			const UINT64& Poi_osm_path_id1 = std::get<0>(path_plan.m_tracking_osm_path_info.at(j));
            const LAT_LNG& poilatlng1 = path_plan.m_trackinglatlng.at(j);

			//const QString& poiName2 = path_plan.m_pathPoints.at(j + 1);
			const UINT64& Poi_osm_path_id2 = std::get<0>(path_plan.m_tracking_osm_path_info.at(j + 1));
            const LAT_LNG& poilatlng2 = path_plan.m_trackinglatlng.at(j + 1);

            const LAT_LNG& Poi_osm_path_latlng1 = std::get<1>(path_plan.m_tracking_osm_path_info.at(j));
            const LAT_LNG& Poi_osm_path_latlng2 = std::get<1>(path_plan.m_tracking_osm_path_info.at(j + 1));

			UINT64& Poi_osm_path_id_calibrate = std::get<0>(path_plan.m_tracking_osm_path_info_calibrate.at(j));
            LAT_LNG& Poi_osm_path_latlng_calibrate = std::get<1>(path_plan.m_tracking_osm_path_info_calibrate.at(j));

			Poi_osm_path_id_calibrate = start_calibrate_id;
			Poi_osm_path_latlng_calibrate = start_calibrate_latlng;

            auto dealgenerateTrackingPoints = [&extendwpslatlngTmp, &extendwpsEndlatlngTmp](const std::vector<std::tuple<LAT_LNG, UINT64>>& trackingpt_ret,
				int appendSize, bool bAppendEnd, bool bDerict = false)
			{
				int iFlag = bDerict ? 2 : 1;
				for (int index = 0; index < appendSize && index < trackingpt_ret.size(); index++)
				{
                    const std::tuple<LAT_LNG, UINT64>& item = trackingpt_ret.at(index);
                    const LAT_LNG& latlng = std::get<0>(item);
					const UINT64& _id = std::get<1>(item);
					if (bAppendEnd)
					{
						extendwpsEndlatlngTmp.push_back(latlng);
					}
					else
					{
						auto _exist_itor = std::find_if(extendwpslatlngTmp.begin(),
                            extendwpslatlngTmp.end(), [&_id](const std::list<std::tuple<LAT_LNG, UINT64, BYTE>>::value_type &vt) {
							return std::get<1>(vt) == _id;
						});
						if (_exist_itor != extendwpslatlngTmp.end())
						{
							BYTE & icount = std::get<2>(*_exist_itor);
							icount++;
						}
						else
						{
							extendwpslatlngTmp.push_back(std::make_tuple(latlng, _id, iFlag));
						}
					}
				}
			};
            auto generateTrackingPoints = [&](const std::vector<std::tuple<LAT_LNG, UINT64>>& trackingpt_poi1poi2, int jnext) ->bool {
				bool bDealPOI1_2_POI2 = true;
				//const QString& poiName3 = path_plan.m_pathPoints.at(jnext);
                const LAT_LNG&  poilatlng3 = path_plan.m_trackinglatlng.at(jnext);
				const UINT64&  Poi_osm_path_id3 = std::get<0>(path_plan.m_tracking_osm_path_info.at(jnext));

                std::vector<std::tuple<LAT_LNG, UINT64>> trackingpt_poi1_poi3;
				//////////////////////////////////////////////////////////////////////////////////
				//对应poi_1/poi_2与poi_1/poi_3之间寻去的最小路径方向一致，即第一点相同的清空处理
				//1.poi1 与 poi_3之间查找最小路径，【此处做路径初次获取】，然后将poi_2映射到此路径中距离其距离最小的点中【po2_distance_min】
				//2.poi1 与 po2_distance_min之间查找最小路径 【此处做在步骤2中获取的路径初选点的最终路径选取】
				//////////////////////////////////////////////////////////////////////////////////
				//对应poi_1/poi_2与poi_1/poi_3之间寻去的最小路径方向不一致，即第一点即不同的清空处理
				//1.先将路径点往poi_1和poi_2之间移动一半【poi1_poi2_mid】【此处做最终路径选取】，确保poi_1出来的路径是朝poi_2方向移动，
				//避免poi_1 和poi_3之间直接查找最小路径，找到了其他最小路径导致条路【十字交叉路口】。
				//2.poi1_poi2_mid 与 poi_3之间查找最小路径，【此处做路径初次获取】，然后将poi_2映射到此路径中距离其距离最小的点中【po2_distance_min】
				//3.poi1_poi2_mid 与 po2_distance_min之间查找最小路径 【此处做在步骤2中获取的路径初选点的最终路径选取】

                auto generateSubTracking = [&](const UINT64& Poi_osm_path_id_mid_poi1_poi2, std::vector<std::tuple<LAT_LNG, UINT64>>* tracking_ret_ptr) {
					if (nullptr == tracking_ret_ptr)
					{
						return;
					}
                    std::vector<std::tuple<LAT_LNG, UINT64>>& tracking_ret = *tracking_ret_ptr;
					//求当前点和后二点之间的最小路径

					dealgenerateTrackingPoints(tracking_ret, tracking_ret.size(), false);


					//求下一点与【当前点和后二点之间的最小路径】中最靠近的一点
					UINT64 po2_distance_min_near_id;
                    LAT_LNG po2_distance_min_near_latlng;
					findNextPoiDistanceMinPoint(po2_distance_min_near_id, po2_distance_min_near_latlng, poilatlng2, tracking_ret);


					//求当前点与【下一点与【当前点和后二点之间的最小路径】中最靠近的一点】的最小路径

                    std::vector<std::tuple<LAT_LNG, UINT64>> trackingpt_midpoint1_2_poi2_ok;
					getExtendWpsExID2ID(Poi_osm_path_id_mid_poi1_poi2, po2_distance_min_near_id, trackingpt_midpoint1_2_poi2_ok);

					start_calibrate_id = po2_distance_min_near_id;
					start_calibrate_latlng = po2_distance_min_near_latlng;

					dealgenerateTrackingPoints(trackingpt_midpoint1_2_poi2_ok, trackingpt_midpoint1_2_poi2_ok.size(), false);
				};


				auto trackingpt_poi1_poi2_itor = trackingpt_poi1poi2.begin();
				trackingpt_poi1_poi2_itor++;
				if (trackingpt_poi1_poi2_itor != trackingpt_poi1poi2.end())
				{
					getExtendWpsExID2ID(start_calibrate_id, Poi_osm_path_id3, trackingpt_poi1_poi3);
					if (!trackingpt_poi1_poi3.empty())
					{
						auto trackingpt_poi1_poi3_itor = trackingpt_poi1_poi3.begin();
						trackingpt_poi1_poi3_itor++;
						if (trackingpt_poi1_poi3_itor != trackingpt_poi1_poi3.end())
						{
                            std::vector<std::tuple<LAT_LNG, UINT64>> trackingpt_midpoint1_2_poi3;
							UINT64 Poi_osm_path_id_mid_poi1_poi2;
                            std::vector<std::tuple<LAT_LNG, UINT64>>* tracking_ret_ptr = nullptr;

							bool bDealType = true;

							if (std::get<1>(*trackingpt_poi1_poi2_itor) != std::get<1>(*trackingpt_poi1_poi3_itor))
							{

								bDealType = true;
							}
							else
							{
								glm::dvec3 directionVector12 = FunctionAssistant::calculateVector(poilatlng1, poilatlng2);
								glm::dvec3 directionVector23 = FunctionAssistant::calculateVector(poilatlng2, poilatlng3);
								double angle = FunctionAssistant::angle_between_vectors(directionVector12, directionVector23);

								double angledegrees = glm::degrees(angle);
								if (angledegrees > DEAL_ANGLE)
								{
									bDealType = true;
								}
								else
								{
									bDealType = false;
								}
							}

							if (bDealType)
							{
								int trackingpt_poi1_poi2_halfindex = trackingpt_poi1poi2.size() / 2;
								Poi_osm_path_id_mid_poi1_poi2 = std::get<1>(trackingpt_poi1poi2[trackingpt_poi1_poi2_halfindex]);
								dealgenerateTrackingPoints(trackingpt_poi1poi2, trackingpt_poi1_poi2_halfindex, false, true);
								getExtendWpsExID2ID(Poi_osm_path_id_mid_poi1_poi2, Poi_osm_path_id3, trackingpt_midpoint1_2_poi3);
								tracking_ret_ptr = &trackingpt_midpoint1_2_poi3;
							}
							else
							{
								Poi_osm_path_id_mid_poi1_poi2 = start_calibrate_id;
								tracking_ret_ptr = &trackingpt_poi1_poi3;
							}
							generateSubTracking(Poi_osm_path_id_mid_poi1_poi2, tracking_ret_ptr);
							bDealPOI1_2_POI2 = false;
						}
					}
				}
				return bDealPOI1_2_POI2;
			};


            std::vector<std::tuple<LAT_LNG, UINT64>> trackingpt_poi1_poi2;
			getExtendWpsExID2ID(start_calibrate_id, Poi_osm_path_id2, trackingpt_poi1_poi2);
			if (!trackingpt_poi1_poi2.empty())
			{
				auto trackingpt_poi1_poi2_itor = trackingpt_poi1_poi2.begin();
				trackingpt_poi1_poi2_itor++;
				if (trackingpt_poi1_poi2_itor != trackingpt_poi1_poi2.end())
				{
					switch (path_plan.m_flight_dep_arr_type)
					{
					case E_FLIGHT_DEP_ARR_TYPE_DEP:
					{
						bool bDealPOI1_2_POI2 = true;
						if (j < validSize - (EXTEND_INDEX - 1) && j > 0)
						{
							int jnext = j + 2;
							bDealPOI1_2_POI2 = generateTrackingPoints(trackingpt_poi1_poi2, jnext);
						}
						if (bDealPOI1_2_POI2)
						{

#ifdef DEP_END_INDEX_DIRECT
							//求当前点和后二点之间的最小路径
							start_calibrate_id = Poi_osm_path_id2;
							start_calibrate_latlng = Poi_osm_path_latlng2;
							if (j == validSize - (EXTEND_INDEX - 1))
							{
                                //std::tuple<LAT_LNG, UINT64> item;
								//bool bFind = false;
								//int iindex = 0;
								//auto rbitor = trackingpt_poi1_poi2.rbegin();
								//while (rbitor != trackingpt_poi1_poi2.rend())
								//{
								//      if (iindex == 1)
								//      {
								//              item = *rbitor;
								//              bFind = true;
								//              break;
								//      }
								//      iindex++;
								//      rbitor++;
								//}
								trackingpt_poi1_poi2.clear();
								trackingpt_poi1_poi2.push_back(std::make_tuple(start_calibrate_latlng, start_calibrate_id));
								//if (bFind)
								//{
								//      trackingpt_poi1_poi2.push_back(item);
								//}
							}
#else
							getExtendWpsExID2ID(start_calibrate_id, Poi_osm_path_id2, trackingpt_poi1_poi2);
							//求当前点和后二点之间的最小路径
							start_calibrate_id = Poi_osm_path_id2;
							start_calibrate_latlng = Poi_osm_path_latlng2;
#endif
							dealgenerateTrackingPoints(trackingpt_poi1_poi2, trackingpt_poi1_poi2.size(), false, true);
						}
					}
					break;
					case E_FLIGHT_DEP_ARR_TYPE_ARR:
					{
						bool bDealPOI1_2_POI2 = true;
						if (j < validSize - (EXTEND_INDEX))
						{
							int jnext = j + 2;
							//#ifdef ARR_END_INDEX_EXINCLUDE_PBNPOINT
							//							if (j == path_plan.m_pathPoints.size() - EXTEND_INDEX - 1)
							//							{
							//								jnext = j + 1;
							//							}
							//#endif
							bDealPOI1_2_POI2 = generateTrackingPoints(trackingpt_poi1_poi2, jnext);
						}
						if (bDealPOI1_2_POI2)
						{
#ifdef ARR_END_INDEX_EXINCLUDE_PBNPOINT
							if (j >= validSize - EXTEND_INDEX)
							{
								if (j == validSize - EXTEND_INDEX)
								{
									//const QString& poiName2_next = path_plan.m_pathPoints.at(j + 1 + 1);
									const UINT64& Poi_osm_path_id2_next = std::get<0>(path_plan.m_tracking_osm_path_info.at(j + 1 + 1));
                                    const LAT_LNG& poilatlng2_next = path_plan.m_trackinglatlng.at(j + 1 + 1);

									getExtendWpsExID2ID(start_calibrate_id, Poi_osm_path_id2_next, trackingpt_poi1_poi2);
								}
								else if (j == validSize - EXTEND_INDEX + 1)
								{
									trackingpt_poi1_poi2.clear();
								}
							}
#endif
							//求当前点和后二点之间的最小路径
							start_calibrate_id = Poi_osm_path_id2;
							start_calibrate_latlng = Poi_osm_path_latlng2;
							dealgenerateTrackingPoints(trackingpt_poi1_poi2, trackingpt_poi1_poi2.size(), (j >= validSize - EXTEND_INDEX) ? true : false, true);
						}
					}
					break;
					default:
						break;
					}
				}
			}
		}

		if (!extendwpslatlngTmp.empty())
		{
			path_plan.m_extendwpslatlng.reserve(extendwpslatlngTmp.size() + extendwpsEndlatlngTmp.size());
			for (auto item : extendwpslatlngTmp)
			{
                LAT_LNG& latlng = std::get<0>(item);
				UINT64& _id = std::get<1>(item);
				BYTE &iCount = std::get<2>(item);

				if (iCount >= 2)
                {
                    path_plan.m_extendwpslatlng.push_back(latlng);

                    if (path_plan.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
                    {
                        auto _linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
                                                                  m_linestringpoints.end(), [&_id,&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {

                                                                      if(_id ==vt.first && std::get<0>(vt.second) == latlng)
                                                                      {
                                                                          return true;
                                                                      }
                                                                      return false;
                                                                  });
                        if (_linestringpoits_itor != m_linestringpoints.end())
                        {
                            LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
                            auto & ePointType = std::get<1>(_linestringpoits_itor->second);
                            auto & ePointTypedes = std::get<2>(_linestringpoits_itor->second);
                            if((ePointType == E_POINT_TYPE_OSM) || (ePointType == E_POINT_TYPE_OSM_EXTEND && ePointTypedes == "runway"))
                            {
                                path_plan.m_extendwpslatlng_simple.push_back(latlng);
                            }
                        }
                    }
                }
			}
		}
		if (!extendwpsEndlatlngTmp.empty())
		{
			path_plan.m_extendwpslatlng.insert(path_plan.m_extendwpslatlng.end(), extendwpsEndlatlngTmp.begin(), extendwpsEndlatlngTmp.end());


            if (path_plan.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
            {
                for(auto iitor = extendwpsEndlatlngTmp.begin(); iitor != extendwpsEndlatlngTmp.end();iitor++)
                {
                    LAT_LNG& latlng = *iitor;

                    auto _linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
                                                              m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {
                                                                  if(std::get<0>(vt.second) == latlng)
                                                                  {
                                                                      return true;
                                                                  }
                                                                  return false;
                                                              });
                    if (_linestringpoits_itor != m_linestringpoints.end())
                    {
                        LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
                        auto & ePointType = std::get<1>(_linestringpoits_itor->second);
                        auto & ePointTypedes = std::get<2>(_linestringpoits_itor->second);
                        if((ePointType == E_POINT_TYPE_OSM) || (ePointType == E_POINT_TYPE_OSM_EXTEND && ePointTypedes == "runway"))
                        {
                            path_plan.m_extendwpslatlng_simple.push_back(latlng);
                        }
                    }
                }
            }
		}

        if(path_plan.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
        {
            UINT64 pbn_id = -1;
            LAT_LNG _pbn_pt;
            for(int i = 0; i < path_plan.m_pathPoints.size(); i++)
            {
                auto & pbn_name = path_plan.m_pathPoints.at(i).m_pt;
                auto&bvalid = path_plan.m_pathPoints.at(i).bvalid;

                if(bvalid && pbn_name.contains("BN"))
                {
                    auto _poiitemsmap_itor = m_poiitemsmap.find(pbn_name);
                    if(_poiitemsmap_itor != m_poiitemsmap.end())
                    {
                        pbn_id = std::get<0>(_poiitemsmap_itor->second.m_calibrate_osm_path_info);
                        _pbn_pt = std::get<1>(_poiitemsmap_itor->second.m_calibrate_osm_path_info);
                    }
                    break;
                }
            }
            int start_index = 0;
            if( pbn_id != -1)
            {
                float dismin = DIJKSTRA_MAX_VALUE;
                for(int j = 0 ; j < path_plan.m_extendwpslatlng.size(); j++)
                {
                    auto & pt_ = path_plan.m_extendwpslatlng.at(j);
                    double dis = FunctionAssistant::calc_dist(pt_, _pbn_pt);
                    if (dis < dismin)
                    {
                        dismin = dis;
                        start_index = j;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            for(int j = 0 ; j < path_plan.m_extendwpslatlng.size(); j++)
            {

                LAT_LNG& latlng  = path_plan.m_extendwpslatlng.at(j);
                auto _linestringpoits_itor = std::find_if(m_linestringpoints.begin(),
                                                          m_linestringpoints.end(), [&latlng](const std::unordered_map<uint64_t, std::tuple<LAT_LNG, E_POINT_TYPE, QString>>::value_type &vt) {
                                                              if(std::get<0>(vt.second) == latlng)
                                                              {
                                                                  return true;
                                                              }
                                                              return false;
                                                          });
                if (_linestringpoits_itor != m_linestringpoints.end())
                {
                    LAT_LNG& latlng = std::get<0>(_linestringpoits_itor->second);
                    auto & ePointType = std::get<1>(_linestringpoits_itor->second);
                    auto & ePointTypedes = std::get<2>(_linestringpoits_itor->second);
                    if((ePointType == E_POINT_TYPE_OSM) || (ePointType == E_POINT_TYPE_OSM_EXTEND && ePointTypedes == "runway"))
                    {
                        if(j < start_index)
                        {
                            path_plan.m_extendwpslatlng_start_simple.push_back(latlng);
                        }
                        else
                        {
                            path_plan.m_extendwpslatlng_simple.push_back(latlng);
                        }
                    }
                }
            }
        }
	}
	///////////////////////////////////////////////////////////////////////////
}

#define ARR_EXTEND_LEN (4500)
#define DEP_EXTEND_LEN (4500)

#include <iostream>
#include <sstream>
void DataManager::generateRunwayExtendWps(tagPath_Plan* path_plan)
{
	switch (path_plan->m_flight_dep_arr_type)
	{
	case E_FLIGHT_DEP_ARR_TYPE_DEP:
	{
        if ((path_plan->m_pathPoints.at(path_plan->m_pathPoints.size() - 1).bvalid) &&
			!path_plan->m_extendwpslatlng.empty())
		{

		}
		else
		{
			return;
		}
	}
	break;
	case E_FLIGHT_DEP_ARR_TYPE_ARR:
	{
        if ((path_plan->m_pathPoints.at(0).bvalid) &&
			!path_plan->m_extendwpslatlng.empty())
		{

		}
		else
		{
			return;
		}
	}
	break;
	}


	QString runwayTmp = path_plan->m_runway;

	runwayTmp.remove(QRegExp("^0+"));

    auto findnearLatLng = [](const LAT_LNG&src, LAT_LNG&dst, const LATLNGS_VECTOR& latlngs)->int {
		int nearindex = 0;
		float dismin = DIJKSTRA_MAX_VALUE;
		for (int i = 0; i < latlngs.size(); i++)
		{
			double dis = FunctionAssistant::calc_dist(latlngs.at(i), src);
			if (dis < dismin)
			{
				dismin = dis;
				nearindex = i;
				dst = latlngs.at(i);
			}
		}
		return nearindex;
	};


    auto generateExtendSampling = [&](const LAT_LNG& latlng, const LAT_LNG& lstlatlng, LATLNGS_VECTOR &retlist)
	{
        double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
        if (dis > m_point_extend_metres)
        {
            int step = dis / m_point_extend_metres;
			glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
            LAT_LNG lstextendpt = lstlatlng;
			for (int i = 1; i < step + 1; i++)
            {
                LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, m_point_extend_metres);
				retlist.push_back(currentextendpt);

				lstextendpt = currentextendpt;
            }
            double rdis = dis - m_point_extend_metres * step;

			retlist.push_back(latlng);
		}
		else
		{
			retlist.push_back(latlng);
		}
	};

	auto _RunwayInfos_itor = m_RunwayInfos.find(runwayTmp);
	if (_RunwayInfos_itor != m_RunwayInfos.end())
    {
        path_plan->m_runway_total.clear();
        path_plan->m_runway_total_simple.clear();
		path_plan->m_runwayextendwpslatlng.clear();

		LATLNGS_VECTOR _runwayextendwpslatlng_tmp;
		const LATLNGS_VECTOR& runwayptlist = _RunwayInfos_itor.value();
		switch (path_plan->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
            LAT_LNG src = path_plan->m_extendwpslatlng.back();
            LAT_LNG neardst;
			int nearindex = findnearLatLng(src, neardst, runwayptlist);
			int runwayptlist_halfindex = runwayptlist.size() / 2;

			//出港的 查找出港路径的最后一点与该列表中的距离最小的一点,看该点前后的数据量，选择多的一边的作为起飞方向，如果是位于列表后端的话，则倒序添加

			if (nearindex < runwayptlist_halfindex)
			{
                LAT_LNG fardlatlng = runwayptlist.back();
				double neardstdis = FunctionAssistant::calc_dist(fardlatlng, neardst);
				double srcdis = FunctionAssistant::calc_dist(fardlatlng, src);
				if (neardstdis > srcdis)
				{
					nearindex += 2;
				}
				_runwayextendwpslatlng_tmp.insert(_runwayextendwpslatlng_tmp.end(), runwayptlist.begin() + nearindex, runwayptlist.end());
			}
			else
			{
                LAT_LNG fardlatlng = runwayptlist.front();
				double neardstdis = FunctionAssistant::calc_dist(fardlatlng, neardst);
				double srcdis = FunctionAssistant::calc_dist(fardlatlng, src);
				if (neardstdis > srcdis)
				{
					nearindex += 2;
				}
				_runwayextendwpslatlng_tmp.insert(_runwayextendwpslatlng_tmp.end(), runwayptlist.rbegin() + (runwayptlist.size() - nearindex), runwayptlist.rend());
			}

			auto runway_extend1 = FunctionAssistant::generateExtendRunWay(_runwayextendwpslatlng_tmp.front(), _runwayextendwpslatlng_tmp.back(), DEP_EXTEND_LEN);
			_runwayextendwpslatlng_tmp.push_back(runway_extend1);

#if 0
			generateExtendSamplingEx(_runwayextendwpslatlng_tmp.front(), _runwayextendwpslatlng_tmp.back(), path_plan->m_runwayextendwpslatlng);
#else
			path_plan->m_runwayextendwpslatlng.push_back(_runwayextendwpslatlng_tmp.at(0));
			for (int i = 1; i < _runwayextendwpslatlng_tmp.size(); i++)
			{
				generateExtendSampling(_runwayextendwpslatlng_tmp.at(i), _runwayextendwpslatlng_tmp.at(i - 1), path_plan->m_runwayextendwpslatlng);
			}
#endif

			path_plan->m_runway_total.insert(path_plan->m_runway_total.end(), path_plan->m_extendwpslatlng.begin(), path_plan->m_extendwpslatlng.end());
			path_plan->m_runway_total.insert(path_plan->m_runway_total.end(), path_plan->m_runwayextendwpslatlng.begin(), path_plan->m_runwayextendwpslatlng.end());

            ///////////////////////////////////////////////////////////////////////////////////////////////////
            path_plan->m_runway_total_simple.insert(path_plan->m_runway_total_simple.end(), path_plan->m_extendwpslatlng_start_simple.begin(), path_plan->m_extendwpslatlng_start_simple.end());
            path_plan->m_runway_total_simple.insert(path_plan->m_runway_total_simple.end(), path_plan->m_extendwpslatlng_simple.begin(), path_plan->m_extendwpslatlng_simple.end());
            path_plan->m_runway_total_simple.push_back(path_plan->m_runwayextendwpslatlng.front());
            path_plan->m_runway_total_simple.push_back(path_plan->m_runwayextendwpslatlng.back());
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
            LAT_LNG src = path_plan->m_extendwpslatlng.front();
            LAT_LNG neardst;
			int nearindex = findnearLatLng(src, neardst, runwayptlist);
			int runwayptlist_halfindex = runwayptlist.size() / 2;
			//进港的，看该列表首尾两点与进港路径的第一点的距离，距离小的选择为进港方向，如果是位于该列表末尾，则将末尾倒序取置该点，否则从列表头选择置该点，

			if (nearindex < runwayptlist_halfindex)
			{
                LAT_LNG fardlatlng = runwayptlist.front();
				double neardstdis = FunctionAssistant::calc_dist(fardlatlng, neardst);
				double srcdis = FunctionAssistant::calc_dist(fardlatlng, src);
				if (neardstdis > srcdis)
				{
					nearindex -= 2;
				}
				_runwayextendwpslatlng_tmp.insert(_runwayextendwpslatlng_tmp.end(), runwayptlist.begin(), runwayptlist.begin() + nearindex);
			}
			else
			{
                LAT_LNG fardlatlng = runwayptlist.back();
				double neardstdis = FunctionAssistant::calc_dist(fardlatlng, neardst);
				double srcdis = FunctionAssistant::calc_dist(fardlatlng, src);
				if (neardstdis > srcdis)
				{
					nearindex -= 2;
				}
				_runwayextendwpslatlng_tmp.insert(_runwayextendwpslatlng_tmp.end(), runwayptlist.rbegin(), runwayptlist.rbegin() + (runwayptlist.size() - nearindex));
			}

			auto runway_extend1 = FunctionAssistant::generateExtendRunWay(_runwayextendwpslatlng_tmp.back(), _runwayextendwpslatlng_tmp.front(), ARR_EXTEND_LEN);
			LATLNGS_VECTOR _runwayextendwpslatlng_tmp2;
			_runwayextendwpslatlng_tmp2.reserve(_runwayextendwpslatlng_tmp.size() + 1);
			_runwayextendwpslatlng_tmp2.push_back(runway_extend1);
			_runwayextendwpslatlng_tmp2.insert(_runwayextendwpslatlng_tmp2.end(), _runwayextendwpslatlng_tmp.begin(), _runwayextendwpslatlng_tmp.end());

#if 0
			generateExtendSamplingEx(_runwayextendwpslatlng_tmp.front(), _runwayextendwpslatlng_tmp.back(), path_plan->m_runwayextendwpslatlng);
#else
			path_plan->m_runwayextendwpslatlng.push_back(_runwayextendwpslatlng_tmp2.at(0));
			for (int i = 1; i < _runwayextendwpslatlng_tmp2.size(); i++)
			{
				generateExtendSampling(_runwayextendwpslatlng_tmp2.at(i), _runwayextendwpslatlng_tmp2.at(i - 1), path_plan->m_runwayextendwpslatlng);
			}
#endif

			path_plan->m_runway_total.insert(path_plan->m_runway_total.end(), path_plan->m_runwayextendwpslatlng.begin(), path_plan->m_runwayextendwpslatlng.end());
			path_plan->m_runway_total.insert(path_plan->m_runway_total.end(), path_plan->m_extendwpslatlng.begin(), path_plan->m_extendwpslatlng.end());


            ///////////////////////////////////////////////////////////////////////////////////////////////////
            path_plan->m_runway_total_simple.push_back(path_plan->m_runwayextendwpslatlng.front());
            path_plan->m_runway_total_simple.push_back(path_plan->m_runwayextendwpslatlng.back());
            path_plan->m_runway_total_simple.insert(path_plan->m_runway_total_simple.end(), path_plan->m_extendwpslatlng_simple.begin(), path_plan->m_extendwpslatlng_simple.end());
		}break;
		default:
			break;
		}
	}
}

void DataManager::deal_generateExtendWps_callback(tagPath_Plan* path_plan)
{
	if (path_plan)
	{
		generateExtendWps(*path_plan);
		if (path_plan->m_pathPoints.size() > 2)
		{
            if ((path_plan->m_pathPoints.at(0).bvalid) &&
                (path_plan->m_pathPoints.at(path_plan->m_pathPoints.size() - 1).bvalid) &&
				!path_plan->m_extendwpslatlng.empty())
			{
				path_plan->m_bValid = true;
			}
			else
			{
				path_plan->m_bValid = false;
			}
		}
		else
		{
			path_plan->m_bValid = false;
		}
		generateRunwayExtendWps(path_plan);
		m_deal_path_num.store(m_deal_path_num.load() + 1);
		int index = m_deal_path_num.load();
		QString tracestr = QString("%1 deal path process:%2 / %3 %4%").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
			.arg(index).arg(m_total_path_num)
			.arg((double)(index) / (double)(m_total_path_num) * 100);
		qDebug() << tracestr;


        QString dirinfo = m_deal_path + "/path/generatepath";
		QDir dirfileInfo(dirinfo);
		if (!dirfileInfo.exists(dirinfo))
		{
			dirfileInfo.mkpath(dirinfo);
		}
        QString geodirinfo = m_deal_path + "/path/generatepath/geojson";
        QDir geodirfileInfo(geodirinfo);
        if (!geodirfileInfo.exists(geodirinfo))
        {
            geodirfileInfo.mkpath(geodirinfo);
        }
		QString filenamejson;
		if (path_plan->m_extendwpslatlng.empty())
		{
			filenamejson = "invalid_" + QString::number(path_plan->m_extendwpslatlng.size()) + "_" + path_plan->m_parkingpoint + "_" + QString::number(path_plan->m_flight_dep_arr_type) + "_" + path_plan->m_runway + ".json";
		}
		else
		{
            filenamejson = "valid_" + QString::number(path_plan->m_extendwpslatlng.size())+"_"+ QString::number(path_plan->m_extendwpslatlng_simple.size()) + "_" + path_plan->m_parkingpoint + "_" + QString::number(path_plan->m_flight_dep_arr_type) + "_" + path_plan->m_runway + ".json";
            QString filenametempjson = "valid_" + QString::number(path_plan->m_extendwpslatlng.size())+"_"+ QString::number(path_plan->m_extendwpslatlng_simple.size()) + "_" + path_plan->m_parkingpoint + "_" + QString::number(path_plan->m_flight_dep_arr_type) + "_" + path_plan->m_runway + ".geojson";

            if (path_plan->m_extendwpslatlng_simple.empty())
            {
                filenamejson = "valid_simpleInvalid_" + QString::number(path_plan->m_extendwpslatlng.size())+"_"+ QString::number(path_plan->m_extendwpslatlng_simple.size()) + "_" + path_plan->m_parkingpoint + "_" + QString::number(path_plan->m_flight_dep_arr_type) + "_" + path_plan->m_runway + ".json";
                filenametempjson = "valid_simpleInvalid_" + QString::number(path_plan->m_extendwpslatlng.size())+"_"+ QString::number(path_plan->m_extendwpslatlng_simple.size()) + "_" + path_plan->m_parkingpoint + "_" + QString::number(path_plan->m_flight_dep_arr_type) + "_" + path_plan->m_runway + ".geojson";
            }
            QString subpathinfo1 = geodirinfo + "/" + filenametempjson;
			QJsonObject pathinfoarritemgeo = path_plan->outputgeojson();
			FunctionAssistant::write_json_file_object(subpathinfo1, pathinfoarritemgeo);
		}
		QString subpathinfo = dirinfo + "/" + filenamejson;

		QDir subfileInfo(subpathinfo);
		QJsonObject pathinfoarritem = path_plan->toJson();


		FunctionAssistant::write_json_file_object(subpathinfo, pathinfoarritem);
		//qDebug() << "****************save path_plan_extends"<< filenamejson ;

		if (m_deal_path_num.load() == m_total_path_num)
		{
            QString pathinfo = m_deal_path + "/path/PathInfo.json";
			QDir fileInfo(pathinfo);
			if (!fileInfo.exists(pathinfo))
			{
				writePathInfoFile(pathinfo);
				qDebug() << "****************save path_plan_extends";
			}
		}

        if(m_deal_data_process_func_callback)
        {
            m_deal_data_process_func_callback(m_total_path_num, index);
        }
	}
}

void DataManager::generatePath_planExtendWps()
{
	int cc = 0;
	qDebug() << "*************begin path_plan_extends";
	m_deal_path_num.store(0);
	auto pathplans_itor = m_Path_Plans.begin();
	while (pathplans_itor != m_Path_Plans.end())
	{
		const QString& parkingpoint = pathplans_itor->first;
		ARR_DEP_RUNWAY_PATH & arr_dep_runway = pathplans_itor->second;

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

			RUNWAY_PATH & runway_path = arr_dep_runway_itor->second;
			auto runway_path_itor = runway_path.begin();
			while (runway_path_itor != runway_path.end())
			{
				const QString& runway = runway_path_itor->first;
				tagPathPlanInfo & pathplaninfo = runway_path_itor->second;

#if 0
				m_pPathPlanExtendTaskManager->appendProcessor(std::bind(&DataManager::deal_generateExtendWps_callback, this, std::placeholders::_1), &pathplaninfo.pathindex);
				//				if (m_pThreadPool)
				//				{
				//					m_pThreadPool->enqueue(ThreadTaskProcessor<tagPath_Plan *>(0, std::bind(&DataManager::deal_generateExtendWps_callback, this, std::placeholders::_1), &pathplaninfo.pathindex));
				//				}
#else
                if ((pathplaninfo.pathindex.m_parkingpoint == "P260")||
                    (pathplaninfo.pathindex.m_parkingpoint == "P261")||
                    (pathplaninfo.pathindex.m_parkingpoint == "P106"))
                {
                    m_pPathPlanExtendTaskManager->appendProcessor(std::bind(&DataManager::deal_generateExtendWps_callback, this, std::placeholders::_1), &pathplaninfo.pathindex);
//					deal_generateExtendWps_callback(&pathplaninfo.pathindex);
				}
#endif
				cc++;
				runway_path_itor++;
			}
			arr_dep_runway_itor++;
		}
		pathplans_itor++;
	}
	qDebug() << "*************begin path_plan_extends " << cc;
}

void DataManager::decode_stopbar()
{
	QDir dirfileInfo;
    QString dirinfo = m_deal_path + "/analysis";
	if (!dirfileInfo.exists(dirinfo))
	{
		dirfileInfo.mkpath(dirinfo);
	}

    std::function<LAT_LNG(const LATLNGS_VECTOR&)>getCenterPt = [](const LATLNGS_VECTOR& coordinatesExtend) ->LAT_LNG {
        LAT_LNG centerpt;
		double lat_total = 0.0;
		double lng_total = 0.0;
		for (auto subcoordinatesitem : coordinatesExtend)
		{
			lat_total += subcoordinatesitem.lat;
			lng_total += subcoordinatesitem.lng;
		}
		centerpt.lat = lat_total / coordinatesExtend.size();
		centerpt.lng = lng_total / coordinatesExtend.size();
		return centerpt;
	};

    auto findPoint = [&](const LATLNGS_VECTOR& coordinatesExtend, const QString& runwayname, QString& ptname, std::tuple<UINT64, LAT_LNG>& pt, std::tuple<UINT64, LAT_LNG>& pt2, const QString& prefix, bool bLandingpt)->bool
	{
		std::get<0>(pt2) = 0;
		auto poiitor = std::find_if(m_poiitemsmap.begin(),
			m_poiitemsmap.end(), [this, &coordinatesExtend, &prefix, &runwayname, &bLandingpt, &pt2, &getCenterPt](const std::unordered_map<QString, tagPoiItem>::value_type &vt) {
			QString poiname = vt.first;

			QString findprefix = prefix + runwayname;

			if (poiname.startsWith(prefix))
			{
				if ((prefix == "W" && poiname.contains("#") && !bLandingpt) || (prefix == "V" && poiname.contains("#") && !bLandingpt))
				{
					QRegularExpression regex(".*(?=#)");

					// 在字符串中查找匹配的部分
					QRegularExpressionMatch match = regex.match(poiname);

					if (match.hasMatch()) {
						// 提取匹配的部分
						QString matchedText = match.captured(0);

						QString matchedTextTmp = match.captured(0);
						QRegularExpression regexnum("\\d+");


						QRegularExpressionMatch matchnum = regexnum.match(matchedText);

						if (matchnum.hasMatch()) {

							QString numText = matchnum.captured(0);

							numText.remove(QRegExp("^0+"));

							matchedText.replace(matchnum.captured(0), numText);
						}
						if (matchedText == findprefix)
						{
							auto poiitortmp = m_poiitemsmap.find(matchedTextTmp);
							if (poiitortmp != m_poiitemsmap.end())
							{
								pt2 = poiitortmp->second.m_calibrate_osm_path_info;
							}
							return true;
						}
					}

				}
				else
				{
					if (bLandingpt)
					{
						// 提取匹配的部分
						QString matchedText = poiname;
						QRegularExpression regexnum("\\d+");


						QRegularExpressionMatch matchnum = regexnum.match(matchedText);

						if (matchnum.hasMatch()) {

							QString numText = matchnum.captured(0);

							numText.remove(QRegExp("^0+"));

							matchedText.replace(matchnum.captured(0), numText);
						}
						if (matchedText == findprefix)
						{
							if (prefix == "V")
							{
								std::get<0>(pt2) = 1;
								std::get<1>(pt2) = getCenterPt(coordinatesExtend);
							}
							return true;
						}
					}
				}
			}
			return false;
		});
		if (poiitor != m_poiitemsmap.end())
		{
			ptname = poiitor->first;
			pt = poiitor->second.m_calibrate_osm_path_info;
			return true;
		}
		return false;
	};

    auto generateList = [&](const UINT64& excludeIDs, const LAT_LNG& latlng, LATLNGS_VECTOR& latlngs, double len = 5) {
		latlngs.clear();
		UINT64 disminid = 0;
        LAT_LNG disminLatlng;
		E_POINT_TYPE ePointType;
		findDistanceMinPointExclude(disminid, disminLatlng, ePointType, latlng, excludeIDs);

		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(disminLatlng, latlng);
        LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, len);
        LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(latlng, -directionVectorArr, len);

		latlngs.push_back(extendpt1);
		latlngs.push_back(latlng);
		latlngs.push_back(extendpt2);
	};

    auto generategenjson = [&](const QString& dir, const std::tuple<UINT64, LAT_LNG>& pt, const QString& typestr, const QString &ptname, double len = 5) {
		UINT64 disminid1 = 0;
        LAT_LNG disminLatlng1;
		E_POINT_TYPE ePointType1;
		findDistanceMinPoint(disminid1, disminLatlng1, ePointType1, std::get<1>(pt));

        const LAT_LNG& latlng = std::get<1>(pt);
		LATLNGS_VECTOR latlngs;
		generateList(disminid1, latlng, latlngs, len);
		QJsonObject properties;
		QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg(typestr).arg(ptname);
		properties.insert("other_tags", other_tags);

		auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

		QString cc = ptname;
		cc.replace("#", "_");
		QString filenametempjson = QString("%1.geojson").arg(cc);
		QString subpathinfo1 = dir + "/" + typestr + "_" + filenametempjson;
		FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
	};

    auto generategenjson_ex = [&](const LATLNGS_VECTOR& coordinatesExtend, const QString& dir, const std::tuple<UINT64, LAT_LNG>& pt, const QString& typestr, const QString &ptname, double len = 5) {
        LAT_LNG centerpt = getCenterPt(coordinatesExtend);

        const LAT_LNG& latlng = std::get<1>(pt);
		LATLNGS_VECTOR latlngs;

		latlngs.clear();

		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(latlng, centerpt);
		double l = FunctionAssistant::calc_dist(latlng, centerpt);
        LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, HOLDPOINT_EXTEND_METER);
        LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, HOLDPOINT_EXTEND_METER + HOLDPOINT_EXTEND_AREA_METER);

		latlngs.push_back(extendpt1);
		latlngs.push_back(extendpt2);

		QJsonObject properties;
		QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg(typestr).arg(ptname);
		properties.insert("other_tags", other_tags);

		auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

		QString cc = ptname;
		cc.replace("#", "_");
		QString filenametempjson = QString("%1.geojson").arg(cc);
		QString subpathinfo1 = dir + "/" + typestr + "_" + filenametempjson;
		FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
	};



	auto _RunwayInfos_itor = m_RunwayInfos.begin();
	while (_RunwayInfos_itor != m_RunwayInfos.end())
	{
		const QString& runwayname = _RunwayInfos_itor.key();
		const LATLNGS_VECTOR& coordinatesExtend = _RunwayInfos_itor.value();


		QString ptname;
        std::tuple<UINT64, LAT_LNG> pt;
        std::tuple<UINT64, LAT_LNG> pt2;
		if (findPoint(coordinatesExtend, runwayname, ptname, pt, pt2, "W", false))
		{
			double len = 60;
			if (std::get<0>(pt2) != 0)
			{
				len = FunctionAssistant::calc_dist(std::get<1>(pt), std::get<1>(pt2)) + 15;

                const LAT_LNG& latlng = std::get<1>(pt);

				glm::dvec3 directionVectorArr1 = FunctionAssistant::calculateVector(latlng, std::get<1>(pt2));

				UINT64 disminid1 = 0;
                LAT_LNG disminLatlng1;
				E_POINT_TYPE ePointType1;
				findDistanceMinPoint(disminid1, disminLatlng1, ePointType1, latlng);

				UINT64 disminid = 0;
                LAT_LNG disminLatlng;
				E_POINT_TYPE ePointType;
				findDistanceMinPointExclude(disminid, disminLatlng, ePointType, latlng, disminid1);

				glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(disminLatlng, latlng);

				float dot_product = glm::dot(directionVectorArr1, directionVectorArr);

				if (dot_product < 0)
				{
					directionVectorArr = -directionVectorArr;
				}

                LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, len);
                LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(latlng, -directionVectorArr, 30);

				LATLNGS_VECTOR latlngs;
				latlngs.push_back(extendpt1);
				latlngs.push_back(latlng);
				latlngs.push_back(extendpt2);


				QJsonObject properties;
				QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg("stopbar").arg(ptname);
				properties.insert("other_tags", other_tags);

				auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

				QString cc = ptname;
				cc.replace("#", "_");
				QString filenametempjson = QString("%1.geojson").arg(cc);
				QString subpathinfo1 = dirinfo + "/" + "stopbar" + "_" + filenametempjson;
				FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
			}
			else
			{
				generategenjson(dirinfo, pt, "stopbar", ptname, len);
			}
		}

		if (findPoint(coordinatesExtend, runwayname, ptname, pt, pt2, "W", true))
		{
			generategenjson_ex(coordinatesExtend, dirinfo, pt, "holdingpoint", ptname);
		}

		if (findPoint(coordinatesExtend, runwayname, ptname, pt, pt2, "V", true))
		{
			if (std::get<0>(pt2) != 0)
			{
                const LAT_LNG& latlng = std::get<1>(pt);
				glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(latlng, std::get<1>(pt2));

                LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, LANDINGPOINT_METER);
                LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(latlng, -directionVectorArr, LANDINGPOINT_METER * 4);

				LATLNGS_VECTOR latlngs;
				latlngs.push_back(extendpt1);
				latlngs.push_back(latlng);
				latlngs.push_back(extendpt2);


				QJsonObject properties;
				QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg("landingpoint").arg(ptname);
				properties.insert("other_tags", other_tags);

				auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

				QString cc = ptname;
				cc.replace("#", "_");
				QString filenametempjson = QString("%1.geojson").arg(cc);
				QString subpathinfo1 = dirinfo + "/" + "landingpoint" + "_" + filenametempjson;
				FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
			}
			else
			{
				generategenjson(dirinfo, pt, "landingpoint", ptname, LANDINGPOINT_METER);
			}
		}

		if (findPoint(coordinatesExtend, runwayname, ptname, pt, pt2, "V", false))
		{
			generategenjson(dirinfo, pt, "vacaterunway", ptname, 10);
		}
		_RunwayInfos_itor++;
	}
}

void DataManager::decode_transrunwaybar()
{
	std::unordered_map<QString, QString> _transpair;
	auto _poiitemsmap_itor = m_poiitemsmap.begin();
	while (_poiitemsmap_itor != m_poiitemsmap.end())
	{
		const QString& poiname = _poiitemsmap_itor->first;
		if (poiname.contains("TR"))
		{
			QRegularExpression regex(".*(?=TR)");

			// 在字符串中查找匹配的部分
			QRegularExpressionMatch match = regex.match(poiname);

			QString matchedText;
			if (match.hasMatch()) {
				// 提取匹配的部分
				matchedText = match.captured(0);
			}

			auto poiitor = std::find_if(_transpair.begin(),
				_transpair.end(), [&poiname, &matchedText](const std::unordered_map<QString, QString >::value_type &vt) {
				if (vt.first == poiname || (vt.first.startsWith(matchedText) && vt.second.isEmpty()))
				{
					return true;
				}
				return false;
			});
			if (poiitor != _transpair.end())
			{
				if (poiitor->first != poiname && poiitor->first.startsWith(matchedText) && poiitor->second.isEmpty())
				{
					poiitor->second = poiname;
				}
			}
			else
			{
				_transpair.insert(std::make_pair(poiname, QString()));
			}
		}
		_poiitemsmap_itor++;
	}

    QString dirinfo = m_deal_path + "/analysis";
	QDir dirfileInfo(dirinfo);
	if (!dirfileInfo.exists(dirinfo))
	{
		dirfileInfo.mkpath(dirinfo);
	}

    auto generateList = [&](const UINT64& excludeIDs, const LAT_LNG& latlng, LATLNGS_VECTOR& latlngs, double len = 5) {
		latlngs.clear();
		UINT64 disminid = 0;
        LAT_LNG disminLatlng;
		E_POINT_TYPE ePointType;
		findDistanceMinPointExclude(disminid, disminLatlng, ePointType, latlng, excludeIDs);
		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(disminLatlng, latlng);
        LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVectorArr, len);
        LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(latlng, -directionVectorArr, len);

		latlngs.push_back(extendpt1);
		latlngs.push_back(latlng);
		latlngs.push_back(extendpt2);
	};

    auto generategenjson = [&](const QString& dir, const std::tuple<UINT64, LAT_LNG>& pt, const QString& typestr, const QString &ptname, double len = 10) {

		UINT64 disminid1 = 0;
        LAT_LNG disminLatlng1;
		E_POINT_TYPE ePointType1;
		findDistanceMinPoint(disminid1, disminLatlng1, ePointType1, std::get<1>(pt));

        const LAT_LNG& latlng = std::get<1>(pt);
		LATLNGS_VECTOR latlngs;
		generateList(disminid1, latlng, latlngs, len);
		QJsonObject properties;
		QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg(typestr).arg(ptname);
		properties.insert("other_tags", other_tags);

		auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

		QString cc = ptname;
		cc.replace("#", "_");
		QString filenametempjson = QString("%1.geojson").arg(cc);
		QString subpathinfo1 = dir + "/" + typestr + "_" + filenametempjson;
		FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
	};

    auto generategenjsonEx2 = [](const QString& dir, const std::tuple<UINT64, LAT_LNG>& targetpt, const std::tuple<UINT64, LAT_LNG>& refpt, const QString& typestr, const QString &ptname, double len = 10) {
        const LAT_LNG& targetlatlng = std::get<1>(targetpt);
        const LAT_LNG& reflatlng = std::get<1>(refpt);
		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(reflatlng, targetlatlng);
        LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(targetlatlng, directionVectorArr, len);
        LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(targetlatlng, -directionVectorArr, len);

		LATLNGS_VECTOR latlngs;
		latlngs.push_back(extendpt1);
		latlngs.push_back(targetlatlng);
		latlngs.push_back(extendpt2);

		QJsonObject properties;
		QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg(typestr).arg(ptname);
		properties.insert("other_tags", other_tags);

		auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

		QString cc = ptname;
		cc.replace("#", "_");
		QString filenametempjson = QString("%1.geojson").arg(cc);
		QString subpathinfo1 = dir + "/" + typestr + "_" + filenametempjson;
		FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
	};

    auto generategenjsonEx = [&](const QString& dir, const std::tuple<UINT64, LAT_LNG>& targetpt, const std::tuple<UINT64, LAT_LNG>& refpt, const QString& typestr, const QString &ptname, double len = 10) {
        const LAT_LNG& targetlatlng = std::get<1>(targetpt);
        const LAT_LNG& reflatlng = std::get<1>(refpt);
		glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(reflatlng, targetlatlng);
        LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(targetlatlng, directionVectorArr, len);

		LATLNGS_VECTOR latlngs;
		latlngs.push_back(targetlatlng);
		latlngs.push_back(extendpt1);

		QJsonObject properties;
		QString other_tags = QString("\"aeroway\"=>\"%1\",\"ref\"=>\"%2\",\"width\"=>\"20\"").arg(typestr).arg(ptname);
		properties.insert("other_tags", other_tags);

		auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

		QString cc = ptname;
		cc.replace("#", "_");
		QString filenametempjson = QString("%1.geojson").arg(cc);
		QString subpathinfo1 = dir + "/" + typestr + "_" + filenametempjson;
		FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
	};

	auto _transpair_itor = _transpair.begin();
	while (_transpair_itor != _transpair.end())
	{
		const QString& pair1 = _transpair_itor->first;
		const QString& pair2 = _transpair_itor->second;
		if (!pair1.isEmpty() && !pair2.isEmpty())
		{
			auto poiitor = m_poiitemsmap.find(pair1);
			auto poiitor2 = m_poiitemsmap.find(pair2);
			if (poiitor != m_poiitemsmap.end() && poiitor2 != m_poiitemsmap.end())
			{
                std::tuple<UINT64, LAT_LNG> targetpta = std::make_tuple(0, LAT_LNG{ 0,0 });
                std::tuple<UINT64, LAT_LNG> targetptb = std::make_tuple(0, LAT_LNG{ 0,0 });

                std::tuple<UINT64, LAT_LNG>* ptargetpt = nullptr;
				QString startpointname;
				QString startpointname2;
                std::tuple < UINT64, LAT_LNG> startpointinfoa = std::make_tuple(0, LAT_LNG{ 0,0 });
                std::tuple < UINT64, LAT_LNG> startpointinfob = std::make_tuple(0, LAT_LNG{ 0,0 });
				int index = INT_MAX;
				bool bFind = false;
				auto plan_path_itor = m_arrpaths.begin();
				while (plan_path_itor != m_arrpaths.end())
				{
					const tagPath_Plan*pItem = *plan_path_itor;
					if (pItem->m_bValid)
					{
						for (int i = 0; i < pItem->m_pathPoints.size(); i++)
                        {
                            const tagPath_Plan::ptinfo &vt = pItem->m_pathPoints.at(i);
                            if (vt.m_pt == pair1 && (vt.bvalid) && std::get<0>(targetpta) == 0)
							{
                                targetpta = pItem->m_tracking_osm_path_info_calibrate.at((vt.index));
								if (i < index)
								{
									index = i;
									ptargetpt = &targetpta;
									startpointname2 = pair1;
								}
							}
                            if (vt.m_pt == pair2 && (vt.bvalid) && std::get<0>(targetptb) == 0)
							{
                                targetptb = pItem->m_tracking_osm_path_info_calibrate.at((vt.index));
								if (i < index)
								{
									index = i;
									ptargetpt = &targetptb;
									startpointname2 = pair2;
								}
							}

							if (std::get<0>(targetpta) != 0 && std::get<0>(targetptb) != 0)
							{
                                if ((pItem->m_pathPoints.at(1).bvalid))
								{
									for (int j = index - 1; j > 1 && j >= index - 2; j--)
									{
                                        const tagPath_Plan::ptinfo&frontvt = pItem->m_pathPoints.at(j);
                                        const tagPath_Plan::ptinfo&frontvtnext = pItem->m_pathPoints.at(j + 1);
                                        if ((frontvt.bvalid) && (frontvtnext.bvalid))
										{
                                            startpointname = (frontvt.m_pt);
                                            startpointinfoa = pItem->m_tracking_osm_path_info_calibrate.at((frontvt.index));
                                            startpointinfob = pItem->m_tracking_osm_path_info_calibrate.at((frontvtnext.index));
										}
									}
								}
								bFind = true;
								break;
							}
						}
					}
					if (bFind)
					{
						break;
					}
					plan_path_itor++;
				}

				if (std::get<0>(targetpta) != 0 && std::get<0>(targetptb) != 0)
				{
					if (std::get<0>(startpointinfoa) != 0 && std::get<0>(startpointinfob) != 0)
					{
						generategenjsonEx2(dirinfo, startpointinfoa, startpointinfob, "tr_checkbar", startpointname, 30);
						////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						{

							UINT64 disminid1 = 0;
                            LAT_LNG disminLatlng1;
							E_POINT_TYPE ePointType1;
							findDistanceMinPoint(disminid1, disminLatlng1, ePointType1, std::get<1>(startpointinfoa));

							UINT64 disminid2 = 0;
                            LAT_LNG disminLatlng2;
							E_POINT_TYPE ePointType2;
							findDistanceMinPoint(disminid2, disminLatlng2, ePointType2, std::get<1>(*ptargetpt));

							LATLNGS_VECTOR latlngs;
							auto pathInfo = getPath(disminid1, disminid2);

							if (!pathInfo.m_path.empty())
							{
								latlngs.reserve(pathInfo.m_path.size());
								for (int j = 0; j < pathInfo.m_path.size(); j++)
								{
									latlngs.push_back(pathInfo.m_path.at(j).m_pos);
								}
							}
							QString ptname = startpointname + "_" + startpointname2;

							QJsonObject properties;
							QString other_tags = QString("\"aeroway\"=>\"tr_checkbar_stopbar\",\"ref\"=>\"%1\",\"width\"=>\"20\"").arg(ptname);
							properties.insert("other_tags", other_tags);

							auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

							ptname.replace("#", "_");
							QString filenametempjson = QString("%1.geojson").arg(ptname);
							QString subpathinfo1 = dirinfo + "/" + "tr_checkbar_stopbar" + "_" + filenametempjson;
							FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
						}
						////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					}
					generategenjsonEx(dirinfo, targetpta, targetptb, "tr_stopbar", poiitor->first, 40);
					generategenjsonEx(dirinfo, targetptb, targetpta, "tr_stopbar", poiitor2->first, 40);
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					{

						UINT64 disminid1 = 0;
                        LAT_LNG disminLatlng1;
						E_POINT_TYPE ePointType1;
						findDistanceMinPoint(disminid1, disminLatlng1, ePointType1, std::get<1>(targetpta));


						UINT64 disminid2 = 0;
                        LAT_LNG disminLatlng2;
						E_POINT_TYPE ePointType2;
						findDistanceMinPoint(disminid2, disminLatlng2, ePointType2, std::get<1>(targetptb));

						LATLNGS_VECTOR latlngs;
						auto pathInfo = getPath(disminid1, disminid2);

						if (!pathInfo.m_path.empty())
						{
							latlngs.reserve(pathInfo.m_path.size());
							for (int j = 0; j < pathInfo.m_path.size(); j++)
							{
								latlngs.push_back(pathInfo.m_path.at(j).m_pos);
							}
						}
						QString ptname;
						QRegularExpression regex(".*(?=TR)");
						// 在字符串中查找匹配的部分
						QRegularExpressionMatch match = regex.match(pair1);

						QString matchedText;
						if (match.hasMatch()) {
							// 提取匹配的部分
							matchedText = match.captured(0);
						}
						ptname = QString("%1TR").arg(matchedText);

						QJsonObject properties;
						QString other_tags = QString("\"aeroway\"=>\"transrunway\",\"ref\"=>\"%1\",\"width\"=>\"20\"").arg(ptname);
						properties.insert("other_tags", other_tags);

						auto linegeojson = FunctionAssistant::generateLineGeoJson(properties, latlngs);

						QString cc = ptname;
						cc.replace("#", "_");
						QString filenametempjson = QString("%1.geojson").arg(cc);
						QString subpathinfo1 = dirinfo + "/" + "transrunway" + "_" + filenametempjson;
						FunctionAssistant::write_json_file_object(subpathinfo1, linegeojson);
					}
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				}
			}
		}
        _transpair_itor++;
    }
}

void DataManager::set_deal_data_process_func_callback(deal_data_process_func_callback newDeal_data_process_func_callback)
{
    m_deal_data_process_func_callback = std::move(newDeal_data_process_func_callback);
}



void DataManager::scheduleWpsprepare(const QString& agentId, FlightPlanConf *pflighltData)
{
	//分机实体分配航班号信息
	std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> > &planenumtoflightnum = DataManager::getInstance().PlaneNumToFlightNum();

	PathPlanValidInfo pathplanvalidinfo;
	tagPath_Plan* _ptagPath_Plan = nullptr;

	auto updateplanenumtoflightnum = [&planenumtoflightnum, &agentId, &pflighltData, &_ptagPath_Plan, &pathplanvalidinfo]() {
		auto planenumtoflightnum_tor = std::find_if(planenumtoflightnum.begin(),
			planenumtoflightnum.end(), [&](const std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> >::value_type &vt) {
			return vt.first == agentId;
		});
		if (planenumtoflightnum_tor != planenumtoflightnum.end())
		{
			std::unordered_map<QString, FLIGHTPLAN_PATH_INFO>& flightInfo = planenumtoflightnum_tor->second;
			auto flightInfo_itor = flightInfo.find(pflighltData->m_FilghtNumber);
			if (flightInfo_itor != flightInfo.end())
			{
				flightInfo_itor->second = std::make_tuple(_ptagPath_Plan, pflighltData, std::move(pathplanvalidinfo));
			}
			else
			{
				flightInfo.insert(std::make_pair(pflighltData->m_FilghtNumber, std::make_tuple(_ptagPath_Plan, pflighltData, std::move(pathplanvalidinfo))));
			}
		}
		else
		{
			std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> flightInfo;
			flightInfo.insert(std::make_pair(pflighltData->m_FilghtNumber, std::make_tuple(_ptagPath_Plan, pflighltData, std::move(pathplanvalidinfo))));
			planenumtoflightnum.insert(std::make_pair(agentId, std::move(flightInfo)));
		}
	};

	QString runwaymatchedText = pflighltData->m_Runway;
	QRegularExpression regexnum("\\d+");
	QRegularExpressionMatch matchnum = regexnum.match(runwaymatchedText);

	if (matchnum.hasMatch()) {

		QString numText = matchnum.captured(0);

		numText.remove(QRegExp("^0+"));

		runwaymatchedText.replace(matchnum.captured(0), numText);
	}

	QString _parkingpoint = pflighltData->m_Seat;

	if (!_parkingpoint.startsWith("P") && !_parkingpoint.isEmpty())
	{
		_parkingpoint = "P" + pflighltData->m_Seat;
	}
	pathplanvalidinfo.target_parkingpoint = _parkingpoint;
	pathplanvalidinfo.target_runway = runwaymatchedText;
	//运行使用的跑道
	/////////////////////////////////////////////////////////////////////////////
	//runway schedule
	QStringList allowRunway;
	allowRunway << "2L";
	allowRunway << "2R";
	allowRunway << "1";

	//allowRunway << "20L";
	//allowRunway << "20R";
	//allowRunway << "19";
	/////////////////////////////////////////////////////////////////////////////

	//分析航班路径的可用性
	auto getTargetWps = [&](PathPlanValidInfo& pathplanvalidinfo){

		QList<tagPath_Plan*> pathplans;
		QVector<int> _validlist;
		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			pathplans = DataManager::getInstance().getArrpaths();
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			pathplans = DataManager::getInstance().getDeppaths();
			allowRunway.removeAll("2R");
		}
		break;
		}

		pathplanvalidinfo.alloc_parkingpoint = "";
		pathplanvalidinfo.alloc_runway = "";
		if (_parkingpoint.isEmpty())
		{
			//该航班停机坪无效
			pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);

			//检查该航班使用的跑道是否有效
			if (allowRunway.contains(runwaymatchedText))
			{
				bool bRunwayRet = true;
				//对于进港航班，检查此航班使用时间段的跑道是否能使用，能使用的则将此刻的跑道使用区间分配给该进港航班
				//对于出港航班，只要是 允许使用的跑道 无需检查起使用跑道的时间段
				if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
				{
					//检查跑道是否使用冲突
					bRunwayRet = checkRunwayUsingTimeConflict(pflighltData, runwaymatchedText);
				}

				if (!bRunwayRet)
				{
					pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
				}
				else
				{
					//分配该航班在此时间段内使用该停跑道
					pathplanvalidinfo.alloc_runway = runwaymatchedText;
					pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_RUNWAY);
				}
			}
			else
			{
				//该航班使用的跑道无效
				pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
			}
		}
		else
		{
			if (allowRunway.contains(runwaymatchedText))
			{
				//按航班计划时刻表分配的时候 必须满足航班路径有效 / 停机位相同 / 跑道相同
				bool bExist = false;
				auto itor = pathplans.begin();
				while (itor != pathplans.end())
				{
					tagPath_Plan*& vt = *itor;
					if (vt &&
						vt->m_bValid &&
						vt->m_parkingpoint == _parkingpoint &&
						vt->m_runway == runwaymatchedText)
					{
						bExist = true;
						break;
					}
					itor++;
				}
				//存在该航班的使用停机坪 跑道 匹配的路径
				if (bExist)
				{
					bool bParkingPointRet = checkParkingPointUsingTimeConflict(pflighltData, _parkingpoint);
					//按航班计划时刻表分配的时候 进出港航班 停机位 存在冲突
					if (!bParkingPointRet)
					{
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
					}
					else
					{
						//分配该航班在此时间段内使用该停机坪
						pathplanvalidinfo.alloc_parkingpoint = _parkingpoint;
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_PARKINGPOINT);
					}
					bool bRunwayRet = true;
					//对于进港航班，检查此航班使用时间段的跑道是否能使用，能使用的则将此刻的跑道使用区间分配给该进港航班
					//对于出港航班，只要是 允许使用的跑道 无需检查起使用跑道的时间段
					if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
					{
						//检查跑道是否使用冲突
						bRunwayRet = checkRunwayUsingTimeConflict(pflighltData, runwaymatchedText);
					}

					if (!bRunwayRet)
					{
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
					}
					else
					{
						//分配该航班在此时间段内使用该跑道
						pathplanvalidinfo.alloc_runway = runwaymatchedText;
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_RUNWAY);
					}
				}
				else
				{
					//查找停机位匹配的路径
					bool bParkingExist = false;
					auto itor = pathplans.begin();
					while (itor != pathplans.end())
					{
						tagPath_Plan*& vt = *itor;
						if (vt &&
							vt->m_bValid &&
							vt->m_parkingpoint == _parkingpoint)
						{
							bParkingExist = true;
							break;
						}
						itor++;
					}
					//查找到停机位匹配的路径，说明runway是无效的
					if (bParkingExist)
					{
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
						bool bParkingPointRet = checkParkingPointUsingTimeConflict(pflighltData, _parkingpoint);
						//按航班计划时刻表分配的时候 停机位 存在冲突
						if (!bParkingPointRet)
						{
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
						}
						else
						{
							//分配该航班在此时间段内使用该停机坪
							pathplanvalidinfo.alloc_parkingpoint = _parkingpoint;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_PARKINGPOINT);
						}

						//pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
						//pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
					}
					else
					{
						//未查找到停机位匹配的路径，说明parkingpoint是无效的
						//该航班停机坪无效
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
						//检查该航班使用的跑道是否有效
						bool bRunwayRet = true;
						//对于进港航班，检查此航班使用时间段的跑道是否能使用，能使用的则将此刻的跑道使用区间分配给该进港航班
						//对于出港航班，只要是 允许使用的跑道 无需检查起使用跑道的时间段
						if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
						{
							//检查跑道是否使用冲突
							bRunwayRet = checkRunwayUsingTimeConflict(pflighltData, runwaymatchedText);
						}

						if (!bRunwayRet)
						{
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
						}
						else
						{
							//分配该航班在此时间段内使用该跑道
							pathplanvalidinfo.alloc_runway = runwaymatchedText;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_RUNWAY);
						}
					}
				}
			}
			else
			{
				//该航班使用的跑道是不被允许的，该航班使用的跑道直接无效
				pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY);
				//对于进出港航班，当进港的跑道无效时，检查此刻停机坪是否能使用，能使用的则将此刻的停机坪使用区间分配给该进港航班
				bool bExist = false;
				auto itor = pathplans.begin();
				while (itor != pathplans.end())
				{
					tagPath_Plan*& vt = *itor;
					if (vt &&
						vt->m_bValid &&
						vt->m_parkingpoint == _parkingpoint)
					{
						bExist = true;
						break;
					}
					itor++;
				}
				//查找该航班使用的停机位匹配的路径，检查停机坪是否存在使用时间段的冲突
				if (bExist)
				{
					bool bParkingPointRet = checkParkingPointUsingTimeConflict(pflighltData, _parkingpoint);
					//按航班计划时刻表分配的时候 停机位 存在冲突
					if (!bParkingPointRet)
					{
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
					}
					else
					{
						//分配该航班在此时间段内使用该停机坪
						pathplanvalidinfo.alloc_runway = _parkingpoint;
						pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_PLAN_PARKINGPOINT);
					}
				}
				else
				{
					//未查找该航班使用的停机位匹配的路径，停机坪无效
					pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT);
				}
			}
		}
	};

	//获取该航班在运行时间段 跑道 / 停机位 是否与其他航班存在使用时间段的冲突
	getTargetWps(pathplanvalidinfo);
	updateplanenumtoflightnum();
}

QJsonArray DataManager::getScheduleWps(const QString& agentId, FlightPlanConf *pflighltData)
{
	QJsonArray wps;

	auto getRunwayConflicInfo = [&](const QString &_runway)->QString {
		QString ConflictInfo;
		if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
		{
			uint64_t landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp /*- 3*60*/;
			uint64_t landingtimestamp_end = landingtimestamp_begin + 3 * 60;
			auto usingpoint_itor = m_runwayuseinfo.find(_runway);
			if (usingpoint_itor != m_runwayuseinfo.end())
			{
				std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

				auto itr = std::find_if(usinglist.begin(),
					usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
					const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
					const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);
					//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
					if ((landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp) ||
						(landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp))
					{
						return true;
					}
					return false;
				});
				if (itr != usinglist.end())
				{
					FlightPlanConf * recordFlightPlanConf = std::get<2>(*itr);
					ConflictInfo = " Runway Conflict with " + recordFlightPlanConf->m_FilghtNumber + " " + recordFlightPlanConf->m_DepArrFlag + " " + recordFlightPlanConf->m_PlanDateTimeLanding+" "+ recordFlightPlanConf->m_Runway;
				}
			}
		}
		return ConflictInfo;
	};

	auto getParkingpointConflicInfo = [&](const QString &allocparkingpoint)->QString {
		
		uint64_t landingtimestamp_begin = 0;
		uint64_t landingtimestamp_end = 0;
		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp;
			landingtimestamp_end = pflighltData->m_PlanDateTimeLanding_behind_timestamp;
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			landingtimestamp_begin = pflighltData->m_PlanDateTimeTakeOff_ahead_timestamp;
			landingtimestamp_end = pflighltData->m_PlanDateTimeTakeOff_timestamp;
		}
		break;
		}

		QString ConflictInfo;
		auto usingpoint_itor = m_parkingpointuseinfo.find(allocparkingpoint);
		if (usingpoint_itor != m_parkingpointuseinfo.end())
		{
			std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

			auto itr = std::find_if(usinglist.begin(),
				usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
				const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
				const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);
				//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
				if ((landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp) ||
					(landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp))
				{
					return true;
				}
				return false;
			});
			if (itr != usinglist.end())
			{
				FlightPlanConf * recordFlightPlanConf = std::get<2>(*itr);
				switch (recordFlightPlanConf->m_flight_dep_arr_type)
				{
				case E_FLIGHT_DEP_ARR_TYPE_ARR:
				{
					ConflictInfo = " Parkingpoint Conflict with " + recordFlightPlanConf->m_FilghtNumber + " " + recordFlightPlanConf->m_DepArrFlag + " " + recordFlightPlanConf->m_PlanDateTimeLanding;
				}break;
				case E_FLIGHT_DEP_ARR_TYPE_DEP:
				{
					ConflictInfo = " Parkingpoint Conflict with " + recordFlightPlanConf->m_FilghtNumber + " " + recordFlightPlanConf->m_DepArrFlag + " " + recordFlightPlanConf->m_PlanDateTimeTakeOff;
				}
				break;
				}
			}
		}
		return ConflictInfo;
	};

	auto addwps = [&wps](double lng, double lat, double alt, double alttype, double timestamp) {
		QJsonArray wpsitem;
		wpsitem.push_back(lng);
		wpsitem.push_back(lat);
		wpsitem.push_back(alt);
		wpsitem.push_back(alttype);
		wpsitem.push_back(timestamp);
		wps.push_back(wpsitem);
	};

	auto genrateWps = [&](tagPath_Plan*& _ptagPath_Plan) {
		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng.size();
            LAT_LNG firstpt = _ptagPath_Plan->m_extendwpslatlng.front();
			int exsize = _ptagPath_Plan->m_runwayextendwpslatlng.size();

			int countindex = 0;
			for (int cc = 0; cc < wpsexsize; cc++)
			{
                LAT_LNG curpt = _ptagPath_Plan->m_extendwpslatlng.at(cc);
				double xx = FunctionAssistant::calc_dist(curpt, firstpt);
				if (xx > LANDINGPOINT_METER)
				{
					countindex = cc;
					break;
				}
			}
			wpsexsize -= countindex;
			exsize += countindex;

			double arrhightstep = TARGET_HEIGHT_METER / exsize;
			for (int i = 0; i < _ptagPath_Plan->m_runway_total.size(); i++)
			{
				double alt = TARGET_HEIGHT_METER - arrhightstep * i;
				alt = alt < 0 ? 0 : alt;
				addwps(_ptagPath_Plan->m_runway_total.at(i).lng, _ptagPath_Plan->m_runway_total.at(i).lat, alt, 1, i * 10);
			}
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng.size();
            LAT_LNG lastpt = _ptagPath_Plan->m_extendwpslatlng.back();
			int exsize = _ptagPath_Plan->m_runwayextendwpslatlng.size();
			int countindex = 0;
			for (int cc = 0; cc < exsize; cc++)
			{
                LAT_LNG curpt = _ptagPath_Plan->m_runwayextendwpslatlng.at(cc);
				double xx = FunctionAssistant::calc_dist(curpt, lastpt);
				if (xx > TAKEOFF_METER)
				{
					countindex = cc;
					break;
				}
			}
			wpsexsize += countindex;
			exsize -= countindex;

			double arrhightstep = TARGET_HEIGHT_METER / exsize;
			for (int i = 0; i < _ptagPath_Plan->m_runway_total.size(); i++)
			{
				double alt = arrhightstep * (i - wpsexsize);
				alt = alt < 0 ? 0 : alt;
				addwps(_ptagPath_Plan->m_runway_total.at(i).lng, _ptagPath_Plan->m_runway_total.at(i).lat, alt, 1, i * 10);
			}
		}
		break;
		}
	};


	//运行使用的跑道
	/////////////////////////////////////////////////////////////////////////////
	//runway schedule
	QStringList allowRunway;
	allowRunway << "2L";
	allowRunway << "2R";
	allowRunway << "1";

	//allowRunway << "20L";
	//allowRunway << "20R";
	//allowRunway << "19";
	/////////////////////////////////////////////////////////////////////////////


	QList<tagPath_Plan*> pathplans;
	switch (pflighltData->m_flight_dep_arr_type)
	{
	case E_FLIGHT_DEP_ARR_TYPE_ARR:
	{
		pathplans = DataManager::getInstance().getArrpaths();
	}break;
	case E_FLIGHT_DEP_ARR_TYPE_DEP:
	{
		pathplans = DataManager::getInstance().getDeppaths();
		allowRunway.removeAll("2R");
	}
	break;
	}

	//分机实体分配航班号信息
	std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> > &planenumtoflightnum = DataManager::getInstance().PlaneNumToFlightNum();


	auto planenumtoflightnum_tor = std::find_if(planenumtoflightnum.begin(),
		planenumtoflightnum.end(), [&](const std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> >::value_type &vt) {
		return vt.first == agentId;
	});
	if (planenumtoflightnum_tor != planenumtoflightnum.end())
	{
		std::unordered_map<QString, FLIGHTPLAN_PATH_INFO>& flightInfo = planenumtoflightnum_tor->second;
		auto flightInfo_itor = flightInfo.find(pflighltData->m_FilghtNumber);
		if (flightInfo_itor != flightInfo.end())
		{
			tagPath_Plan*& _ptagPath_Plan = std::get<0>(flightInfo_itor->second);
			PathPlanValidInfo &pathplanvalidinfo = std::get<2>(flightInfo_itor->second);

			bool bRet = true;
			if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_PLAN_PARKINGPOINT &&
				pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_PLAN_RUNWAY)
			{
				//停机位可用 跑道可用
				auto itor = pathplans.begin();
				while (itor != pathplans.end())
				{
					tagPath_Plan*& vt = *itor;
					if (vt &&
						vt->m_bValid &&
						vt->m_parkingpoint == pathplanvalidinfo.alloc_parkingpoint &&
						vt->m_runway == pathplanvalidinfo.alloc_runway)
					{
						_ptagPath_Plan = vt;
						break;
					}
					itor++;
				}
				if (_ptagPath_Plan)
				{
					bRet = true;
				}
				else 
				{
					bRet = false;
				}
			}
			else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_PLAN_PARKINGPOINT &&
				pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY)
			{
				QVector<int> _validlist;
				//停机位可用 跑道不可用
				//获取到对应时间段的跑道冲突列表
				std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>> runwayconflicts = getRunwayUsingTimeConflict(pflighltData);
				for (int i = 0; i < pathplans.size(); i++)
				{
					tagPath_Plan* ptagPath_Plan = pathplans.at(i);
					if (ptagPath_Plan->m_bValid &&
						ptagPath_Plan->m_parkingpoint == pathplanvalidinfo.alloc_parkingpoint &&
						allowRunway.contains(ptagPath_Plan->m_runway))
					{
						//过滤出路径的停机坪匹配 且 路径使用的跑道是允许使用
						if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
						{
							if (runwayconflicts.find(ptagPath_Plan->m_runway) == runwayconflicts.end())
							{
								//进港 当前时间段的跑道冲突中未包含路径使用跑道的项
								_validlist.push_back(i);
							}
						}
						else
						{
							_validlist.push_back(i);
						}
					}
				}

				bool bRunwayRet = false;
				if (!_validlist.empty())
				{
					for (int index = 0; index < _validlist.size(); index++)
					{
						int iselindex = _validlist.at(index);
						_ptagPath_Plan = pathplans.at(iselindex);

						if (_ptagPath_Plan)
						{
							if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
							{
								//检查跑道是否使用冲突
								bRunwayRet = checkRunwayUsingTimeConflict(pflighltData, _ptagPath_Plan->m_runway);
							}
							else
							{
								bRunwayRet = true;
							}

							if (!bRunwayRet)
							{
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY);
							}
							else
							{
								pathplanvalidinfo.alloc_runway = _ptagPath_Plan->m_runway;
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_RUNWAY);
							}
						}
						else
						{
							bRunwayRet = false;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY);
						}

						if (bRunwayRet)
						{
							break;
						}
					}
				}
				bRet = bRunwayRet;
			}
			else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT &&
				pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_PLAN_RUNWAY)
			{

				QVector<int> _validlist;
				//停机位不可用 跑道可用
				//获取到对应时间段的停机坪冲突列表
				std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>> parkingpointconflicts = getParkingPointUsingTimeConflict(pflighltData);
				for (int i = 0; i < pathplans.size(); i++)
				{
					tagPath_Plan* ptagPath_Plan = pathplans.at(i);
					if (ptagPath_Plan->m_bValid &&
						ptagPath_Plan->m_runway == pathplanvalidinfo.alloc_runway &&
						allowRunway.contains(ptagPath_Plan->m_runway) &&
						parkingpointconflicts.find(ptagPath_Plan->m_parkingpoint) == parkingpointconflicts.end())
					{
						//过滤出路径的跑道匹配 且 路径使用的跑道是允许使用 当前时间段的停机坪冲突中未包含路径使用停机坪的项
						_validlist.push_back(i);
					}
				}
				
				bool bParkingPointRet = false;
				if (!_validlist.empty())
				{
					for (int index = 0; index < _validlist.size(); index++)
					{
						int iselindex = _validlist.at(index);
						_ptagPath_Plan = pathplans.at(iselindex);

						if (_ptagPath_Plan)
						{
							//检查停机坪是否使用冲突
							bParkingPointRet = checkParkingPointUsingTimeConflict(pflighltData, _ptagPath_Plan->m_parkingpoint);

							if (!bParkingPointRet)
							{
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT);
							}
							else
							{
								pathplanvalidinfo.alloc_parkingpoint = _ptagPath_Plan->m_parkingpoint;
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_PARKINGPOINT);
							}
						}
						else
						{
							bParkingPointRet = false;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT);
						}
						if (bParkingPointRet)
						{
							break;
						}
					}
				}
				bRet = bParkingPointRet;
			}
			else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_CONFLICT_PARKINGPOINT &&
				pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_CONFLICT_RUNWAY)
			{
				QVector<int> _validlist;
				//停机位不可用 跑道不可用
				//获取到对应时间段的跑道冲突列表
				std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>> runwayconflicts = getRunwayUsingTimeConflict(pflighltData);

				//停机位不可用 跑道可用
				//获取到对应时间段的停机坪冲突列表
				std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>> parkingpointconflicts = getParkingPointUsingTimeConflict(pflighltData);

				for (int i = 0; i < pathplans.size(); i++)
				{
					tagPath_Plan* ptagPath_Plan = pathplans.at(i);
					if (ptagPath_Plan->m_bValid &&
						allowRunway.contains(ptagPath_Plan->m_runway) &&
						parkingpointconflicts.find(ptagPath_Plan->m_parkingpoint) == parkingpointconflicts.end())
					{
						//过滤出路径的跑道匹配 且 路径使用的跑道是允许使用 当前时间段的停机坪冲突中未包含路径使用停机坪的项
						if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
						{
							if (runwayconflicts.find(ptagPath_Plan->m_runway) == runwayconflicts.end())
							{
								//进港 当前时间段的跑道冲突中未包含路径使用跑道的项
								_validlist.push_back(i);
							}
						}
						else 
						{
							_validlist.push_back(i);
						}
					}
				}

				bool bRunwayRet = false;
				bool bParkingPointRet = false;
				if (!_validlist.empty())
				{
					for (int index = 0; index < _validlist.size(); index++)
					{
						int iselindex = _validlist.at(index);
						_ptagPath_Plan = pathplans.at(iselindex);
						if (_ptagPath_Plan)
						{
							if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
							{
								//检查跑道是否使用冲突
								bRunwayRet = checkParkingPointUsingTimeConflict(pflighltData, _ptagPath_Plan->m_runway);
							}
							else
							{
								bRunwayRet = true;
							}

							if (!bRunwayRet)
							{
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY);
							}
							else
							{
								pathplanvalidinfo.alloc_runway = _ptagPath_Plan->m_runway;
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_RUNWAY);
							}

							//检查停机坪是否使用冲突
							bParkingPointRet = checkParkingPointUsingTimeConflict(pflighltData, _ptagPath_Plan->m_parkingpoint);

							if (!bParkingPointRet)
							{
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT);
							}
							else
							{
								pathplanvalidinfo.alloc_parkingpoint = _ptagPath_Plan->m_parkingpoint;
								pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_PARKINGPOINT);
							}
						}
						else
						{
							bRunwayRet = false;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY);
							bParkingPointRet = false;
							pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT);
						}
						if (bRunwayRet && bParkingPointRet)
						{
							break;
						}
					}
				}
				else 
				{
					pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY);
					pathplanvalidinfo.eStatus = (PathPlanValidInfo::E_PATH_TYPE) (pathplanvalidinfo.eStatus | PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT);
				}
				bRet = (bParkingPointRet && bRunwayRet) ? true:false;
			}
		
			if (bRet && _ptagPath_Plan)
			{
				genrateWps(_ptagPath_Plan);
				//if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_PARKINGPOINT && pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_RUNWAY)
				//{
				//	qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << " *************** wps realloc traget_parkingpoint:" << pathplanvalidinfo.target_parkingpoint << " traget_runway " << pathplanvalidinfo.target_runway << " alloc pk " << pathplanvalidinfo.alloc_parkingpoint << " alloc runway " << pathplanvalidinfo.alloc_runway << " " << getParkingpointConflicInfo(pathplanvalidinfo.target_parkingpoint) << " " << getRunwayConflicInfo(pathplanvalidinfo.target_runway);
				//}
				//else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_PARKINGPOINT)
				//{
				//	qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << " *************** wps realloc traget_parkingpoint:" << pathplanvalidinfo.target_parkingpoint << " alloc pk " << pathplanvalidinfo.alloc_parkingpoint << " " << getParkingpointConflicInfo(pathplanvalidinfo.target_parkingpoint);
				//}
				//else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_RUNWAY)
				//{
				//	qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << pflighltData->m_PlanDateTimeLanding << " *************** wps realloc traget_runway " << pathplanvalidinfo.target_runway << " alloc runway " << pathplanvalidinfo.alloc_runway <<" "<< getRunwayConflicInfo(pathplanvalidinfo.target_runway);
				//}
			}
			else
			{
				if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT && 
					pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY)
				{
					qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << pflighltData->m_PlanDateTimeLanding << "-------------------------- wps realloc runway parkingpoint failed------ parkingpoint:" << pathplanvalidinfo.target_parkingpoint << " runway " << pathplanvalidinfo.target_runway << " alloc pk " << pathplanvalidinfo.alloc_parkingpoint << "alloc runway" << pathplanvalidinfo.alloc_runway << getParkingpointConflicInfo(pathplanvalidinfo.target_parkingpoint) << " " << getRunwayConflicInfo(pathplanvalidinfo.target_runway);
				}
				else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_PARKINGPOINT)
				{
					qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << pflighltData->m_PlanDateTimeLanding << "-------------------------- wps realloc parkingpoint failed------ parkingpoint:" << pathplanvalidinfo.target_parkingpoint << " runway " << pathplanvalidinfo.target_runway << " alloc pk " << pathplanvalidinfo.alloc_parkingpoint << "alloc runway" << pathplanvalidinfo.alloc_runway << getParkingpointConflicInfo(pathplanvalidinfo.target_parkingpoint) << " " << getRunwayConflicInfo(pathplanvalidinfo.target_runway);
				}
				else if (pathplanvalidinfo.eStatus & PathPlanValidInfo::E_PATH_TYPE_REALLOC_CONFLICT_RUNWAY)
				{
					qDebug() << pflighltData->m_FilghtNumber << pflighltData->m_DepArrFlag << pflighltData->m_PlanDateTimeLanding << "-------------------------- wps realloc runway failed------ parkingpoint:" << pathplanvalidinfo.target_parkingpoint << " runway " << pathplanvalidinfo.target_runway <<" alloc pk "<< pathplanvalidinfo.alloc_parkingpoint<<"alloc runway"<< pathplanvalidinfo.alloc_runway<< getRunwayConflicInfo(pathplanvalidinfo.target_runway);;
				}
			}
		}
	}
	return wps;
}

bool DataManager::checkParkingPointUsingTimeConflict(FlightPlanConf *pflighltData, const QString&_parkingpoint)
{
	bool bValid = false;
	uint64_t landingtimestamp_begin = 0;
	uint64_t landingtimestamp_end = 0;
	switch (pflighltData->m_flight_dep_arr_type)
	{
	case E_FLIGHT_DEP_ARR_TYPE_ARR:
	{
		landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp;
		landingtimestamp_end = pflighltData->m_PlanDateTimeLanding_behind_timestamp;
	}break;
	case E_FLIGHT_DEP_ARR_TYPE_DEP:
	{
		landingtimestamp_begin = pflighltData->m_PlanDateTimeTakeOff_ahead_timestamp;
		landingtimestamp_end = pflighltData->m_PlanDateTimeTakeOff_timestamp;
	}
	break;
	}

	bValid = true;
	auto usingpoint_itor = m_parkingpointuseinfo.find(_parkingpoint);
	if (usingpoint_itor != m_parkingpointuseinfo.end())
	{
		std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

		auto itr = std::find_if(usinglist.begin(),
			usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
			const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
			const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);

			//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
			if ((landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp) ||
				(landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp))
			{
				return true;
			}
			return false;
		});
		if (itr != usinglist.end())
		{
			//ptagPath_Plan = nullptr;
			bValid = false;
		}
		else
		{
			usinglist.push_back(std::make_tuple(landingtimestamp_begin, landingtimestamp_end, pflighltData));
		}
	}
	else
	{
		std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>> usinglist;
		usinglist.push_back(std::make_tuple(landingtimestamp_begin, landingtimestamp_end, pflighltData));
		m_parkingpointuseinfo.insert(std::make_pair(_parkingpoint, std::move(usinglist)));
	}
	return bValid;
};

std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> DataManager::getParkingPointUsingTimeConflict(FlightPlanConf *pflighltData)
{
	uint64_t landingtimestamp_begin = 0;
	uint64_t landingtimestamp_end = 0;
	switch (pflighltData->m_flight_dep_arr_type)
	{
	case E_FLIGHT_DEP_ARR_TYPE_ARR:
	{
		landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp;
		landingtimestamp_end = pflighltData->m_PlanDateTimeLanding_behind_timestamp;
	}break;
	case E_FLIGHT_DEP_ARR_TYPE_DEP:
	{
		landingtimestamp_begin = pflighltData->m_PlanDateTimeTakeOff_ahead_timestamp;
		landingtimestamp_end = pflighltData->m_PlanDateTimeTakeOff_timestamp;
	}
	break;
	}

	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> conflicts;
	auto usingpoint_itor = m_parkingpointuseinfo.begin();
	while (usingpoint_itor != m_parkingpointuseinfo.end())
	{
		std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

		auto itr = std::find_if(usinglist.begin(),
			usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
			const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
			const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);


			//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
			if ((landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp) ||
				(landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp))
			{
				return true;
			}
			return false;
		});
		if (itr != usinglist.end())
		{
			conflicts.insert(std::make_pair(usingpoint_itor->first, usinglist));
		}
		usingpoint_itor++;
	}
	return conflicts;
};

bool DataManager::checkRunwayUsingTimeConflict(FlightPlanConf *pflighltData, const QString&_runway)
{
	bool bValid = false;
	if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
	{
		bValid = true;
		uint64_t landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp /*- 3*60*/;
		uint64_t landingtimestamp_end = landingtimestamp_begin + 3 * 60;
		auto usingpoint_itor = m_runwayuseinfo.find(_runway);
		if (usingpoint_itor != m_runwayuseinfo.end())
		{
			std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

			auto itr = std::find_if(usinglist.begin(),
				usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
				const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
				const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);
				//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
				if ((landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp) ||
					(landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp))
				{
					return true;
				}
				return false;
			});
			if (itr != usinglist.end())
			{
				//ptagPath_Plan = nullptr;
				bValid = false;
			}
			else
			{
				usinglist.push_back(std::make_tuple(landingtimestamp_begin, landingtimestamp_end, pflighltData));
			}
		}
		else
		{
			std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>> usinglist;
			usinglist.push_back(std::make_tuple(landingtimestamp_begin, landingtimestamp_end, pflighltData));
			m_runwayuseinfo.insert(std::make_pair(_runway, std::move(usinglist)));
		}
	}
	return bValid;
}

std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> DataManager::getRunwayUsingTimeConflict(FlightPlanConf *pflighltData)
{

	std::unordered_map<QString, std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>>> conflicts;
	if (pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
	{
		return conflicts;
	}
	uint64_t landingtimestamp_begin = pflighltData->m_PlanDateTimeLanding_timestamp /*- 3*60*/;
	uint64_t landingtimestamp_end = landingtimestamp_begin + 3 * 60;
	auto usingpoint_itor = m_runwayuseinfo.begin();
	while (usingpoint_itor != m_runwayuseinfo.end())
	{
		std::list<std::tuple<UINT64, UINT64, FlightPlanConf *>> &usinglist = usingpoint_itor->second;

		auto itr = std::find_if(usinglist.begin(),
			usinglist.end(), [&](const std::list<std::tuple<UINT64, UINT64, FlightPlanConf*>>::value_type &vt) {
			const uint64_t &_PlanDateTimeTakeOff_ahead_timestamp = std::get<0>(vt);
			const uint64_t &_PlanDateTimeTakeOff_timestamp = std::get<1>(vt);
			//查找 停机坪的使用时间段 是否与当前班次使用时间段冲突的，即当前班次的使用时间起始与别的时间段重合
			if ((landingtimestamp_begin >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_begin <= _PlanDateTimeTakeOff_timestamp) ||
				(landingtimestamp_end >= _PlanDateTimeTakeOff_ahead_timestamp && landingtimestamp_end <= _PlanDateTimeTakeOff_timestamp))
			{
				return true;
			}
			return false;
		});
		if (itr != usinglist.end())
		{
			conflicts.insert(std::make_pair(usingpoint_itor->first, usinglist));
		}
		usingpoint_itor++;
	}
	return conflicts;
}


std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> >& DataManager::flightdata()
{
	return m_flightdata;
}

void DataManager::setFlightData(const std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> > &newData)
{
	m_flightdata = newData;
}

void DataManager::Parkingpointuseinfo_clear()
{
	m_parkingpointuseinfo.clear();
}

void DataManager::PlaneNumToFlightNum_clear()
{
	m_PlaneNumToFlightNum.clear();
}

std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> > &DataManager::PlaneNumToFlightNum()
{
	return m_PlaneNumToFlightNum;
}

void DataManager::setPlaneNumToFlightNum(const std::unordered_map<QString, std::unordered_map<QString, FLIGHTPLAN_PATH_INFO> > &newPlaneNumToFlightNum)
{
	m_PlaneNumToFlightNum = newPlaneNumToFlightNum;
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



std::unordered_map<QString, tagPoiItem> & DataManager::getPoiitems()
{
	return m_poiitemsmap;
}

DataManager::~DataManager()
{
	if (m_pPathPlanExtendTaskManager != nullptr)
	{
		m_pPathPlanExtendTaskManager->deleteLater();
	}
#ifdef CALC_PATH
	if (m_pDijkstra)
	{
		delete m_pDijkstra;
	}
#endif

	if (m_pThreadPool != nullptr)
	{
		delete m_pThreadPool;
	}
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

#ifdef CALC_PATH
                    LAT_LNG latlng = sublatlnglist.back();
                    updateDijkstraMap(latlng, lstlatlng, bBegin,geoinfoitem.m_tags.value("aeroway"));
#endif
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
        m_GeoJsonInfos.insert(namefile, geoinfos);
	}
}


void DataManager::readWaypointsGeoJsonFile(const std::string & filename)
{
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
                for(auto parentcoordinatesarrayitem :parentcoordinatesarray)
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

#ifdef CALC_PATH
                        LAT_LNG latlng = sublatlnglist.back();
                        if(aeroway.isEmpty() || aeroway != "runway")
                        {
                            auto _m_aerowayinfo_itor = m_aerowayinfo.find(latlng);
                            if(_m_aerowayinfo_itor != m_aerowayinfo.end())
                            {
                                aeroway = _m_aerowayinfo_itor->second;
                            }
                            else
                            {
                                float dismin = DIJKSTRA_MAX_VALUE;
                                auto _linestringpoits_itor = m_aerowayinfo.begin();
                                while (_linestringpoits_itor != m_aerowayinfo.end())
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

                       updateDijkstraMap(latlng, lstlatlng, bBegin, aeroway);
#endif
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
        m_GeoJsonInfos.insert(namefile, geoinfos);
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
		m_WPSRunwayInfos.insert(namefile, geoinfos);
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
                    auto generateExtendSamplingEx = [&](const LAT_LNG& startLatLng, const LAT_LNG& endLatLng, LATLNGS_VECTOR &retlist)
					{
						retlist.push_back(startLatLng);
                        double dis = FunctionAssistant::calc_dist(startLatLng, endLatLng);
                        if (dis > m_point_extend_metres)
                        {
                            int step = dis / m_point_extend_metres;
							glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(startLatLng, endLatLng);
                            LAT_LNG lstextendpt = startLatLng;
							for (int i = 1; i < step + 1; i++)
                            {
                                LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, m_point_extend_metres);
								retlist.push_back(currentextendpt);

								lstextendpt = currentextendpt;
							}
						}
						retlist.push_back(endLatLng);
					};

                    auto generateExtendSampling = [&](const LAT_LNG& latlng, const LAT_LNG& lstlatlng, LATLNGS_VECTOR &retlist)
					{
                        double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
                        if (dis > m_point_extend_metres)
                        {
                            int step = dis / m_point_extend_metres;
							glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
                            LAT_LNG lstextendpt = lstlatlng;
							for (int i = 1; i < step + 1; i++)
                            {
                                LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, m_point_extend_metres);
								retlist.push_back(currentextendpt);

								lstextendpt = currentextendpt;
                            }
                            double rdis = dis - m_point_extend_metres * step;

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
									m_RunwayInfos.insert(poiKeywordTmp, runway_retlist_1);
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

void DataManager::readWpsGeoDirPath(const QString& path)
{
	QFileInfo fileInfo(path);
	if (fileInfo.isDir())
	{
		QDir dir(path);
		dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
		QFileInfoList list = dir.entryInfoList();
		for (int i = 0; i < list.size(); i++)
		{
			readWpsGeoDirPath(list.at(i).filePath());
		}
	}
	else if (fileInfo.isFile()) // �ҵ���Ƭ
	{
		QString newPath = path;
		readWpsGeoJsonFile(newPath.toStdString());
	}
}

void DataManager::readWpsGeoJsonFile(const std::string& filename)
{
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
        m_WPSInfos.insert(namefile, geoinfos);
    }
}

void DataManager::readAerowayFile(const std::string &filename)
{
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
                    auto _m_aerowayinfo_itor = m_aerowayinfo.find(latlng);
                    if(_m_aerowayinfo_itor != m_aerowayinfo.end())
                    {
                        if(_m_aerowayinfo_itor->second != "runway")
                        {
                            _m_aerowayinfo_itor->second = geoinfoitem.m_tags.value("aeroway");
                        }
                    }
                    else
                    {
                        m_aerowayinfo.insert(std::make_pair(latlng, geoinfoitem.m_tags.value("aeroway")));
                    }
                }
                geoinfoitem.coordinates.emplace_back(std::move(sublatlnglist));
                geoinfoitem.type = E_GEOTYPE_LINE;
                geoinfos.type = E_GEOTYPE_LINE;
            }
        }
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



void DataManager::readPathDir(const QString &path)
{
    QString pathinfo = path + "/PathInfo.json";

#if 1
    QDir fileInfo(pathinfo);
    if (fileInfo.exists(pathinfo))
    {
        readPoisJsonFile(path + "/Agent.json");
        readPathInfoFile(pathinfo);
        readPathInfoFileDir(path + "/generatepath");

        QString pathinfo = m_deal_path+ "/path/PathInfo_.json";
        QDir fileInfo(pathinfo);
        if (!fileInfo.exists(pathinfo))
        {
            writePathInfoFile(pathinfo);
            qDebug() << "****************save path_plan_extends";
        }
    }
    else
    {
		readPoisJsonFile(path + "/Agent.json");
		decode_Standard_Taxiing_Path(path + "/Standard_Taxiing_Path.xlsx");
		decode_Path_Plan(path + "/Path_Plan.xlsx");

		generatePath_planExtendWps();
    }
#else
    readPoisJsonFile(path + "/Agent.json");
    decode_Standard_Taxiing_Path(path + "/Standard_Taxiing_Path.xlsx");
    decode_Path_Plan(path + "/Path_Plan.xlsx");
    generatePath_planExtendWps();
#endif

	decode_stopbar();
	decode_transrunwaybar();
}

void DataManager::readPoisJsonFile(const QString & filename)
{
	auto appendPoiMapItem = [&](tagPoiItem& poiitemnew) {
#ifdef CALC_PATH
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
            QString aeroway = "poi_"+ poiitemnew.poiKeyword;
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
            item_id = appendDijkstraMap(poiitemnew.poipoint, disminLatlng, disminid, eExtendPointType, E_POINT_TYPE_POI,aeroway);
			poiitemnew.m_calibrate_osm_path_info = std::make_tuple(disminid, disminLatlng);
		}
#endif
#endif

		m_poiitemsmap.insert(std::make_pair(poiitemnew.poiKeyword, std::move(poiitemnew)));
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

void DataManager::readPathInfoFileDir(const QString &path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.isDir())
    {
        QDir dir(path);
        dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); i++)
        {
            readPathInfoFileDir(list.at(i).filePath());
        }
    }
    else if (fileInfo.isFile()) // �ҵ���Ƭ
    {
        QString newPath = path;
        //		readGeoJsonFile(newPath.toStdString());
        readPathInfoFileInfo(newPath);
    }
}

void DataManager::readPathInfoFileInfo(const QString &filename)
{
    QMap<QString, bool> m_poiinvalids;
    QJsonObject jsonobj = FunctionAssistant::read_json_file_object(filename);
    {
        tagPath_Plan path_plan;
        path_plan.fromJson(jsonobj);

        if (!m_runways.contains(path_plan.m_runway))
        {
            m_runways.push_back(path_plan.m_runway);
        }

        E_FLIGHT_DEP_ARR_TYPE flight_dep_arr_type = path_plan.m_flight_dep_arr_type;
        QString parkingpoint = path_plan.m_parkingpoint;
        QString runway = path_plan.m_runway;
        auto _Path_Plans_itor = m_Path_Plans.find(path_plan.m_parkingpoint);
        if (_Path_Plans_itor != m_Path_Plans.end())
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

            m_Path_Plans.insert(std::make_pair(parkingpoint, std::move(arr_dep_runway_path)));
        }

        tagPath_Plan *ppath = &m_Path_Plans.at(parkingpoint).at(flight_dep_arr_type).at(runway).pathindex;

        if (ppath)
        {
            if (ppath->m_pathPoints.size() > 2)
            {
                if ((ppath->m_pathPoints.at(0).bvalid) &&
                    (ppath->m_pathPoints.at(ppath->m_pathPoints.size() - 1).bvalid) &&
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
#ifdef REBUILD_EXTEND_WPS
            generateRunwayExtendWps(ppath);
#endif

            switch (flight_dep_arr_type)
            {
            case E_FLIGHT_DEP_ARR_TYPE_DEP:
            {
                if (!m_deppaths.contains(ppath))
                {
                    m_deppaths.push_back(ppath);
                }
            }
            break;
            case E_FLIGHT_DEP_ARR_TYPE_ARR:
            {
                if (!m_arrpaths.contains(ppath))
                {
                    m_arrpaths.push_back(ppath);
                }
            }
            break;
            default:
                break;
            }

            for (int i = 0; i < ppath->m_pathPoints.size(); i++)
            {
                if (!(ppath->m_pathPoints.at(i).bvalid))
                {
                    m_poiinvalids.insert((ppath->m_pathPoints.at(i).m_pt), false);
                }
            }
        }
    }
//    std::cout << "************************" << std::endl;
//    auto itor = m_poiinvalids.begin();
//    while (itor != m_poiinvalids.end())
//    {
//        std::cout << itor.key().toStdString() << std::endl;
//        itor++;
//    }
//    std::cout << "************************" << std::endl;
}


//#define  REBUILD_EXTEND_WPS

void DataManager::readPathInfoFile(const QString & filename)
{
    struct tagCarPathInfo
    {
        QString p_name;
        QString pbn_name;
        QString pbn_extend_name;
        LAT_LNG p_pt;
        LAT_LNG pbn_pt;
        LAT_LNG pbn_extend_pt;
        glm::dvec3 direction_p;
        glm::dvec3 direction_pbn;
        glm::dvec3 direction_pbn_extend;

        LAT_LNG p_prev_pt;
        LAT_LNG p_next_pt;
        LAT_LNG pbn_prev_pt;
        LAT_LNG pbn_next_pt;

        bool bValid;
        tagCarPathInfo()
        {
            bValid = false;
        }
        void init(tagPath_Plan *ppath)
        {
            int car_p_index = -1;
            int car_pbn_index = -1;
            for(int i = 0; i < ppath->m_pathPoints.size(); i++)
            {
                if(i == 0)
                {
                    this->p_name = (ppath->m_pathPoints.at(i).m_pt);
                    auto&bvalid = (ppath->m_pathPoints.at(i).bvalid);
                    auto&index =  (ppath->m_pathPoints.at(i).index);

                    if(bvalid)
                    {
                        car_p_index = index;
                    }
                }
                else
                {
                    this->pbn_name = (ppath->m_pathPoints.at(i).m_pt);
                    auto&bvalid = (ppath->m_pathPoints.at(i).bvalid);
                    auto&index = (ppath->m_pathPoints.at(i).index);

                    if(bvalid && this->pbn_name.contains("BN"))
                    {
                        car_pbn_index = index;
                    }
                }

                if(car_p_index != -1 && car_pbn_index != -1)
                {
                    break;
                }
            }

            if(car_p_index != -1 && car_pbn_index != -1)
            {
                this->pbn_extend_name =  this->pbn_name+"_extend";
                std::tuple< UINT64, LAT_LNG> car_p_point = ppath->m_tracking_osm_path_info_calibrate.at(car_p_index);
                std::tuple< UINT64, LAT_LNG> car_pbn_point = ppath->m_tracking_osm_path_info_calibrate.at(car_pbn_index);

                this->p_pt = std::get<1>(car_p_point);
                this->pbn_pt = std::get<1>(car_pbn_point);

                int car_p_next_point_index = -1;
                int car_pbn_prev_point_index = -1;
                int car_pbn_next_point_index = -1;

                for(int j = 0; j < ppath->m_extendwpslatlng.size(); j++)
                {
                    if(this->p_pt == ppath->m_extendwpslatlng.at(j))
                    {
                        car_p_next_point_index = j+1;
                    }

                    if(this->pbn_pt == ppath->m_extendwpslatlng.at(j))
                    {
                        car_pbn_prev_point_index = j-1;
                        car_pbn_next_point_index = j+1;
                    }

                    if(car_p_next_point_index != -1 && car_pbn_prev_point_index != -1 && car_pbn_next_point_index != -1)
                    {
                        break;
                    }
                }

                if((car_p_next_point_index >= 0 && car_p_next_point_index < ppath->m_extendwpslatlng.size()) &&
                    (car_pbn_prev_point_index >= 0 && car_pbn_prev_point_index < ppath->m_extendwpslatlng.size()) &&
                    (car_pbn_next_point_index >= 0 && car_pbn_next_point_index < ppath->m_extendwpslatlng.size()))
                {
                    this->p_next_pt = ppath->m_extendwpslatlng.at(car_p_next_point_index);
                    this->pbn_prev_pt = ppath->m_extendwpslatlng.at(car_pbn_prev_point_index);
                    this->pbn_next_pt = ppath->m_extendwpslatlng.at(car_pbn_next_point_index);
                    bValid = true;

                    calc();
                }
            }
        }

        void calc()
        {
            this->direction_p = FunctionAssistant::calculateVector(p_next_pt, p_pt);
//            this->direction_pbn = FunctionAssistant::calculateVector(pbn_prev_pt, pbn_pt);
            this->direction_pbn = FunctionAssistant::calculateVector(pbn_pt,pbn_next_pt);

            glm::dvec3 direction_pbn_1 = glm::dvec3(-direction_pbn.y, direction_pbn.x, direction_pbn.z);
            glm::dvec3 direction_pbn_2 = glm::dvec3(direction_pbn.y, -direction_pbn.x, direction_pbn.z);


            double distance = 40;
            p_prev_pt = FunctionAssistant::calculateDirectionExtendPoint(p_pt, direction_p, 10);

            LAT_LNG extendpt1 = FunctionAssistant::calculateDirectionExtendPoint(pbn_pt, direction_pbn_1, distance);
            LAT_LNG extendpt2 = FunctionAssistant::calculateDirectionExtendPoint(pbn_pt, direction_pbn_2, distance);

            if(FunctionAssistant::calc_dist(p_pt, extendpt1) < FunctionAssistant::calc_dist(p_pt, extendpt2))
            {
                pbn_extend_pt = extendpt1;
                direction_pbn_extend = direction_pbn_1;
            }
            else
            {
                pbn_extend_pt = extendpt2;
                direction_pbn_extend = direction_pbn_2;
            }
        }
    };

    QMap<QString, tagCarPathInfo> m_carpaths;

    QMap<QString, bool> m_poiinvalids;
    QJsonObject jsonobj = FunctionAssistant::read_json_file_object(filename);
    QJsonArray pathinfoarr = jsonobj.value("pathInfos").toArray();

    for (auto pathinfoarritem : pathinfoarr)
    {
        auto pathinfoarritemobj = pathinfoarritem.toObject();

        tagPath_Plan path_plan;
        path_plan.fromJson(pathinfoarritemobj);

        if (!m_runways.contains(path_plan.m_runway))
        {
            m_runways.push_back(path_plan.m_runway);
        }

        E_FLIGHT_DEP_ARR_TYPE flight_dep_arr_type = path_plan.m_flight_dep_arr_type;
        QString parkingpoint = path_plan.m_parkingpoint;
        QString runway = path_plan.m_runway;
        auto _Path_Plans_itor = m_Path_Plans.find(path_plan.m_parkingpoint);
        if (_Path_Plans_itor != m_Path_Plans.end())
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

            m_Path_Plans.insert(std::make_pair(parkingpoint, std::move(arr_dep_runway_path)));
        }

        tagPath_Plan *ppath = &m_Path_Plans.at(parkingpoint).at(flight_dep_arr_type).at(runway).pathindex;

        if (ppath)
        {
            if (ppath->m_pathPoints.size() > 2)
            {
                if ((ppath->m_pathPoints.at(0).bvalid) &&
                    (ppath->m_pathPoints.at(ppath->m_pathPoints.size() - 1).bvalid) &&
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
#ifdef REBUILD_EXTEND_WPS
            generateRunwayExtendWps(ppath);
#endif

            switch (flight_dep_arr_type)
            {
            case E_FLIGHT_DEP_ARR_TYPE_DEP:
            {
                if (!m_deppaths.contains(ppath))
                {
                    if(!m_carpaths.contains(ppath->m_parkingpoint))
                    {
                        tagCarPathInfo carpathinfo;
                        carpathinfo.init(ppath);
                        if(carpathinfo.bValid)
                        {
                            m_carpaths.insert(carpathinfo.p_name, carpathinfo);
                        }
                    }
                    m_deppaths.push_back(ppath);
                }
            }
            break;
            case E_FLIGHT_DEP_ARR_TYPE_ARR:
            {
                if (!m_arrpaths.contains(ppath))
                {
                    m_arrpaths.push_back(ppath);
                }
            }
            break;
            default:
                break;
            }

            for (int i = 0; i < ppath->m_pathPoints.size(); i++)
            {
                if (!(ppath->m_pathPoints.at(i).bvalid))
                {
                    m_poiinvalids.insert((ppath->m_pathPoints.at(i).m_pt), false);
                }
            }
        }
    }
	std::cout << "************************" << std::endl;
	auto itor = m_poiinvalids.begin();
	while (itor != m_poiinvalids.end())
	{
		std::cout << itor.key().toStdString() << std::endl;
		itor++;
	}
	std::cout << "************************" << std::endl;



    {
        QString pathinfo = m_deal_path+ "/path/CarPathInfo.json";


        QJsonArray pathinfoarr;
        auto pathplans_itor = m_carpaths.begin();
        while (pathplans_itor != m_carpaths.end())
        {
            const QString& parkingpoint = pathplans_itor.key();
            const tagCarPathInfo & carpathiteminfo = pathplans_itor.value();

            QJsonObject carpathitem;

            carpathitem.insert("p_name",carpathiteminfo.p_name);
            carpathitem.insert("pbn_name",carpathiteminfo.pbn_name);

            QJsonArray pt_p;
            pt_p.push_back(carpathiteminfo.p_pt.lng);
            pt_p.push_back(carpathiteminfo.p_pt.lat);
            pt_p.push_back(0);
            pt_p.push_back(0);
            carpathitem.insert("p_point",pt_p);

            QJsonArray pt_pbn;
            pt_pbn.push_back(carpathiteminfo.pbn_pt.lng);
            pt_pbn.push_back(carpathiteminfo.pbn_pt.lat);
            pt_pbn.push_back(0);
            pt_pbn.push_back(0);
            carpathitem.insert("pbn_point",pt_pbn);

            QJsonArray pt_pbn_return_extend;
            pt_pbn_return_extend.push_back(carpathiteminfo.pbn_extend_pt.lng);
            pt_pbn_return_extend.push_back(carpathiteminfo.pbn_extend_pt.lat);
            pt_pbn_return_extend.push_back(0);
            pt_pbn_return_extend.push_back(0);
            carpathitem.insert("pbn_return_extend_point",pt_pbn_return_extend);


            QJsonArray pt_p_extend;
            pt_p_extend.push_back(carpathiteminfo.p_prev_pt.lng);
            pt_p_extend.push_back(carpathiteminfo.p_prev_pt.lat);
            pt_p_extend.push_back(0);
            pt_p_extend.push_back(0);
            carpathitem.insert("p_prevd_extend_point",pt_p_extend);


            QJsonArray pt_pbn_extend;
            pt_pbn_extend.push_back(carpathiteminfo.pbn_next_pt.lng);
            pt_pbn_extend.push_back(carpathiteminfo.pbn_next_pt.lat);
            pt_pbn_extend.push_back(0);
            pt_pbn_extend.push_back(0);
            carpathitem.insert("pbn_next_extendpoint",pt_pbn_extend);


            pathinfoarr.push_back(carpathitem);
            pathplans_itor++;
        }

        QJsonObject jsonobj;
        jsonobj.insert("carpathInfos", pathinfoarr);

        FunctionAssistant::write_json_file_object(pathinfo, jsonobj);
        qDebug() << "****************save car_path_plan_extends";
    }
#ifdef REBUILD_EXTEND_WPS
    QString pathinfo = m_deal_path+ "/path/PathInfo_.json";
	QDir fileInfo(pathinfo);
	if (!fileInfo.exists(pathinfo))
	{
		writePathInfoFile(pathinfo);
		qDebug() << "****************save path_plan_extends";
	}
#endif
	}

void DataManager::writePathInfoFile(const QString & filename)
{
	QJsonArray pathinfoarr;
	auto pathplans_itor = m_Path_Plans.begin();
	while (pathplans_itor != m_Path_Plans.end())
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

void DataManager::decode_Standard_Taxiing_Path(const QString & filename)
{
	m_Standard_Taxiing_Paths.clear();
	auto getXlsxDocumentVal = [](const QXlsx::Document &worksheet, const QString& title, QString& val)
	{
		val = worksheet.read(title).toString();
	};

	QXlsx::Document _Xlsx(filename);
	int lastColumn = _Xlsx.dimension().columnCount();
	int lastRow = _Xlsx.dimension().rowCount();

	E_FLIGHT_DEP_ARR_TYPE _flight_dep_arr_type;

	for (int i = 1; i < lastRow; i++)
	{
		tagStandard_Taxiing_Path standardpath;
		QString val;
		getXlsxDocumentVal(_Xlsx, "A" + QString::number(i + 1), val);
		if (val.trimmed().contains("进港"))
		{
			_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_ARR;
		}
		else if (val.trimmed().contains("出港"))
		{
			_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_DEP;
		}
		else if (val.trimmed().contains("编号") || val.isEmpty())
		{
		}
		else
		{
			standardpath.m_title = val;
			standardpath.m_flight_dep_arr_type = _flight_dep_arr_type;
			getXlsxDocumentVal(_Xlsx, "B" + QString::number(i + 1), standardpath.m_path);
			getXlsxDocumentVal(_Xlsx, "C" + QString::number(i + 1), standardpath.m_runway);
			getXlsxDocumentVal(_Xlsx, "D" + QString::number(i + 1), standardpath.m_parkingarea);
			getXlsxDocumentVal(_Xlsx, "E" + QString::number(i + 1), standardpath.m_pathdetail);
			standardpath.analysis();
			m_Standard_Taxiing_Paths.insert(std::make_pair(val, std::move(standardpath)));
		}
	}
}

void DataManager::decode_Path_Plan(const QString & filename)
{
	m_total_path_num = 0;
	m_Path_Plans.clear();
	m_runways.clear();
	auto getXlsxDocumentVal = [](const QXlsx::Document &worksheet, const QString& title, QString& val)
	{
		val = worksheet.read(title).toString();
	};

	QXlsx::Document _Xlsx(filename);
	int lastColumn = _Xlsx.dimension().columnCount();
	int lastRow = _Xlsx.dimension().rowCount();

	E_FLIGHT_DEP_ARR_TYPE _flight_dep_arr_type;

	for (int i = 1; i < lastRow; i++)
	{
		tagPath_Plan path_plan;
		getXlsxDocumentVal(_Xlsx, "A" + QString::number(i + 1), path_plan.m_runway);
		getXlsxDocumentVal(_Xlsx, "B" + QString::number(i + 1), path_plan.m_parkingpoint);
		QString dep_arr_type;
		getXlsxDocumentVal(_Xlsx, "C" + QString::number(i + 1), dep_arr_type);

		getXlsxDocumentVal(_Xlsx, "D" + QString::number(i + 1), path_plan.m_airportcode);
		getXlsxDocumentVal(_Xlsx, "E" + QString::number(i + 1), path_plan.m_path);
		if (!path_plan.m_runway.isEmpty() &&
			!path_plan.m_parkingpoint.isEmpty() &&
			!dep_arr_type.isEmpty() &&
			!path_plan.m_airportcode.isEmpty() &&
			!path_plan.m_path.isEmpty())
		{
			if (!path_plan.m_parkingpoint.startsWith("P"))
			{
				if (m_poiitemsmap.find(path_plan.m_parkingpoint) == m_poiitemsmap.end())
				{
					QString tempParkingpoint = "P" + path_plan.m_parkingpoint;
					if (m_poiitemsmap.find(tempParkingpoint) != m_poiitemsmap.end())
					{
						path_plan.m_parkingpoint = tempParkingpoint;
					}
				}
			}
			if (dep_arr_type.contains("A"))
			{
				path_plan.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_ARR;
			}
			else if (dep_arr_type.contains("D"))
			{
				path_plan.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_DEP;
			}

			path_plan.analysis(m_Standard_Taxiing_Paths, m_poiitemsmap);

			if (!m_runways.contains(path_plan.m_runway))
			{
				m_runways.push_back(path_plan.m_runway);
			}

			E_FLIGHT_DEP_ARR_TYPE flight_dep_arr_type = path_plan.m_flight_dep_arr_type;
			QString parkingpoint = path_plan.m_parkingpoint;
			QString runway = path_plan.m_runway;
			auto _Path_Plans_itor = m_Path_Plans.find(path_plan.m_parkingpoint);
			if (_Path_Plans_itor != m_Path_Plans.end())
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

						m_total_path_num++;
					}
				}
				else
				{
					tagPathPlanInfo pathinfo;
					pathinfo.pathindex = std::move(path_plan);
					RUNWAY_PATH runway_path;
					runway_path.insert(std::make_pair(runway, std::move(pathinfo)));
					arr_dep_runway_path.insert(std::make_pair(flight_dep_arr_type, std::move(runway_path)));

					m_total_path_num++;
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

				m_Path_Plans.insert(std::make_pair(parkingpoint, std::move(arr_dep_runway_path)));

				m_total_path_num++;
			}


			tagPath_Plan *ppath = &m_Path_Plans.at(parkingpoint).at(flight_dep_arr_type).at(runway).pathindex;

			switch (flight_dep_arr_type)
			{
			case E_FLIGHT_DEP_ARR_TYPE_DEP:
			{
				if (!m_deppaths.contains(ppath))
				{
					m_deppaths.push_back(ppath);
				}
			}
			break;
			case E_FLIGHT_DEP_ARR_TYPE_ARR:
			{
				if (!m_arrpaths.contains(ppath))
				{
					m_arrpaths.push_back(ppath);
				}
			}
			break;
			default:
				break;
			}
		}
	}

    if(m_deal_data_process_func_callback)
    {
        m_deal_data_process_func_callback(m_total_path_num, -1);
    }
}

QHash<QString, GeoJsonInfos> & DataManager::getGeoJsonInfos()
{
	return m_GeoJsonInfos;
}

QHash<QString, GeoJsonInfos>& DataManager::getWpsInfos()
{
	return m_WPSInfos;
}

QHash<QString, GeoJsonInfos>& DataManager::getWpsRunwayInfos()
{
	return m_WPSRunwayInfos;
}



