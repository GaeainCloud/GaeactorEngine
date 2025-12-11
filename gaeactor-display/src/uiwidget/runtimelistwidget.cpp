#pragma execution_character_set("utf-8")
#include "runtimelistwidget.h"

#include "../components/eventdriver/eventdriver.h"
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QJsonArray>
#include <QDateTime>
#include <QQmlContext>
#include <QStackedWidget>
#include "mapwidget.h"
#include "runtimeeditwidget.h"
#include "../components/global_variables.h"
#include <QTimer>

#include "loghelper.h"

#define DISTANCE_ARR (1000)
#define DISTANCE_DEP (800)




RuntimeListWidget::RuntimeListWidget(QWidget *parent)
	:QWidget(parent),
	m_qmlWidget(nullptr),
	m_pRuntimeEditWidget(nullptr)
{
	m_qmlWidget = new QQuickWidget();
	m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_qmlWidget->rootContext()->setContextProperty("parentWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);

	m_qmlWidget->setSource(QUrl("qrc:/qml/runtimelistwidget.qml"));

	QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
		connect(pRoot, SIGNAL(qml_add_runtime_signal()), this, SLOT(qml_add_runtime_slot()));

		connect(pRoot, SIGNAL(qml_send_agent_data_signal()), this, SLOT(qml_send_agent_data_slot()));
		//connect(m_pButton, SIGNAL(clicked(bool)), pRoot, SIGNAL(cSignal()));
	}
	m_qmlWidget->hide();
	m_pRuntimeEditWidget = new RuntimeEditWidget(this);
	m_pRuntimeEditWidget->setShow(false);

	connect(m_pRuntimeEditWidget, &RuntimeEditWidget::qml_quit_agent_edit_panel_sig, this, &RuntimeListWidget::closeModelWidget_slot);

	this->setStyleSheet("RuntimeListWidget{background-color:#2e2f30;}");

	//     m_pLayout = new QVBoxLayout(this);
	//     m_pLayout->addWidget(m_qmlWidget);
	//     m_pLayout->addWidget(m_pRuntimeEditWidget);
	//     m_pLayout->setSpacing(0);
	//     m_pLayout->setContentsMargins(0, 0, 0, 0);
	//     m_pLayout->setStretch(0, 5);
	//     m_pLayout->setStretch(1, 5);
	//     setLayout(m_pLayout);

	m_pQStackedWidget = new QStackedWidget(this);

	m_pQStackedWidget->addWidget(m_qmlWidget);
	m_pQStackedWidget->addWidget(m_pRuntimeEditWidget);

	m_pLayout = new QVBoxLayout(this);
	m_pLayout->addWidget(m_pQStackedWidget);
	m_pLayout->setSpacing(0);
	m_pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_pLayout);

	m_pQStackedWidget->setCurrentIndex(0);

	connect(m_pRuntimeEditWidget, &RuntimeEditWidget::add_runtime_style_sig, this, &RuntimeListWidget::add_runtime_style_slot);

	request_agentdata();
	request_agentruntimedata();


}

RuntimeListWidget::~RuntimeListWidget()
{
	if (m_qmlWidget)
	{
		m_qmlWidget->deleteLater();
	}
	if (m_pLayout)
	{
		m_pLayout->deleteLater();
	}
	if (m_pRuntimeEditWidget)
	{
		m_pRuntimeEditWidget->deleteLater();
	}
}

void RuntimeListWidget::runRuntimeStyle(const QVariant &runtiemstyleid)
{
	auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
		m_runtimedatas.end(),
		[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
		return vt.first == runtiemstyleid.toString();
	});
	if (_runtimedatas_itor != m_runtimedatas.end())
	{
		auto runtimestyleobj = std::get<0>(_runtimedatas_itor->second);

		QJsonObject senddata;
		senddata.insert("patternSig", runtimestyleobj.value("patternSig"));

		QJsonArray patternAgentsarr;
		auto patternAgents = runtimestyleobj.value("patternAgents").toArray();

		for (int i = 0; i < patternAgents.count(); i++)
		{
			auto patternAgentsobj = patternAgents.at(i).toObject();
			QJsonObject patternAgentsobjtmp;
			patternAgentsobjtmp.insert("agentId", patternAgentsobj.value("agentId"));
			patternAgentsobjtmp.insert("agentKey", patternAgentsobj.value("agentKey"));
			patternAgentsobjtmp.insert("agentInstId", patternAgentsobj.value("agentInstId"));
			patternAgentsobjtmp.insert("agentEntityId", patternAgentsobj.value("agentEntityId"));
			patternAgentsobjtmp.insert("agentLabel", patternAgentsobj.value("agentLabel"));
			patternAgentsobjtmp.insert("agentNote", patternAgentsobj.value("agentNote"));
			patternAgentsobjtmp.insert("agentIcon", patternAgentsobj.value("agentIcon"));
			patternAgentsobjtmp.insert("azimuth", patternAgentsobj.value("azimuth"));
			patternAgentsobjtmp.insert("speed0", patternAgentsobj.value("speed0"));
			if (patternAgentsobj.value("waypoints").isObject() && !patternAgentsobj.value("waypoints").isArray())
			{
				QJsonArray waypoints;
				waypoints.push_back(patternAgentsobj.value("waypoints"));
				patternAgentsobjtmp.insert("waypoints", waypoints);
			}
			else
			{
				patternAgentsobjtmp.insert("waypoints", patternAgentsobj.value("waypoints"));
			}

			patternAgentsarr.push_back(patternAgentsobjtmp);
		}
		senddata.insert("patternAgents", patternAgentsarr);
//		static int cc = 0;
//        //qDebug() << senddata;
//		std::cout << "running runtimestyle " << runtimestyleobj.value("patternSig").toString().toStdString() << "------------ count ------------ " << cc << std::endl;
//		cc++;
		run_agentruntime_data(senddata);
	}
}

void RuntimeListWidget::editRuntimeStyle(const QVariant &runtiemstyleid)
{
	request_agentdata();
	if (!m_pRuntimeEditWidget->isVisible())
	{
		m_pRuntimeEditWidget->setShow(true);
	}
	m_pRuntimeEditWidget->resetData();
	auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
		m_runtimedatas.end(),
		[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
		return vt.first == runtiemstyleid.toString();
	});
	if (_runtimedatas_itor != m_runtimedatas.end())
	{
		auto runtimestyleobj = std::get<0>(_runtimedatas_itor->second);
		m_pRuntimeEditWidget->decodeRuntimeStyle(runtimestyleobj);
		m_pQStackedWidget->setCurrentIndex(1);
	}
}

void RuntimeListWidget::deleteRuntimeStyle(const QVariant &runtiemstyleid)
{
	auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
		m_runtimedatas.end(),
		[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
		return vt.first == runtiemstyleid.toString();
	});
	if (_runtimedatas_itor != m_runtimedatas.end())
	{

		QJsonObject removejson;
		removejson.insert("id", _runtimedatas_itor->first);
		m_runtimedatas.erase(_runtimedatas_itor);
		delete_agentruntime_data(removejson);
	}
}


void RuntimeListWidget::deal_instagentData_slot(const QString& airport_code, const QStringList& allowRunway)
{
	//飞机实体信息
	std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();
	m_runfilghtId.clear();

	auto appendInfo = [&](const std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>& flightinfo, const AgentInstanceInfo &agentinstanceinfo, FlightPlanConf *pflighltData) {
		quint64 _runtiemstyle_id = FunctionAssistant::generate_random_positive_uint64();
		QJsonArray patternAgents;

		QJsonObject patternAgentsitem;
		patternAgentsitem.insert("azimuth", 140.0);
		patternAgentsitem.insert("speed0", RUN_SPEED);
		patternAgentsitem.insert("altitudeType", 0);
        patternAgentsitem.insert("agentKey", agentinstanceinfo.m_agentinfo.agentKeyItem.agentKey);
        patternAgentsitem.insert("ncycles", 1);

		patternAgentsitem.insert("agentId", agentinstanceinfo.m_agentinfo.agentKeyItem.agentId);

        patternAgentsitem.insert("agentInstId", agentinstanceinfo.m_agentinfo.agentKeyItem.agentNameI18n);
        patternAgentsitem.insert("agentEntityId", pflighltData->flightid);
        QString Label = QString("%1___%2").arg(pflighltData->m_PlaneNum).arg(pflighltData->m_PlaneType);
        patternAgentsitem.insert("agentLabel", Label);
		QJsonArray waypoints;
		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			patternAgentsitem.insert("agentNote", QString("Arrival_%1").arg(LANDING_AHEADTIME_MIN * 60 * 1000));
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			patternAgentsitem.insert("agentNote", QString("Departure_%1").arg(TAKEOFF_AHEADTIME_MIN * 60 * 1000));
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
		}
		patternAgentsitem.insert("agentIcon", "");
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		patternAgentsitem.insert("waypoints", waypoints);

		patternAgents.push_back(patternAgentsitem);

		QJsonObject runtimestyle;
		runtimestyle.insert("patternSig", QString::number(_runtiemstyle_id));
		runtimestyle.insert("patternAgents", patternAgents);

		auto runtimedata = runtimestyle.value("patternAgents").toArray();
		QString runtimeid = runtimestyle.value("patternSig").toString();

		auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
			m_runtimedatas.end(),
			[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
			return vt.first == runtimeid;
		});
		if (_runtimedatas_itor == m_runtimedatas.end())
		{
			QString item_name = pflighltData->m_FilghtNumber;
			QString item_time = QString("%1 %2").arg(pflighltData->m_DepArrFlag)
				.arg(pflighltData->m_Date);

			QString item_founder = QString("%1 %2 %3 %4 %5").arg(pflighltData->m_PlaneNum)
				.arg(pflighltData->m_PlaneType)
				.arg(pflighltData->m_Seat)
				.arg(pflighltData->m_Terminal)
				.arg(pflighltData->m_Runway);
			int item_num = runtimedata.count();
			bool item_status = false;
			switch (pflighltData->m_flight_dep_arr_type)
			{
			case E_FLIGHT_DEP_ARR_TYPE_ARR:
			{
				item_status = true;
				item_time = QString("%1 %2").arg(pflighltData->m_DepArrFlag)
					.arg(pflighltData->m_PlanDateTimeLanding);
			}break;
			case E_FLIGHT_DEP_ARR_TYPE_DEP:
			{
				item_status = false;
				item_time = QString("%1 %2").arg(pflighltData->m_DepArrFlag)
					.arg(pflighltData->m_PlanDateTimeTakeOff);
			}
			break;
            }
//			{
//				QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//				QMetaObject::invokeMethod(pRoot, "appendRuntimeStyle",
//					Q_ARG(QVariant, QVariant::fromValue(runtimeid)),
//					Q_ARG(QVariant, QVariant::fromValue(item_name)),
//					Q_ARG(QVariant, QVariant::fromValue(item_time)),
//					Q_ARG(QVariant, QVariant::fromValue(item_founder)),
//					Q_ARG(QVariant, QVariant::fromValue(QString::number(item_num))),
//					Q_ARG(QVariant, QVariant::fromValue(item_status)));
//			}


			m_runtimedatas.insert(std::make_pair(runtimeid, std::make_tuple(runtimestyle, false)));

			auto _runfilghtId_itor = m_runfilghtId.find(std::get<0>(flightinfo));
			if (_runfilghtId_itor == m_runfilghtId.end())
			{
				std::unordered_map<QString, tagTriggerFlightInfo> idlst;
				tagTriggerFlightInfo triggerFlightInfo;
				triggerFlightInfo.m_FilghtNumber = pflighltData->m_FilghtNumber;
				triggerFlightInfo.m_flight_dep_arr_type = pflighltData->m_flight_dep_arr_type;
				triggerFlightInfo.m_Runway = pflighltData->m_Runway;
				triggerFlightInfo.pflighltData = pflighltData;
				idlst.insert(std::make_pair(runtimeid, triggerFlightInfo));
				m_runfilghtId.insert(std::make_pair(std::get<0>(flightinfo), std::move(idlst)));
			}
			else
			{
				std::unordered_map<QString, tagTriggerFlightInfo>  &idlst = _runfilghtId_itor->second;
				tagTriggerFlightInfo triggerFlightInfo;
				triggerFlightInfo.m_FilghtNumber = pflighltData->m_FilghtNumber;
				triggerFlightInfo.m_flight_dep_arr_type = pflighltData->m_flight_dep_arr_type;
				triggerFlightInfo.m_Runway = pflighltData->m_Runway;
				triggerFlightInfo.pflighltData = pflighltData;
				idlst.insert(std::make_pair(runtimeid, triggerFlightInfo));
			}
		}
    };

	int flightcout = 0;
	//航班时刻表信息
	std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& day_flights = DataManager::getInstance().total_flightdata;
	auto day_flights_itor = day_flights.begin();
	while (day_flights_itor != day_flights.end())
	{
		FlightPlanConf *pflighltData = day_flights_itor->second;
		if (!pflighltData)
		{
			day_flights_itor++;
			continue;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////

		auto agentInstances_itor = std::find_if(agentInstances.begin(),
			agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
			return vt.second.m_agentinfo.agentInstId == pflighltData->m_PlaneNum;
		});
		if (agentInstances_itor != agentInstances.end())
		{
			const AgentInstanceInfo &agentinstanceinfo = agentInstances_itor->second;
			appendInfo(day_flights_itor->first, agentinstanceinfo, pflighltData);

			flightcout++;
		}
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		day_flights_itor++;
	}
	m_airport_code = airport_code;
	m_allowRunway = allowRunway;
    std::stringstream ss;
    ss << "仿真航班数量： " << flightcout;
    DataManager::getInstance().trans_log("读取航班时刻表： ", ss, std::stringstream());

    std::stringstream ss2;
    ss2 << "*************时间驱动事件 实例表 个数: " << m_runfilghtId.size();
    DataManager::getInstance().trans_log("读取航班时刻表： ", ss2, std::stringstream(), false);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////

}

void RuntimeListWidget::trigger_flight_to_run_slot(uint64_t triggertimestamp, const QList<tagTriggerFlightInfo>& triggerflights)
{
	auto updatewps = [&](QJsonObject &patternAgentsitem, FlightPlanConf *&pflighltData, const QJsonArray &wpstmp)
	{
		QJsonArray waypoints;
		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			QJsonObject waypointsItemTracking;

			waypointsItemTracking.insert("wpsUsage", "Tracking");
			waypointsItemTracking.insert("wpsKeyword", "Tracking");
			waypointsItemTracking.insert("wpsFrame", 0);

			waypointsItemTracking.insert("wpsGenPOIs", QJsonArray());
			waypointsItemTracking.insert("wpsPathPlanner", "");
			waypointsItemTracking.insert("wpsGenFences", QJsonArray());

			waypointsItemTracking.insert("wpsGenTimeConsumed", 12.0);
			UINT64 wps_id = FunctionAssistant::generate_random_positive_uint64();
			waypointsItemTracking.insert("wpsKey", "wps_" + QString::number(wps_id));


            QJsonArray wps;
//            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            double virtual_t = 0;
//            //起飞添加后续指定航线
//            if(!pflighltData->wps.isEmpty())
//            {
//                auto wpsjsa = FunctionAssistant::string_to_json_object(pflighltData->wps);
//                if(wpsjsa.contains("wps") && wpsjsa.value("wps").isArray())
//                {
//                    auto wps_ex =  wpsjsa.value("wps").toArray();
//                    for(int icount = 0; icount < wps_ex.size(); icount++)
//                    {
//                        auto itemjsa = wps_ex.at(icount).toArray();
//                        QJsonArray wpsCore;
//                        wpsCore.append(itemjsa.at(0));
//                        wpsCore.append(itemjsa.at(1));
//                        wpsCore.append(itemjsa.at(2));
//                        wpsCore.append(1);
//                        virtual_t += 10*icount;
//                        wpsCore.append(virtual_t);
//                        QJsonObject wps_item;
//                        wps_item.insert("wpsCore", wpsCore);
//                        wps_item.insert("useExt", 0);
//                        wps_item.insert("speed", 0.0);
//                        wps_item.insert("roll", 0.0);
//                        wps_item.insert("pitch", 0.0);
//                        wps_item.insert("yaw", 0.0);
//                        wps_item.insert("yawEx", 0.0);
//                        wps.push_back(wps_item);
//                    }
//                }
//            }
//          //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            for (int m = 0; m < wpstmp.size(); m++)
			{
                auto wpsitem = wpstmp.at(m);
                QJsonArray wpsCore = wpsitem.toArray();
//                virtual_t += 10*m;
//                wpsCore[4] = virtual_t;
				QJsonObject wps_item;
				wps_item.insert("wpsCore", wpsCore);
				wps_item.insert("useExt", 0);
				wps_item.insert("speed", 0.0);
				wps_item.insert("roll", 0.0);
				wps_item.insert("pitch", 0.0);
				wps_item.insert("yaw", 0.0);
				wps_item.insert("yawEx", 0.0);
				wps.push_back(wps_item);
			}
			waypointsItemTracking.insert("wps", wps);
			waypoints.push_back(waypointsItemTracking);
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
#if 0
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            QJsonObject waypointsItemTracking;

            waypointsItemTracking.insert("wpsUsage", "Tracking");
            waypointsItemTracking.insert("wpsKeyword", "Tracking");
            waypointsItemTracking.insert("wpsFrame", 0);

            waypointsItemTracking.insert("wpsGenPOIs", QJsonArray());
            waypointsItemTracking.insert("wpsPathPlanner", "");
            waypointsItemTracking.insert("wpsGenFences", QJsonArray());

            waypointsItemTracking.insert("wpsGenTimeConsumed", 12.0);

            UINT64 wps_id = FunctionAssistant::generate_random_positive_uint64();
            waypointsItemTracking.insert("wpsKey", "wps_" + QString::number(wps_id));

            double virtual_t = 0;
            QJsonArray wps;
            /////////////////////////////////////////////////////////////////////////////
            for (int i = 2; i < wpstmp.size(); i++)
            {
                auto wpsitem = wpstmp.at(i);
                QJsonArray wpsCore = wpsitem.toArray();
                virtual_t = wpsCore.at(4).toDouble();
                QJsonObject wps_item;
                wps_item.insert("wpsCore", wpsCore);
                wps_item.insert("useExt", 0);
                wps_item.insert("speed", 0.0);
                wps_item.insert("roll", 0.0);
                wps_item.insert("pitch", 0.0);
                wps_item.insert("yaw", 0.0);
                wps_item.insert("yawEx", 0.0);
                wps.push_back(wps_item);
            }
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//            //起飞添加后续指定航线
//            if(!pflighltData->wps.isEmpty())
//            {
//                auto wpsjsa = FunctionAssistant::string_to_json_object(pflighltData->wps);
//                if(wpsjsa.contains("wps") && wpsjsa.value("wps").isArray())
//                {
//                    auto wps_ex =  wpsjsa.value("wps").toArray();
//                    for(int icount = 0; icount < wps_ex.size(); icount++)
//                    {
//                        auto itemjsa = wps_ex.at(icount).toArray();
//                        QJsonArray wpsCore;
//                        wpsCore.append(itemjsa.at(0));
//                        wpsCore.append(itemjsa.at(1));
//                        wpsCore.append(itemjsa.at(2));
//                        wpsCore.append(1);
//                        wpsCore.append(virtual_t+10*icount);
//                        QJsonObject wps_item;
//                        wps_item.insert("wpsCore", wpsCore);
//                        wps_item.insert("useExt", 0);
//                        wps_item.insert("speed", 0.0);
//                        wps_item.insert("roll", 0.0);
//                        wps_item.insert("pitch", 0.0);
//                        wps_item.insert("yaw", 0.0);
//                        wps_item.insert("yawEx", 0.0);
//                        wps.push_back(wps_item);
//                    }
//                }
//            }
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            waypointsItemTracking.insert("wps", wps);
            waypoints.push_back(waypointsItemTracking);
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
            {
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                QJsonObject waypointsItemTracking;

                waypointsItemTracking.insert("wpsUsage", "Tracking");
                waypointsItemTracking.insert("wpsKeyword", "Tracking");
                waypointsItemTracking.insert("wpsFrame", 0);

                waypointsItemTracking.insert("wpsGenPOIs", QJsonArray());
                waypointsItemTracking.insert("wpsPathPlanner", "");
                waypointsItemTracking.insert("wpsGenFences", QJsonArray());

                waypointsItemTracking.insert("wpsGenTimeConsumed", 12.0);

                UINT64 wps_id = FunctionAssistant::generate_random_positive_uint64();
                waypointsItemTracking.insert("wpsKey", "wps_" + QString::number(wps_id));

                QJsonArray wps;
                /////////////////////////////////////////////////////////////////////////////
                for (int i = 2; i < wpstmp.size(); i++)
                {
                    auto wpsitem = wpstmp.at(i);
                    QJsonArray wpsCore = wpsitem.toArray();
                    QJsonObject wps_item;
                    wps_item.insert("wpsCore", wpsCore);
                    wps_item.insert("useExt", 0);
                    wps_item.insert("speed", 0.0);
                    wps_item.insert("roll", 0.0);
                    wps_item.insert("pitch", 0.0);
                    wps_item.insert("yaw", 0.0);
                    wps_item.insert("yawEx", 0.0);
                    wps.push_back(wps_item);
                }
                waypointsItemTracking.insert("wps", wps);
                waypoints.push_back(waypointsItemTracking);
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            }

            {
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                QJsonObject waypointsItemTrackingEx;

                waypointsItemTrackingEx.insert("wpsUsage", "TrackingEx");
                waypointsItemTrackingEx.insert("wpsKeyword", "TrackingEx");
                waypointsItemTrackingEx.insert("wpsFrame", 0);

                waypointsItemTrackingEx.insert("wpsGenPOIs", QJsonArray());
                waypointsItemTrackingEx.insert("wpsPathPlanner", "");
                waypointsItemTrackingEx.insert("wpsGenFences", QJsonArray());

                waypointsItemTrackingEx.insert("wpsGenTimeConsumed", 12.0);

                UINT64 wps_id_ex = FunctionAssistant::generate_random_positive_uint64();
                waypointsItemTrackingEx.insert("wpsKey", "wps_" + QString::number(wps_id_ex));

                double virtual_t = 0;
                QJsonArray wps_trackingex;
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                //起飞添加后续指定航线
                if(!pflighltData->wps.isEmpty())
                {
                    auto wpsjsa = FunctionAssistant::string_to_json_object(pflighltData->wps);
                    if(wpsjsa.contains("wps") && wpsjsa.value("wps").isArray())
                    {
                        auto wps_ex =  wpsjsa.value("wps").toArray();
                        /////////////////////////////////////////////////////////////////////////////
                        if(!wps_ex.isEmpty() && !wpstmp.isEmpty())
                        {
                            auto wpsitem = wpstmp.at(wpstmp.size()-1);
                            QJsonArray wpsCore = wpsitem.toArray();
                            wpsCore[4] = virtual_t;
                            QJsonObject wps_item;
                            wps_item.insert("wpsCore", wpsCore);
                            wps_item.insert("useExt", 0);
                            wps_item.insert("speed", 0.0);
                            wps_item.insert("roll", 0.0);
                            wps_item.insert("pitch", 0.0);
                            wps_item.insert("yaw", 0.0);
                            wps_item.insert("yawEx", 0.0);
                            wps_trackingex.push_back(wps_item);

                            virtual_t += 10;
                        }
                        for(int icount = 0; icount < wps_ex.size(); icount++)
                        {
                            auto itemjsa = wps_ex.at(icount).toArray();
                            QJsonArray wpsCore;
                            wpsCore.append(itemjsa.at(0));
                            wpsCore.append(itemjsa.at(1));
                            wpsCore.append(itemjsa.at(2));
                            wpsCore.append(1);
                            virtual_t += 10*icount;
                            wpsCore.append(virtual_t);
                            QJsonObject wps_item;
                            wps_item.insert("wpsCore", wpsCore);
                            wps_item.insert("useExt", 0);
                            wps_item.insert("speed", 0.0);
                            wps_item.insert("roll", 0.0);
                            wps_item.insert("pitch", 0.0);
                            wps_item.insert("yaw", 0.0);
                            wps_item.insert("yawEx", 0.0);
                            wps_trackingex.push_back(wps_item);
                        }
                    }
                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if(!wps_trackingex.isEmpty())
                {
                    waypointsItemTrackingEx.insert("wps", wps_trackingex);
                    waypoints.push_back(waypointsItemTrackingEx);
                }
            }
#endif
            {
                QJsonObject waypointsItemInit;

                waypointsItemInit.insert("wpsUsage", "Init");
                waypointsItemInit.insert("wpsKeyword", "Init");
                waypointsItemInit.insert("wpsFrame", 0);

                waypointsItemInit.insert("wpsGenPOIs", QJsonArray());
                waypointsItemInit.insert("wpsPathPlanner", "");
                waypointsItemInit.insert("wpsGenFences", QJsonArray());

                waypointsItemInit.insert("wpsGenTimeConsumed", 12.0);

                UINT64 wps_id_init = FunctionAssistant::generate_random_positive_uint64();
                waypointsItemInit.insert("wpsKey", "wps_" + QString::number(wps_id_init));


                QJsonArray wpsinit;
                /////////////////////////////////////////////////////////////////////////////
                for (int i = 0; i < 2; i++)
                {
                    auto wpsitem = wpstmp.at(i);
                    QJsonArray wpsCore = wpsitem.toArray();
                    QJsonObject wps_item;
                    wps_item.insert("wpsCore", wpsCore);
                    wps_item.insert("useExt", 0);
                    wps_item.insert("speed", 0.0);
                    wps_item.insert("roll", 0.0);
                    wps_item.insert("pitch", 0.0);
                    wps_item.insert("yaw", 0.0);
                    wps_item.insert("yawEx", 0.0);
                    wpsinit.push_back(wps_item);
                }
                waypointsItemInit.insert("wps", wpsinit);

                waypoints.push_back(waypointsItemInit);
            }
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		}
		break;
		}

		patternAgentsitem.insert("waypoints", waypoints);
	};
	auto _runfilghtId_itor = std::find_if(m_runfilghtId.begin(),
		m_runfilghtId.end(), [&](const std::unordered_map<uint64_t, std::unordered_map<QString, tagTriggerFlightInfo>>::value_type &vt) {
		return vt.first == triggertimestamp;
	});
	if (_runfilghtId_itor != m_runfilghtId.end())
	{
		std::unordered_map<QString, tagTriggerFlightInfo> &idlst = _runfilghtId_itor->second;
		auto idlst_itor = idlst.begin();
		while (idlst_itor != idlst.end())
		{
			const QString& runtimeid = idlst_itor->first;
			const tagTriggerFlightInfo& flightinfo = idlst_itor->second;
			if (triggerflights.contains(flightinfo))
			{
				FlightPlanConf *pflighltData = flightinfo.pflighltData;
				if (pflighltData)
                {
                    quint64 flightid = pflighltData->flightid.toULongLong();
                    UINT64 agentId = 0;
                    QVariant runtiemstyleid = QVariant::fromValue(runtimeid);
//					std::cout << "flightid " << flightid << std::endl;
                    {
                        auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
                                                               m_runtimedatas.end(),
                                                               [&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
                                                                   return vt.first == runtiemstyleid.toString();
                                                               });
                        if (_runtimedatas_itor != m_runtimedatas.end())
                        {
                            auto &runtimestyleobj = std::get<0>(_runtimedatas_itor->second);

                            QJsonArray patternAgentsarr;
                            auto patternAgents = runtimestyleobj.value("patternAgents").toArray();
                            for (int i = 0; i < patternAgents.count(); i++)
                            {
                                auto patternAgentsobj = patternAgents.at(i).toObject();

                                agentId = patternAgentsobj.value("agentId").toString().toULongLong();
                            }
                        }
                    }
					QJsonArray wpstmp;
                    auto flightinstanceid = getPath(pflighltData, flightid, wpstmp,QString::number(agentId));

					//if (flightinfo.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
					{
						bool bSend = false;
						auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
							m_runtimedatas.end(),
							[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
							return vt.first == runtiemstyleid.toString();
						});
						if (_runtimedatas_itor != m_runtimedatas.end())
						{
                            auto &runtimestyleobj = std::get<0>(_runtimedatas_itor->second);

							QJsonArray patternAgentsarr;
							auto patternAgents = runtimestyleobj.value("patternAgents").toArray();
							for (int i = 0; i < patternAgents.count(); i++)
							{
								auto patternAgentsobj = patternAgents.at(i).toObject();
								updatewps(patternAgentsobj, pflighltData, wpstmp);
								if (!wpstmp.empty())
								{
									bSend = true;
								}

//								patternAgentsobj.insert("agentEntityId", QString::number(flightid));
								patternAgentsarr.push_back(patternAgentsobj);
							}
							runtimestyleobj.insert("patternAgents", patternAgentsarr);
						}
						if (bSend)
                        {
							runRuntimeStyle(runtiemstyleid);

							switch (pflighltData->m_flight_dep_arr_type)
							{
							case E_FLIGHT_DEP_ARR_TYPE_ARR:
							{
								//增加设定跑道占用移除定时器事件
								tagEventInfo timereventinfo;
								timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE;
								timereventinfo.m_eventId = flightinstanceid;
								timereventinfo.timeout = 3 * 60 * 1000;// pflightevent->m_day_senscod_offset_ms;
								timereventinfo.repeattimes = 0;
								timereventinfo.m_eventtype = E_EVENT_TYPE_ID_RUNWAY_RELEASE;
								timereventinfo.flightevent = nullptr;
								timereventinfo.bEnableAdjustSpeed = true;
								m_peventDriver->addevent(timereventinfo);
							}
							break;
							case E_FLIGHT_DEP_ARR_TYPE_DEP:
							{
								//增加停机位占用移除事件
								tagEventInfo timereventinfo;
								timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_ONCE;
								timereventinfo.m_eventId = flightinstanceid;
								timereventinfo.timeout = TAKEOFF_AHEADTIME_MIN * 60 * 1000;// pflightevent->m_day_senscod_offset_ms;
								timereventinfo.repeattimes = 0;
								timereventinfo.m_eventtype = E_EVENT_TYPE_ID_PARKINGPOINT_RELEASE;
								timereventinfo.flightevent = nullptr;
								timereventinfo.bEnableAdjustSpeed = true;
								m_peventDriver->addevent(timereventinfo);
							}
							break;
							}
						}
						else
						{
//							std::cout << "skip ------------------ wps empty" << std::endl;
						}

					}
				}
			}
			idlst_itor++;
		}
	}
}

void RuntimeListWidget::setEventDriver(EventDriver *_peventDriver)
{
	m_peventDriver = _peventDriver;
}

void RuntimeListWidget::eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype)
{
	switch (eventtype) {
	case E_EVENT_TYPE_ID_PARKINGPOINT_RELEASE:
	{
		auto itor = std::find_if(DataManager::getInstance().m_parkingpointuseinfo.begin(),
			DataManager::getInstance().m_parkingpointuseinfo.end(), [&](const std::unordered_map<QString, flightinstance>::value_type &vt) {
			return vt.second.flightinstanceid == event_id;
		});
		if (itor != DataManager::getInstance().m_parkingpointuseinfo.end())
		{
			emit updatepoicolor_sig(itor->first, QColor(0, 255, 0), DEFAULT_FONT_SIZE);
			std::stringstream ss;
			ss << "停机位占用移除： " << itor->first.toStdString() << itor->second.printf() << " " << itor->second.pathplanvalidinfo.alloc_parkingpoint.toStdString() << " " << itor->second.pathplanvalidinfo.alloc_runway.toStdString();;
			DataManager::getInstance().trans_log("", ss, std::stringstream(), false);
			DataManager::getInstance().m_parkingpointuseinfo.erase(itor);
		}
	}
	break;
	case E_EVENT_TYPE_ID_RUNWAY_RELEASE:
	{
		auto itor = std::find_if(DataManager::getInstance().m_runwayuseinfo.begin(),
			DataManager::getInstance().m_runwayuseinfo.end(), [&](const std::unordered_map<QString, flightinstance>::value_type &vt) {
			return vt.second.flightinstanceid == event_id;
		});
		if (itor != DataManager::getInstance().m_runwayuseinfo.end())
		{
			emit updatepoicolor_sig(itor->first, QColor(0, 255, 0), DEFAULT_FONT_SIZE);
			std::stringstream ss;
			ss << "跑道占用移除： " << itor->first.toStdString()<< itor->second.printf()<<" "<< itor->second.pathplanvalidinfo.alloc_parkingpoint.toStdString() << " " << itor->second.pathplanvalidinfo.alloc_runway.toStdString();;
			DataManager::getInstance().trans_log("", ss, std::stringstream(), false);
			DataManager::getInstance().m_runwayuseinfo.erase(itor);

		}
	}
	break;
	default:break;
	}
}

void RuntimeListWidget::closeModelWidget_slot()
{
	m_pQStackedWidget->setCurrentIndex(0);
}

void RuntimeListWidget::showEvent(QShowEvent *event)
{
	m_pQStackedWidget->currentWidget()->show();
	if (m_pQStackedWidget->currentIndex() == 1)
	{
		m_pRuntimeEditWidget->setShow(true);
	}
	QWidget::showEvent(event);
}

void RuntimeListWidget::hideEvent(QHideEvent *event)
{
	m_pQStackedWidget->currentWidget()->hide();
	if (m_pQStackedWidget->currentIndex() == 1)
	{
		m_pRuntimeEditWidget->setShow(false);
	}
	QWidget::hideEvent(event);
}

void RuntimeListWidget::qml_add_runtime_slot()
{
	request_agentdata();
	quint64 _runtiemstyle_id = FunctionAssistant::generate_random_positive_uint64();;
	m_pRuntimeEditWidget->setShow(true);
	m_pRuntimeEditWidget->resetData();
	QJsonObject _runtime_simple;
	_runtime_simple.insert("patternSig", QString::number(_runtiemstyle_id));

	m_pRuntimeEditWidget->decodeRuntimeStyle(_runtime_simple);

	m_pQStackedWidget->setCurrentIndex(1);
}

void RuntimeListWidget::qml_send_agent_data_slot()
{
	QJsonObject sendjson;

	QJsonObject jsobj;
	std::unordered_map<QString, std::tuple<QString, QString>> _agentdata;
	if (DataManager::getInstance().pHttpClient() && DataManager::getInstance().pHttpClient()->requeset_agent_data(jsobj))
	{
		auto dataarray = jsobj.value("data").toArray();
		QJsonArray agents;
		for (auto dataarrayitem : dataarray)
		{
			auto dataarrayitemobj = dataarrayitem.toObject();
			agents.push_back(dataarrayitemobj);
		}
		sendjson.insert("agents", agents);
		//qDebug() << sendjson;
		run_agent_data(sendjson);
	}

	QVariant runtiemstyleid = QVariant::fromValue(m_senceruntimeid);
	std::cout << "trigger sence " << m_senceruntimeid.toStdString() << "to active" << std::endl;
	runRuntimeStyle(runtiemstyleid);


	QString title;
	auto it = std::find_if(DataManager::getInstance().getAirPortNameList().begin(),
		DataManager::getInstance().getAirPortNameList().end(), [&](const std::unordered_map<QString, std::tuple<QString, QString>>::value_type& vt) {
		return vt.first == m_airport_code;
	});
	if (it != DataManager::getInstance().getAirPortNameList().end())
	{
		title = std::get<1>(it->second) + "_" + m_airport_code;
	}

	DataManager::getInstance().trans_log("初始化仿真机场数据： ", std::stringstream()<< title.toStdString(), std::stringstream());
}

void RuntimeListWidget::add_runtime_style_slot(const QJsonObject &runtimestyle)
{
	auto runtimedata = runtimestyle.value("patternAgents").toArray();
	QString runtimeid = runtimestyle.value("patternSig").toString();

	auto _runtimedatas_itor = std::find_if(m_runtimedatas.begin(),
		m_runtimedatas.end(),
		[&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type &vt) {
		return vt.first == runtimeid;
	});
	if (_runtimedatas_itor == m_runtimedatas.end())
	{
		QString item_name = "undefined name";
		QString item_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString item_founder = "undefined user";
		int item_num = runtimedata.count();
		bool item_status = false;

//		{
//			QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//			QMetaObject::invokeMethod(pRoot, "appendRuntimeStyle",
//				Q_ARG(QVariant, QVariant::fromValue(runtimeid)),
//				Q_ARG(QVariant, QVariant::fromValue(item_name)),
//				Q_ARG(QVariant, QVariant::fromValue(item_time)),
//				Q_ARG(QVariant, QVariant::fromValue(item_founder)),
//				Q_ARG(QVariant, QVariant::fromValue(QString::number(item_num))),
//				Q_ARG(QVariant, QVariant::fromValue(item_status)));
//		}

		m_runtimedatas.insert(std::make_pair(runtimeid, std::make_tuple(runtimestyle, false)));


		append_agentruntime_data(runtimestyle);
	}
	else
	{
		std::get<0>(_runtimedatas_itor->second) = runtimestyle;
		QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

		QString item_name = "undefined name";
		QString item_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
		QString item_founder = "undefined user";
		int item_num = runtimedata.count();
		bool item_status = false;

//		QMetaObject::invokeMethod(pRoot, "updateRuntimeStyle",
//			Q_ARG(QVariant, QVariant::fromValue(runtimeid)),
//			Q_ARG(QVariant, QVariant::fromValue(item_name)),
//			Q_ARG(QVariant, QVariant::fromValue(item_time)),
//			Q_ARG(QVariant, QVariant::fromValue(item_founder)),
//			Q_ARG(QVariant, QVariant::fromValue(QString::number(item_num))),
//			Q_ARG(QVariant, QVariant::fromValue(item_status)));

		update_agentruntime_data(runtimestyle);
	}
	//m_pRuntimeEditWidget->resetData();
}

void RuntimeListWidget::request_agentdata()
{
	std::unordered_map<QString, std::tuple<QString, QString>> _agentdata;
	m_pRuntimeEditWidget->updateAgentKeys(_agentdata);
}

void RuntimeListWidget::request_agentruntimedata()
{
	QJsonObject jsobj;
	std::unordered_map<QString, std::tuple<QString, QString>> _agentdata;
	if (DataManager::getInstance().pHttpClient() && DataManager::getInstance().pHttpClient()->requeset_agentruntime_data(jsobj))
	{
		auto dataarray = jsobj.value("data").toArray();
		for (auto dataarrayitem : dataarray)
		{
			auto dataarrayitemobj = dataarrayitem.toObject();
			add_runtime_style_slot(dataarrayitemobj);


			auto runtimedata = dataarrayitemobj.value("patternAgents").toArray();
			for (auto item : runtimedata)
			{
				auto itemobj = item.toObject();
				if (itemobj.value("agentKey").toString() == "AGENTKEY_11250312941639682925")
				{
					m_senceruntimeid = dataarrayitemobj.value("patternSig").toString();
					break;
				}
			}
		}
	}
}

void RuntimeListWidget::append_agentruntime_data(const QJsonObject &jsobj)
{
	qDebug() << jsobj;
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->append_agentruntime_data(jsobj))
	{
		std::cout << "http error" << std::endl;
	}
}

void RuntimeListWidget::update_agentruntime_data(const QJsonObject &jsobj)
{
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->update_agentruntime_data(jsobj))
	{
		std::cout << "http error" << std::endl;
	}
}

void RuntimeListWidget::delete_agentruntime_data(const QJsonObject &jsobj)
{
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->delete_agentruntime_data(jsobj))
	{
		std::cout << "http error" << std::endl;
	}
}

void RuntimeListWidget::run_agentruntime_data(const QJsonObject &jsobj)
{
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_agentruntime_data(jsobj))
	{
		m_peventDriver->stop();
		std::cout << "http error" << std::endl;
	}
}

void RuntimeListWidget::run_agent_data(const QJsonObject &jsobj)
{
	if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_agent_data(jsobj))
	{
		std::cout << "http error" << std::endl;
	}

	if (m_peventDriver)
	{
		float ff = m_peventDriver->speed_coeff();
		QJsonObject jsobj;
		jsobj.insert("speed", ff);
        jsobj.insert("ctrlparam", "prgctrl");
        jsobj.insert("ctrltype", "prgctrl");
		if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_running_speed(jsobj))
		{
			std::cout << "http error" << std::endl;
		}
	}
}


#define TARGET_HEIGHT_METER (300.0f)
#define HOLDPOINT_EXTEND_METER (25)
#define HOLDPOINT_EXTEND_AREA_METER (150)
#define TAKEOFF_METER (1750)
#define LANDINGPOINT_METER (15)

//#define USING_NEW_RELLOC_PARKING_POINT

quint64 RuntimeListWidget::getPath(FlightPlanConf *pflighltData, quint64 flightid, QJsonArray &wps,const QString& runtimeid)
{
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

#if 1
		LATLNGS_VECTOR &_extendwpslatlng_tmp = _ptagPath_Plan->m_extendwpslatlng_simple;

		LATLNGS_VECTOR &_runway_total = _ptagPath_Plan->m_runway_total_simple;
		//		switch (pflighltData->m_flight_dep_arr_type)
		//		{
		//		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		//		{
		//			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng_simple.size();
		//			LAT_LNG firstpt = _ptagPath_Plan->m_extendwpslatlng_simple.front();

		//			//获取降落航线的高度为0 的点，即距离V一定距离的点
		//			int countindex = 0;
		//			LAT_LNG pt_zero_height;
		//			for (int cc = 0; cc < wpsexsize; cc++)
		//			{
		//				LAT_LNG curpt = _ptagPath_Plan->m_extendwpslatlng_simple.at(cc);
		//				double xx = FunctionAssistant::calc_dist(curpt, firstpt);
		//				if (xx > LANDINGPOINT_METER)
		//				{
		//					pt_zero_height = curpt;
		//					countindex = cc;
		//					break;
		//				}
		//			}

		//			//降落航线的第一点
		//			LAT_LNG arr_first_pt = _ptagPath_Plan->m_runway_total_simple.front();
		//			//获取降落航线的第一点 和 降落航线的高度为 0 的点的距离 (m)
		//			double distance_merters1 = FunctionAssistant::calc_dist(pt_zero_height, arr_first_pt);
		//			//每米下降的高度
		//			auto pre_meter_height = (TARGET_HEIGHT_METER) / (distance_merters1);

		//			for (int i = 0; i < _ptagPath_Plan->m_runway_total_simple.size(); i++)
		//			{
		//				//计算当前点和降落航线的第一点 的距离, 距离逐渐变大，逐渐下降，超过distance_merters1，高度为负
		//				double distance_merters = FunctionAssistant::calc_dist(_ptagPath_Plan->m_runway_total_simple.at(i), arr_first_pt);
		//				//计算当前应该的高度
		//				double alt = 0;
		//				if (i < countindex + 2)
		//				{
		//					alt = TARGET_HEIGHT_METER - pre_meter_height * distance_merters;
		//				}
		//				alt = alt < 0 ? 0 : alt;
		//				addwps(_ptagPath_Plan->m_runway_total_simple.at(i).lng, _ptagPath_Plan->m_runway_total_simple.at(i).lat, alt, 1, i * 10);
		//			}
		//		}break;
		//		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		//		{
		//			addwps(_ptagPath_Plan->m_extendwpslatlng.at(0).lng, _ptagPath_Plan->m_extendwpslatlng.at(0).lat, 0, 1, 0);
		//			addwps(_ptagPath_Plan->m_extendwpslatlng.at(1).lng, _ptagPath_Plan->m_extendwpslatlng.at(1).lat, 0, 1, 0);

		//			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng_simple.size();
		//			LAT_LNG lastpt = _ptagPath_Plan->m_extendwpslatlng_simple.back();
		//			//起飞航线的最后一点
		//			LAT_LNG dep_last_pt = _ptagPath_Plan->m_runway_total_simple.back();
		//			//起飞航班 跑道 发车点 与 航线最后一点的距离
		//			double dis = FunctionAssistant::calc_dist(lastpt, dep_last_pt);
		//			//起飞航班 跑道 发车点 与 航线最后一点的方向，采样一个中间点
		//			glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lastpt, dep_last_pt);
		//			LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(lastpt, directionVectorArr, dis/3);

		//			//获取起飞航线的采样点 和 起飞航线的最后一个 高度为 0 的点的距离 (m)
		////			double distance_merters1 = FunctionAssistant::calc_dist(extendpt, dep_last_pt) - TAKEOFF_METER;
		//			double distance_merters1 = FunctionAssistant::calc_dist(extendpt, dep_last_pt);
		//			//每米上升的高度
		//			auto pre_meter_height = (TARGET_HEIGHT_METER) / (distance_merters1);

		//			for (int i = 0; i < _ptagPath_Plan->m_runway_total_simple.size() - 1; i++)
		//			{
		//				double alt = 0;
		//				addwps(_ptagPath_Plan->m_runway_total_simple.at(i).lng, _ptagPath_Plan->m_runway_total_simple.at(i).lat, alt, 1, i * 10);
		//			}
		//			//计算当前点和起飞航线的最后一点 的距离,距离逐渐变小，超过distance_merters1，高度为正，逐渐提升
		//			addwps(extendpt.lng, extendpt.lat, 0, 1, _ptagPath_Plan->m_runway_total_simple.size()*10);
		//			addwps(dep_last_pt.lng, dep_last_pt.lat, TARGET_HEIGHT_METER, 1, (_ptagPath_Plan->m_runway_total_simple.size() + 1) * 10);
		//		}
		//		break;
		//		}
#else
		LATLNGS_VECTOR &_extendwpslatlng_tmp = _ptagPath_Plan->m_extendwpslatlng;
		LATLNGS_VECTOR &_runway_total = _ptagPath_Plan->m_runway_total;
		//		switch (pflighltData->m_flight_dep_arr_type)
		//		{
		//		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		//		{
		//			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng.size();
		//			LAT_LNG firstpt = _ptagPath_Plan->m_extendwpslatlng.front();
		//			int exsize = _ptagPath_Plan->m_runwayextendwpslatlng.size();

		//			int countindex = 0;
		//			for (int cc = 0; cc < wpsexsize; cc++)
		//			{
		//				LAT_LNG curpt = _ptagPath_Plan->m_extendwpslatlng.at(cc);
		//				double xx = FunctionAssistant::calc_dist(curpt, firstpt);
		//				if (xx > LANDINGPOINT_METER)
		//				{
		//					countindex = cc;
		//					break;
		//				}
		//			}
		//			wpsexsize -= countindex;
		//			exsize += countindex;

		//			double arrhightstep = TARGET_HEIGHT_METER / exsize;
		//			for (int i = 0; i < _ptagPath_Plan->m_runway_total.size(); i++)
		//			{
		//				double alt = TARGET_HEIGHT_METER - arrhightstep * i;
		//				alt = alt < 0 ? 0 : alt;
		//				addwps(_ptagPath_Plan->m_runway_total.at(i).lng, _ptagPath_Plan->m_runway_total.at(i).lat, alt, 1, i * 10);
		//			}
		//		}break;
		//		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		//		{
		//			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng.size();
		//			LAT_LNG lastpt = _ptagPath_Plan->m_extendwpslatlng.back();
		//			int exsize = _ptagPath_Plan->m_runwayextendwpslatlng.size();
		//			int countindex = 0;
		//			for (int cc = 0; cc < exsize; cc++)
		//			{
		//				LAT_LNG curpt = _ptagPath_Plan->m_runwayextendwpslatlng.at(cc);
		//				double xx = FunctionAssistant::calc_dist(curpt, lastpt);
		//				if (xx > TAKEOFF_METER)
		//				{
		//					countindex = cc;
		//					break;
		//				}
		//			}
		//			wpsexsize += countindex;
		//			exsize -= countindex;

		//			double arrhightstep = TARGET_HEIGHT_METER / exsize;
		//			for (int i = 0; i < _ptagPath_Plan->m_runway_total.size(); i++)
		//			{
		//				double alt = arrhightstep * (i - wpsexsize);
		//				alt = alt < 0 ? 0 : alt;
		//				addwps(_ptagPath_Plan->m_runway_total.at(i).lng, _ptagPath_Plan->m_runway_total.at(i).lat, alt, 1, i * 10);
		//			}
		//		}
		//		break;
		//		}
#endif


		switch (pflighltData->m_flight_dep_arr_type)
		{
		case E_FLIGHT_DEP_ARR_TYPE_ARR:
		{
			int wpsexsize = _extendwpslatlng_tmp.size();
			LAT_LNG firstpt = _extendwpslatlng_tmp.front();

			//获取降落航线的高度为0 的点，即距离V一定距离的点
			int countindex = 0;
			LAT_LNG pt_zero_height;
			for (int cc = 0; cc < wpsexsize; cc++)
			{
				LAT_LNG curpt = _extendwpslatlng_tmp.at(cc);
				double xx = FunctionAssistant::calc_dist(curpt, firstpt);
				if (xx > LANDINGPOINT_METER)
				{
					pt_zero_height = curpt;
					countindex = cc;
					break;
				}
			}

			//降落航线的第一点
			LAT_LNG arr_first_pt = _runway_total.front();
			//获取降落航线的第一点 和 降落航线的高度为 0 的点的距离 (m)
			double distance_merters1 = FunctionAssistant::calc_dist(pt_zero_height, arr_first_pt);
			//每米下降的高度
			auto pre_meter_height = (TARGET_HEIGHT_METER) / (distance_merters1);
			int i = 0;
			for (i = 0; i < _runway_total.size(); i++)
			{
				//计算当前点和降落航线的第一点 的距离, 距离逐渐变大，逐渐下降，超过distance_merters1，高度为负
				double distance_merters = FunctionAssistant::calc_dist(_runway_total.at(i), arr_first_pt);
				//计算当前应该的高度
				double alt = 0;
				if (i < countindex + 2)
				{
					alt = TARGET_HEIGHT_METER - pre_meter_height * distance_merters;
				}
				alt = alt < 0 ? 0 : alt;
				addwps(_runway_total.at(i).lng, _runway_total.at(i).lat, alt, 1, i * 10);
			}

			addwps(_ptagPath_Plan->m_runway_total.at(_ptagPath_Plan->m_runway_total.size() - 2).lng, _ptagPath_Plan->m_runway_total.at(_ptagPath_Plan->m_runway_total.size() - 2).lat, 0, 1, (i+1) * 10);

			addwps(_ptagPath_Plan->m_runway_total.at(_ptagPath_Plan->m_runway_total.size() - 1).lng, _ptagPath_Plan->m_runway_total.at(_ptagPath_Plan->m_runway_total.size() - 1).lat, 0, 1, (i+1) * 10);
		}break;
		case E_FLIGHT_DEP_ARR_TYPE_DEP:
		{
			addwps(_ptagPath_Plan->m_extendwpslatlng.at(0).lng, _ptagPath_Plan->m_extendwpslatlng.at(0).lat, 0, 1, 0);
			addwps(_ptagPath_Plan->m_extendwpslatlng.at(1).lng, _ptagPath_Plan->m_extendwpslatlng.at(1).lat, 0, 1, 0);

			int wpsexsize = _extendwpslatlng_tmp.size();
			LAT_LNG lastpt = _extendwpslatlng_tmp.back();
			//起飞航线的最后一点
			LAT_LNG dep_last_pt = _runway_total.back();
			//起飞航班 跑道 发车点 与 航线最后一点的距离
			double dis = FunctionAssistant::calc_dist(lastpt, dep_last_pt);
			//起飞航班 跑道 发车点 与 航线最后一点的方向，采样一个中间点
			glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lastpt, dep_last_pt);
			LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(lastpt, directionVectorArr, dis / 3);

			//获取起飞航线的采样点 和 起飞航线的最后一个 高度为 0 的点的距离 (m)
			//			double distance_merters1 = FunctionAssistant::calc_dist(extendpt, dep_last_pt) - TAKEOFF_METER;
			double distance_merters1 = FunctionAssistant::calc_dist(extendpt, dep_last_pt);
			//每米上升的高度
			auto pre_meter_height = (TARGET_HEIGHT_METER) / (distance_merters1);

			for (int i = 0; i < _runway_total.size() - 1; i++)
			{
				double alt = 0;
				addwps(_runway_total.at(i).lng, _runway_total.at(i).lat, alt, 1, i * 10);
			}
			//计算当前点和起飞航线的最后一点 的距离,距离逐渐变小，超过distance_merters1，高度为正，逐渐提升
			addwps(extendpt.lng, extendpt.lat, 0, 1, _runway_total.size() * 10);
			addwps(dep_last_pt.lng, dep_last_pt.lat, TARGET_HEIGHT_METER, 1, (_runway_total.size() + 1) * 10);


#if 0
			addwps(_ptagPath_Plan->m_extendwpslatlng.at(0).lng, _ptagPath_Plan->m_extendwpslatlng.at(0).lat, 0, 1, 0);
			addwps(_ptagPath_Plan->m_extendwpslatlng.at(1).lng, _ptagPath_Plan->m_extendwpslatlng.at(1).lat, 0, 1, 0);

			int wpsexsize = _ptagPath_Plan->m_extendwpslatlng_simple.size();
			LAT_LNG lastpt = _ptagPath_Plan->m_extendwpslatlng_simple.back();
			//起飞航线的最后一点
			LAT_LNG dep_last_pt = _ptagPath_Plan->m_runway_total_simple.back();
			//获取起飞航线的最后一点 和 起飞航线的最后一个 高度为 0 的点的距离 (m)
			double distance_merters1 = FunctionAssistant::calc_dist(lastpt, dep_last_pt) - TAKEOFF_METER;
			//每米上升的高度
			auto pre_meter_height = (TARGET_HEIGHT_METER) / (distance_merters1);

			for (int i = 0; i < _ptagPath_Plan->m_runway_total_simple.size(); i++)
			{
				//计算当前点和降落航线的第一点 的距离,距离逐渐变小，超过distance_merters1，高度为正，逐渐提升
				double distance_merters = FunctionAssistant::calc_dist(_ptagPath_Plan->m_runway_total_simple.at(i), dep_last_pt);
				double alt = 0;
				if (i > wpsexsize)
				{
					alt = TARGET_HEIGHT_METER - pre_meter_height * distance_merters;
				}
				alt = alt < 0 ? 0 : alt;
				addwps(_ptagPath_Plan->m_runway_total_simple.at(i).lng, _ptagPath_Plan->m_runway_total_simple.at(i).lat, alt, 1, i * 10);
			}
#endif
		}
		break;
		}
	};

	PathPlanValidInfo pathplanvalidinfo;
	QString runwaymatchedText = pflighltData->m_Runway;
	QRegularExpression regexnum("\\d+");
	QRegularExpressionMatch matchnum = regexnum.match(runwaymatchedText);

	if (matchnum.hasMatch()) {

		QString numText = matchnum.captured(0);

		numText.remove(QRegExp("^0+"));

		runwaymatchedText.replace(matchnum.captured(0), numText);
	}

	QString _parkingpoint;


	QString _parkingpointtmp = pflighltData->m_Seat;
	////_parkingpointtmp = "132L";
	//_parkingpointtmp = "GY01";

	QRegularExpression regex("^[^0-9]\\S*");

	QRegularExpressionMatch match = regex.match(_parkingpointtmp);

	if (match.hasMatch())
	{
		_parkingpoint = _parkingpointtmp;
	}
	else
	{
		_parkingpoint = "P" + _parkingpointtmp;
	}

	pathplanvalidinfo.target_parkingpoint = _parkingpoint;
	pathplanvalidinfo.target_runway = runwaymatchedText;

	quint64 flightinstanceid = FunctionAssistant::generate_random_positive_uint64();
	QStringList allowRunway = m_allowRunway;
	tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getAirportInfo(m_airport_code);
	if (nullptr == ptagAirPortInfo)
	{
		return flightinstanceid;
	}
	std::list<tagPath_Plan*> pathplans;

	tagPath_Plan* _ptagPath_Plan = nullptr;

	std::stringstream resonlog_result;

	std::stringstream resonlog;

	switch (pflighltData->m_flight_dep_arr_type)
	{
	case E_FLIGHT_DEP_ARR_TYPE_ARR:
	{
		pathplans = ptagAirPortInfo->m_arrpaths;

		allowRunway.removeAll("2L");
		auto getplanpath = [&](const QStringList &validRunwaylist, bool bMatchParkingPoint, bool bMatchRunway, bool bNewAlloc)->tagPath_Plan* {
			tagPath_Plan* alloc_ptagPath_Plan = nullptr;
			for (auto pathplansitor = pathplans.begin(); pathplansitor != pathplans.end(); pathplansitor++)
			{
				tagPath_Plan* ptagPath_Plan = *pathplansitor;
				if (ptagPath_Plan->m_bValid &&
					allowRunway.contains(ptagPath_Plan->m_runway))
				{
					//需要停机坪匹配
					if (bMatchParkingPoint && ptagPath_Plan->m_parkingpoint != pathplanvalidinfo.target_parkingpoint)
					{
						continue;
					}
					//需要跑道匹配
					if (bMatchRunway && !validRunwaylist.contains(ptagPath_Plan->m_runway))
					{
						continue;
					}
					//需要未分配使用过
					if (bNewAlloc && (DataManager::getInstance().m_parkingpointuseinfo.find(ptagPath_Plan->m_parkingpoint) != DataManager::getInstance().m_parkingpointuseinfo.end()))
					{
						continue;
					}
					//查找到有效的 满足条件的
					alloc_ptagPath_Plan = ptagPath_Plan;
					break;
				}
			}
			return alloc_ptagPath_Plan;
		};

		auto findOtherRunwayParkingpath = [&](const QStringList& validRunwaylist)
		{
			if (validRunwaylist.isEmpty())
			{
				resonlog << " 【未分配到 不相同的有效的进港跑道 进港航路 -----【无法仿真】-----】 ";
				for (auto _arr_runway_itor = allowRunway.begin(); _arr_runway_itor != allowRunway.end(); _arr_runway_itor++)
				{
					auto runway_str = *_arr_runway_itor;
					if (DataManager::getInstance().m_runwayuseinfo.find(runway_str) != DataManager::getInstance().m_runwayuseinfo.end())
					{
						resonlog << runway_str.toStdString() << " " << DataManager::getInstance().m_runwayuseinfo.at(runway_str).printf() << " " << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " " << pathplanvalidinfo.alloc_runway.toStdString();
					}
				}
			}
			else
			{
                //停机位被占用
                if (DataManager::getInstance().m_parkingpointuseinfo.find(pathplanvalidinfo.target_parkingpoint) != DataManager::getInstance().m_parkingpointuseinfo.end())
                {
                    auto& occypyflightinstance = DataManager::getInstance().m_parkingpointuseinfo.at(pathplanvalidinfo.target_parkingpoint);
                    //停机坪被
                    resonlog << " 停机位冲突 " << " 与 " << occypyflightinstance.printf();
                    //存在使用其他停机坪 首先申请跑道相同的 不同停机坪
#ifdef USING_NEW_RELLOC_PARKING_POINT
                                    //重新分配一个停机坪不相同的 出港航路相同的 未使用过的
                                    _ptagPath_Plan = getplanpath(false, true, true);
                    if (!_ptagPath_Plan)
                    {
                        resonlog << " 【未分配到 停机坪不相同 出港跑道相同的 出港航路】 ";
                        //重新分配一个停机坪不相同的 出港航路不相同的 未使用过的
                        _ptagPath_Plan = getplanpath(false, false, true);
                        if (!_ptagPath_Plan)
                        {
                            resonlog << " 【未分配到 停机坪不相同 出港跑道不相同的 出港航路 -----【无法仿真】-----】 ";
                        }
                        else
                        {
                            resonlog << " 【重新分配 停机坪不相同 出港跑道不相同 的出港航班航路】 ";
                        }
                    }
                    else
                    {
                        resonlog << " 【重新分配 停机坪不相同 出港跑道相同 的出港航班航路】 ";
                    }
#else
                                    resonlog << " 【-----【无法仿真】-----】 ";
#endif
                }
                else
                {
                    //在有效的跑道中 查找出停机位相同 跑道为指定列表中的 且停机位未被使用的进港路径
                    _ptagPath_Plan = getplanpath(validRunwaylist, true, true, true);
                    if (!_ptagPath_Plan)
                    {
                        resonlog << " 未分配到 停机位相同 进港跑道不相同的 进港航路";
#ifdef USING_NEW_RELLOC_PARKING_POINT
                        //在有效的跑道中 查找出停机位不同 且停机位未被使用的进港路径
                        _ptagPath_Plan = getplanpath(validRunwaylist, false, true, true);
                        if (!_ptagPath_Plan)
                        {
                            resonlog << " 【未分配到 停机位不相同 进港跑道不相同的 进港航路 -----【无法仿真】-----】";
                        }
                        else
                        {
                            resonlog << " 【重新分配到 停机位不相同 进港跑道不相同的 进港航路】 ";
                        }
#else
                        resonlog << " 【-----【无法仿真】-----】 ";
#endif
                    }
                    else
                    {
                        resonlog << " 【重新分配到 停机位相同 进港跑道不相同的 进港航路】 ";
                    }

                }
			}
		};

		//过滤出有效的跑道
		QStringList validRunwaylist;
		for (auto _arr_runway_itor = allowRunway.begin(); _arr_runway_itor != allowRunway.end(); _arr_runway_itor++)
		{
			auto runway_str = *_arr_runway_itor;
			if (DataManager::getInstance().m_runwayuseinfo.find(runway_str) == DataManager::getInstance().m_runwayuseinfo.end())
			{
				validRunwaylist.push_back(runway_str);
			}
		}

		//首先检查进港航班的跑道是否可用
		if (DataManager::getInstance().m_runwayuseinfo.find(pathplanvalidinfo.target_runway) != DataManager::getInstance().m_runwayuseinfo.end())
		{
			auto& occypyflightinstance = DataManager::getInstance().m_runwayuseinfo.at(pathplanvalidinfo.target_runway);
			resonlog << " 进港跑道冲突 " << " 与 " << occypyflightinstance.printf();
			findOtherRunwayParkingpath(validRunwaylist);
		}
		else
        {
            //停机位被占用
            if (DataManager::getInstance().m_parkingpointuseinfo.find(pathplanvalidinfo.target_parkingpoint) != DataManager::getInstance().m_parkingpointuseinfo.end())
            {
                auto& occypyflightinstance = DataManager::getInstance().m_parkingpointuseinfo.at(pathplanvalidinfo.target_parkingpoint);
                //停机坪被
                resonlog << " 停机位冲突 " << " 与 " << occypyflightinstance.printf();
                //存在使用其他停机坪 首先申请跑道相同的 不同停机坪
#ifdef USING_NEW_RELLOC_PARKING_POINT
                                //重新分配一个停机坪不相同的 出港航路相同的 未使用过的
                                _ptagPath_Plan = getplanpath(false, true, true);
                if (!_ptagPath_Plan)
                {
                    resonlog << " 【未分配到 停机坪不相同 出港跑道相同的 出港航路】 ";
                    //重新分配一个停机坪不相同的 出港航路不相同的 未使用过的
                    _ptagPath_Plan = getplanpath(false, false, true);
                    if (!_ptagPath_Plan)
                    {
                        resonlog << " 【未分配到 停机坪不相同 出港跑道不相同的 出港航路 -----【无法仿真】-----】 ";
                    }
                    else
                    {
                        resonlog << " 【重新分配 停机坪不相同 出港跑道不相同 的出港航班航路】 ";
                    }
                }
                else
                {
                    resonlog << " 【重新分配 停机坪不相同 出港跑道相同 的出港航班航路】 ";
                }
#else
                                resonlog << " 【-----【无法仿真】-----】 ";
#endif
            }
            else
            {

                QStringList validRunwaylist1;
                validRunwaylist1.append(pathplanvalidinfo.target_runway);
                //首先查看指定的跑道及停机位能否正常被使用
            _ptagPath_Plan = getplanpath(validRunwaylist1, true, true, true);
                if (!_ptagPath_Plan)
                {
                    resonlog << " 未分配到 停机位相同 进港跑道相同的 进港航路";
#ifdef USING_NEW_RELLOC_PARKING_POINT
                    //查找停机位不同 进港跑道相同的路径
                    _ptagPath_Plan = getplanpath(validRunwaylist1, false, true, true);
                    if (!_ptagPath_Plan)
                    {
                        resonlog << " 未分配到 停机位不相同 进港跑道相同的 进港航路";
#else
                    resonlog << " 【-----【无法仿真】-----】 ";
#endif
                        validRunwaylist.removeAll(pathplanvalidinfo.target_runway);
                        findOtherRunwayParkingpath(validRunwaylist);
#ifdef USING_NEW_RELLOC_PARKING_POINT
                    }
                    else
                    {
                        resonlog << " 重新分配到 停机位不相同 进港跑道相同的 进港航路";
                    }
#endif
                }

            }
		}

		if (_ptagPath_Plan)
		{
			pathplanvalidinfo.alloc_parkingpoint = _ptagPath_Plan->m_parkingpoint;
			pathplanvalidinfo.alloc_runway = _ptagPath_Plan->m_runway;

			//设置停机位占用状态，由同机身号的出港航班解除该停机位的占用
			DataManager::getInstance().m_parkingpointuseinfo.insert(std::make_pair(pathplanvalidinfo.alloc_parkingpoint, flightinstance{ flightinstanceid,pathplanvalidinfo, pflighltData ,_ptagPath_Plan }));
			//设置跑道占用状态
			DataManager::getInstance().m_runwayuseinfo.insert(std::make_pair(pathplanvalidinfo.alloc_runway, flightinstance{ flightinstanceid,pathplanvalidinfo, pflighltData ,_ptagPath_Plan }));

			auto flightinfos = flightinstance{ flightid,pathplanvalidinfo, pflighltData ,_ptagPath_Plan };
			DataManager::getInstance().appendFlight(flightid, flightinfos);

			resonlog_result << flightinfos.printf();
			genrateWps(_ptagPath_Plan);

            emit updatepoicolor_sig(pathplanvalidinfo.alloc_parkingpoint, QColor(0, 0, 255), DEFAULT_FONT_SIZE);
            std::stringstream ssTMP2;
            ssTMP2 <<"停机位占用： " << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " 进港航班：" << flightinfos.printf()<<" wps "<<wps.size()<<"\n";
            DataManager::getInstance().trans_log("", ssTMP2, std::stringstream(),false);
			std::stringstream ssTMPP;
			ssTMPP << "跑道占用： " << pathplanvalidinfo.alloc_runway.toStdString() << " 停机位占用： " << pathplanvalidinfo.alloc_parkingpoint.toStdString()<<" 进港航班 "<< flightinfos.printf();
			DataManager::getInstance().trans_log("", ssTMPP, std::stringstream(), false);
		}
		else 
		{
			resonlog_result << "[" << "计划停机位：" << pathplanvalidinfo.target_parkingpoint.toStdString() << " "
				<< "计划跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
				<< "使用停机位：" << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " "
				<< "使用跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
				<< "航班计划：" << pflighltData->printf() << "]";
		}
		DataManager::getInstance().trans_log("仿真进港航班： ", resonlog_result, resonlog);
	}break;
	case E_FLIGHT_DEP_ARR_TYPE_DEP:
	{

		pathplans = ptagAirPortInfo->m_deppaths;
		allowRunway.removeAll("2R");

		auto getplanpath = [&](bool bMatchParkingPoint, bool bMatchRunway, bool bNewAlloc)->tagPath_Plan*{
			tagPath_Plan* alloc_ptagPath_Plan = nullptr;
			for (auto pathplansitor = pathplans.begin(); pathplansitor != pathplans.end(); pathplansitor++)
			{
				tagPath_Plan* ptagPath_Plan = *pathplansitor;
				if (ptagPath_Plan->m_bValid &&
					allowRunway.contains(ptagPath_Plan->m_runway))
				{
					//需要停机坪匹配
					if (bMatchParkingPoint && ptagPath_Plan->m_parkingpoint != pathplanvalidinfo.target_parkingpoint)
					{
						continue;
					}
					//需要跑道匹配
					if (bMatchRunway && ptagPath_Plan->m_runway != pathplanvalidinfo.target_runway)
					{
						continue;
					}
					//需要未分配使用过
					if (bNewAlloc && (DataManager::getInstance().m_parkingpointuseinfo.find(ptagPath_Plan->m_parkingpoint) != DataManager::getInstance().m_parkingpointuseinfo.end()))
					{
						continue;
					}
					//查找到有效的 满足条件的
					alloc_ptagPath_Plan = ptagPath_Plan;
					break;
				}
			}
			return alloc_ptagPath_Plan;
		};
        bool  bchanged = false;
		//停机位被占用
		if (DataManager::getInstance().m_parkingpointuseinfo.find(pathplanvalidinfo.target_parkingpoint) != DataManager::getInstance().m_parkingpointuseinfo.end())
		{
			auto& occypyflightinstance = DataManager::getInstance().m_parkingpointuseinfo.at(pathplanvalidinfo.target_parkingpoint);
			//停机位同一架飞机由进港状态变为出港状态，机身号相同
			if (occypyflightinstance.pflightdata->m_PlaneNum == pflighltData->m_PlaneNum &&
				occypyflightinstance.pflightdata->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR &&
				pflighltData->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
			{
				resonlog << " 飞机进出港状态切换 " << " 与 " << occypyflightinstance.printf();

				//检查原始分配的出港航路是否有效
				_ptagPath_Plan = getplanpath(true, true, false);
				if (!_ptagPath_Plan)
				{
					resonlog << " 【计划指定 停机坪-出港跑道 的出港航班航路 不可用】";
					//重新分配一个停机坪相同的 出港航路可不同的
					_ptagPath_Plan = getplanpath(true, false, false);
					if (!_ptagPath_Plan)
					{
						resonlog << " 【未分配到 停机坪相同 出港跑道可不同 的出港航班航路】 ";
#ifdef USING_NEW_RELLOC_PARKING_POINT
						//重新分配一个停机坪不相同的 出港航路相同的 未使用过的
						_ptagPath_Plan = getplanpath(false, true, true);
						if (!_ptagPath_Plan)
						{
							resonlog << " 【未分配到 停机坪不相同 出港跑道相同的 出港航路】 ";
							//重新分配一个停机坪不相同的 出港航路不相同的 未使用过的
							_ptagPath_Plan = getplanpath(false, false, true);
							if (!_ptagPath_Plan)
							{
								resonlog << " 【未分配到 停机坪不相同 出港跑道不相同的 出港航路 -----【无法仿真】-----】";
							}
							else 
							{
								resonlog << " 【重新分配 停机坪不相同 出港跑道不相同 的出港航班航路】 ";
							}
						}
						else
						{
							resonlog << " 【重新分配 停机坪不相同 出港跑道相同 的出港航班航路】 ";
						}
#else
						resonlog << " 【-----【无法仿真】-----】 ";
#endif
					}
					else
					{
						resonlog << " 【重新分配 停机坪相同 出港跑道不同 的出港航班航路】";
					}
				}

				if (_ptagPath_Plan)
                {
                    bchanged = true;
                    flightinstanceid = occypyflightinstance.flightinstanceid;
				}
			}
			else
			{
				//停机坪被
				resonlog << " 停机位冲突 " << " 与 " << occypyflightinstance.printf();
				//存在使用其他停机坪 首先申请跑道相同的 不同停机坪
#ifdef USING_NEW_RELLOC_PARKING_POINT
				//重新分配一个停机坪不相同的 出港航路相同的 未使用过的
				_ptagPath_Plan = getplanpath(false, true, true);
				if (!_ptagPath_Plan)
				{
					resonlog << " 【未分配到 停机坪不相同 出港跑道相同的 出港航路】 ";
					//重新分配一个停机坪不相同的 出港航路不相同的 未使用过的
					_ptagPath_Plan = getplanpath(false, false, true);
					if (!_ptagPath_Plan)
					{
						resonlog << " 【未分配到 停机坪不相同 出港跑道不相同的 出港航路 -----【无法仿真】-----】 ";
					}
					else
					{
						resonlog << " 【重新分配 停机坪不相同 出港跑道不相同 的出港航班航路】 ";
					}
				}
				else
				{
					resonlog << " 【重新分配 停机坪不相同 出港跑道相同 的出港航班航路】 ";
				}
#else
				resonlog << " 【-----【无法仿真】-----】 ";
#endif
			}
		}
		else
		{
			//检查原始分配的出港航路是否有效，必须满足是未使用过的
			_ptagPath_Plan = getplanpath(true, true, true);
			if (!_ptagPath_Plan)
			{
				resonlog << " 【计划指定 停机坪-出港跑道 的出港航班航路 不可用】";
				//重新分配一个停机坪相同的 出港航路可不同的
				_ptagPath_Plan = getplanpath(true, false, true);
				if (!_ptagPath_Plan)
				{
					resonlog << " 【未分配到 停机坪相同 出港跑道可不同 的出港航班航路】";
#ifdef USING_NEW_RELLOC_PARKING_POINT
					//重新分配一个停机坪不相同的 出港航路相同的
					_ptagPath_Plan = getplanpath(false, true, true);
					if (!_ptagPath_Plan)
					{
						resonlog << " 【未分配到 停机坪不相同 出港跑道相同的 出港航路】";
						//重新分配一个停机坪不相同的 出港航路不相同的
						_ptagPath_Plan = getplanpath(false, false, true);
						if (!_ptagPath_Plan)
						{
							resonlog << " 【未分配到 停机坪不相同 出港跑道不相同的 出港航路 -----【无法仿真】-----】";
						}
						else
						{
							resonlog << " 【重新分配 停机坪不相同 出港跑道不相同 的出港航班航路】";
						}
					}
					else
					{
						resonlog << " 【重新分配 停机坪不相同 出港跑道相同 的出港航班航路】";
					}
#else
					resonlog << " 【-----【无法仿真】-----】 ";
#endif
				}
				else
				{
					resonlog << " 【重新分配 停机坪相同 出港跑道不同 的出港航班航路】";
				}
			}
		}
		if (_ptagPath_Plan)
		{
			pathplanvalidinfo.alloc_parkingpoint = _ptagPath_Plan->m_parkingpoint;
			pathplanvalidinfo.alloc_runway = _ptagPath_Plan->m_runway;

			//设置停机位占用状态，进港航班和首次出港航班
			DataManager::getInstance().m_parkingpointuseinfo.insert(std::make_pair(pathplanvalidinfo.alloc_parkingpoint, flightinstance{ flightinstanceid,pathplanvalidinfo, pflighltData ,_ptagPath_Plan }));
		
			auto flightinfos = flightinstance{ flightid,pathplanvalidinfo, pflighltData ,_ptagPath_Plan };
			DataManager::getInstance().appendFlight(flightid, flightinfos);

			resonlog_result << flightinfos.printf();
            genrateWps(_ptagPath_Plan);

            if(bchanged)
            {
                std::stringstream ssTMP;
                ssTMP <<" 进出港状态切换 " << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " " << runtimeid.toStdString()<<" wps "<<wps.size()<<"\n";
                std::cout<<ssTMP.str();
            }

            emit updatepoicolor_sig(pathplanvalidinfo.alloc_parkingpoint, QColor(255, 0, 0), DEFAULT_FONT_SIZE);
			std::stringstream ssTMP;
			ssTMP <<"停机位占用： " << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " 出港航班：" << flightinfos.printf();
			DataManager::getInstance().trans_log("", ssTMP, std::stringstream(),false);
		}
		else
		{
			resonlog_result << "[" << "计划停机位：" << pathplanvalidinfo.target_parkingpoint.toStdString() << " "
				<< "计划跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
				<< "使用停机位：" << pathplanvalidinfo.alloc_parkingpoint.toStdString() << " "
				<< "使用跑道：" << pathplanvalidinfo.target_runway.toStdString() << " "
				<< "航班计划：" << pflighltData->printf() << "]";
		}

		DataManager::getInstance().trans_log("仿真出港航班： ",resonlog_result, resonlog);

	}
	break;
    }
	return flightinstanceid;
}
