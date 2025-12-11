#pragma execution_character_set("utf-8")
#include "runningwidget.h"

#include "../components/eventdriver/eventdriver.h"
#include <QVBoxLayout>

#include "mapwidget.h"
#include "playwidget.h"

#include "src/algorithm/Dijkstra.h"

#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QCoreApplication>
#include "runtimelistwidget.h"
#include "../components/configmanager.h"
#include "runningmodeconfig.h"
#include "settingsconfig.h"

#include <QDir>
RunningWidget::RunningWidget(QWidget* parent)
	:QWidget(parent),
	m_qmlWidget(nullptr),
	m_mapWidget(nullptr),
	m_bInit(false),
	m_pRuntimeListWidget(nullptr)
{


	this->setStyleSheet("RunningWidget{background-color:#2e2f30;}");
    m_mapWidget = new MapWidget(MapWidget::E_MAP_MODE_DISPLAY, this);
	m_mapWidget->hide();

	qRegisterMetaType<TYPE_ULID>("TYPE_ULID");
	qRegisterMetaType<transdata_entityposinfo>("transdata_entityposinfo");

	m_qmlWidget = new PlayWidget(PlayWidget::E_PLAY_MODE_REALTIME, m_mapWidget);

	connect(m_qmlWidget, &PlayWidget::btn_click_sig, this, &RunningWidget::btn_click_slot);

	connect(this, &RunningWidget::setSliderValue_sig, this, &RunningWidget::setSliderValue_slot);

	connect(m_mapWidget, &MapWidget::select_airport_sig, this, &RunningWidget::select_airport_slot);

	connect(this, &RunningWidget::sim_displayHexidxPosCallback_sig, m_mapWidget, &MapWidget::sim_displayHexidxPosCallback_slot);

#if 0

	m_btnRefresh = creatToolButton("takeoff.svg");
	m_btnRefresh->show();
	m_srccombox = new QComboBox(m_mapWidget);
	m_srccombox->show();

	QStringList vallist;
	//vallist << "all";
	vallist << "W01#F10";
	vallist << "W02L#A10";
	m_srccombox->addItems(vallist);
	m_srccombox->setCurrentIndex(0);


	connect(m_btnRefresh, &QPushButton::clicked, this, [&]() {
		QString vallist1 = m_srccombox->currentText();
		QJsonObject jsobj;
		jsobj.insert("poiname", vallist1);
		jsobj.insert("poicmd", "takeoff");
		if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_poicmd(jsobj))
		{
			std::cout << "http error" << std::endl;
		}
	});
#endif



	///////////////////////////////////////////////////////////////////////////


	auto readWpsRunwayPath = [&](const QString& path)
	{
		auto readWpsRunwayFile = [&](const std::string& filename)
		{

			auto appendwpstracking = [&](int other_tags, int order_tags, LATLNGS_VECTOR sublatlnglisttmp) {


				///////////////////////////////////////////////////////////////////////////////////////////

				LATLNGS_VECTOR sublatlnglists;

				auto addwps = [&sublatlnglists](double lng, double lat, double alt, double alttype, double timestamp) {
					sublatlnglists.push_back(LAT_LNG{ lat , lng });
				};

				auto equal = [](LAT_LNG& newpt, LAT_LNG& oldpt)
				{
					const double eps = 0.0000001f;

					return ((fabs(newpt.lat - oldpt.lat) < eps) & (fabs(newpt.lng - oldpt.lng) < eps));
				};

				bool bExtendBegin = false;
				if (other_tags == 1)
				{
					bExtendBegin = false;
				}
				else
				{
					bExtendBegin = true;
				}

				LAT_LNG lastlatlng;
				glm::vec3 lastvec;
				int icount = 0;
				LATLNGS_VECTOR sublatlnglist;
				if (order_tags == 1)
				{
					sublatlnglist.insert(sublatlnglist.begin(), sublatlnglisttmp.begin(), sublatlnglisttmp.end());
				}
				else
				{
					sublatlnglist.insert(sublatlnglist.begin(), sublatlnglisttmp.rbegin(), sublatlnglisttmp.rend());
				}
				for (int i = 0; i < sublatlnglist.size(); i++)
				{
					bool bApeend = false;
					LAT_LNG latlng = sublatlnglist.at(i);
					if (i == 0)
					{
						if (bExtendBegin)
						{
							LAT_LNG newxtlatlng = sublatlnglist.at(i + 1);
							glm::vec3 directionVector = FunctionAssistant::calculateVector(newxtlatlng, latlng);
							LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(latlng, directionVector, 3000);

							addwps(extendpt.lng, extendpt.lat, 0, 0, icount * 10);
							icount++;
						}
						lastlatlng = latlng;
						bApeend = true;
					}
					else if (i == 1)
					{
						lastvec = FunctionAssistant::calculateVector(lastlatlng, latlng);
						bApeend = true;
						lastlatlng = latlng;
					}
					else
					{
						if (equal(lastlatlng, latlng))
						{
							continue;
						}
						glm::vec3 newvec = FunctionAssistant::calculateVector(lastlatlng, latlng);

						lastlatlng = latlng;

						float dot = glm::dot(newvec, lastvec);
						if (dot > 0)
						{
							//std::cout << "Vectors in the same direction." << std::endl;
							bApeend = true;
						}
						else if (dot < 0) {
							std::cout << "Vectors in the opposite direction." << std::endl;
							bApeend = false;
						}
						else {
							std::cout << "Vectors orthogonal (perpendicular)." << std::endl;
							bApeend = false;
						}
						lastvec = newvec;
					}
					if (bApeend)
					{
						addwps(latlng.lng, latlng.lat, 0, 0, icount * 10);
						icount++;
					}
				}
				if (!bExtendBegin)
				{
					LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(lastlatlng, lastvec, 3000);
					addwps(extendpt.lng, extendpt.lat, 0, 0, icount * 10);
					icount++;
				}
				LATLNGS_VECTOR sublatlnglists_ret;
				LAT_LNG lstlatlng;
				double m_point_extend_metres = 1.0;

				for (int cc = 0; cc < sublatlnglists.size(); cc++)
				{
					if (bExtendBegin)
					{
						if (cc <= 1)
						{
							m_point_extend_metres = 6.66;
						}
						else
						{
							m_point_extend_metres = 1.0;
						}
					}
					else
					{
						if (cc >= sublatlnglists.size() - 2)
						{
							m_point_extend_metres = 6.66;
						}
						else
						{
							m_point_extend_metres = 1.0;
						}
					}
					LAT_LNG latlng = sublatlnglists.at(cc);
					if (cc == 0)
					{
						sublatlnglists_ret.push_back(sublatlnglists.at(cc));
					}
					else
					{
						double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
						if (dis > m_point_extend_metres)
						{
							int step = dis / m_point_extend_metres;
							glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
							LAT_LNG lstextendpt = lstlatlng;
							for (int j = 1; j < step + 1; j++)
							{
								LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, m_point_extend_metres);
								sublatlnglists_ret.push_back(currentextendpt);
								lstextendpt = currentextendpt;
							}
							double rdis = dis - m_point_extend_metres * step;

							sublatlnglists_ret.push_back(latlng);
						}
						else
						{
							sublatlnglists_ret.push_back(latlng);
						}
					}
					lstlatlng = latlng;
				}
				std::cout << "get pts " << sublatlnglists_ret.size() << std::endl;
				auto osm_id = FunctionAssistant::generate_random_positive_uint64();

				///////////////////////////////////////////////////////////////////////////////////////////
				m_wps.insert(std::make_pair(osm_id, std::make_tuple(other_tags, 0, std::move(sublatlnglists_ret))));
			};

			QFileInfo fileinfo(filename.c_str());
			if (fileinfo.exists())
			{
				QString namefile = fileinfo.baseName();
				auto jsonobj = FunctionAssistant::read_json_file_object(QString::fromStdString(filename));
				GeoJsonInfos geoinfos;
				auto featuresarray = jsonobj.value("features").toArray();
				for (auto featuresarrayitem : featuresarray)
				{
					auto properties = featuresarrayitem.toObject().value("properties").toObject();
					auto geometry = featuresarrayitem.toObject().value("geometry").toObject();

					auto coordinatestype = geometry.value("type").toString().toLower();

					if (coordinatestype == "multilinestring")
					{
						int other_tags = properties.value("ftype").toInt();
						int order_tags = properties.value("order").toInt();

						auto parentcoordinatesarray = geometry.value("coordinates").toArray();
						for (auto parentcoordinatesarrayitem : parentcoordinatesarray)
						{
							auto coordinatesarray = parentcoordinatesarrayitem.toArray();
							LATLNGS_VECTOR sublatlnglisttmp;
							sublatlnglisttmp.reserve(coordinatesarray.size());
							for (auto coordinatesitem : coordinatesarray)
							{

								PARSE_LATLNG_FROME_JSON(sublatlnglisttmp, coordinatesitem);
							}
							appendwpstracking(other_tags, order_tags, sublatlnglisttmp);
						}
					}
					else if (coordinatestype == "linestring")
					{
						int other_tags = properties.value("ftype").toInt();
						int order_tags = properties.value("order").toInt();

						auto coordinatesarray = geometry.value("coordinates").toArray();
						LATLNGS_VECTOR sublatlnglisttmp;
						sublatlnglisttmp.reserve(coordinatesarray.size());
						for (auto coordinatesitem : coordinatesarray)
						{
							PARSE_LATLNG_FROME_JSON(sublatlnglisttmp, coordinatesitem);
						}
						appendwpstracking(other_tags, order_tags, sublatlnglisttmp);


					}
				}
			}
		};

		QFileInfo fileInfo(path);
		if (fileInfo.isDir())
		{
			QDir dir(path);
			dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
			QFileInfoList list = dir.entryInfoList();
			for (int i = 0; i < list.size(); i++)
			{
				readWpsRunwayFile(list.at(i).filePath().toStdString());
			}
		}
	};

	readWpsRunwayPath(QCoreApplication::applicationDirPath() + "/data/wps");

}

RunningWidget::~RunningWidget()
{
	if (m_qmlWidget)
	{
		m_qmlWidget->deleteLater();
	}
	if (m_mapWidget)
	{
		m_mapWidget->deleteLater();
	}
}

void RunningWidget::updateText(const QString& context)
{
	if (m_mapWidget)
	{
		m_mapWidget->updateText(context);
	}
}

void RunningWidget::updateSpeed(const float fspeed)
{
	m_qmlWidget->setReadSpeedContext(fspeed);
}

void RunningWidget::setEventDriver(EventDriver* _peventDriver)
{
	m_peventDriver = _peventDriver;
	m_mapWidget->setEventDriver(m_peventDriver);
}

QToolButton* RunningWidget::creatToolButton(const QString& icon)
{
	QToolButton* btn = new QToolButton(m_mapWidget);
	btn->resize(QSize(36, 36));
	btn->setIconSize(QSize(36, 36));
	btn->setAutoRaise(true);
	btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	QIcon ic = QIcon(QCoreApplication::applicationDirPath() + "/res/svg/" + icon);
	btn->setIcon(ic);
	return btn;
}

void RunningWidget::setSliderRange(int min, int max)
{
	if (m_qmlWidget)
	{
		m_qmlWidget->setSliderRange(min, max);
	}
}

void RunningWidget::setSliderValue(uint64_t val)
{
	emit setSliderValue_sig(val);
}

void RunningWidget::addSliderValue(uint64_t val)
{
	emit setSliderValue_sig(DataManager::getInstance().m_play_cur);
}

void RunningWidget::driverSimTracking()
{
	auto  cc = m_wps.begin();
	while (cc != m_wps.end())
	{
		UINT64 id = cc->first;
		std::tuple<int, int, LATLNGS_VECTOR>& trckinginfo = cc->second;
		int& trackingindx = std::get<1>(trckinginfo);
		auto& trackingpts = std::get<2>(trckinginfo);
		double yaw = 0;
		LAT_LNG curlstpt;

		H3INDEX h3index = 0;
		transdata_entityposinfo entityinfo;
		bool bUpdate = false;

		double spd = 10.0;
		if (trackingindx < trackingpts.size())
		{
			if (trackingindx == 0)
			{
				LAT_LNG lstpt = trackingpts.at(1);
                curlstpt = trackingpts.at(trackingindx);
                LAT_LNG pta{curlstpt.lat, curlstpt.lng};
                LAT_LNG ptb{lstpt.lat, lstpt.lng};
				yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(pta, ptb));

				if (std::get<1>(trckinginfo) == 1)
				{
					spd = 10.0;
				}
				else
				{
					spd = 66.66;
				}
			}
			else
			{
				LAT_LNG lstpt = trackingpts.at(trackingindx - 1);

				curlstpt = trackingpts.at(trackingindx);

                LAT_LNG pta{lstpt.lat, lstpt.lng};
                LAT_LNG ptb{curlstpt.lat, curlstpt.lng};
				yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(pta, ptb));

				double curdis = FunctionAssistant::calc_dist(curlstpt, lstpt);

				UINT64 timeinterval = 100;//QDateTime::currentMSecsSinceEpoch() - entityrunninginfo.lsttimetamp;
				double sec = (double)(timeinterval) / 1000.0f;
				spd = curdis / sec;

			}
			LocationHelper::getIndexInfo(h3index, curlstpt.lat, curlstpt.lng, INDEX_MAPPING_RESOLUTION_ENTITY_POS);
			bUpdate = true;
		}
		else
		{
			if (trackingindx == trackingpts.size())
			{
				bUpdate = true;
			}
			if (trackingindx == trackingpts.size() + 10)
			{
				trackingindx = -1;
			}
		}
		trackingindx++;

		if (bUpdate)
		{
			entityinfo.PARAM_pos_pack_index = trackingindx;
			entityinfo.PARAM_timestamp = QDateTime::currentMSecsSinceEpoch();
			entityinfo.PARAM_pos_hexidx = h3index;
			entityinfo.PARAM_longitude = curlstpt.lng * LON_LAT_ACCURACY;
			entityinfo.PARAM_latitude = curlstpt.lat * LON_LAT_ACCURACY;
			entityinfo.PARAM_amsl = 0;
			entityinfo.PARAM_ref = 0;
			entityinfo.PARAM_roll = 0;
			entityinfo.PARAM_pitch = 0;
			entityinfo.PARAM_yaw = yaw;

			entityinfo.PARAM_speed = spd * 1000;
			entityinfo.PARAM_reserved[0] = 0;
			entityinfo.PARAM_reserved[1] = 1;
			entityinfo.PARAM_reserved[2] = 2;
			entityinfo.PARAM_reserved[3] = 0xff;

			PROPERTY_SET_TYPE(entityinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_NORMAL);
			if (std::get<0>(cc->second) == 1)
			{
				PROPERTY_SET_RESERVED(entityinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_RESERVED_TYPE1);
			}
			else
			{
				PROPERTY_SET_RESERVED(entityinfo.PARAM_sensor_property, AGENT_ENTITY_PROPERTY_RESERVED_TYPE2);
			}
			//emit sim_displayHexidxPosCallback_sig(id, entityinfo);
		}
		cc++;
	}

}

void RunningWidget::addSliderValue_dt(double val)
{
	//	DataManager::getInstance().m_play_cur_d += val;
	//    DataManager::getInstance().m_play_cur = DataManager::getInstance().m_play_min + DataManager::getInstance().m_play_cur_d;
		//	emit setSliderValue_sig(DataManager::getInstance().m_play_cur);
}

void RunningWidget::setPlayClickSlot(bool bPlay)
{
	if (m_qmlWidget)
	{
		m_qmlWidget->setPlayClickSlot(bPlay);
	}
}
void RunningWidget::setRuntimeListWidget(RuntimeListWidget* pRuntimeListWidget)
{
    m_pRuntimeListWidget = pRuntimeListWidget;
}

void RunningWidget::setQtOSGWidget(QtOSGWidget* pModelWidget2)
{
	if (m_mapWidget)
	{
		m_mapWidget->setQtOSGWidget(pModelWidget2);
	}
}


void RunningWidget::setPauseClickSlot(bool bPause)
{
	if (m_qmlWidget)
	{
		m_qmlWidget->setPauseClickSlot(bPause);
	}
}

void RunningWidget::updatepoitextcolor_slot(const QString& poiname, const QColor& color, float textsize)
{
    if(m_poisinfo.find("CAN") != m_poisinfo.end())
    {
        tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo("CAN");
        if (ptagAirPortInfo)
        {
            auto itor2 = m_poisinfo.at("CAN").find(poiname);
            if (itor2 != m_poisinfo.at("CAN").end())
            {
                auto item_id = itor2->second.m_id;
                auto& subcoordinatesitem = itor2->second.m_pos;
                m_mapWidget->updateTextColor(item_id, subcoordinatesitem, itor2->first, textsize, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT, color);
            }
        }
    }
}



void RunningWidget::setSliderValue_slot(uint64_t val)
{
	if (val >= DataManager::getInstance().m_play_min && val <= DataManager::getInstance().m_play_max)
	{
		if (m_qmlWidget)
		{
			m_qmlWidget->setSliderValue(val);
		}
	}
}

void RunningWidget::btn_click_slot(E_PLAY_OPERATE_TYPE type)
{
	auto find_target_event = [](uint64_t timestamp)->tagFlightEventTime*
	{
		tagFlightEventTime* pflightevent = nullptr;

		std::map<uint64_t, tagFlightEventTime>& day_flight_events = DataManager::getInstance().total_flightEventTimedata;

		auto day_flight_events_itor = day_flight_events.begin();
		while (day_flight_events_itor != day_flight_events.end())
		{
			if (day_flight_events_itor->first <= timestamp)
			{
				tagFlightEventTime& flightevent = day_flight_events_itor->second;
				pflightevent = &flightevent;
			}
			else
			{
				break;
			}
			day_flight_events_itor++;
		}
		return pflightevent;
	};

	auto start_event_trigger = [&](tagFlightEventTime* pflightevent)
	{
		if (m_peventDriver && pflightevent)
		{
			tagEventInfo eventinfo;
			eventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE_TRIGGER;
			eventinfo.m_eventId = pflightevent->m_eventid;
			eventinfo.timeout = 1000;// pflightevent->m_day_senscod_offset_ms;
			eventinfo.repeattimes = 0;
			eventinfo.m_eventtype = E_EVENT_TYPE_ID_RUNTIME;
			eventinfo.flightevent = pflightevent;
			eventinfo.bEnableAdjustSpeed = true;
			m_peventDriver->addevent(eventinfo);
		}
	};

	switch (type)
	{
	case E_PLAY_OPERATE_TYPE_ADJUST_SPEED:
	{
		if (m_peventDriver)
		{
			float ff = m_peventDriver->speed_coeff();
			float ff_new = m_qmlWidget->getSpeedContext();
			if (fabs(ff - ff_new) > 1E-7)
			{
				m_peventDriver->set_speed_coeff(ff_new);
			}
		}
	}
	break;
	case E_PLAY_OPERATE_TYPE_PLAY:
    {
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
            {
                runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
            }
            runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-hub");
        }
	}
	break;
	case E_PLAY_OPERATE_TYPE_TERMINATION:
	{
		if (m_peventDriver)
		{
            m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP);
            DataManager::getInstance().exportexcel();
        }
        if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
            {
                runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
            }

            runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
        }
	}
	break;

	case E_PLAY_OPERATE_TYPE_PASUE:
	{
		if (m_peventDriver)
		{
			m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_PAUSE);
		}
	}
	break;
	case E_PLAY_OPERATE_TYPE_RESUME:
	{
		if (m_peventDriver)
		{
			m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_RESUME);
		}
	}
	break;

	case E_PLAY_OPERATE_TYPE_ADJUST_PERCENT:
	{
		if (m_peventDriver)
		{
			m_peventDriver->event_stop();

			auto adjust_pos = m_qmlWidget->getSliderAdjust();
			DataManager::getInstance().m_play_cur = adjust_pos;
			auto adjust_pos_timestamp = adjust_pos + DataManager::getInstance().m_play_min;

			tagFlightEventTime* pflightevent = find_target_event(adjust_pos_timestamp);
			if (pflightevent)
			{
				start_event_trigger(pflightevent);
			}
		}
	}
	break;


	default:
		break;
	}

}


void RunningWidget::select_airport_slot(const QString& airport_code)
{
	//std::vector<LAT_LNG> ret = DataManager::getInstance().getpath(LAT_LNG{ 40.0814574 ,116.5809071 }, LAT_LNG{ 40.1001822 ,116.5705704 });

	//if (!ret.empty())
	//{
	//	std::cout << "GET PATH  size" << ret.size() << std::endl;
	//	m_mapWidget->drawTrackingLine(FunctionAssistant::generate_random_positive_uint64(), ret, QColor(255, 0, 0, 255), 10.0);
	//}
}

void RunningWidget::showEvent(QShowEvent* event)
{
	m_qmlWidget->show();
	m_mapWidget->setVisible(true);

	if (!m_bInit)
	{
		auto airport_codes = DataManager::getInstance().getAirPortList();
		for (auto airport_code_item : airport_codes)
		{
			tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo(airport_code_item);
			if (ptagAirPortInfo)
			{
				if (m_poisinfo.find(airport_code_item) == m_poisinfo.end())
				{
					m_poisinfo.insert(std::make_pair(airport_code_item, std::unordered_map<QString, tagPoiShowinfo>()));
				}

				std::unordered_map<QString, tagPoiShowinfo>& _poisinfo = m_poisinfo.at(airport_code_item);

				QColor cl = FunctionAssistant::randColor(255);
				uint64_t item_id;
				auto itor2 = ptagAirPortInfo->m_poiitemsmap.begin();
				while (itor2 != ptagAirPortInfo->m_poiitemsmap.end())
				{
					auto& subcoordinatesitem = itor2->second.poipoint;
					item_id = FunctionAssistant::generate_random_positive_uint64();
					tagPoiShowinfo poiinfo = tagPoiShowinfo(item_id, subcoordinatesitem, itor2->first);
					auto _poisinfo_itor = _poisinfo.find(itor2->first);
					if (_poisinfo_itor != _poisinfo.end())
					{
						_poisinfo_itor->second = std::move(poiinfo);
					}
					else
					{
						_poisinfo.insert(std::make_pair(itor2->first, std::move(poiinfo)));
					}
					m_mapWidget->drawPoint(_poisinfo.at(itor2->first).m_id, _poisinfo.at(itor2->first).m_pos, QString(), DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
					//m_mapWidget->drawText(m_poisinfo.at(itor2->first).id, m_poisinfo.at(itor2->first).pos, QString(),DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
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
			}
		}

		register_sim_tracking();
		m_bInit = true;
	}

	QWidget::showEvent(event);
}

void RunningWidget::hideEvent(QHideEvent* event)
{
	m_qmlWidget->hide();
	m_mapWidget->setVisible(false);
	QWidget::hideEvent(event);
}

void RunningWidget::resizeEvent(QResizeEvent* event)
{
	m_mapWidget->setGeometry(0, 0, this->width(), this->height());
	m_qmlWidget->setGeometry(0, this->height() - this->height() / 15, this->width(), this->height());
#if 0
	m_btnRefresh->setGeometry(0, this->height() - this->height() / 15 - 40, 36, 36);

	m_srccombox->setGeometry(m_btnRefresh->geometry().right() + 10, this->height() - this->height() / 15 - 40, 120, 36);
#endif
	QWidget::resizeEvent(event);
}

void RunningWidget::setPoiPoint(bool bVisable)
{
	auto _airport_itor = m_poisinfo.begin();
	while (_airport_itor != m_poisinfo.end())
	{
		std::unordered_map<QString, tagPoiShowinfo>& _poisinfo = _airport_itor->second;
		auto _poisinfo_itor = _poisinfo.begin();
		while (_poisinfo_itor != _poisinfo.end())
		{
			if (bVisable)
			{
				m_mapWidget->drawPoint(_poisinfo_itor->second.m_id, _poisinfo_itor->second.m_pos, QString(), DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
			}
			else
			{
				m_mapWidget->drawPoint(_poisinfo_itor->second.m_id, _poisinfo_itor->second.m_pos, QString(), DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT, true);
				//m_mapWidget->drawText(_poisinfo_itor->second.m_id, _poisinfo_itor->second.m_pos, _poisinfo_itor->second.m_context,true);
			}
			_poisinfo_itor++;
		}
		_airport_itor++;
	}
}

void RunningWidget::register_sim_tracking()
{
	//tagEventInfo timereventinfo;
	//timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_REPEAT_PERIOD;
	//timereventinfo.m_eventId = FunctionAssistant::generate_random_positive_uint64();
	//timereventinfo.timeout = 100;// pflightevent->m_day_senscod_offset_ms;
	//timereventinfo.repeattimes = 0;
	//timereventinfo.m_eventtype = E_EVENT_TYPE_ID_SIM_TRACKING;
	//timereventinfo.bEnableAdjustSpeed = false;
	//timereventinfo.flightevent = nullptr;
	//m_peventDriver->addevent(timereventinfo);

	//auto  cc = m_wps.begin();
	//while (cc != m_wps.end())
	//{
	//	if (std::get<0>(cc->second) == 1)
	//	{
	//		m_mapWidget->drawTrackingLine(cc->first, std::get<2>(cc->second), QColor(0, 255, 0, 255), 10.0);

	//	}
	//	else
	//	{
	//		m_mapWidget->drawTrackingLine(cc->first, std::get<2>(cc->second), QColor(255, 0, 255, 255), 10.0);
	//	}

	//	cc++;
	//}

}
