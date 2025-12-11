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
#define TAKEOFF_METER (1150)

#define LANDINGPOINT_METER (15)




PathPlanExtendTaskManager::PathPlanExtendTaskManager(QObject* parent)
    :QObject(parent)
{
    m_threadPool = new QThreadPool(this);
#if 0
    m_threadPool->setMaxThreadCount((std::thread::hardware_concurrency() > 1) ? std::thread::hardware_concurrency() : 1);
#else
    //线程池中最大工作线程数量设置为4
    m_threadPool->setMaxThreadCount(15);
#endif
}

PathPlanExtendTaskManager::~PathPlanExtendTaskManager()
{
    if (m_threadPool)
    {
        m_threadPool->clear();
        m_threadPool->waitForDone();
        delete m_threadPool;
        m_threadPool = nullptr;
    }
}

void PathPlanExtendTaskManager::appendProcessor(deal_pathplan_func_callback _pCallbackfunc, tagPath_Plan *path_plan)
{
    PathPlanExtendProcessor *pThreadTaskProcessor = new PathPlanExtendProcessor(_pCallbackfunc, path_plan);
    pThreadTaskProcessor->setAutoDelete(true);
    m_threadPool->start(pThreadTaskProcessor, QThread::HighestPriority);
}


int PathPlanExtendTaskManager::getActiveThreadCount() const
{
    return m_threadPool->activeThreadCount();
}

PathPlanExtendProcessor::PathPlanExtendProcessor(deal_pathplan_func_callback _pCallbackfunc, tagPath_Plan *path_plan)
    :m_pCallbackfunc(std::move(_pCallbackfunc))
    , m_path_plan(path_plan)
{
}

PathPlanExtendProcessor::~PathPlanExtendProcessor()
{

}


void PathPlanExtendProcessor::run()
{
    if (m_pCallbackfunc)
    {
        m_pCallbackfunc(m_path_plan);
    }
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
            m_pathPoints.push_back(tagPath_Plan::ptinfo{parkingpoint, true, index});
		}
		else
        {
            m_pathPoints.push_back(tagPath_Plan::ptinfo{parkingpoint, false, index});
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

		m_trackingcl = FunctionAssistant::randColor(255);;

        auto getPtInfo=[](const QString &ptname,QString& matchedText, int& numText)->bool
        {
            QRegularExpression regex("[^0-9]+");
            QRegularExpression regexnum("\\d+");
            // 在字符串中查找匹配的部分
            QRegularExpressionMatch match = regex.match(ptname);
            QRegularExpressionMatch matchnum = regexnum.match(ptname);

            if (match.hasMatch() && matchnum.hasMatch())
            {
                matchedText = match.captured(0);
                numText = matchnum.captured(0).toInt();
                return true;
            }
            return false;
        };

        auto getindex=[&](const tagStandard_Taxiing_Path& standard_Taxiing_Path)->int{

            QString ptname = externpath2;
            QString matchedText;
            int numText;

            int count = 0;
            if(getPtInfo(ptname, matchedText, numText))
            {
                for(int index = 0; index < standard_Taxiing_Path.m_pathPoints.size() - 1; index++)
                {
                    QString ptname1 = standard_Taxiing_Path.m_pathPoints.at(index);
                    QString ptname2 = standard_Taxiing_Path.m_pathPoints.at(index+1);
                    QString matchedText1;
                    int numText1;
                    QString matchedText2;
                    int numText2;

                    if(getPtInfo(ptname1, matchedText1, numText1) && getPtInfo(ptname2, matchedText2, numText2))
                    {
                        if(matchedText1 == matchedText && matchedText2 == matchedText)
                        {
                            if((numText - numText1) * (numText - numText2) < 0)
                            {
                                count =  index;
                                break;
                            }

                            if(numText1 > numText2)
                            {
                                // 10 9 8
                                if(numText > numText1)
                                {
                                    count =  index-1;
                                    break;
                                }
                            }
                            else if(numText1 < numText2)
                            {
                                // 8 9 10
                                if(numText < numText1)
                                {
                                    count =  index-1;
                                    break;
                                }
                            }

                        }
                    }
                }
            }
            return count;
        };

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


                        int count = getindex(standard_Taxiing_Path);

                        if(count == 0)
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
                        else
                        {
                            QString parkingpointtmp = m_parkingpoint;
                            auto parkpointstart = getParkpointstart(parkingpointtmp);
                            if(parkpointstart.isEmpty() || parkpointstart.startsWith("P"))
                            {
                                parkingpointtmp.insert(1, "BN");
                            }
                            else
                            {
                                parkingpointtmp.insert(0, "PBN");
                            }

                            int index = count;
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
                        int count = getindex(standard_Taxiing_Path);

                        if(count == 0)
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
                        else
                        {
                            int wpssize = count+1 + pathPoints.size() - 1 + 1;
                            resize(wpssize);

                            for (int i = 0; i < count+1; i++)
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
				}
				break;
				default:
					break;
				}
			}
			if (this->m_pathPoints.size() > 2)
			{
                if ((this->m_pathPoints.at(0).bvalid) &&
                    (this->m_pathPoints.at(this->m_pathPoints.size() - 1).bvalid) &&
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
                if ((this->m_pathPoints.at(0).bvalid) &&
                    (this->m_pathPoints.at(this->m_pathPoints.size() - 1).bvalid) &&
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
        pathPoint.insert("poiname", (m_pathPoints.at(i).m_pt));
        pathPoint.insert("poivalid", (m_pathPoints.at(i).bvalid));
        pathPoint.insert("index", (m_pathPoints.at(i).index));
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
        m_pathPoints.push_back(tagPath_Plan::ptinfo{pathPoint.value("poiname").toString(), pathPoint.value("poivalid").toBool(), pathPoint.value("index").toInt()});
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
        QString name = (m_pathPoints.at(i).m_pt);
        bool bvalid = (m_pathPoints.at(i).bvalid);
        int index = (m_pathPoints.at(i).index);
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

