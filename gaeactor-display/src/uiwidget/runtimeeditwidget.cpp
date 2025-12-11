#include "runtimeeditwidget.h"

#include <QHBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QJsonObject>
#include <QJsonArray>
#include "mapeditwidget.h"
#include "LocationHelper.h"
#include "../components/global_variables.h"

RuntimeEditWidget::RuntimeEditWidget(QWidget *parent)
    :QWidget(parent),
    m_qmlWidget(nullptr),
    m_mapWidget(nullptr),
    m_currententityid(0),
    m_runtiemstyle_id(0)
{
    m_qmlWidget = new QQuickWidget();
    m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qmlWidget->rootContext()->setContextProperty("parentWidget",this);
    QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);
    m_qmlWidget->setSource(QUrl("qrc:/qml/runtimestylewidget.qml"));
    QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
    //QObject *pRoot = (QObject*)pWidget->rootObject();
    if (pRoot != NULL) {
        connect(pRoot, SIGNAL(qml_add_entity_signal()), this, SLOT(qml_add_entity_slot()));
        connect(pRoot, SIGNAL(qml_quit_agent_edit_panel_sig()), this, SIGNAL(qml_quit_agent_edit_panel_sig()));

        connect(this, SIGNAL(sendmsg_sig()), pRoot, SIGNAL(cSignal()));
    }
    m_qmlWidget->hide();

    this->setStyleSheet("RuntimeEditWidget{background-color:#2e2f30;}");
    m_mapWidget = new MapEditWidget(MapWidget::E_MAP_MODE_SELECT,this);
    m_mapWidget->hide();

    m_pLayout = new QHBoxLayout(this);
    m_pLayout->addWidget(m_mapWidget);
    m_pLayout->addWidget(m_qmlWidget);
    m_pLayout->setSpacing(10);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_pLayout);

    connect(m_mapWidget,&MapEditWidget::appendWaypoint_sig,this,&RuntimeEditWidget::appendWaypoint_slot);
    connect(m_mapWidget,&MapEditWidget::updateWaypoint_sig,this,&RuntimeEditWidget::updateWaypoint_slot);
    connect(m_mapWidget,&MapEditWidget::selectWaypoint_sig,this,&RuntimeEditWidget::selectWaypoint_slot);
}

RuntimeEditWidget::~RuntimeEditWidget()
{
    if(m_qmlWidget)
    {
        m_qmlWidget->deleteLater();
    }
    if(m_mapWidget)
    {
        m_mapWidget->deleteLater();
    }
    if(m_pLayout)
    {
        m_pLayout->deleteLater();
    }
}

void RuntimeEditWidget::setShow(bool bVisiable)
{
    m_qmlWidget->setVisible(bVisiable);
    m_mapWidget->setVisible(bVisiable);
    this->setVisible(bVisiable);
}

void RuntimeEditWidget::resetData()
{
    auto _entitywaypointmap_itor = m_entitywaypointmap.begin();
    while(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        auto _entityid = _entitywaypointmap_itor->first;
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;

        auto waypointlist_itor = waypointlist.begin();
        while(waypointlist_itor != waypointlist.end())
        {
            m_mapWidget->remove_entity_waypoint(_entityid, waypointlist_itor->waypointid,waypointlist, _entitywaypointmap_itor->second.cl);
            waypointlist_itor++;
        }
        waypointlist.clear();
        m_mapWidget->clear_entity_waypoint_tracking(_entityid);
        _entitywaypointmap_itor = m_entitywaypointmap.erase(_entitywaypointmap_itor);
    }
    m_runtiemstyle_id = 0;
    m_currententityid = 0;
    m_mapWidget->locate_entity_tracking(0);

//    QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//    QMetaObject::invokeMethod(pRoot,"resetData");

}

void RuntimeEditWidget::processArray(const QVariantList &arr)
{
    for (const QVariant &item : arr)
    {
        //qDebug() <<"recv"<< item.toString();
    }
}

void RuntimeEditWidget::setCurrentEntity(const QVariant &entityid)
{
    m_currententityid = entityid.toString().toULongLong();
    m_mapWidget->locate_entity_tracking(m_currententityid);
}

void RuntimeEditWidget::deleteEntity(const QVariant &entityid)
{
    qint64 _entityid = entityid.toString().toULongLong();
    auto _entitywaypointmap_itor = m_entitywaypointmap.find(_entityid);
    if(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;

        auto waypointlist_itor = waypointlist.begin();
        while(waypointlist_itor != waypointlist.end())
        {
            m_mapWidget->remove_entity_waypoint(_entityid, waypointlist_itor->waypointid,waypointlist, _entitywaypointmap_itor->second.cl);
            waypointlist_itor++;
        }
        waypointlist.clear();
        m_mapWidget->clear_entity_waypoint_tracking(_entityid);
        if(_entityid == m_currententityid)
        {
            m_mapWidget->locate_entity_tracking(0);
        }
        m_entitywaypointmap.erase(_entitywaypointmap_itor);
    }
}

void RuntimeEditWidget::locateWaypoint(const QVariant &entityid, const QVariant &waypointid)
{
    qint64 _entityid = entityid.toString().toULongLong();
    qint64 _waypointid = waypointid.toString().toULongLong();
    m_currententityid = _entityid;
    m_mapWidget->locate_entity_tracking(_entityid);
    m_mapWidget->locate_entity_waypoint(_entityid, _waypointid);
}

void RuntimeEditWidget::deleteWaypoint(const QVariant &entityid, const QVariant &waypointid)
{
    qint64 _entityid = entityid.toString().toULongLong();
    qint64 _waypointid = waypointid.toString().toULongLong();
    auto _entitywaypointmap_itor = m_entitywaypointmap.find(_entityid);
    if(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;
        auto waypointlist_itor = std::find_if(waypointlist.begin(),
                                              waypointlist.end(),
                                              [&](const std::list<waypointinfo>::value_type& vt){
                                                  return vt.waypointid == _waypointid;
                                              });
        if(waypointlist_itor != waypointlist.end())
        {
            waypointlist.erase(waypointlist_itor);
            m_mapWidget->remove_entity_waypoint(_entityid, _waypointid,waypointlist, _entitywaypointmap_itor->second.cl);
        }
    }
}

QVariant RuntimeEditWidget::getAgentKeysArray()
{
    return QVariant::fromValue(m_agentkeysarr);
}


void RuntimeEditWidget::saveRuntimeData(const QVariant &runtimedata)
{
    QMap<QString, QVariant>  data = runtimedata.toMap();
    if(m_runtiemstyle_id == 0)
    {
        return;
    }

    for(auto key:data.keys())
    {
        QVariant dt = data.value(key);
        switch(dt.type())
        {
//        case QVariant::String:runtimejsobj.insert(key, dt.toString());break;
        case QVariant::List:
        {
            QJsonArray patternAgentsobj;
            QList<QVariant>  lst = dt.toList();
            for(auto lstitem:lst)
            {
                QJsonObject patternAgentsitemobj;
                switch(lstitem.type())
                {
                case QVariant::Map:
                {
                    QMap<QString, QVariant> valmap = lstitem.toMap();
                    for(auto valmap_key:valmap.keys())
                    {
                        QVariant subdt = valmap.value(valmap_key);
                        switch(subdt.type())
                        {
                        case QVariant::String:patternAgentsitemobj.insert(valmap_key, subdt.toString());break;
                        case QVariant::Double:patternAgentsitemobj.insert(valmap_key, subdt.toDouble());break;
                        case QVariant::List:
                        {
							QJsonArray waypoints;
							/////////////////////////////////////////////////////////////////////////////
                            QList<QVariant>  waypointlst = subdt.toList();
                            for(auto waypointlstitem:waypointlst)
							{
								QJsonObject waypointsitem;

								waypointsitem.insert("wpsUsage", "");
								waypointsitem.insert("wpsKeyword", "");
								waypointsitem.insert("wpsFrame", 0);

								waypointsitem.insert("wpsGenPOIs", QJsonArray());
								waypointsitem.insert("wpsPathPlanner", "");
								waypointsitem.insert("wpsGenFences", QJsonArray());

								waypointsitem.insert("wpsGenTimeConsumed", 12.0);

								waypointsitem.insert("wpsKey", "wps_123456789");


								QJsonArray wps;
                                switch(waypointlstitem.type())
                                {
                                case QVariant::List:
                                {
									QJsonObject wpsitem;
									QJsonArray wpsCore;
                                    QList<QVariant>  waypointitemlst = waypointlstitem.toList();
                                    for(auto waypointitemlst_item:waypointitemlst)
                                    {
                                        switch(waypointitemlst_item.type())
                                        {
                                        case QVariant::Double:wpsCore.push_back(waypointitemlst_item.toDouble());break;
                                        default:break;
                                        }
                                    }
									wpsitem.insert("wpsCore", wpsCore);

									wpsitem.insert("useExt", 0);
									wpsitem.insert("speed", 0.0);
									wpsitem.insert("roll", 0.0);
									wpsitem.insert("pitch", 0.0);
									wpsitem.insert("yaw", 0.0);
									wpsitem.insert("yawEx", 0.0);

									wps.push_back(wpsitem);
                                }break;
                                default:break;
                                }
								waypointsitem.insert("wps", wps);
								waypoints.push_back(waypointsitem);
							}
							/////////////////////////////////////////////////////////////////////////////
							patternAgentsitemobj.insert("waypoints", waypoints);
                        }break;
                        default:
                            break;
                        }
                    }
                }break;
                default:break;
                }
                patternAgentsobj.push_back(patternAgentsitemobj);
            }
            m_runtimestyle_data.insert("patternAgents",patternAgentsobj);
        }break;
        default:break;
        }
    }
    emit add_runtime_style_sig(m_runtimestyle_data);
}

void RuntimeEditWidget::qml_add_entity_slot()
{
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();
    AgentInstanceInfo* pAgentInstanceInfo = nullptr;
    auto agentInstances_itor = agentInstances.begin();
    while (agentInstances_itor != agentInstances.end())
    {
        pAgentInstanceInfo = &agentInstances_itor->second;
        break;
        agentInstances_itor++;
    }

    quint64 entity_id = FunctionAssistant::generate_random_positive_uint64();
    if(pAgentInstanceInfo)
    {
        entity_id = pAgentInstanceInfo->m_agentinfo.agentKeyItem.agentId.toULongLong();
    }
    if(m_entitywaypointmap.empty())
    {
        m_currententityid = entity_id;
    }
    entity_waypoints_info info;
    if(pAgentInstanceInfo)
    {
        info = entity_waypoints_info{FunctionAssistant::randColor(255),
                                     pAgentInstanceInfo->m_agentinfo.agentKeyItem.agentKey,
                                     140.0,
                                     RUN_SPEED,
                                     0,
                                     pAgentInstanceInfo->m_agentinfo.agentKeyItem.agentId,
                                     pAgentInstanceInfo->m_agentinfo.agentKeyItem.agentNameI18n,
                                     "",
                                     pAgentInstanceInfo->m_agentinfo.agentKeyItem.agentName,
                                     "",
                                     "",
                                     std::list<waypointinfo>()};
    }
    else
    {
        info = entity_waypoints_info{FunctionAssistant::randColor(255),"", 140.0, RUN_SPEED, 0,QString::number(entity_id),"","","","","", std::list<waypointinfo>()};
    }

    m_entitywaypointmap.insert(std::make_pair(entity_id, std::move(info)));
    m_mapWidget->add_entity_waypoint_tracking(entity_id, info.cl);


//    QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//    QMetaObject::invokeMethod(pRoot,"appendEntity",
//                              Q_ARG(QVariant,QVariant::fromValue(QString::number(entity_id))),
//                              Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_azimuth))),
//                              Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_speed0))),
//                              Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_altitudeType))),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_AgentKey)),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentInstId)),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentEntityId)),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentLabel)),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentNote)),
//                              Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentIcon)));
}

void RuntimeEditWidget::appendWaypoint_slot(quint64 waypointid, double lng, double lat)
{    
	LAT_LNG curpos{ lat,lng };

	//if (m_mapWidget->bTransfer())
	//{
	//	auto corretgeo = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(lat, lng);
	//	curpos = LAT_LNG{ corretgeo.latitude(), corretgeo.longitude() };
	//}
    auto _entitywaypointmap_itor = m_entitywaypointmap.find(m_currententityid);
    if(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;
        waypointinfo val{waypointid,lng,lat,0,1,(int)(waypointlist.size())*10};
        waypointlist.push_back(val);
//        QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//        QMetaObject::invokeMethod(pRoot,"appendWaypoint",
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(m_currententityid))),
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(waypointid))),
//                                  Q_ARG(QVariant,QVariant::fromValue(val.lng)),
//                                  Q_ARG(QVariant,QVariant::fromValue(val.lat)),
//                                  Q_ARG(QVariant,QVariant::fromValue(val.alt)),
//                                  Q_ARG(QVariant,QVariant::fromValue(val.timestamp)));

        m_mapWidget->add_entity_waypoint(m_currententityid,waypointid,curpos, waypointlist, _entitywaypointmap_itor->second.cl);

        m_mapWidget->locate_entity_tracking(m_currententityid);
    }
}

void RuntimeEditWidget::updateWaypoint_slot(quint64 waypointid, double lng, double lat)
{
    auto _entitywaypointmap_itor = m_entitywaypointmap.begin();
    while(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        quint64 _currententityid = _entitywaypointmap_itor->first;
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;
        auto waypointlist_itor = std::find_if(waypointlist.begin(),
                                              waypointlist.end(),
                                              [&](const std::list<waypointinfo>::value_type &vt){
                                                  return vt.waypointid == waypointid;
                                              });
        if(waypointlist_itor != waypointlist.end())
        {
            waypointinfo &info = *waypointlist_itor;
            info.lng = lng;
            info.lat = lat;
            m_currententityid = _currententityid;
//            QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//            QMetaObject::invokeMethod(pRoot,"updateWaypoint",
//                                      Q_ARG(QVariant,QVariant::fromValue(QString::number(m_currententityid))),
//                                      Q_ARG(QVariant,QVariant::fromValue(QString::number(waypointid))),
//                                      Q_ARG(QVariant,QVariant::fromValue(lng)),
//                                      Q_ARG(QVariant,QVariant::fromValue(lat)));

            LAT_LNG curpos{lat,lng};
            m_mapWidget->add_entity_waypoint(m_currententityid,waypointid,curpos, waypointlist, _entitywaypointmap_itor->second.cl);
            m_mapWidget->locate_entity_tracking(m_currententityid);
            m_mapWidget->locate_entity_waypoint(m_currententityid,waypointid);

            break;
        }
        _entitywaypointmap_itor++;
        //        QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

        //        QMetaObject::invokeMethod(pRoot,"updateWaypoint",
        //                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(m_currententityid))),
        //                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(waypointid))),
        //                                  Q_ARG(QVariant,QVariant::fromValue(lng)),
        //                                  Q_ARG(QVariant,QVariant::fromValue(lat)));

        //        LAT_LNG curpos{lat,lng};
        //        m_mapWidget->add_entity_waypoint(m_currententityid,waypointid,curpos, waypointlist, std::get<0>(_entitywaypointmap_itor->second));

        //        m_mapWidget->locate_entity_tracking(m_currententityid);
    }
}

void RuntimeEditWidget::selectWaypoint_slot(quint64 waypointid)
{
    bool bExist = false;
    auto _entitywaypointmap_itor = m_entitywaypointmap.begin();
    while(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        quint64 _currententityid = _entitywaypointmap_itor->first;
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;
        auto waypointlist_itor = std::find_if(waypointlist.begin(),
                                              waypointlist.end(),
                                              [&](const std::list<waypointinfo>::value_type &vt){
                                                  return vt.waypointid == waypointid;
                                              });
        if(waypointlist_itor != waypointlist.end())
        {
            waypointinfo &info = *waypointlist_itor;
            m_currententityid = _currententityid;
            m_mapWidget->locate_entity_tracking(m_currententityid);
            m_mapWidget->locate_entity_waypoint(m_currententityid,waypointid);
            bExist = true;
            break;
        }
        _entitywaypointmap_itor++;
    }
    if(!bExist)
    {
        m_mapWidget->locate_entity_tracking(0);
        m_mapWidget->locate_entity_waypoint(0,0);
    }
}

quint64 RuntimeEditWidget::runtiemstyle_id() const
{
    return m_runtiemstyle_id;
}

void RuntimeEditWidget::setRuntiemstyle_id(quint64 newRuntiemstyle_id)
{
    m_runtiemstyle_id = newRuntiemstyle_id;
}

void RuntimeEditWidget::updateAgentKeys(const std::unordered_map<QString, std::tuple<QString, QString> > &_agentkeysdata)
{
	m_agentkeysarr = QJsonArray();
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();
    auto agentInstances_itor = agentInstances.begin();
    while (agentInstances_itor != agentInstances.end())
    {
		QJsonObject subjs;
        agentInstances_itor->second.toJson(subjs);
		m_agentkeysarr.push_back(subjs);
        agentInstances_itor++;
    }
}

void RuntimeEditWidget::showEvent(QShowEvent *event)
{
    m_qmlWidget->show();
    m_mapWidget->setVisible(true);
    QWidget::showEvent(event);
}

void RuntimeEditWidget::hideEvent(QHideEvent *event)
{
    m_qmlWidget->hide();
    m_mapWidget->setVisible(false);
    QWidget::hideEvent(event);
}

void RuntimeEditWidget::decodeRuntimeStyle(const QJsonObject &runtimestyle_data)
{
    m_runtimestyle_data = runtimestyle_data;
    setRuntiemstyle_id(m_runtimestyle_data.value("patternSig").toString().toULongLong());
    auto runtimedata = m_runtimestyle_data.value("patternAgents").toArray();
    for(int i = 0;i < runtimedata.count();i++)
    {
        auto patternAgentsitem = runtimedata.at(i).toObject();

        entity_waypoints_info en_way_info;

        en_way_info.cl = FunctionAssistant::randColor(255);
        en_way_info.m_azimuth = patternAgentsitem.value("azimuth").toDouble();
        en_way_info.m_speed0 = patternAgentsitem.value("speed0").toDouble();
        en_way_info.m_altitudeType = patternAgentsitem.value("altitudeType").toInt(0);
        en_way_info.m_AgentKey = patternAgentsitem.value("agentKey").toString();


        en_way_info.m_agentId = patternAgentsitem.value("agentId").toString();

        quint64 entity_id =  en_way_info.m_agentId.toULongLong();
        en_way_info.m_agentInstId = patternAgentsitem.value("agentInstId").toString();
        en_way_info.m_agentEntityId = patternAgentsitem.value("agentEntityId").toString();
        en_way_info.m_agentLabel = patternAgentsitem.value("agentLabel").toString();
        en_way_info.m_agentNote = patternAgentsitem.value("agentNote").toString();
        en_way_info.m_agentIcon = patternAgentsitem.value("agentIcon").toString();


		auto waypoints = patternAgentsitem.value("waypoints").toArray();
		for (int i = 0; i < waypoints.count(); i++)
		{
			auto waypointsitem = waypoints[i].toObject();
			auto wps = waypointsitem.value("wps").toArray();
			for (int j = 0; j < wps.count(); j++)
			{
				auto wpsCore = wps[j].toObject().value("wpsCore").toArray();
				waypointinfo waypt;
				if (!wpsCore.empty())
				{
					waypt.waypointid = FunctionAssistant::generate_random_positive_uint64();
					waypt.lng = wpsCore[0].toDouble();
					waypt.lat = wpsCore[1].toDouble();
					waypt.alt = wpsCore[2].toDouble();
					waypt.alttype = wpsCore[3].toDouble();
					waypt.timestamp = wpsCore[4].toDouble();
				}
				en_way_info.m_altitudeType = waypt.alttype;
				en_way_info.m_waypts.push_back(waypt);
			}
		}

        m_entitywaypointmap.insert(std::make_pair(entity_id, std::move(en_way_info)));
        m_mapWidget->add_entity_waypoint_tracking(entity_id, m_entitywaypointmap.at(entity_id).cl);
//        QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//        QMetaObject::invokeMethod(pRoot,"appendEntity",
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(entity_id))),
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_azimuth))),
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_speed0))),
//                                  Q_ARG(QVariant,QVariant::fromValue(QString::number(m_entitywaypointmap.at(entity_id).m_altitudeType))),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_AgentKey)),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentInstId)),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentEntityId)),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentLabel)),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentNote)),
//                                  Q_ARG(QVariant,QVariant::fromValue(m_entitywaypointmap.at(entity_id).m_agentIcon)));




        m_currententityid = entity_id;
        m_mapWidget->locate_entity_tracking(m_currententityid);


        auto _wayptsitor = m_entitywaypointmap.at(entity_id).m_waypts.begin();
        while(_wayptsitor != m_entitywaypointmap.at(entity_id).m_waypts.end())
        {
            waypointinfo &waypt = *_wayptsitor;

//            QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//            QMetaObject::invokeMethod(pRoot,"appendWaypoint",
//                                      Q_ARG(QVariant,QVariant::fromValue(QString::number(entity_id))),
//                                      Q_ARG(QVariant,QVariant::fromValue(QString::number(waypt.waypointid))),
//                                      Q_ARG(QVariant,QVariant::fromValue(waypt.lng)),
//                                      Q_ARG(QVariant,QVariant::fromValue(waypt.lat)),
//                                      Q_ARG(QVariant,QVariant::fromValue(waypt.alt)),
//                                      Q_ARG(QVariant,QVariant::fromValue(waypt.timestamp)));

            LAT_LNG curpos{waypt.lat, waypt.lng};
            m_mapWidget->add_entity_waypoint(entity_id, waypt.waypointid, curpos, m_entitywaypointmap.at(entity_id).m_waypts, m_entitywaypointmap.at(entity_id).cl,true);
            _wayptsitor++;
        }
    }
}
