#pragma execution_character_set("utf-8")
#include "trackingwidget.h"

#include "mapwidget.h"
#include <QComboBox>
#include <QLabel>
#include <unordered_map>
#include "pathpanel.h"
#include <QToolButton>
#include <QPushButton>
#include <QCoreApplication>

TrackingWidget::TrackingWidget(QWidget *parent)
	:QWidget(parent),
	m_mapWidget(nullptr),
	m_bInit(false)
{
	this->setStyleSheet("RunningWidget{background-color:#2e2f30;}");
	m_mapWidget = new MapWidget(MapWidget::E_MAP_MODE_SELECT_PATH, this);
	m_mapWidget->hide();


	connect(m_mapWidget, &MapWidget::select_airport_sig, this, &TrackingWidget::select_airport_slot);

	m_btnRefresh = creatToolButton("refresh.svg");
	m_btnRefresh->show();
	m_srccombox = new QComboBox(this);
	m_srccombox->show();

	QStringList vallist;
	vallist << "all";
	vallist << "valid";
	vallist << "invalie";
	m_srccombox->addItems(vallist);
	m_srccombox->setCurrentIndex(0);


	m_arrtype = new QComboBox(this);
	m_arrtype->show();

	QStringList vallist1;
	vallist1 << "all";
	vallist1 << "arr";
	vallist1 << "dep";
	m_arrtype->addItems(vallist1);
	m_arrtype->setCurrentIndex(0);

	m_runways = new QComboBox(this);
	m_runways->show();


	m_parkingpoint = new QComboBox(this);
	m_parkingpoint->show();



	connect(m_btnRefresh, &QPushButton::clicked, this, [&]() {
		QStringList vallist1;
		vallist1 << m_srccombox->currentText();
		vallist1 << m_arrtype->currentText();
		vallist1 << m_runways->currentText();
		vallist1 << m_parkingpoint->currentText();

		m_pPathPanel->initDataSlot(vallist1);
	});
	m_pPathPanel = new PathPanel(m_mapWidget);
	m_pPathPanel->setVisableCallback(std::bind(&TrackingWidget::visiableSlot, this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4,
		std::placeholders::_5));
	m_pPathPanel->show();
}

TrackingWidget::~TrackingWidget()
{
	m_pPathPanel->setVisableCallback(nullptr);
	if (m_mapWidget)
	{
		m_mapWidget->deleteLater();
	}
}

QToolButton * TrackingWidget::creatToolButton(const QString& icon)
{
	QToolButton *btn = new QToolButton(this);
	btn->resize(QSize(36, 36));
	btn->setIconSize(QSize(36, 36));
	btn->setAutoRaise(true);
	btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	QIcon ic = QIcon(QCoreApplication::applicationDirPath() + "/res/svg/" + icon);
	btn->setIcon(ic);
	return btn;
}

void TrackingWidget::refreshTracking()
{
	tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getCurrentAirportInfo();
	if (ptagAirPortInfo)
	{
		m_runways->clear();
		QStringList vallist1 = ptagAirPortInfo->m_runways;
		vallist1.push_front("all");
		m_runways->addItems(vallist1);
		m_runways->setCurrentIndex(0);


		QStringList vallist2;
		vallist2.append("all");

		std::map<QString, ARR_DEP_RUNWAY_PATH>& pathplans = ptagAirPortInfo->m_Path_Plans;

		auto pathplans_itor = pathplans.begin();
		while (pathplans_itor != pathplans.end())
		{
			const QString& parkingpoint = pathplans_itor->first;
			vallist2.append(parkingpoint);
			pathplans_itor++;
		}
		m_parkingpoint->clear();
		m_parkingpoint->addItems(vallist2);
		m_parkingpoint->setCurrentIndex(0);

		auto itor3 = m_poisinfo.begin();
		while (itor3 != m_poisinfo.end())
		{
			auto item_id = itor3->second;
			m_mapWidget->drawPoint(item_id, LAT_LNG(), itor3->first, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT, true);
			//m_mapWidget->drawText(item_id, LAT_LNG(), itor2->first,DEFAULT_FONT_SIZE,DrawElements::E_ELEMENT_TYPE_POI_CONTEXT, true);
			itor3++;
		}
		m_poisinfo.clear();

		QColor cl = FunctionAssistant::randColor(255);
		uint64_t item_id;
		auto itor2 = ptagAirPortInfo->m_poiitemsmap.begin();
		while (itor2 != ptagAirPortInfo->m_poiitemsmap.end())
		{
			auto& subcoordinatesitem = itor2->second.poipoint;
			item_id = FunctionAssistant::generate_random_positive_uint64();
			auto _poisinfo_itor = m_poisinfo.find(itor2->first);
			if (_poisinfo_itor != m_poisinfo.end())
			{
				_poisinfo_itor->second = item_id;
			}
			else
			{
				m_poisinfo.insert(std::make_pair(itor2->first, item_id));
			}
			m_mapWidget->drawPoint(item_id, subcoordinatesitem, itor2->first, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
			//m_mapWidget->drawText(item_id, subcoordinatesitem, itor2->first,DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
			itor2++;
		}


#if 0
		std::vector<LAT_LNG> wayptsOSM;
		std::vector<LAT_LNG> wayptsOSMextend;
		std::vector<LAT_LNG> wayptsPOI;
		QColor color = QColor(255, 0, 255, 255);
		QColor colorextend = QColor(255, 0, 0, 255);
		QColor colorPOI = QColor(0, 255, 255, 255);
		UINT64 trackingid = FunctionAssistant::generate_random_positive_uint64();
		UINT64 trackingextendid = FunctionAssistant::generate_random_positive_uint64();
		UINT64 trackingPOIid = FunctionAssistant::generate_random_positive_uint64();

		auto& linestringpoints = ptagAirPortInfo->m_linestringpoints;

		wayptsOSM.reserve(linestringpoints.size());
		wayptsOSMextend.reserve(linestringpoints.size());
		wayptsPOI.reserve(linestringpoints.size());
		auto itor3 = linestringpoints.begin();
		while (itor3 != linestringpoints.end())
		{
			switch (std::get<1>(itor3->second))
			{
			case E_POINT_TYPE_OSM:
			{
				wayptsOSM.push_back(std::get<0>(itor3->second));
			}break;
			case E_POINT_TYPE_OSM_EXTEND:
			{
				wayptsOSMextend.push_back(std::get<0>(itor3->second));
			}break;
			case E_POINT_TYPE_POI:
			case E_POINT_TYPE_POI_EXTEND_OSM:
			case E_POINT_TYPE_POI_EXTEND_OSM_EXTEND:
			case E_POINT_TYPE_POI_EXTEND_POI:
			case E_POINT_TYPE_POI_EXTEND_POI_EXTEND:
			{
				wayptsPOI.push_back(std::get<0>(itor3->second));
			}break;
			default:
				break;
			}
			itor3++;
		}
		m_mapWidget->drawTrackingPoints(trackingid, wayptsOSM, color, 3.0);
		m_mapWidget->drawTrackingPoints(trackingextendid, wayptsOSMextend, colorextend, 1.0);
		//m_mapWidget->drawTrackingPoints(trackingPOIid, wayptsPOI, colorPOI, 10.0);

		std::vector<LAT_LNG> waypts1demo;
		waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
		waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
		QColor colorPOIdemo = QColor(0, 255, 255, 255);
		UINT64 trackingiddemo = FunctionAssistant::generate_random_positive_uint64();
		m_mapWidget->drawTrackingLine(trackingiddemo, waypts1demo, colorPOIdemo, 1.0);
#endif


		//std::vector<std::tuple<LAT_LNG, UINT64>> ret;
		//DataManager::getInstance().getExtendWpsExName2Name("A#T2", "W02L#A10", ret);
		//if (!ret.empty())
		//{
		//	std::vector<LAT_LNG> wayptspath;
		//	//QColor color = FunctionAssistant::randColor(255);
		//	//QColor colorextend = FunctionAssistant::randColor(255);
		//	QColor colorpath = QColor(0, 255, 0, 255);
		//	UINT64 trackingpathid = FunctionAssistant::generate_random_positive_uint64();
		//	wayptspath.resize(ret.size());
		//	for (int j = 0; j < ret.size(); j++)
		//	{
		//		wayptspath[j] = std::get<0>(ret.at(j));
		//	}
		//	m_mapWidget->drawTrackingLine(trackingpathid, wayptspath, colorpath, 10.0);
		//}
	}
}

void TrackingWidget::visiableSlot(PathPanel::E_PATH_OPERATE_TYPE bVisable,
	const QString& parkingpoint,
	const QString& arr_dep_runway_str,
	const QString& runway,
	const tagPath_Plan* pathplaninfo)
{
	QString trackingname = QString("%1_%2_%3").arg(parkingpoint).arg(arr_dep_runway_str).arg(runway);
	switch (bVisable)
	{
	case PathPanel::E_PATH_OPERATE_TYPE_SELECT_POINT:
	case PathPanel::E_PATH_OPERATE_TYPE_UNSELECT_POINT:
	{
		bool bSelect = (bVisable == PathPanel::E_PATH_OPERATE_TYPE_SELECT_POINT) ? true : false;
		if (!parkingpoint.isEmpty() && !m_poisinfo.empty())
		{
			if (m_poisinfo.find(parkingpoint) != m_poisinfo.end())
			{
				m_mapWidget->updateEntityItemSelect(m_poisinfo.at(parkingpoint), bSelect);
			}
		}
	}
	break;

	case PathPanel::E_PATH_OPERATE_TYPE_SELECT:
	case PathPanel::E_PATH_OPERATE_TYPE_UNSELECT:
	{
		bool bSelect = (bVisable == PathPanel::E_PATH_OPERATE_TYPE_SELECT) ? true : false;
		auto _trackingsinfo_itor = m_trackingsinfo.find(trackingname);
		if (_trackingsinfo_itor != m_trackingsinfo.end())
		{
			if (!_trackingsinfo_itor->second.pathplaninfo->m_trackinglatlng.empty())
			{
				m_mapWidget->updateElementSelect(_trackingsinfo_itor->second.m_oriTrackingId, bSelect);
			}
			if (!_trackingsinfo_itor->second.pathplaninfo->m_extendwpslatlng.empty())
			{
				m_mapWidget->updateElementSelect(_trackingsinfo_itor->second.m_extendTrackingId, bSelect);
			}
			if (!_trackingsinfo_itor->second.pathplaninfo->m_runwayextendwpslatlng.empty())
			{
				m_mapWidget->updateElementSelect(_trackingsinfo_itor->second.m_tracking_runway_extendId, bSelect);
			}
			if (!_trackingsinfo_itor->second.m_tracking_osm_path_infoData.empty())
			{
				m_mapWidget->updateElementSelect(_trackingsinfo_itor->second.m_tracking_osm_path_infoId, bSelect);
			}

			if (!_trackingsinfo_itor->second.m_tracking_osm_path_infoId_calibrateData.empty())
			{
				m_mapWidget->updateElementSelect(_trackingsinfo_itor->second.m_tracking_osm_path_infoId_calibrate, bSelect);
			}
		}
	}
	break;
	case PathPanel::E_PATH_OPERATE_TYPE_VISIABLE:
	{
		auto _trackingsinfo_itor = m_trackingsinfo.find(trackingname);
		if (_trackingsinfo_itor == m_trackingsinfo.end())
		{
			UINT64 item_id = FunctionAssistant::generate_random_positive_uint64();
			UINT64 item_id2 = FunctionAssistant::generate_random_positive_uint64();
			UINT64 item_id3 = FunctionAssistant::generate_random_positive_uint64();
			UINT64 item_id4 = FunctionAssistant::generate_random_positive_uint64();
			UINT64 item_id5 = FunctionAssistant::generate_random_positive_uint64();
			if (!pathplaninfo->m_trackinglatlng.empty() &&
				!pathplaninfo->m_tracking_osm_path_info.empty() &&
				!pathplaninfo->m_tracking_osm_path_info_calibrate.empty() &&
				!pathplaninfo->m_extendwpslatlng.empty() /*&&
				pathplaninfo->m_pathPoints.size() == pathplaninfo->m_tracking_osm_path_info.size() &&
				pathplaninfo->m_pathPoints.size() == pathplaninfo->m_tracking_osm_path_info_calibrate.size() &&
				pathplaninfo->m_pathPoints.size() == pathplaninfo->m_trackinglatlng.size()*/)
			{
				std::vector<LAT_LNG> m_tracking_osm_path_infoData;
				std::vector<LAT_LNG>	m_tracking_osm_path_infoId_calibrateData;
				m_tracking_osm_path_infoData.resize(pathplaninfo->m_tracking_osm_path_info.size());
				m_tracking_osm_path_infoId_calibrateData.resize(pathplaninfo->m_tracking_osm_path_info_calibrate.size());
				for (int index = 0; index < pathplaninfo->m_tracking_osm_path_info.size(); index++)
				{
					m_tracking_osm_path_infoData[index] = std::get<1>(pathplaninfo->m_tracking_osm_path_info[index]);
					m_tracking_osm_path_infoId_calibrateData[index] = std::get<1>(pathplaninfo->m_tracking_osm_path_info_calibrate[index]);
				}
				tagTrackingInfo trackinginfo = tagTrackingInfo{ item_id,
																item_id2,
																item_id3,
																item_id4,
																item_id5,
																pathplaninfo,
																m_tracking_osm_path_infoData,
																m_tracking_osm_path_infoId_calibrateData,
																pathplaninfo->m_trackingcl };
				m_trackingsinfo.insert(std::make_pair(trackingname, std::move(trackinginfo)));
			}
		}

		auto _trackingsinfo2_itor = m_trackingsinfo.find(trackingname);
		if (_trackingsinfo2_itor != m_trackingsinfo.end())
		{
			if (!_trackingsinfo2_itor->second.pathplaninfo->m_trackinglatlng.empty())
			{
				m_mapWidget->drawTrackingLine(_trackingsinfo2_itor->second.m_oriTrackingId, _trackingsinfo2_itor->second.pathplaninfo->m_trackinglatlng, _trackingsinfo2_itor->second.cl);
			}
			if (!_trackingsinfo2_itor->second.pathplaninfo->m_extendwpslatlng.empty())
			{
				m_mapWidget->drawTrackingLine(_trackingsinfo2_itor->second.m_extendTrackingId, _trackingsinfo2_itor->second.pathplaninfo->m_extendwpslatlng, _trackingsinfo2_itor->second.cl);
			}
			if (!_trackingsinfo2_itor->second.pathplaninfo->m_runwayextendwpslatlng.empty())
			{
				m_mapWidget->drawTrackingDashedLine(_trackingsinfo2_itor->second.m_tracking_runway_extendId, _trackingsinfo2_itor->second.pathplaninfo->m_runwayextendwpslatlng, _trackingsinfo2_itor->second.cl);
			}
			if (!_trackingsinfo2_itor->second.m_tracking_osm_path_infoData.empty())
			{
				QColor color = QColor(255, 0, 255, 255);
				m_mapWidget->drawTrackingPoints(_trackingsinfo2_itor->second.m_tracking_osm_path_infoId, _trackingsinfo2_itor->second.m_tracking_osm_path_infoData, color, 8.0);
			}
			if (!_trackingsinfo2_itor->second.m_tracking_osm_path_infoId_calibrateData.empty())
			{
				QColor color = QColor(255, 255, 0, 255);
				m_mapWidget->drawTrackingPoints(_trackingsinfo2_itor->second.m_tracking_osm_path_infoId_calibrate, _trackingsinfo2_itor->second.m_tracking_osm_path_infoId_calibrateData, color, 12.0);
			}
		}
	}
	break;
	case PathPanel::E_PATH_OPERATE_TYPE_UNVISIABLE:
	{
		auto _trackingsinfo_itor = m_trackingsinfo.find(trackingname);
		if (_trackingsinfo_itor != m_trackingsinfo.end())
		{
			if (!_trackingsinfo_itor->second.pathplaninfo->m_trackinglatlng.empty())
			{
				std::vector<LAT_LNG>tmp;
				m_mapWidget->drawTrackingLine(_trackingsinfo_itor->second.m_oriTrackingId, tmp, _trackingsinfo_itor->second.cl);
			}
			if (!_trackingsinfo_itor->second.pathplaninfo->m_extendwpslatlng.empty())
			{
				std::vector<LAT_LNG>tmp;
				m_mapWidget->drawTrackingLine(_trackingsinfo_itor->second.m_extendTrackingId, tmp, _trackingsinfo_itor->second.cl);
			}

			if (!_trackingsinfo_itor->second.pathplaninfo->m_runwayextendwpslatlng.empty())
			{
				std::vector<LAT_LNG>tmp;
				m_mapWidget->drawTrackingDashedLine(_trackingsinfo_itor->second.m_tracking_runway_extendId, tmp, _trackingsinfo_itor->second.cl);
			}

			if (!_trackingsinfo_itor->second.m_tracking_osm_path_infoData.empty())
			{
				QColor color = QColor(0, 255, 0, 255);
				std::vector<LAT_LNG>tmp;
				m_mapWidget->drawTrackingPoints(_trackingsinfo_itor->second.m_tracking_osm_path_infoId, tmp, color, 8.0);
			}

			if (!_trackingsinfo_itor->second.m_tracking_osm_path_infoId_calibrateData.empty())
			{
				QColor color = QColor(0, 0, 255, 255);
				std::vector<LAT_LNG>tmp;
				m_mapWidget->drawTrackingPoints(_trackingsinfo_itor->second.m_tracking_osm_path_infoId_calibrate, tmp, color, 10.0);
			}
		}
	}
	break;
	default:
		break;
	}

}

void TrackingWidget::select_airport_slot(const QString& airport_code)
{
	DataManager::getInstance().setCurrentAirport(airport_code);
	refreshTracking();


	tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getCurrentAirportInfo();
	if (ptagAirPortInfo)
	{
		m_runways->clear();
		QStringList vallist1 = ptagAirPortInfo->m_runways;
		vallist1.push_front("all");
		m_runways->addItems(vallist1);
		m_runways->setCurrentIndex(0);


		QStringList vallist2;
		vallist2.append("all");

		std::map<QString, ARR_DEP_RUNWAY_PATH>& pathplans = ptagAirPortInfo->m_Path_Plans;

		auto pathplans_itor = pathplans.begin();
		while (pathplans_itor != pathplans.end())
		{
			const QString& parkingpoint = pathplans_itor->first;
			vallist2.append(parkingpoint);
			pathplans_itor++;
		}
		m_parkingpoint->clear();
		m_parkingpoint->addItems(vallist2);
		m_parkingpoint->setCurrentIndex(0);

#if 0
		std::vector<LAT_LNG> wayptsOSM;
		std::vector<LAT_LNG> wayptsOSMextend;
		std::vector<LAT_LNG> wayptsPOI;
		QColor color = QColor(255, 0, 255, 255);
		QColor colorextend = QColor(255, 0, 0, 255);
		QColor colorPOI = QColor(0, 255, 255, 255);
		UINT64 trackingid = FunctionAssistant::generate_random_positive_uint64();
		UINT64 trackingextendid = FunctionAssistant::generate_random_positive_uint64();
		UINT64 trackingPOIid = FunctionAssistant::generate_random_positive_uint64();

		auto& linestringpoints = ptagAirPortInfo->m_linestringpoints;

		wayptsOSM.reserve(linestringpoints.size());
		wayptsOSMextend.reserve(linestringpoints.size());
		wayptsPOI.reserve(linestringpoints.size());
		auto itor3 = linestringpoints.begin();
		while (itor3 != linestringpoints.end())
		{
			switch (std::get<1>(itor3->second))
			{
			case E_POINT_TYPE_OSM:
			{
				wayptsOSM.push_back(std::get<0>(itor3->second));
			}break;
			case E_POINT_TYPE_OSM_EXTEND:
			{
				wayptsOSMextend.push_back(std::get<0>(itor3->second));
			}break;
			case E_POINT_TYPE_POI:
			case E_POINT_TYPE_POI_EXTEND_OSM:
			case E_POINT_TYPE_POI_EXTEND_OSM_EXTEND:
			case E_POINT_TYPE_POI_EXTEND_POI:
			case E_POINT_TYPE_POI_EXTEND_POI_EXTEND:
			{
				wayptsPOI.push_back(std::get<0>(itor3->second));
			}break;
			default:
				break;
			}
			itor3++;
		}
		m_mapWidget->drawTrackingPoints(trackingid, wayptsOSM, color, 3.0);
		m_mapWidget->drawTrackingPoints(trackingextendid, wayptsOSMextend, colorextend, 1.0);
		//m_mapWidget->drawTrackingPoints(trackingPOIid, wayptsPOI, colorPOI, 10.0);

		std::vector<LAT_LNG> waypts1demo;
		waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
		waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
		QColor colorPOIdemo = QColor(0, 255, 255, 255);
		UINT64 trackingiddemo = FunctionAssistant::generate_random_positive_uint64();
		m_mapWidget->drawTrackingLine(trackingiddemo, waypts1demo, colorPOIdemo, 1.0);
#endif

		//std::vector<std::tuple<LAT_LNG, UINT64>> ret;
		//DataManager::getInstance().getExtendWpsExName2Name("A#T2", "W02L#A10", ret);
		//if (!ret.empty())
		//{
		//	std::vector<LAT_LNG> wayptspath;
		//	//QColor color = FunctionAssistant::randColor(255);
		//	//QColor colorextend = FunctionAssistant::randColor(255);
		//	QColor colorpath = QColor(0, 255, 0, 255);
		//	UINT64 trackingpathid = FunctionAssistant::generate_random_positive_uint64();
		//	wayptspath.resize(ret.size());
		//	for (int j = 0; j < ret.size(); j++)
		//	{
		//		wayptspath[j] = std::get<0>(ret.at(j));
		//	}
		//	m_mapWidget->drawTrackingLine(trackingpathid, wayptspath, colorpath, 10.0);
		//}
	}
}

void TrackingWidget::showEvent(QShowEvent *event)
{
	m_mapWidget->setVisible(true);
	m_pPathPanel->setVisible(true);


	tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getCurrentAirportInfo();
	if (ptagAirPortInfo)
	{
		m_runways->clear();
		QStringList vallist1 = ptagAirPortInfo->m_runways;
		vallist1.push_front("all");
		m_runways->addItems(vallist1);
		m_runways->setCurrentIndex(0);


		QStringList vallist2;
		vallist2.append("all");

		std::map<QString, ARR_DEP_RUNWAY_PATH>& pathplans = ptagAirPortInfo->m_Path_Plans;

		auto pathplans_itor = pathplans.begin();
		while (pathplans_itor != pathplans.end())
		{
			const QString& parkingpoint = pathplans_itor->first;
			vallist2.append(parkingpoint);
			pathplans_itor++;
		}
		m_parkingpoint->clear();
		m_parkingpoint->addItems(vallist2);
		m_parkingpoint->setCurrentIndex(0);
		if (!m_bInit)
		{
			QColor cl = FunctionAssistant::randColor(255);
			uint64_t item_id;
			auto itor2 = ptagAirPortInfo->m_poiitemsmap.begin();
			while (itor2 != ptagAirPortInfo->m_poiitemsmap.end())
			{
				auto& subcoordinatesitem = itor2->second.poipoint;
				item_id = FunctionAssistant::generate_random_positive_uint64();
				auto _poisinfo_itor = m_poisinfo.find(itor2->first);
				if (_poisinfo_itor != m_poisinfo.end())
				{
					_poisinfo_itor->second = item_id;
				}
				else
				{
					m_poisinfo.insert(std::make_pair(itor2->first, item_id));
				}
				m_mapWidget->drawPoint(item_id, subcoordinatesitem, itor2->first, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
				//m_mapWidget->drawText(item_id, subcoordinatesitem, itor2->first,DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
				itor2++;
			}


#if 0
			std::vector<LAT_LNG> wayptsOSM;
			std::vector<LAT_LNG> wayptsOSMextend;
			std::vector<LAT_LNG> wayptsPOI;
			QColor color = QColor(255, 0, 255, 255);
			QColor colorextend = QColor(255, 0, 0, 255);
			QColor colorPOI = QColor(0, 255, 255, 255);
			UINT64 trackingid = FunctionAssistant::generate_random_positive_uint64();
			UINT64 trackingextendid = FunctionAssistant::generate_random_positive_uint64();
			UINT64 trackingPOIid = FunctionAssistant::generate_random_positive_uint64();

			auto& linestringpoints = ptagAirPortInfo->m_linestringpoints;

			wayptsOSM.reserve(linestringpoints.size());
			wayptsOSMextend.reserve(linestringpoints.size());
			wayptsPOI.reserve(linestringpoints.size());
			auto itor3 = linestringpoints.begin();
			while (itor3 != linestringpoints.end())
			{
				switch (std::get<1>(itor3->second))
				{
				case E_POINT_TYPE_OSM:
				{
					wayptsOSM.push_back(std::get<0>(itor3->second));
				}break;
				case E_POINT_TYPE_OSM_EXTEND:
				{
					wayptsOSMextend.push_back(std::get<0>(itor3->second));
				}break;
				case E_POINT_TYPE_POI:
				case E_POINT_TYPE_POI_EXTEND_OSM:
				case E_POINT_TYPE_POI_EXTEND_OSM_EXTEND:
				case E_POINT_TYPE_POI_EXTEND_POI:
				case E_POINT_TYPE_POI_EXTEND_POI_EXTEND:
				{
					wayptsPOI.push_back(std::get<0>(itor3->second));
				}break;
				default:
					break;
				}
				itor3++;
			}
			m_mapWidget->drawTrackingPoints(trackingid, wayptsOSM, color, 3.0);
			m_mapWidget->drawTrackingPoints(trackingextendid, wayptsOSMextend, colorextend, 1.0);
			//m_mapWidget->drawTrackingPoints(trackingPOIid, wayptsPOI, colorPOI, 10.0);

			std::vector<LAT_LNG> waypts1demo;
			waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
			waypts1demo.push_back(LAT_LNG{ 23.4099197,113.3146023 });
			QColor colorPOIdemo = QColor(0, 255, 255, 255);
			UINT64 trackingiddemo = FunctionAssistant::generate_random_positive_uint64();
			m_mapWidget->drawTrackingLine(trackingiddemo, waypts1demo, colorPOIdemo, 1.0);
#endif
			m_bInit = true;
		}

		//std::vector<std::tuple<LAT_LNG, UINT64>> ret;
		//DataManager::getInstance().getExtendWpsExName2Name("A#T2", "W02L#A10", ret);
		//if (!ret.empty())
		//{
		//	std::vector<LAT_LNG> wayptspath;
		//	//QColor color = FunctionAssistant::randColor(255);
		//	//QColor colorextend = FunctionAssistant::randColor(255);
		//	QColor colorpath = QColor(0, 255, 0, 255);
		//	UINT64 trackingpathid = FunctionAssistant::generate_random_positive_uint64();
		//	wayptspath.resize(ret.size());
		//	for (int j = 0; j < ret.size(); j++)
		//	{
		//		wayptspath[j] = std::get<0>(ret.at(j));
		//	}
		//	m_mapWidget->drawTrackingLine(trackingpathid, wayptspath, colorpath, 10.0);
		//}
	}
	QWidget::showEvent(event);
}

void TrackingWidget::hideEvent(QHideEvent *event)
{
	m_mapWidget->setVisible(false);
	m_pPathPanel->setVisible(false);
	QWidget::hideEvent(event);
}

void TrackingWidget::resizeEvent(QResizeEvent *event)
{
	m_mapWidget->setGeometry(0, 0, this->width(), this->height());

	m_btnRefresh->setGeometry(0, this->height() - this->height() / 4 - 40, 36, 36);

	m_srccombox->setGeometry(m_btnRefresh->geometry().right() + 10, this->height() - this->height() / 4 - 40, 120, 36);
	m_arrtype->setGeometry(m_srccombox->geometry().right() + 10, this->height() - this->height() / 4 - 40, 120, 36);
	m_runways->setGeometry(m_arrtype->geometry().right() + 10, this->height() - this->height() / 4 - 40, 120, 36);
	m_parkingpoint->setGeometry(m_runways->geometry().right() + 10, this->height() - this->height() / 4 - 40, 120, 36);

	m_pPathPanel->setGeometry(0, this->height() - this->height() / 4, this->width(), this->height() / 4);

	QWidget::resizeEvent(event);
}
