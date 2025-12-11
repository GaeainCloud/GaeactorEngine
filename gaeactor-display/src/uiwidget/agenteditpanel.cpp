#pragma execution_character_set("utf-8")
#include "agenteditpanel.h"

#include <QHBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QStackedWidget>

#include "widget3d/OSGManager.h"
#include "widget3d/ModelSceneData.h"
#include "components/function.h"
#include "../components/global_variables.h"
#include "mapeditwidget.h"
#include "widget3d/NodeInstance.h"

#define DISTANCE_ARR_AREA_LEN (2500)
#define DISTANCE_DEP_AREA_LEN (1500)

///////////////////////////////////////////////////////////////////////////////////////
QtModelWidget::QtModelWidget(QtOSGWidget::E_OSG_SHOW_TYPE show_type, QWidget* parent)
    :QtModelWidgetBase(parent),
	m_qmlpanelWidget(nullptr),
	m_pModelWidget(nullptr),
	m_pMapEditWidget(nullptr)
{
	setMouseTracking(true);
	m_treeViewController = new TreeViewController(this);
	connect(m_treeViewController, &TreeViewController::edit_type_sig, this, &QtModelWidget::edit_type_slot);
	connect(m_treeViewController, &TreeViewController::add_type_sig, this, &QtModelWidget::add_type_slot);

	m_pModelWidget = new QtOSGWidget(show_type, this);
	m_pModelWidget->hide();
	if (m_pModelWidget)
    {
        m_pModelWidget->pOSGManager()->getModelSenceData()->setEditWidget(this);
	}

	m_pMapEditWidget = new MapEditWidget(MapWidget::E_MAP_MODE_SELECT, this);
	connect(m_pMapEditWidget, &MapEditWidget::appendWaypoint_sig, this, &QtModelWidget::appendWaypoint_slot);
	connect(m_pMapEditWidget, &MapEditWidget::updateWaypoint_sig, this, &QtModelWidget::updateWaypoint_slot);
	connect(m_pMapEditWidget, &MapEditWidget::selectWaypoint_sig, this, &QtModelWidget::selectWaypoint_slot);
	m_pModelWidget->hide();


	m_pQStackedWidget = new QStackedWidget(this);


	m_pQStackedWidget->addWidget(m_pModelWidget);
	m_pQStackedWidget->addWidget(m_pMapEditWidget);

	QVBoxLayout* _pLayout = new QVBoxLayout(this);
	_pLayout->addWidget(m_pQStackedWidget);
	_pLayout->setSpacing(0);
	_pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(_pLayout);

	m_pQStackedWidget->setCurrentIndex(0);

	/////////////////////////////////////////////////////////////////////////
	m_qmlpanelWidget = new QQuickWidget(this);
	m_qmlpanelWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

	m_qmlpanelWidget->rootContext()->setContextProperty("mControl", m_treeViewController);
	m_qmlpanelWidget->rootContext()->setContextProperty("parentWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlpanelWidget);
	m_qmlpanelWidget->setSource(QUrl("qrc:/qml/agenteditpanelwidget.qml"));

	m_qmlpanelWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
	m_qmlpanelWidget->setAttribute(Qt::WA_TranslucentBackground);
	m_qmlpanelWidget->setClearColor(Qt::transparent);  // 设置 QML 视图的背景为透明

	QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
		connect(pRoot, SIGNAL(qml_quit_agent_edit_panel_sig()), this, SIGNAL(qml_quit_agent_edit_panel_sig()));
		connect(pRoot, SIGNAL(qml_quit_agent_edit_panel_sig()), this, SLOT(qml_quit_agent_edit_panel_slot()));
	}
	m_qmlpanelWidget->hide();

	//initTreemodel();
}

QtModelWidget::~QtModelWidget()
{
	if (m_qmlpanelWidget)
	{
		m_qmlpanelWidget->deleteLater();
	}
}

void QtModelWidget::saveAgent()
{
	QJsonObject savejson = m_agentData.m_general;

	auto encodePorperty = [&](const QString& keystr, std::unordered_map<uint64_t, QJsonObject>& databuf)
	{
		QJsonArray jsarr;
		auto itor = databuf.begin();
		while (itor != databuf.end())
		{
			uint64_t id = itor->first;
			if (keystr == "pois")
			{
				QJsonObject jsonobj = itor->second;

				auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
					m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
					return vt.first == id && vt.second.m_geometry_id == id && vt.second.m_pts.size() == 1;
				});
				if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
				{
					QJsonArray wpslistitem;
					auto ptitor = _geometrypointmap_itor->second.m_pts.begin();
					while (ptitor != _geometrypointmap_itor->second.m_pts.end())
					{
						wpslistitem.push_back(ptitor->lng);
						wpslistitem.push_back(ptitor->lat);
						wpslistitem.push_back(ptitor->alt);
						ptitor++;
					}
					jsonobj.insert("poiPoint", wpslistitem);

					QJsonObject geometryitem;
					QJsonArray  coordinates;
					QJsonArray  coordinatesitem;
					auto coordinatesExtenditor = _geometrypointmap_itor->second.coordinatesExtend.begin();
					while (coordinatesExtenditor != _geometrypointmap_itor->second.coordinatesExtend.end())
					{
						std::vector<LAT_LNG>& subcoordinates = std::get<0>(*coordinatesExtenditor);
						for (int i = 0; i < subcoordinates.size(); i++)
						{
							QJsonArray  coordinate;
							coordinate.push_back(subcoordinates.at(i).lng);
							coordinate.push_back(subcoordinates.at(i).lat);
							coordinatesitem.push_back(coordinate);
						}
						coordinatesExtenditor++;
					}
					coordinates.push_back(coordinatesitem);
					geometryitem.insert("coordinates", coordinates);
					geometryitem.insert("type", "Polygon");


					QJsonArray features;
					QJsonObject featuresitem;
					featuresitem.insert("type", "Feature");
					featuresitem.insert("properties", QJsonObject());
					featuresitem.insert("geometry", geometryitem);
					features.push_back(featuresitem);

					QJsonObject geojson;
					geojson.insert("type", "FeatureCollection");
					geojson.insert("features", features);
					jsonobj.insert("poiGeoJSON", geojson);

					QJsonArray poiVarDefs;

					QJsonObject poiVarDefsItem;
					poiVarDefsItem.insert("varKeyword", "tkflnd");
					poiVarDefsItem.insert("varSig", "tkflnd");
					poiVarDefsItem.insert("varName", "tkflnd");
					poiVarDefsItem.insert("varNameI18n", "tkflnd");
					poiVarDefsItem.insert("i18nLabels", QJsonArray());
					poiVarDefsItem.insert("varType", "Integer");
					poiVarDefsItem.insert("stdCode", "123456");
					poiVarDefsItem.insert("varSchema", QJsonObject());
					QJsonArray varDefault;
					varDefault.push_back("0");
					poiVarDefsItem.insert("varDefault", varDefault);
					poiVarDefsItem.insert("access", 0);
					poiVarDefs.push_back(poiVarDefsItem);

					jsonobj.insert("poiVarDefs",poiVarDefs);					
				}
				jsarr.push_back(jsonobj);

			}
			else if (keystr == "fences")
			{
				QJsonObject jsonobj = itor->second;

				auto _geometrypointmap_itor = std::find_if(m_agentData.m_fence_geometrypointmap.begin(),
					m_agentData.m_fence_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
					return vt.first == id && vt.second.m_geometry_id == id && !vt.second.m_pts.empty();
				});
				if (_geometrypointmap_itor != m_agentData.m_fence_geometrypointmap.end())
				{
					QJsonArray wpslist;
					auto ptitor = _geometrypointmap_itor->second.m_pts.begin();
					while (ptitor != _geometrypointmap_itor->second.m_pts.end())
					{
						QJsonArray wpslistitem;
						wpslistitem.push_back(ptitor->lng);
						wpslistitem.push_back(ptitor->lat);
						wpslistitem.push_back(ptitor->alt);
						wpslist.push_back(wpslistitem);
						ptitor++;
					}
					jsonobj.insert("fencePoints", wpslist);
				}
				jsarr.push_back(jsonobj);
			}
			else
			{
				auto jsonobj = itor->second;
				jsarr.push_back(jsonobj);
			}
			itor++;
		}
		savejson.insert(keystr, jsarr);
	};

	encodePorperty("pois", m_agentData.m_pois);
	encodePorperty("fences", m_agentData.m_fences);


	encodePorperty("vardefs", m_agentData.m_params);
	encodePorperty("waypoints", m_agentData.m_waypoints);
	encodePorperty("axns", m_agentData.m_actions);
	encodePorperty("oodas", m_agentData.m_oodas);
	encodePorperty("modelAbstracted", m_agentData.m_modelAbstracted);


	///////////////////////////////////////////////////////////////////////////
	QJsonArray fieldarr;
	auto fielditor = m_agentData.m_fieldsensing.begin();
	while (fielditor != m_agentData.m_fieldsensing.end())
	{
		QJsonObject fieldjson = m_agentData.m_field.at(fielditor->first);
		QJsonArray smdarr;
		std::list<uint64_t>& sensinglist = fielditor->second;
		auto sensinglistitor = sensinglist.begin();
		while (sensinglistitor != sensinglist.end())
		{
            QJsonObject smdval = m_agentData.m_sensing.at(*sensinglistitor);
            smdval.remove("roll");
            smdval.remove("smdkey");
			smdarr.push_back(m_agentData.m_sensing.at(*sensinglistitor));
			sensinglistitor++;
		}


		fieldjson.insert("smds", smdarr);
		fieldarr.push_back(fieldjson);
		fielditor++;
	}
	savejson.insert("fldmds", fieldarr);
	///////////////////////////////////////////////////////////////////////////

	QJsonArray freelanceableDynamicsarray;
	QJsonObject freelanceableDynamics;
	freelanceableDynamics.insert("dynPluginName", "");
	freelanceableDynamics.insert("dynKeyword", "");
	freelanceableDynamics.insert("dynAsDefault", true);
	freelanceableDynamics.insert("initPathName", "");

	QJsonObject freelanceablesettingobj;
	freelanceablesettingobj.insert("name", "");
	freelanceableDynamics.insert("dynSettings", freelanceablesettingobj);
	freelanceableDynamicsarray.push_back(freelanceableDynamics);


	QJsonArray missionableDynamicsarray;
	QJsonObject missionableDynamics;
    missionableDynamics.insert("dynPluginName", "iagnt_dynamics_linear_trajectory");
//	missionableDynamics.insert("dynPluginName", "iagnt_dynamics_linear_trajectory");
	
	missionableDynamics.insert("dynKeyword", "MissionDynamicsDefault");
	missionableDynamics.insert("dynAsDefault", true);
	missionableDynamics.insert("initPathName", "");

	QJsonObject missionablesettingobj;
	missionablesettingobj.insert("name", "");
	missionableDynamics.insert("dynSettings", missionablesettingobj);
	missionableDynamicsarray.push_back(missionableDynamics);


	savejson.insert("missionableDynamics", missionableDynamicsarray);
	savejson.insert("freelanceableDynamics", freelanceableDynamicsarray);
	savejson.insert("locatableDynamics", QJsonArray());
	savejson.insert("navigatableDynamics", QJsonArray());

	std::cout << "save agent data" << FunctionAssistant::json_object_to_string(savejson,false).toStdString()<<std::endl;



	if (DataManager::getInstance().pHttpClient())
	{
		if (m_bNewAgent)
		{
			DataManager::getInstance().pHttpClient()->append_agent_data(savejson);
		}
		else
		{
			DataManager::getInstance().pHttpClient()->update_agent_data(savejson);
		}
	}
}

void QtModelWidget::deleteAgent()
{
	QJsonObject savejson = m_agentData.m_general;
	if (DataManager::getInstance().pHttpClient())
	{
		DataManager::getInstance().pHttpClient()->delete_agent_data(savejson);
	}
}

void QtModelWidget::addSubNode(const QModelIndex& index)
{
	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		uint64_t id;

		QModelIndex addmodelindex = pTreeModel->appendSubChild(index, id);

		QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
		QMetaObject::invokeMethod(pRoot, "expand",
			Q_ARG(QVariant, QVariant::fromValue(index)));

		QMetaObject::invokeMethod(pRoot, "selectIndex",
			Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));
	}
}

#define DRAW_AIRPORT_POLYGONS
#define DRAW_AIRPORT_LINES
#define DRAW_AIRPORT_POINTS

void QtModelWidget::addModelNode(const QModelIndex& index, const QVariant& modelpath)
{
	QList<QUrl> modelslist = modelpath.value<QList<QUrl>>();

	for (auto modelslistitem : modelslist)
	{
		auto filepathlist = modelslistitem.toString().split("///");
		if (filepathlist.size() == 2)
		{
			QString modelfilepath = filepathlist.at(1);

			switch (m_eAgentType)
			{
			case E_AGENT_TYPE_INSTAGENT:
			{
				if (m_pModelWidget)
				{
                    m_pModelWidget->pOSGManager()->addModelNode(modelfilepath, index, true);
				}
			}
			break;
			case E_AGENT_TYPE_SCENE:
			{
				if (m_pMapEditWidget)
				{
					uint64_t geodataid = FunctionAssistant::generate_random_positive_uint64();
					GeoJsonInfos geoinfos = m_pMapEditWidget->addJsonFile(modelfilepath);

					QJsonObject jsondata;

					QJsonArray latlngsarray;

					for (auto subcoordinates : geoinfos.subItem)
					{
						switch (geoinfos.type) {
						case E_GEOTYPE_POINT:
						{
							for (auto subcoordinatessub : subcoordinates.coordinates)
							{
								QJsonArray pointLatLngarray;
								for (auto subcoordinatesitem : subcoordinatessub)
								{
									QJsonArray ptarray;
									//0 x --> lng  1 y-->lat
									ptarray.push_back(subcoordinatesitem.lng);
									ptarray.push_back(subcoordinatesitem.lat);
									pointLatLngarray.push_back(ptarray);
									///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								}
								latlngsarray.push_back(pointLatLngarray);
							}
						}
						break;
						case E_GEOTYPE_LINE:
						{
							for (auto coordinatesExtenditem : subcoordinates.coordinatesExtend)
							{
								std::vector<LAT_LNG>& subcoordinates = std::get<0>(coordinatesExtenditem);

								QJsonArray lineLatLngarray;

								for (int i = 0; i < subcoordinates.size(); i++)
								{
									double latx = subcoordinates.at(i).lat;
									double lonx = subcoordinates.at(i).lng;

									QJsonArray ptarray;
									//0 x --> lng  1 y-->lat
									ptarray.push_back(lonx);
									ptarray.push_back(latx);
									lineLatLngarray.push_back(ptarray);
								}
								latlngsarray.push_back(lineLatLngarray);
							}
						}
						break;
						case E_GEOTYPE_POLYGON:
						case E_GEOTYPE_MULITPOLYGON:
						{
							for (auto listitem : subcoordinates.coordinates)
							{
								QJsonArray polygonLngarray;
								for (int i = 0; i < listitem.size(); i++)
								{
									double latx = listitem.at(i).lat;
									double lonx = listitem.at(i).lng;
									QJsonArray ptarray;
									//0 x --> lng  1 y-->lat
									ptarray.push_back(lonx);
									ptarray.push_back(latx);
									polygonLngarray.push_back(ptarray);
								}
								latlngsarray.push_back(polygonLngarray);
							}
						}
						break;
						default:
							break;
						}
					}

					jsondata.insert("abstProfileKeyword", QString::number(geodataid));
					jsondata.insert("abstProfileFrame", 0);
					jsondata.insert("abstHeights", QJsonArray());
                    jsondata.insert("abstHeightResolution", INDEX_MAPPING_RESOLUTION_AREA_POS);
                    jsondata.insert("abstProfileResolution", INDEX_MAPPING_RESOLUTION_AREA_POS);
					jsondata.insert("abstProfileLonLat", latlngsarray);
					jsondata.insert("abstProfileCarteXY", QJsonArray());

					m_agentData.m_modelAbstracted.insert(std::make_pair(geodataid, std::move(jsondata)));
					QFileInfo fileinfo(modelfilepath);

					QPair<uint64_t, QString> agentInfo = qMakePair(geodataid, fileinfo.fileName());
					TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());

					if (pTreeModel)
					{
						QModelIndex addmodelindex = pTreeModel->appendSubAgentChild(index, agentInfo);

						QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
						QMetaObject::invokeMethod(pRoot, "expand",
							Q_ARG(QVariant, QVariant::fromValue(index)));

						QMetaObject::invokeMethod(pRoot, "selectIndex",
							Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));
					}
				}
			}
			break;
			default:
				break;
			}

		}
	}
}

void QtModelWidget::switchHideModel(const QVariant& datatye, const QVariant& instanceid, const QVariant& modelid)
{
	int datatypeval = datatye.toInt();
	uint64_t nodeid = modelid.toString().toULongLong();
	uint64_t instanceidval = instanceid.toString().toULongLong();
	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
			switch (datatypeval)
			{
			case 0:
			{
                m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSwitch(nodeid, true);
			}
			break;
			case 1:
			{
                m_pModelWidget->pOSGManager()->getModelSenceData()->switchSlot(instanceidval, nodeid, true);
			}
			break;
			}
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}
}

void QtModelWidget::selectModel(const QVariant& datatye, const QVariant& instanceid, const QVariant& modelid)
{
	int datatypeval = datatye.toInt();
	uint64_t nodeid = modelid.toString().toULongLong();
	uint64_t instanceidval = instanceid.toString().toULongLong();
	switch (datatypeval)
	{
	case 0:
	{
		TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
		if (pTreeModel)
		{
			QModelIndex agentmodelindex;
			if (pTreeModel->getTargetModelIndex(nodeid, agentmodelindex))
			{
				QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();

				QMetaObject::invokeMethod(pRoot, "selectIndex",
					Q_ARG(QVariant, QVariant::fromValue(agentmodelindex)));
			}
		}
	}
	break;
	case 1:
	{
	}
	break;
	}
	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSelect(datatypeval, instanceidval, nodeid, true);
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

}

void QtModelWidget::updateFieldName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString contextstr = context.toString();
	updateFieldNodeContext(type, edit_instance_id, contextstr);
}

void QtModelWidget::updateActionName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
    QString contextstr = context.toString();
    updateActionNodeContext(type, edit_instance_id, contextstr);
}

void QtModelWidget::updatePoiName(const QVariant &dataType_Id, const QVariant &edit_id, const QVariant &context)
{
    TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
    uint64_t edit_instance_id = edit_id.toULongLong();
    QString contextstr = context.toString();
    updatePoiNodeContext(type, edit_instance_id, contextstr);
}

void QtModelWidget::updateOODAName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString contextstr = context.toString();
	updateOODANodeContext(type, edit_instance_id, contextstr);
}

Q_INVOKABLE void QtModelWidget::updateSensingRadius(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString contextstr = context.toString();

	float radius = contextstr.toDouble();

	auto itor = std::find_if(m_agentData.m_sensing.begin(),
		m_agentData.m_sensing.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == edit_id;
	});
	if (itor != m_agentData.m_sensing.end())
	{
		QJsonObject& datacontext = itor->second;
		datacontext["radius"] = radius;
	}

	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->updateSensingRadius(edit_instance_id, radius);
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

}

void QtModelWidget::updateFieldPos(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& offsetx, const QVariant& offsety, const QVariant& offsetz)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString offsetxstr = offsetx.toString();
	QString offsetystr = offsety.toString();
	QString offsetzstr = offsetz.toString();

	auto itor = std::find_if(m_agentData.m_field.begin(),
		m_agentData.m_field.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == edit_id;
	});
	if (itor != m_agentData.m_field.end())
	{
		QJsonObject& datacontext = itor->second;

		datacontext["offsetx"] = offsetxstr.toDouble();
		datacontext["offsety"] = offsetystr.toDouble();
		datacontext["offsetz"] = offsetzstr.toDouble();
	}

	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->updateField(edit_instance_id, osg::Vec3f(offsetxstr.toDouble(), offsetystr.toDouble(), offsetzstr.toDouble()));
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

}

void QtModelWidget::updateSensingAngle(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& yawAngle, const QVariant& yawFlareAngle, const QVariant& pitchAngle, const QVariant& pitchFlareAngle)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString yawAnglelestr = yawAngle.toString();
	QString yawFlareAnglestr = yawFlareAngle.toString();
	QString pitchAnglestr = pitchAngle.toString();
	QString pitchFlareAnglestr = pitchFlareAngle.toString();


	auto itor = std::find_if(m_agentData.m_sensing.begin(),
		m_agentData.m_sensing.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == edit_id;
	});
	if (itor != m_agentData.m_sensing.end())
	{
		QJsonObject& datacontext = itor->second;

		datacontext["pitch"] = pitchAngle.toDouble();
		datacontext["dpch"] = pitchFlareAngle.toDouble();
		datacontext["azimuth"] = yawAnglelestr.toDouble();
		datacontext["dazm"] = yawFlareAngle.toDouble();
	}

	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->updateSensing(edit_instance_id,
				yawAnglelestr.toDouble(),
				yawFlareAngle.toDouble(),
				pitchAngle.toDouble(),
				pitchFlareAngle.toDouble());
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

}

Q_INVOKABLE void QtModelWidget::updatePoiPos(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& offsetx, const QVariant& offsety, const QVariant& offsetz)
{

	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString offsetxstr = offsetx.toString();
	QString offsetystr = offsety.toString();
	QString offsetzstr = offsetz.toString();
	
	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		auto _poisitor = m_agentData.m_pois.find(edit_instance_id);
		if (_poisitor != m_agentData.m_pois.end())
		{
			QJsonObject& poiobj = _poisitor->second;

			QString poiName = poiobj.value("poiName").toString();
			QString poiKeyword = poiobj.value("poiKeyword").toString();
			int poiFrame = poiobj.value("poiFrame").toInt();
			QJsonArray poipoint = poiobj.value("poiPoint").toArray();
			if (poiFrame == 1 && poipoint.size() == 3)
			{
				QJsonArray poipointnew;
				poipointnew.push_back(offsetxstr.toDouble());
				poipointnew.push_back(offsetystr.toDouble());
				poipointnew.push_back(offsetzstr.toDouble());
				poiobj.insert("poiPoint", poipointnew);
				if (m_pModelWidget)
				{
                    m_pModelWidget->pOSGManager()->updatePoi(edit_instance_id, offsetxstr.toDouble(), offsetystr.toDouble(), offsetzstr.toDouble());
				}
			}
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}
}

QVariant QtModelWidget::getAgentType()
{
	return QVariant::fromValue(m_agent_type);
}


void QtModelWidget::updateContext(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QMap<QString, QVariant>  data = context.toMap();

	QJsonObject contextJsonobj;
	for (auto key : data.keys())
	{
		QVariant dt = data.value(key);
		switch (dt.type())
		{
		case QVariant::String:contextJsonobj.insert(key, dt.toString()); break;
		case QVariant::Bool:contextJsonobj.insert(key, dt.toBool()); break;
		case QVariant::Int:contextJsonobj.insert(key, dt.toInt()); break;
		case QVariant::LongLong:contextJsonobj.insert(key, dt.toLongLong()); break;
		case QVariant::Double:contextJsonobj.insert(key, dt.toDouble()); break;
		case QVariant::List:
		{
			QJsonArray jsarry;

			QList<QVariant>  sublst = dt.toList();
			for (auto subitem : sublst)
			{
				switch (subitem.type())
				{
				case QVariant::String:
				{
					if (!subitem.toString().isEmpty())
					{
						jsarry.push_back(subitem.toString());
					}
				}break;
				case QVariant::Bool:jsarry.push_back(subitem.toBool()); break;
				case QVariant::Int:jsarry.push_back(subitem.toInt()); break;
				case QVariant::LongLong:jsarry.push_back(subitem.toLongLong()); break;
				case QVariant::Double:jsarry.push_back(subitem.toDouble()); break;
				default:break;
				}
			}
			contextJsonobj.insert(key, jsarry);
		}; break;
		default:break;
		}
	}

	auto updateContextjson = [&](std::unordered_map<uint64_t, QJsonObject>& datamap)
	{
		auto itor = std::find_if(datamap.begin(),
			datamap.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == edit_instance_id;
		});
		if (itor != datamap.end())
		{
			itor->second = contextJsonobj;
		}
	};
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_NULL:
		break;
	case TreeNode::E_NODE_TYPE_CONFIGS:
	{
		if (contextJsonobj.contains("stopCond"))
		{
			QString vardefschemastr = contextJsonobj.value("stopCond").toString();
			QJsonObject vardefschemaobj = FunctionAssistant::string_to_json_object(vardefschemastr);
			contextJsonobj.insert("stopCond", vardefschemaobj);
		}
		m_agentData.m_general = contextJsonobj;
		saveAgent();
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS:
		break;
	case TreeNode::E_NODE_TYPE_ACTIONS:
	{
		qDebug() << "save action " << contextJsonobj;
		updateContextjson(m_agentData.m_actions);

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->update_action_data(m_currentagentKey, contextJsonobj);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
		break;
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	{
		qDebug() << "save waypoints " << contextJsonobj;
		updateContextjson(m_agentData.m_waypoints);
		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->update_action_data(m_currentagentKey,contextJsonobj);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_OODAS:
	{
		qDebug() << "save oodas " << contextJsonobj;
		updateContextjson(m_agentData.m_oodas);
		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->update_ooda_data(m_currentagentKey, contextJsonobj);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qDebug() << "save poi " << contextJsonobj;
		updateContextjson(m_agentData.m_pois);

		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->update_action_data(m_currentagentKey,contextJsonobj);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qDebug() << "save fence " << contextJsonobj;
		updateContextjson(m_agentData.m_fences);

		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->update_action_data(m_currentagentKey,contextJsonobj);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	{
		qDebug() << "save param " << contextJsonobj;
        if (contextJsonobj.contains("varSchema"))
		{
            QString vardefschemastr = contextJsonobj.value("varSchema").toString();
			QJsonObject vardefschemaobj = FunctionAssistant::string_to_json_object(vardefschemastr);
            contextJsonobj.insert("varSchema", vardefschemaobj);
		}
		updateContextjson(m_agentData.m_params);
		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->update_variable_data(m_currentagentKey, contextJsonobj);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
		break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		qDebug() << "save field media" << contextJsonobj;
		updateContextjson(m_agentData.m_field);
		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->update_field_data(m_currentagentKey, contextJsonobj);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		updateContextjson(m_agentData.m_sensing);
		if (DataManager::getInstance().pHttpClient())
        {
            contextJsonobj.remove("roll");
            contextJsonobj.remove("smdkey");
            qDebug() << "save seinsing media " << contextJsonobj;
            DataManager::getInstance().pHttpClient()->update_sensing_data(m_currentagentKey, contextJsonobj);
		}
	}
	break;
	default:
		break;
	}
}

void QtModelWidget::addPoiGeoJson(const QVariant &dataType_Id, const QVariant &edit_id, const QVariant &modelpath)
{
	TreeNode::E_NODE_TYPE type = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
	uint64_t edit_instance_id = edit_id.toULongLong();
	QString jsfilename = modelpath.toString();

	GeoJsonInfos geoinfos = m_pMapEditWidget->addJsonFile(jsfilename);

	for (auto subcoordinates : geoinfos.subItem)
	{
		switch (geoinfos.type)
		{
		case E_GEOTYPE_LINE:
		{
			bool bShowArea = false;

			bool bExtendArea = false;
			QString poiKeyword;
			QStringList namelist;
			if (subcoordinates.m_tags.contains("ref"))
			{
				QString name = subcoordinates.m_tags.value("ref");
				namelist = name.split("/");

				if (namelist.size() == 0)
				{
					poiKeyword = name;
				}
				if (namelist.size() == 1)
				{
					poiKeyword = namelist.at(0);
				}
				if (namelist.size() == 2)
				{
					poiKeyword = namelist.at(0);
				}
				//updatePoiNodeContext(type, edit_instance_id, poiKeyword);

				auto itor = std::find_if(m_agentData.m_pois.begin(),
					m_agentData.m_pois.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
					return vt.first == edit_instance_id;
				});
				if (itor != m_agentData.m_pois.end())
				{
					QJsonObject& datacontext = itor->second;
					datacontext["poiKeyword"] = poiKeyword;

					datacontext["poiName"] = poiKeyword;
					datacontext["poiNameI18n"] = poiKeyword;

					m_treeViewController->updateNodeContext(type, edit_instance_id, poiKeyword);
				}
			}
			
			if (subcoordinates.m_tags.contains("aeroway"))
			{
				if (subcoordinates.m_tags.value("aeroway") == "runway")
				{
					bShowArea = true;
					bExtendArea = true;
				}
				else if (subcoordinates.m_tags.value("aeroway") == "taxiway")
				{
				}
				else if (subcoordinates.m_tags.value("aeroway") == "parking_position")
				{
				}
				else if (subcoordinates.m_tags.value("aeroway") == "stopbar" ||
					subcoordinates.m_tags.value("aeroway") == "holdingpoint" ||
					subcoordinates.m_tags.value("aeroway") == "landingpoint" ||					
					subcoordinates.m_tags.value("aeroway") == "transrunway" ||
					subcoordinates.m_tags.value("aeroway") == "vacaterunway" ||
					subcoordinates.m_tags.value("aeroway") == "tr_stopbar"||
					subcoordinates.m_tags.value("aeroway") == "tr_checkbar"||
					subcoordinates.m_tags.value("aeroway") == "tr_checkbar_stopbar")
				{
					bShowArea = true;
					bExtendArea = false;
				}				
			}
			if (bShowArea)
			{
				auto getCenterPt = [](std::vector<std::tuple<std::vector<LAT_LNG>, QColor>>& coordinatesExtend) ->LAT_LNG {
					LAT_LNG centerpt;
					for (auto coordinatesExtenditem : coordinatesExtend)
					{
						double lat_total = 0.0;
						double lng_total = 0.0;
						std::vector<LAT_LNG>& subcoordinates = std::get<0>(coordinatesExtenditem);
						if (!subcoordinates.empty())
						{
							for (auto subcoordinatesitem : subcoordinates)
							{
								lat_total += subcoordinatesitem.lat;
								lng_total += subcoordinatesitem.lng;
							}
							centerpt.lat = lat_total / subcoordinates.size();
							centerpt.lng = lng_total / subcoordinates.size();

						}
					}
					return centerpt;
				};

				double width = 4.0f;
				width = subcoordinates.m_tags.value("width", "4.0").toDouble();
				width = width / 2;



				auto generateExtendRunWay = [&](const QColor& extendcl, const QString& title, const LAT_LNG &start, const LAT_LNG &end, int len)->uint64_t {
					uint64_t id;
					glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(end, start);
					LAT_LNG extendpt = FunctionAssistant::calculateDirectionExtendPoint(start, directionVectorArr, len);
					std::vector<LAT_LNG> latlnglist;
					latlnglist.push_back(start);
					latlnglist.push_back(extendpt);


					auto generateExtendPoiArea = [](double width, QColor cl, const std::vector<LAT_LNG>& latlnglist)-> std::vector<std::tuple<std::vector<LAT_LNG>, QColor>> {

						std::vector<std::tuple<std::vector<LAT_LNG>, QColor>> coordinatesExtend;
						std::vector<LAT_LNG> extentmp = FunctionAssistant::extendLineToPolygonPlane(latlnglist, width);
						coordinatesExtend.emplace_back(std::make_tuple(std::move(extentmp), cl));
						return coordinatesExtend;
					};


					std::vector<std::tuple<std::vector<LAT_LNG>, QColor>> coordinatesExtend = generateExtendPoiArea(width, extendcl, latlnglist);
					LAT_LNG centerpt = getCenterPt(coordinatesExtend);

					TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
					if (pTreeModel)
					{
						auto appendextendpoi = [&](const QString & context, const LAT_LNG& centerpt)->uint64_t {
							uint64_t id;
							QModelIndex agentmodelindex;
							uint64_t fieldid = 0;
							bool bExist = pTreeModel->getSubModelIndex(TreeNode::E_NODE_TYPE_POIS, agentmodelindex, fieldid);
							if (bExist)
							{

								QModelIndex addmodelindex = pTreeModel->appendSubChild(agentmodelindex, id);

								pTreeModel->setNodeName(context, addmodelindex);
								{

									auto itor = std::find_if(m_agentData.m_pois.begin(),
										m_agentData.m_pois.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
										return vt.first == id;
									});
									if (itor != m_agentData.m_pois.end())
									{
										QJsonObject& datacontext = itor->second;
										datacontext["poiKeyword"] = context;
										QJsonArray wpslistitem;
										wpslistitem.push_back(QString::number(id));
										wpslistitem.push_back(centerpt.lng);
										wpslistitem.push_back(centerpt.lat);
										wpslistitem.push_back(0.0);
										datacontext.insert("poiPoint", wpslistitem);
										datacontext["poiPoint"] = wpslistitem;
										datacontext["poiName"] = context;
										datacontext["poiNameI18n"] = context;
										//m_treeViewController->updateNodeContext(type, edit_instance_id, poiKeyword);

									}
									else
									{
										QString key = "poi_" + QString::number(id);
										QJsonObject datacontext;
										datacontext.insert("poiKey", key);
										datacontext.insert("poiKeyword", context);
										datacontext.insert("poiFrame", 0);

										QJsonArray wpslistitem;
										wpslistitem.push_back(QString::number(id));
										wpslistitem.push_back(centerpt.lng);
										wpslistitem.push_back(centerpt.lat);
										wpslistitem.push_back(0.0);
										datacontext.insert("poiPoint", wpslistitem);
										datacontext.insert("poiName", context);
										datacontext.insert("poiNameI18n", context);
										m_agentData.m_pois.insert(std::make_pair(id, std::move(datacontext)));
									}

									geometry_points_info ptinfo(id, FunctionAssistant::randColor(255));
									waypointinfo val{ id, centerpt.lng, centerpt.lat, 0.0, 1, 0 };
									ptinfo.m_pts.push_back(val);
									m_agentData.m_poi_geometrypointmap.insert(std::make_pair(id, std::move(ptinfo)));

									//showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(8)), id, m_agentData.m_pois.at(id));
								}

								appendWaypoint(TreeNode::E_NODE_TYPE_POIS, id, id, centerpt.lng, centerpt.lat);
							}
							return id;
						};
						id = appendextendpoi(title, centerpt);

						GeoJsonInfos geoinfostmp;
						geoinfostmp.type = E_GEOTYPE_LINE;
						tagGeoJsonInfo geojsoninfo;


						geojsoninfo.name = title;
						geojsoninfo.m_color = extendcl;
						geojsoninfo.type = E_GEOTYPE_LINE;
						//geojsoninfo.properties;
						std::vector<LAT_LNG> ptline;
						ptline.push_back(start);
						ptline.push_back(extendpt);
						geojsoninfo.coordinates.push_back(std::move(ptline));

						////////////////////////////////////////////////////////
						// lines info
						geojsoninfo.m_tags.insert("aeroway", "runway_extend");
						//geojsoninfo.z_order;
						geojsoninfo.coordinatesExtend = coordinatesExtend;
						geoinfostmp.subItem.insert(title, std::move(geojsoninfo));
						m_pMapEditWidget->drawGeoData(geoinfostmp, true, true,true);

						/////////////////////////////////////////////////////////////
						auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
							m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
							return vt.first == id && vt.second.m_geometry_id == id;
						});
						if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
						{
							_geometrypointmap_itor->second.coordinatesExtend = coordinatesExtend;
						}
					}
					return id;
				};

				if (bExtendArea)
				{
					for (auto subcoordinatessub : subcoordinates.coordinates)
					{
						LAT_LNG ptstart = subcoordinatessub.front();
						LAT_LNG ptend = subcoordinatessub.back();

						generateExtendRunWay(QColor(0, 255, 255, 128), "TKF"+poiKeyword, ptstart, ptend, DISTANCE_DEP_AREA_LEN);
						generateExtendRunWay(QColor(255, 0, 255, 128), "LND"+poiKeyword, ptend, ptstart, DISTANCE_ARR_AREA_LEN);
					}
				}

				LAT_LNG centerpt = getCenterPt(subcoordinates.coordinatesExtend);
				appendWaypoint(TreeNode::E_NODE_TYPE_POIS,edit_instance_id, edit_instance_id, centerpt.lng, centerpt.lat);

				auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
					m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
					return vt.first == edit_instance_id && vt.second.m_geometry_id == edit_instance_id;
				});
				if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
				{
					_geometrypointmap_itor->second.coordinatesExtend = subcoordinates.coordinatesExtend;
				}

				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				
				for (auto poiKeywordTmp : namelist)
				{
					if (poiKeywordTmp == poiKeyword)
					{
						continue;
					}

					uint64_t id;

					TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
					if (pTreeModel)
					{
						auto appendextendpoi = [&](const QString & context, const LAT_LNG& centerpt)->uint64_t {
							uint64_t id;
							QModelIndex agentmodelindex;
							uint64_t fieldid = 0;
							bool bExist = pTreeModel->getSubModelIndex(TreeNode::E_NODE_TYPE_POIS, agentmodelindex, fieldid);
							if (bExist)
							{

								QModelIndex addmodelindex = pTreeModel->appendSubChild(agentmodelindex, id);

								pTreeModel->setNodeName(context, addmodelindex);
								{

									auto itor = std::find_if(m_agentData.m_pois.begin(),
										m_agentData.m_pois.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
										return vt.first == id;
									});
									if (itor != m_agentData.m_pois.end())
									{
										QJsonObject& datacontext = itor->second;
										datacontext["poiKeyword"] = context;
										QJsonArray wpslistitem;
										wpslistitem.push_back(QString::number(id));
										wpslistitem.push_back(centerpt.lng);
										wpslistitem.push_back(centerpt.lat);
										wpslistitem.push_back(0.0);
										datacontext.insert("poiPoint", wpslistitem);
										datacontext["poiPoint"] = wpslistitem;
										datacontext["poiName"] = context;
										datacontext["poiNameI18n"] = context;
										//m_treeViewController->updateNodeContext(type, edit_instance_id, poiKeyword);

									}
									else
									{
										QString key = "poi_" + QString::number(id);
										QJsonObject datacontext;
										datacontext.insert("poiKey", key);
										datacontext.insert("poiKeyword", context);
										datacontext.insert("poiFrame", 0);

										QJsonArray wpslistitem;
										wpslistitem.push_back(QString::number(id));
										wpslistitem.push_back(centerpt.lng);
										wpslistitem.push_back(centerpt.lat);
										wpslistitem.push_back(0.0);
										datacontext.insert("poiPoint", wpslistitem);
										datacontext.insert("poiName", context);
										datacontext.insert("poiNameI18n", context);
										m_agentData.m_pois.insert(std::make_pair(id, std::move(datacontext)));
									}

									geometry_points_info ptinfo(id, FunctionAssistant::randColor(255));
									waypointinfo val{ id, centerpt.lng, centerpt.lat, 0.0, 1, 0 };
									ptinfo.m_pts.push_back(val);
									m_agentData.m_poi_geometrypointmap.insert(std::make_pair(id, std::move(ptinfo)));

									//showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(8)), id, m_agentData.m_pois.at(id));
								}

								appendWaypoint(TreeNode::E_NODE_TYPE_POIS, id, id, centerpt.lng, centerpt.lat);
							}
							return id;
						};
						id = appendextendpoi(poiKeywordTmp, centerpt);

						GeoJsonInfos geoinfostmp;
						geoinfostmp.type = E_GEOTYPE_LINE;
						tagGeoJsonInfo geojsoninfo;


						geojsoninfo.name = poiKeywordTmp;
						geojsoninfo.m_color = QColor(255, 255, 0, 255);;
						geojsoninfo.type = E_GEOTYPE_LINE;
						geojsoninfo.coordinates = subcoordinates.coordinates;

						////////////////////////////////////////////////////////
						// lines info
						geojsoninfo.m_tags.insert("aeroway", "runway");
						//geojsoninfo.z_order;
						geojsoninfo.coordinatesExtend = subcoordinates.coordinatesExtend;
						geoinfostmp.subItem.insert(poiKeywordTmp, std::move(geojsoninfo));
						m_pMapEditWidget->drawGeoData(geoinfostmp, true, true, true);

						/////////////////////////////////////////////////////////////
						auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
							m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
							return vt.first == id && vt.second.m_geometry_id == id;
						});
						if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
						{
							_geometrypointmap_itor->second.coordinatesExtend = subcoordinates.coordinatesExtend;
						}
					}


					for (auto subcoordinatessub : subcoordinates.coordinates)
					{
						LAT_LNG ptend = subcoordinatessub.front();
						LAT_LNG ptstart = subcoordinatessub.back();

						generateExtendRunWay(QColor(0, 255, 255, 128), "TKF" + poiKeywordTmp, ptstart, ptend, DISTANCE_DEP_AREA_LEN);
						generateExtendRunWay(QColor(255, 0, 255, 128), "LND" + poiKeywordTmp, ptend, ptstart, DISTANCE_ARR_AREA_LEN);
					}


					appendWaypoint(TreeNode::E_NODE_TYPE_POIS, id, id, centerpt.lng, centerpt.lat);

					auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
						m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
						return vt.first == edit_instance_id && vt.second.m_geometry_id == edit_instance_id;
					});
					if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
					{
						_geometrypointmap_itor->second.coordinatesExtend = subcoordinates.coordinatesExtend;
					}
				}
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(8)), edit_instance_id, m_agentData.m_pois.at(edit_instance_id));
			}
		}break;
		default:break;
        }
    }
}

void QtModelWidget::selectPoiPoint(const QVariant &dataType_Id, const QVariant &edit_id, const QVariant &waypoint_id)
{
    TreeNode::E_NODE_TYPE _currenttype = (TreeNode::E_NODE_TYPE)(dataType_Id.toInt());
    uint64_t editid = edit_id.toULongLong();
    uint64_t waypointid = waypoint_id.toULongLong();
    if (editid == 0)
    {
        return;
    }

    switch (_currenttype)
    {
    case TreeNode::E_NODE_TYPE_NULL:
    case TreeNode::E_NODE_TYPE_WAYPOINTS:
        break;
    case TreeNode::E_NODE_TYPE_POIS:
    case TreeNode::E_NODE_TYPE_FENCES:
    {
        if(m_pMapEditWidget)
        {

            m_pMapEditWidget->locate_entity_waypoint(editid, waypointid);

            m_pMapEditWidget->locate_entity_tracking(editid);
        }
    }
    break;
    default:
        break;
    }
}
#if 0

Q_INVOKABLE void QtModelWidget::fenceAppendPoint(const QVariant& pointid, const QVariant& lngval, const QVariant& latval, const QVariant& data)
{
	// 获取 QML 中的 ListModel 对象
	uint64_t waypointid = pointid.toString().toULongLong();
	std::list<waypointinfo> waypointlist;
	// 从 ListModel 中获取数据并打印
	QVariantList dataList = data.toList();
	for (const QVariant& data : dataList)
	{
		QVariantMap item = data.toMap();
		QString pointid = item.value("pointid").toString();
		uint64_t poipointid = pointid.toULongLong();
		double lngold = item.value("lng").toDouble();
		double latold = item.value("lat").toDouble();
		double altold = item.value("alt").toDouble();
		waypointinfo val{ poipointid, lngold, latold, altold, 1, (int)(waypointlist.size()) * 10 };
		waypointlist.push_back(val);
	}

	QColor color = QColor(255, 0, 0, 255);
	LAT_LNG curpos{ latval.toDouble(),lngval.toDouble() };
	m_pMapEditWidget->add_entity_waypoint(m_currentEditId, waypointid, curpos, waypointlist, color);
	m_pMapEditWidget->locate_entity_tracking(m_currentEditId);

}

Q_INVOKABLE void QtModelWidget::fenceUpdatePoint(const QVariant& pointid, const QVariant& lngval, const QVariant& latval, const QVariant& data)
{
	QString pointidstr = pointid.toString();
	if (!pointidstr.isEmpty())
	{
		uint64_t waypointid = pointidstr.toULongLong();

		std::list<waypointinfo> waypointlist;
		// 从 ListModel 中获取数据并打印
		QVariantList dataList = data.toList();
		for (const QVariant& data : dataList)
		{
			QVariantMap item = data.toMap();
			QString pointid = item.value("pointid").toString();
			uint64_t poipointid = pointid.toULongLong();
			double lngold = item.value("lng").toDouble();
			double latold = item.value("lat").toDouble();
			double altold = item.value("alt").toDouble();
			waypointinfo val{ poipointid, lngold, latold, altold, 1, (int)(waypointlist.size()) * 10 };
			waypointlist.push_back(val);
		}
		QColor color = QColor(255, 0, 0, 255);
		LAT_LNG curpos{ latval.toDouble(),lngval.toDouble() };
		m_pMapEditWidget->add_entity_waypoint(m_currentEditId, waypointid, curpos, waypointlist, color);
		m_pMapEditWidget->locate_entity_tracking(m_currentEditId);;
		m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, waypointid);
	}
}

Q_INVOKABLE void QtModelWidget::fenceSelectPoint(const QVariant& pointid, const QVariant& data)
{
	bool bExist = false;
	QString pointidstr = pointid.toString();
	if (!pointidstr.isEmpty())
	{
		uint64_t waypointid = pointidstr.toULongLong();
		// 从 ListModel 中获取数据并打印
		QVariantList dataList = data.toList();
		for (const QVariant& data : dataList)
		{
			QVariantMap item = data.toMap();
			QString pointid = item.value("pointid").toString();
			uint64_t poipointid = pointid.toULongLong();
			if (poipointid == waypointid)
			{
				m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
				m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, poipointid);
				bExist = true;
				break;
			}
		}
	}
	if (!bExist)
	{
		m_pMapEditWidget->locate_entity_tracking(0);
		m_pMapEditWidget->locate_entity_waypoint(0, 0);
	}
}
#endif
void QtModelWidget::setModelListWidget(QVector<QWidget*>& listwidgets)
{
	m_listwidgets = listwidgets;

	auto setWidgetDataTypeid = [&](int index, TreeNode::E_NODE_TYPE mode_type)
	{
		QQuickWidget* qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(index));
		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();

			QMetaObject::invokeMethod(pRoot, "setDataTypeId",
				Q_ARG(QVariant, QVariant::fromValue((int32_t)mode_type)));

		}
	};
	setWidgetDataTypeid(0, TreeNode::E_NODE_TYPE_ACTIONS);
	setWidgetDataTypeid(1, TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS);
	setWidgetDataTypeid(2, TreeNode::E_NODE_TYPE_OODAS);
	setWidgetDataTypeid(3, TreeNode::E_NODE_TYPE_VAEDEFS);
	setWidgetDataTypeid(4, TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE);
	setWidgetDataTypeid(5, TreeNode::E_NODE_TYPE_WAYPOINTS);
	setWidgetDataTypeid(6, TreeNode::E_NODE_TYPE_SUBMODELS);

	setWidgetDataTypeid(7, TreeNode::E_NODE_TYPE_CONFIGS);
	setWidgetDataTypeid(8, TreeNode::E_NODE_TYPE_POIS);
	setWidgetDataTypeid(9, TreeNode::E_NODE_TYPE_FENCES);

}

void QtModelWidget::setAgentType(E_AGENT_TYPE agentType)
{
	m_eAgentType = agentType;

	m_pQStackedWidget->setCurrentIndex(m_eAgentType);
}

void QtModelWidget::loadNewAgent(const QJsonObject& contextJsonobj)
{
	m_bNewAgent = true;
	m_currentAgentJson = contextJsonobj;

	m_agentData.m_general = contextJsonobj;

	clearData();

	//initTreemodel();

	m_agentData.m_general.insert("modelUrlSlim", "");
	m_agentData.m_general.insert("modelUrlFat", "");
	m_agentData.m_general.insert("modelUrlMedium", "");
	m_agentData.m_general.insert("modelAbstracted", "");
	m_agentData.m_general.insert("freelanceable", false);
	m_agentData.m_general.insert("freelanceableDynamics", "");
	m_agentData.m_general.insert("locatable", false);
	m_agentData.m_general.insert("locatableDynamics", "");
	m_agentData.m_general.insert("navigatable", false);
	m_agentData.m_general.insert("navigatableDynamics", "");
	m_agentData.m_general.insert("missionable", false);
	m_agentData.m_general.insert("missionableDynamics", "");
	m_agentData.m_general.insert("operatable", false);
	m_agentData.m_general.insert("useIb", false);
	m_agentData.m_general.insert("stopCond", QJsonValue());
	m_agentData.m_general.insert("axnViewDetails", "");
	m_agentData.m_general.insert("isDelete", false);

	loadAgent();

	saveAgent();
}

void QtModelWidget::loadOldAgent(const QJsonObject& contextJsonobj)
{
	m_bNewAgent = false;
	m_currentAgentJson = contextJsonobj;

	auto setData = [&](const QString& key)
	{
		if (m_currentAgentJson.contains(key))
		{
			m_agentData.m_general.insert(key, m_currentAgentJson.value(key));
		}
		else
		{
			m_agentData.m_general.insert(key, "");
		}
	};

	setData("agentKey");
	setData("agentKeyword");
	setData("agentName");
	setData("agentNameI18n");
	setData("agentType");
	setData("agentPath");
	setData("modelUrlSlim");
	setData("modelUrlFat");
	setData("modelUrlMedium");
	setData("modelAbstracted");
	setData("freelanceable");
	setData("freelanceableDynamics");
	setData("locatable");
	setData("locatableDynamics");
	setData("navigatable");
	setData("navigatableDynamics");
	setData("missionable");
	setData("missionableDynamics");
	setData("operatable");
	setData("useIb");
	setData("stopCond");
	setData("axnViewDetails");
	setData("isDelete");
	setData("agentDesc");

	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());

	clearData();

	//initTreemodel();
	auto decodePorperty = [&](TreeNode::E_NODE_TYPE type, const QString& prepertyname, const QString& keystr, const QString& namestr, std::unordered_map<uint64_t, QJsonObject>& databuf)
	{
		if (m_currentAgentJson.contains(prepertyname) && m_currentAgentJson.value(prepertyname).isArray())
		{
			auto arr = m_currentAgentJson.value(prepertyname).toArray();
			for (auto item : arr)
			{
				auto itemobj = item.toObject();
				QString key = itemobj.value(keystr).toString();
				QString nodenamestr = itemobj.value(namestr).toString();
				QStringList keylist = key.split("_");
				uint64_t id = 0;
				if (keylist.size() == 2)
				{
					id = keylist.at(1).toULongLong();
					databuf.insert(std::make_pair(id, itemobj));
				}
				else if (keylist.size() == 1)
				{
					id = keylist.at(0).toULongLong();
					if (id == 0)
					{
						id = FunctionAssistant::generate_random_positive_uint64();
					}
					databuf.insert(std::make_pair(id, itemobj));
				}
				if (id == 0)
				{
					id = FunctionAssistant::generate_random_positive_uint64();
					databuf.insert(std::make_pair(id, itemobj));
				}

				if (pTreeModel && id != 0)
				{
					if (TreeNode::E_NODE_TYPE_POIS == type)
					{
						geometry_points_info ptinfo(id, FunctionAssistant::randColor(255));
						if (itemobj.contains("poiPoint") && itemobj.value("poiPoint").isArray())
						{
							auto poiPointArray = itemobj.value("poiPoint").toArray();
							if (poiPointArray.size() == 3)
							{
								double lng = poiPointArray.at(0).toDouble();
								double lat = poiPointArray.at(1).toDouble();
								double alt = poiPointArray.at(2).toDouble();
								waypointinfo val{ id, lng, lat, alt,1,0 };
								ptinfo.m_pts.push_back(val);
							}
						}
						ptinfo.cl = QColor(255, 255, 0, 255);
						ptinfo.m_name = itemobj.value("poiName").toString();

						auto geojson = itemobj.value("poiGeoJSON").toObject();

						auto decodegeojson = [](const QJsonObject &jsonobj, geometry_points_info& ptinfo, E_GEOTYPE &type) {
							auto featuresarray = jsonobj.value("features").toArray();
							for (auto featuresarrayitem : featuresarray)
							{								
								auto properties = featuresarrayitem.toObject().value("properties").toObject();

								QString osm_id = QString::number(FunctionAssistant::generate_random_positive_uint64());

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
											std::vector<LAT_LNG> sublatlnglist;

											auto subsubarray = subarrayitem.toArray();
											for (auto subsubarrayitem : subsubarray)
											{
												PARSE_LATLNG_FROME_JSON(sublatlnglist, subsubarrayitem);
											}
											ptinfo.coordinatesExtend.emplace_back(std::make_tuple((sublatlnglist), ptinfo.cl));
										}
									}
									type = E_GEOTYPE_MULITPOLYGON;
								}
								else if (coordinatestype == "polygon")
								{
									auto coordinatesarray = geometry.value("coordinates").toArray();
									for (auto coordinatesitem : coordinatesarray)
									{
										std::vector<LAT_LNG> sublatlnglist;
										auto subarray = coordinatesitem.toArray();
										for (auto subarrayitem : subarray)
										{
											PARSE_LATLNG_FROME_JSON(sublatlnglist, subarrayitem);
										}
										ptinfo.coordinatesExtend.emplace_back(std::make_tuple((sublatlnglist), ptinfo.cl));
									}
									type = E_GEOTYPE_POLYGON;
								}
								else if (coordinatestype == "linestring")
								{
									auto coordinatesarray = geometry.value("coordinates").toArray();
									std::vector<LAT_LNG> sublatlnglist;
									for (auto coordinatesitem : coordinatesarray)
									{
										PARSE_LATLNG_FROME_JSON(sublatlnglist, coordinatesitem);
									}
									ptinfo.coordinatesExtend.emplace_back(std::make_tuple((sublatlnglist), ptinfo.cl));

									type = E_GEOTYPE_LINE;
								}
								else if (coordinatestype == "point")
								{
									std::vector<LAT_LNG> sublatlnglist;
									PARSE_LATLNG_FROME_JSON(sublatlnglist, geometry.value("coordinates"));
									ptinfo.coordinatesExtend.emplace_back(std::make_tuple((sublatlnglist), ptinfo.cl));

									type = E_GEOTYPE_POINT;
								}
							}
						};
						E_GEOTYPE type;

						decodegeojson(geojson, ptinfo, type);

						m_agentData.m_poi_geometrypointmap.insert(std::make_pair(id, std::move(ptinfo)));
					}
					else if (TreeNode::E_NODE_TYPE_FENCES == type)
					{
						geometry_points_info ptinfo(id, FunctionAssistant::randColor(255));
						if (itemobj.contains("fencePoints") && itemobj.value("fencePoints").isArray())
						{
							auto fencePointsArray = itemobj.value("fencePoints").toArray();
							for (auto fencePoint : fencePointsArray)
							{
								auto fencePointarray = fencePoint.toArray();
								if (fencePointarray.size() == 3)
								{
									auto fencepointid = FunctionAssistant::generate_random_positive_uint64();
									waypointinfo val{ fencepointid, fencePointarray.at(0).toDouble(), fencePointarray.at(1).toDouble(), fencePointarray.at(2).toDouble(),1,0 };
									ptinfo.m_pts.push_back(val);
								}
							}
						}
						m_agentData.m_fence_geometrypointmap.insert(std::make_pair(id, std::move(ptinfo)));
					}
					pTreeModel->appendNodeChild(type, nodenamestr, id, 0);
				}
			}
		}
	};

	decodePorperty(TreeNode::E_NODE_TYPE_POIS, "pois", "poiKey", "poiKeyword", m_agentData.m_pois);
	decodePorperty(TreeNode::E_NODE_TYPE_FENCES, "fences", "fenceKey", "fenceName", m_agentData.m_fences);
	decodePorperty(TreeNode::E_NODE_TYPE_VAEDEFS, "vardefs", "varSig", "varName", m_agentData.m_params);
	decodePorperty(TreeNode::E_NODE_TYPE_WAYPOINTS, "waypoints", "wpsKeyword", "wpsKeyword", m_agentData.m_waypoints);
	decodePorperty(TreeNode::E_NODE_TYPE_ACTIONS, "axns", "scriptId", "axnName", m_agentData.m_actions);
	decodePorperty(TreeNode::E_NODE_TYPE_OODAS, "oodas", "scriptId", "scriptId", m_agentData.m_oodas);


	m_currentAgentJson.remove("modelAbstracted");
	if (m_currentAgentJson.contains("modelAbstracted") && m_currentAgentJson.value("modelAbstracted").isArray())
	{
		auto arr = m_currentAgentJson.value("modelAbstracted").toArray();
		for (auto item : arr)
		{
			auto itemobj = item.toObject();
			QString key = itemobj.value("abstProfileKeyword").toString();
			m_agentData.m_modelAbstracted.insert(std::make_pair(key.toULongLong(), itemobj));
		}
	}

	for (auto poi_itor = m_agentData.m_pois.begin(); poi_itor != m_agentData.m_pois.end(); poi_itor++)
	{
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
				QJsonObject poiobj = poi_itor->second;
				QString poiName = poiobj.value("poiName").toString();
				QString poiKeyword = poiobj.value("poiKeyword").toString();
				int poiFrame = poiobj.value("poiFrame").toInt();
				QJsonArray poipoint = poiobj.value("poiPoint").toArray();
				if (poiFrame == 1 && poipoint.size() == 3)
				{
                    m_pModelWidget->pOSGManager()->addPoi(poi_itor->first, poiName, poipoint.at(0).toDouble(), poipoint.at(1).toDouble(), poipoint.at(2).toDouble());
				}
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
	}
	


	//m_currentAgentJson.remove("fldmds");
	if (m_currentAgentJson.contains("fldmds") && m_currentAgentJson.value("fldmds").isArray())
	{
		auto arr = m_currentAgentJson.value("fldmds").toArray();
		for (auto item : arr)
		{
			auto itemobj = item.toObject();
			QString key = itemobj.value("fldmdkey").toString();
			QString nodenamestr = itemobj.value("fldmdName").toString();
			QStringList keylist = key.split("_");
			uint64_t fieldid = 0;
			if (keylist.size() == 2)
			{
				fieldid = keylist.at(1).toULongLong();
				m_agentData.m_field.insert(std::make_pair(fieldid, itemobj));
			}
			else if (keylist.size() == 1)
			{
				fieldid = keylist.at(0).toULongLong();
                itemobj.insert("fldmdkey","field_"+keylist.at(0));
				m_agentData.m_field.insert(std::make_pair(fieldid, itemobj));
			}
			if (fieldid == 0)
			{
				fieldid = FunctionAssistant::generate_random_positive_uint64();

                itemobj.insert("fldmdkey","field_"+QString::number(fieldid));
				m_agentData.m_field.insert(std::make_pair(fieldid, itemobj));
			}

			if (pTreeModel && fieldid != 0)
			{
				pTreeModel->appendNodeChild(TreeNode::E_NODE_TYPE_FIELDMEDIAS, nodenamestr, fieldid, 0);

				switch (m_eAgentType)
				{
				case E_AGENT_TYPE_INSTAGENT:
				{
					if (m_pModelWidget)
					{
                        m_pModelWidget->pOSGManager()->addField(fieldid);
					}
				}
				break;
				case E_AGENT_TYPE_SCENE:
					break;
				default:
					break;
				}
			}
			auto fielditor = std::find_if(m_agentData.m_fieldsensing.begin(),
				m_agentData.m_fieldsensing.end(), [&](const std::unordered_map<uint64_t, std::list<uint64_t>>::value_type& vt) {
				return vt.first == fieldid;
			});
			if (fielditor == m_agentData.m_fieldsensing.end())
			{
				m_agentData.m_fieldsensing.insert(std::make_pair(fieldid, std::list<uint64_t>()));
			}

			auto smdarr = itemobj.value("smds").toArray();
			for (auto smditem : smdarr)
			{
				uint64_t smdid = 0;

				auto smditemobj = smditem.toObject();
				if (!smditemobj.contains("smdkey"))
				{
					QString key = "sensing_" + QString::number(FunctionAssistant::generate_random_positive_uint64());
					smditemobj.insert("smdkey", key);
				}

				QString smdkey = smditemobj.value("smdkey").toString();
				QString smdnodenamestr = smditemobj.value("smdkey").toString();
				QStringList smdkeylist = smdkey.split("_");
				if (smdkeylist.size() == 2)
				{
					smdid = smdkeylist.at(1).toULongLong();
					m_agentData.m_sensing.insert(std::make_pair(smdid, smditemobj));
				}
				else if (smdkeylist.size() == 1)
				{
					smdid = smdkeylist.at(0).toULongLong();
					m_agentData.m_sensing.insert(std::make_pair(smdid, smditemobj));
				}
				if (smdid == 0)
				{
					smdid = FunctionAssistant::generate_random_positive_uint64();
					m_agentData.m_sensing.insert(std::make_pair(smdid, smditemobj));
				}

				if (pTreeModel && smdid != 0)
				{
					pTreeModel->appendNodeChild(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS, smdnodenamestr, smdid, fieldid);

					float yawAngle = smditemobj.value("azimuth").toDouble();
					float yawFlareAngle = smditemobj.value("dazm").toDouble();
					float pitchAngle = smditemobj.value("pitch").toDouble();
					float pitchFlareAngle = smditemobj.value("dpch").toDouble();

					switch (m_eAgentType)
					{
					case E_AGENT_TYPE_INSTAGENT:
					{
						if (m_pModelWidget)
						{
                            m_pModelWidget->pOSGManager()->addSensing(fieldid, smdid, yawAngle, yawFlareAngle, pitchAngle, pitchFlareAngle);
						}
					}
					break;
					case E_AGENT_TYPE_SCENE:
						break;
					default:
						break;
					}
				}

				auto fielditor = std::find_if(m_agentData.m_fieldsensing.begin(),
					m_agentData.m_fieldsensing.end(), [&](const std::unordered_map<uint64_t, std::list<uint64_t>>::value_type& vt) {
					return vt.first == fieldid;
				});
				if (fielditor != m_agentData.m_fieldsensing.end())
				{
					std::list<uint64_t>& sensinglist = fielditor->second;
					sensinglist.push_back(smdid);
				}
			}
		}
	}


	loadAgent();

	auto poi_itor = m_agentData.m_poi_geometrypointmap.begin();
	while (poi_itor != m_agentData.m_poi_geometrypointmap.end())
	{
		auto pt_itor = poi_itor->second.m_pts.begin();
		while (pt_itor != poi_itor->second.m_pts.end())
		{
			appendWaypoint(TreeNode::E_NODE_TYPE_POIS, poi_itor->second.m_geometry_id, poi_itor->second.m_geometry_id, pt_itor->lng, pt_itor->lat);
			pt_itor++;
		}

		if (!poi_itor->second.coordinatesExtend.empty() && std::get<0>(poi_itor->second.coordinatesExtend.at(0)).size() > 1)
		{
			GeoJsonInfos geoinfostmp;
			geoinfostmp.type = E_GEOTYPE_LINE;
			tagGeoJsonInfo geojsoninfo;


			geojsoninfo.name = poi_itor->second.m_name;
			geojsoninfo.m_color = poi_itor->second.cl;
			geojsoninfo.type = E_GEOTYPE_LINE;
			////////////////////////////////////////////////////////
			// lines info
			geojsoninfo.m_tags.insert("aeroway", "runway_extend");
			//geojsoninfo.z_order;
			geojsoninfo.coordinatesExtend = poi_itor->second.coordinatesExtend;
			geoinfostmp.subItem.insert(poi_itor->second.m_name, std::move(geojsoninfo));
			m_pMapEditWidget->drawGeoData(geoinfostmp, true, true, true);
		}
		poi_itor++;
	}
}

void QtModelWidget::loadAgent()
{
	m_currentagentKey = m_agentData.m_general.value("agentKey").toString();
	m_agent_type = m_agentData.m_general.value("agentType").toString();

	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());

	if (m_agentData.m_general.contains("agentPath"))
	{
		QString modelfilepath = m_agentData.m_general.value("agentPath").toString();
		QFileInfo fileinfo(modelfilepath);
		if (fileinfo.exists())
		{
			QModelIndex index;
			switch (m_eAgentType)
			{
			case E_AGENT_TYPE_INSTAGENT:
			{
				if (m_pModelWidget)
				{
                    m_pModelWidget->pOSGManager()->addModelNode(modelfilepath, index, false);
				}
			}
			break;
			case E_AGENT_TYPE_SCENE:
				break;
			default:
				break;
			}
		}
	}

	QJsonObject datacontexttmp = m_agentData.m_general;

	auto stopCondobj = m_agentData.m_general.value("stopCond").toObject();
	datacontexttmp.insert("stopCond", FunctionAssistant::json_object_to_string(stopCondobj));

	showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(7)), 0, datacontexttmp);

	if (pTreeModel)
	{
		QModelIndex addmodelindex = pTreeModel->getRootModelInx();

		if (m_qmlpanelWidget)
		{
			QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "expand",
				Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));

			//QMetaObject::invokeMethod(pRoot, "selectIndex",
			//	Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));
		}
	}

	updateAgentDesc(m_agentData.m_general.value("agentType").toString() + " " + m_agentData.m_general.value("agentName").toString());

	edit_type_slot(TreeNode::E_NODE_TYPE_CONFIGS, 0, true, false);
}

void QtModelWidget::updateOpreateNodeInfo(uint64_t instanceid, uint64_t id, int level, int mode, osg::Vec3f trans, osg::Vec3f rotate, osg::Vec3f scale)
{
	QQuickWidget* qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(6));
	if (qmlWidget)
	{
		//auto datacontext = this->m_pOSGManager->getModelSenceData()->getModelInfos();

		QJsonObject modeljsonobj;
		modeljsonobj.insert("nodeid", QString::number(id));
		modeljsonobj.insert("instanceid", QString::number(instanceid));
		modeljsonobj.insert("level", level);
		modeljsonobj.insert("mode", mode);
		modeljsonobj.insert("translat_x", trans.x());
		modeljsonobj.insert("translat_y", trans.y());
		modeljsonobj.insert("translat_z", trans.z());

		modeljsonobj.insert("roll", rotate.x());
		modeljsonobj.insert("pitch", rotate.y());
		modeljsonobj.insert("yaw", rotate.z());

		modeljsonobj.insert("scale", scale.x());

		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		QMetaObject::invokeMethod(pRoot, "updateData",
			Q_ARG(QVariant, QVariant::fromValue(modeljsonobj)));

	}
}

void QtModelWidget::updateOpreateNodeSelect(uint64_t instanceid, uint64_t id)
{
	QQuickWidget* qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(6));
	if (qmlWidget)
	{
		QObject* pRoot = (QObject*)qmlWidget->rootObject();

		QMetaObject::invokeMethod(pRoot, "selsectIndex",
			Q_ARG(QVariant, QVariant::fromValue(QString::number(instanceid))),
			Q_ARG(QVariant, QVariant::fromValue(QString::number(id))));

	}
}

void QtModelWidget::updateFieldInfo(uint64_t iFieldId, osg::Vec3f trans)
{
	auto itor = std::find_if(m_agentData.m_field.begin(),
		m_agentData.m_field.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == iFieldId;
	});
	if (itor != m_agentData.m_field.end())
	{
		QJsonObject& datacontext = itor->second;
		datacontext["offsetx"] = trans.x();
		datacontext["offsety"] = trans.y();
		datacontext["offsetz"] = trans.z();
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(1)), iFieldId, datacontext);
	}
}

void QtModelWidget::updateSensingInfo(uint64_t iFieldId, uint64_t iSensingId, float yawAngle, float yawFlareAngle, float pitchAngle, float pitchFlareAngle)
{
	auto itor = std::find_if(m_agentData.m_sensing.begin(),
		m_agentData.m_sensing.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == iSensingId;
	});
	if (itor != m_agentData.m_sensing.end())
	{
		QJsonObject& datacontext = itor->second;

		datacontext["pitch"] = pitchAngle;
		datacontext["dpch"] = pitchFlareAngle;
		datacontext["azimuth"] = yawAngle;
		datacontext["dazm"] = yawFlareAngle;

		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(4)), iSensingId, datacontext);
	}
}

void QtModelWidget::updateFieldSelect(uint64_t iFieldId)
{
	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		QModelIndex agentmodelindex;
		if (pTreeModel->getTargetIndex(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS, iFieldId, agentmodelindex))
		{
			auto itor = std::find_if(m_agentData.m_field.begin(),
				m_agentData.m_field.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
				return vt.first == iFieldId;
			});
			if (itor != m_agentData.m_field.end())
			{
				QJsonObject& datacontext = itor->second;
				showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(1)), iFieldId, datacontext);
			}
			emit edit_type_sig(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS, iFieldId, true, false);

			QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "selectIndex",
				Q_ARG(QVariant, QVariant::fromValue(agentmodelindex)));
		}
	}
}

void QtModelWidget::updateSensingSelect(uint64_t iFieldId, uint64_t iSensingId)
{
	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		QModelIndex agentmodelindex;
		if (pTreeModel->getTargetIndex(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE, iSensingId, agentmodelindex))
		{
			auto itor = std::find_if(m_agentData.m_sensing.begin(),
				m_agentData.m_sensing.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
				return vt.first == iSensingId;
			});
			if (itor != m_agentData.m_sensing.end())
			{
				QJsonObject& datacontext = itor->second;
				showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(4)), iSensingId, datacontext);
			}
			emit edit_type_sig(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE, iSensingId, true, false);

			QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "selectIndex",
				Q_ARG(QVariant, QVariant::fromValue(agentmodelindex)));
		}
	}
}

void QtModelWidget::clearData()
{
	m_agentData.m_actions.clear();
	m_agentData.m_waypoints.clear();
	m_agentData.m_oodas.clear();
	m_agentData.m_params.clear();
	m_agentData.m_pois.clear();
	m_agentData.m_fences.clear();
	m_agentData.m_field.clear();
	m_agentData.m_sensing.clear();
	m_agentData.m_agentinstance.clear();
	m_agentData.m_fieldsensing.clear();
	m_agentData.m_modelAbstracted.clear();

	m_agentData.m_poi_geometrypointmap.clear();
	m_agentData.m_fence_geometrypointmap.clear();

	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->clearModel();
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
		auto collapseNode = [&](TreeNode::E_NODE_TYPE type) {
			QModelIndex agentmodelindex;
			if (pTreeModel->getSubModelIndex(type, agentmodelindex, 0))
			{
				TreeNode* clickNode = static_cast<TreeNode*>(agentmodelindex.internalPointer());
				if (clickNode)
				{
					QMetaObject::invokeMethod(pRoot, "collapse",
						Q_ARG(QVariant, QVariant::fromValue(agentmodelindex)));
					clickNode->deleteAllChild();
				}
			}
		};

		collapseNode(TreeNode::E_NODE_TYPE_CONFIGS);
		collapseNode(TreeNode::E_NODE_TYPE_SUBMODELS);
		collapseNode(TreeNode::E_NODE_TYPE_ACTIONS);
		collapseNode(TreeNode::E_NODE_TYPE_FIELDMEDIAS);
		collapseNode(TreeNode::E_NODE_TYPE_WAYPOINTS);
		collapseNode(TreeNode::E_NODE_TYPE_OODAS);
		collapseNode(TreeNode::E_NODE_TYPE_POIS);
		collapseNode(TreeNode::E_NODE_TYPE_VAEDEFS);
		collapseNode(TreeNode::E_NODE_TYPE_FENCES);

		QMetaObject::invokeMethod(pRoot, "clearmodel");
		initTreemodel();
		QMetaObject::invokeMethod(pRoot, "initmodel");

	}
//	pTreeModel->clearData();

	m_pMapEditWidget->clearGeoData();

	auto  _add_waypoints_itor = m_add_waypoints.begin();
	while (_add_waypoints_itor != m_add_waypoints.end())
	{
		if (m_pMapEditWidget)
		{
			quint64 waypointid = _add_waypoints_itor->first;
			quint64 entityid =_add_waypoints_itor->second;
			m_pMapEditWidget->remove_entity_waypoint(entityid, waypointid, std::list<waypointinfo>(), QColor());
			m_pMapEditWidget->clear_entity_waypoint_tracking(entityid);
		}
		_add_waypoints_itor++;
	}
	m_add_waypoints.clear();
}

void QtModelWidget::edit_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id, bool expand, bool bSubRootNode)
{
	m_currentEditId = 0;
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_CONFIGS:
	{
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS:
	{
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                auto datacontext = m_pModelWidget->pOSGManager()->getModelSenceData()->getModelInfos();
				showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(6)), id, datacontext);
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}


		QQuickWidget* qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(6));
		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();

			QMetaObject::invokeMethod(pRoot, "selsectIndex",
				Q_ARG(QVariant, QVariant::fromValue(QString::number(id))),
				Q_ARG(QVariant, QVariant::fromValue(QString::number(id))));
		}
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
				if (!expand)
				{
                    m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSelect(0, id, id, true);
				}
				else
				{
                    m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSelect(0, id, id, true);
				}
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
	{
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                auto datacontext = m_pModelWidget->pOSGManager()->getModelSenceData()->getModelNodeInfos(id);

                m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSelect(0, id, id, true);
				showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(6)), id, datacontext);
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_ACTIONS:
	{
		auto itor = std::find_if(m_agentData.m_actions.begin(),
			m_agentData.m_actions.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_actions.end())
		{
			QJsonObject& datacontext = itor->second;
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(0)), id, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	{
		//this->m_pOSGManager->adjustField();
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		auto itor = std::find_if(m_agentData.m_field.begin(),
			m_agentData.m_field.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_field.end())
		{
			QJsonObject& datacontext = itor->second;
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(1)), id, datacontext);
		}
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                m_pModelWidget->pOSGManager()->getModelSenceData()->setFieldSelect(id, true);
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}

	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		auto itor = std::find_if(m_agentData.m_sensing.begin(),
			m_agentData.m_sensing.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_sensing.end())
		{
			QJsonObject& datacontext = itor->second;
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(4)), id, datacontext);
		}
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                m_pModelWidget->pOSGManager()->getModelSenceData()->setSensingSelect(id, true);
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	{
		auto itor = std::find_if(m_agentData.m_waypoints.begin(),
			m_agentData.m_waypoints.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_waypoints.end())
		{
			QJsonObject& datacontext = itor->second;
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(5)), id, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_OODAS:
	{
		auto itor = std::find_if(m_agentData.m_oodas.begin(),
			m_agentData.m_oodas.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_oodas.end())
		{
			QJsonObject& datacontext = itor->second;
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(2)), id, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		auto itor = std::find_if(m_agentData.m_pois.begin(),
			m_agentData.m_pois.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_pois.end())
		{
			QJsonObject datacontextTmp = itor->second;

			auto _geometrypointmap_itor = std::find_if(m_agentData.m_poi_geometrypointmap.begin(),
				m_agentData.m_poi_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type &vt) {
				return vt.first == id && vt.second.m_geometry_id == id && vt.second.m_pts.size() == 1;
			});
			if (_geometrypointmap_itor != m_agentData.m_poi_geometrypointmap.end())
			{
				QJsonArray wpslistitem;
				auto ptitor = _geometrypointmap_itor->second.m_pts.begin();
				while (ptitor != _geometrypointmap_itor->second.m_pts.end())
				{
					wpslistitem.push_back(QString::number(ptitor->waypointid));
					wpslistitem.push_back(ptitor->lng);
					wpslistitem.push_back(ptitor->lat);
					wpslistitem.push_back(ptitor->alt);
					ptitor++;
				}
				datacontextTmp.insert("poiPoint", wpslistitem);
			}

			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(8)), id, datacontextTmp);
			m_currentEditId = id;
		}
	}
	break;

	case TreeNode::E_NODE_TYPE_FENCES:
	{
		auto itor = std::find_if(m_agentData.m_fences.begin(),
			m_agentData.m_fences.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_fences.end())
		{
			QJsonObject datacontextTmp = itor->second;

			auto _geometrypointmap_itor = std::find_if(m_agentData.m_fence_geometrypointmap.begin(),
				m_agentData.m_fence_geometrypointmap.end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
				return vt.first == id && vt.second.m_geometry_id == id && !vt.second.m_pts.empty();
			});
			if (_geometrypointmap_itor != m_agentData.m_fence_geometrypointmap.end())
			{
				QJsonArray wpslist;
				auto ptitor = _geometrypointmap_itor->second.m_pts.begin();
				while (ptitor != _geometrypointmap_itor->second.m_pts.end())
				{
					QJsonArray wpslistitem;
					wpslistitem.push_back(QString::number(ptitor->waypointid));
					wpslistitem.push_back(ptitor->lng);
					wpslistitem.push_back(ptitor->lat);
					wpslistitem.push_back(ptitor->alt);
					wpslist.push_back(wpslistitem);
					ptitor++;
				}
				datacontextTmp.insert("fencePoints", wpslist);
			}

			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(9)), id, datacontextTmp);
			m_currentEditId = id;
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	{
		auto itor = std::find_if(m_agentData.m_params.begin(),
			m_agentData.m_params.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
			return vt.first == id;
		});
		if (itor != m_agentData.m_params.end())
		{
			QJsonObject& datacontext = itor->second;
			QJsonObject datacontexttmp = datacontext;

            auto vardefschemaobj = datacontext.value("varSchema").toObject();
            datacontexttmp.insert("varSchema", FunctionAssistant::json_object_to_string(vardefschemaobj));
			showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(3)), id, datacontexttmp);
		}
	}
	break;
	default:break;
	}
	emit edit_type_sig(type, id, expand, bSubRootNode);
	m_currenttype = type;

	E_MODEL_MODE  _eModel_mode = E_MODEL_MODE_DISPLAY;

	if (type == TreeNode::E_NODE_TYPE_SUBMODELS ||
		type == TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE ||
		type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS ||
		type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE)
	{
		_eModel_mode = E_MODEL_MODE_EDIT;
	}
	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->updateModelMode(_eModel_mode);
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}
}

void QtModelWidget::add_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id)
{
	m_currentEditId = 0;
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_CONFIGS:
	{
	}
	break;
	case TreeNode::E_NODE_TYPE_ACTIONS:
	{
		QString key = "action_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("axnActivated", true);
		datacontext.insert("scriptId", key);
		datacontext.insert("axnKeyword", "123456");
		datacontext.insert("axnName", "123456");
		datacontext.insert("axnNameI18n", "123456");
		datacontext.insert("axnIcon", "123456");
		datacontext.insert("scriptLang", 0);
        datacontext.insert("axnVersion", 1);
        datacontext.insert("axnRunningPlan", 0);
		QJsonArray scriptstr;
		datacontext.insert("axnScript", scriptstr);

		m_agentData.m_actions.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(0)), id, m_agentData.m_actions.at(id));

		updateActionNodeContext(TreeNode::E_NODE_TYPE_ACTIONS, id, m_agentData.m_actions.at(id)["axnName"].toString());

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->append_action_data(m_currentagentKey, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	{
		QString key = "waypoint_" + QString::number(id);

		QJsonObject waypointsitem;

		waypointsitem.insert("wpsFrame", 0);
		waypointsitem.insert("wpsGenFences", QJsonArray());
		waypointsitem.insert("wpsGenTimeConsumed", 0);
        waypointsitem.insert("wpsGenPOIs", QJsonArray());
		waypointsitem.insert("wpsKey", "wps_123456789");
		waypointsitem.insert("wpsKeyword", key);
		waypointsitem.insert("wpsPathPlanner", "");
		waypointsitem.insert("wpsUsage", "");

		QJsonArray wps;

		QJsonObject wps_item;
		QJsonArray wpsCore;
		wpsCore.push_back(0.0);
		wpsCore.push_back(0.0);
		wpsCore.push_back(0.0);
		wpsCore.push_back(0.0);
		wpsCore.push_back(0.0);
		wps.push_back(wpsCore);
		wps_item.insert("wpsCore", wpsCore);
		wps_item.insert("useExt", 0);
		wps_item.insert("speed", 0.0);
		wps_item.insert("roll", 0.0);
		wps_item.insert("pitch", 0.0);
		wps_item.insert("yaw", 0.0);
		wps_item.insert("yawEx", 0.0);
		wps.push_back(wps_item);

		waypointsitem.insert("wps", wps);

		m_agentData.m_waypoints.insert(std::make_pair(id, std::move(waypointsitem)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(5)), id, m_agentData.m_waypoints.at(id));

		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->append_action_data(m_currentagentKey,datacontext);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_OODAS:
	{
		QString key = "ooda_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("oodaActivated", true);
		datacontext.insert("scriptId", key);
		datacontext.insert("oodaKeyword", "123456");
		datacontext.insert("oodaUsageFilter", 0);
		datacontext.insert("scriptLang", 0);
		datacontext.insert("oodaVersion", "1");
		QJsonArray scriptstr;
		datacontext.insert("oodaScript", scriptstr);

		m_agentData.m_oodas.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(2)), id, m_agentData.m_oodas.at(id));
		updateOODANodeContext(TreeNode::E_NODE_TYPE_OODAS, id, m_agentData.m_oodas.at(id)["oodaKeyword"].toString());

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->append_ooda_data(m_currentagentKey, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		QString key = "poi_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("poiKey", key);
		datacontext.insert("poiKeyword", "123456");
		datacontext.insert("poiFrame", 0);

		QJsonArray wpslistitem;
		wpslistitem.push_back(QString::number(id));
		wpslistitem.push_back(0.0);
		wpslistitem.push_back(0.0);
		wpslistitem.push_back(0.0);
		datacontext.insert("poiPoint", wpslistitem);
		datacontext.insert("poiName", "123456");
		datacontext.insert("poiNameI18n", "123456");

		geometry_points_info ptinfo(id, FunctionAssistant::randColor(255));
		//waypointinfo val{ id, 0.0, 0.0, 0.0, 1, 0 };
		//ptinfo.m_pts.push_back(val);
		m_agentData.m_poi_geometrypointmap.insert(std::make_pair(id, std::move(ptinfo)));

		m_agentData.m_pois.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(8)), id, m_agentData.m_pois.at(id));
		m_currentEditId = id;
		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->append_ooda_data(m_currentagentKey,datacontext);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		QString key = "fence_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("fenceKey", key);
		datacontext.insert("fenceKeyword", "123456");
		datacontext.insert("fenceFrame", 0);

		QJsonArray wpslist;
		QJsonArray wpslistitem;
		wpslist.push_back(wpslistitem);
		datacontext.insert("fencePoints", wpslist);
		datacontext.insert("fenceName", "123456");
		datacontext.insert("fenceNameI18n", "123456");

		m_agentData.m_fence_geometrypointmap.insert(std::make_pair(id, geometry_points_info(id, FunctionAssistant::randColor(255))));
		m_agentData.m_fences.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(9)), id, m_agentData.m_fences.at(id));
		m_currentEditId = id;
		//        if(DataManager::getInstance().pHttpClient())
		//        {
		//            DataManager::getInstance().pHttpClient()->append_ooda_data(m_currentagentKey,datacontext);
		//        }
	}
	break;
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	{
		QString key = "var_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("varKeyword", key);
		datacontext.insert("varName", "123456");
		datacontext.insert("varNameI18n", "123456");
		datacontext.insert("varType", "string");
		datacontext.insert("stdCode", 0);
		QJsonObject vardefschemaobj;
		QString vardefschemastr = FunctionAssistant::json_object_to_string(vardefschemaobj);
        datacontext.insert("varSchema", vardefschemastr);
		QJsonArray varDefault;
		varDefault.push_back("0");
		datacontext.insert("varDefault", varDefault);

		m_agentData.m_params.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(3)), id, m_agentData.m_params.at(id));

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->append_variable_data(m_currentagentKey, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	{

	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		QString key = "field_" + QString::number(id);

		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                m_pModelWidget->pOSGManager()->addField(id);
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
		QJsonObject datacontext;
		datacontext.insert("fldmdName", "field");
		datacontext.insert("fldmdNameI18n", "场域");
		datacontext.insert("fldmdkey", key);
		datacontext.insert("offsetx", 0.0);
		datacontext.insert("offsety", 0.0);
		datacontext.insert("offsetz", 0.0);
		datacontext.insert("orient", 0);
		datacontext.insert("passive", false);

		auto fielditor = std::find_if(m_agentData.m_fieldsensing.begin(),
			m_agentData.m_fieldsensing.end(), [&](const std::unordered_map<uint64_t, std::list<uint64_t>>::value_type& vt) {
			return vt.first == id;
		});
		if (fielditor == m_agentData.m_fieldsensing.end())
		{
			m_agentData.m_fieldsensing.insert(std::make_pair(id, std::list<uint64_t>()));
		}

		m_agentData.m_field.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(1)), id, m_agentData.m_field.at(id));

		updateFieldNodeContext(TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS, id, m_agentData.m_field.at(id)["fldmdName"].toString());

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->append_field_data(m_currentagentKey, datacontext);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
		uint64_t fieldid = 0;
		if (pTreeModel)
		{
			if (pTreeModel->getTargetParentId(type, id, fieldid))
			{
				switch (m_eAgentType)
				{
				case E_AGENT_TYPE_INSTAGENT:
				{
					if (m_pModelWidget)
					{
                        m_pModelWidget->pOSGManager()->addSensing(fieldid, id);
					}
				}
				break;
				case E_AGENT_TYPE_SCENE:
					break;
				default:
					break;
				}
			}
		}

		auto fielditor = std::find_if(m_agentData.m_fieldsensing.begin(),
			m_agentData.m_fieldsensing.end(), [&](const std::unordered_map<uint64_t, std::list<uint64_t>>::value_type& vt) {
			return vt.first == fieldid;
		});
		if (fielditor != m_agentData.m_fieldsensing.end())
		{
			std::list<uint64_t>& sensinglist = fielditor->second;
			sensinglist.push_back(id);
		}

		QString key = "sensing_" + QString::number(id);
		QJsonObject datacontext;
		datacontext.insert("mode", 0);
		datacontext.insert("smdkey", key);
		datacontext.insert("roll", 0.0);
		datacontext.insert("pitch", 0.0);
		datacontext.insert("azimuth", 0.0);
		datacontext.insert("dpch", 45.0);
		datacontext.insert("dazm", 45.0);
		datacontext.insert("epsln", 1e-7);
		datacontext.insert("emgwpty", 0.0);
		datacontext.insert("sndwpty", 0.0);
		datacontext.insert("frqmean", 0);
		datacontext.insert("frqvarn", 0);
		datacontext.insert("freqdis", 0);
		datacontext.insert("wavescale", 0);
		datacontext.insert("frqusage", 0);
		datacontext.insert("wavesndrcv", 0);
		datacontext.insert("silencegap", 0);
		QString fieldkey = "field_" + QString::number(fieldid);
		datacontext.insert("fldid", fieldkey);
		datacontext.insert("modsig", QString::number(FunctionAssistant::generate_random_positive_uint64()));
		datacontext.insert("stopSensingIn", 0);
		datacontext.insert("multiplier", 0);
		datacontext.insert("radius", 25);

		m_agentData.m_sensing.insert(std::make_pair(id, std::move(datacontext)));
		showSubPage(dynamic_cast<QQuickWidget*>(m_listwidgets.at(4)), id, m_agentData.m_sensing.at(id));

		if (DataManager::getInstance().pHttpClient())
		{
			DataManager::getInstance().pHttpClient()->append_sensing_data(m_currentagentKey, datacontext);
		}
	}
	break;
	default:break;
	}
	emit edit_type_sig(type, id, true, false);
	emit add_type_sig(type, id);
	m_currenttype = type;

	E_MODEL_MODE  _eModel_mode = E_MODEL_MODE_DISPLAY;

	if (type == TreeNode::E_NODE_TYPE_SUBMODELS ||
		type == TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE ||
		type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS ||
		type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE)
	{
		_eModel_mode = E_MODEL_MODE_EDIT;
	}
	switch (m_eAgentType)
	{
	case E_AGENT_TYPE_INSTAGENT:
	{
		if (m_pModelWidget)
		{
            m_pModelWidget->pOSGManager()->updateModelMode(_eModel_mode);
		}
	}
	break;
	case E_AGENT_TYPE_SCENE:
		break;
	default:
		break;
	}

}

void QtModelWidget::qml_quit_agent_edit_panel_slot()
{
	clearData();
}

#if 0
void QtModelWidget::appendWaypoint_slot(quint64 waypointid, double lng, double lat)
{
	if (m_currentEditId == 0)
	{
		return;
	}
	double alt = 0.0;
	QQuickWidget* qmlWidget = nullptr;
	switch (m_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			if (pRoot)
			{
				QVariant value = pRoot->property("mpointid");
				QString mpointstr = value.toString();
				uint64_t poipointid = 0;
				if (mpointstr.isEmpty())
				{
					QMetaObject::invokeMethod(pRoot, "appendPoint",
						Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))),
						Q_ARG(QVariant, QVariant::fromValue(lng)),
						Q_ARG(QVariant, QVariant::fromValue(lat)),
						Q_ARG(QVariant, QVariant::fromValue(alt)));
					poipointid = waypointid;

					std::list<waypointinfo> waypointlist;// = _entitywaypointmap_itor->second.m_waypts;
					waypointinfo val{ poipointid,lng,lat,0,1,(int)(waypointlist.size()) * 10 };
					waypointlist.push_back(val);


					QColor color = QColor(255, 0, 0, 255);
					LAT_LNG curpos{ lat,lng };
					m_pMapEditWidget->add_entity_waypoint(m_currentEditId, poipointid, curpos, waypointlist, color);

					m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
				}
			}
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));

		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "appendPoint",
				Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))),
				Q_ARG(QVariant, QVariant::fromValue(lng)),
				Q_ARG(QVariant, QVariant::fromValue(lat)),
				Q_ARG(QVariant, QVariant::fromValue(alt)));


			//// 获取 QML 中的 ListModel 对象
			//QObject* agent_edit_attributeObject = pRoot->findChild<QObject*>("agent_edit_config");
			//if (agent_edit_attributeObject)
			//{
			//	QObject* listModelObject = pRoot->findChild<QObject*>("fence_points_list_listModel");
			//	if (listModelObject) {
			//		std::list<waypointinfo> waypointlist;
			//		// 从 ListModel 中获取数据并打印
			//		QVariantList dataList = listModelObject->property("data").toList();
			//		for (const QVariant& data : dataList)
			//		{
			//			QVariantMap item = data.toMap();
			//			QString pointid = item.value("pointid").toString();
			//			uint64_t poipointid = pointid.toULongLong();
			//			double lngold = item.value("lng").toDouble();
			//			double latold = item.value("lat").toDouble();
			//			double altold = item.value("alt").toDouble();
			//			waypointinfo val{ poipointid, lngold, latold, altold, 1, (int)(waypointlist.size()) * 10 };
			//			waypointlist.push_back(val);
			//		}

			//		waypointinfo val{ waypointid,lng,lat,0,1,(int)(waypointlist.size()) * 10 };
			//		waypointlist.push_back(val);

			//		QColor color = QColor(255, 0, 0, 255);
			//		LAT_LNG curpos{ lat,lng };
			//		m_pMapEditWidget->add_entity_waypoint(m_currentEditId, waypointid, curpos, waypointlist, color);

			//		m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
			//	}
			//}
		}
	}
	break;
	default:
		break;
	}
}

void QtModelWidget::updateWaypoint_slot(quint64 waypointid, double lng, double lat)
{
	if (m_currentEditId == 0)
	{
		return;
	}
	double alt = 0.0;
	QQuickWidget* qmlWidget = nullptr;
	switch (m_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			if (pRoot)
			{
				QVariant value = pRoot->property("mpointid");
				QString mpointstr = value.toString();
				uint64_t poipointid = 0;
				if (!mpointstr.isEmpty())
				{
					QMetaObject::invokeMethod(pRoot, "updatePoint",
						Q_ARG(QVariant, QVariant::fromValue(mpointstr)),
						Q_ARG(QVariant, QVariant::fromValue(lng)),
						Q_ARG(QVariant, QVariant::fromValue(lat)),
						Q_ARG(QVariant, QVariant::fromValue(alt)));
					poipointid = mpointstr.toULongLong();

					std::list<waypointinfo> waypointlist;
					waypointinfo val{ poipointid,lng,lat,0,1,(int)(waypointlist.size()) * 10 };
					waypointlist.push_back(val);

					QColor color = QColor(255, 0, 0, 255);
					LAT_LNG curpos{ lat,lng };
					m_pMapEditWidget->add_entity_waypoint(m_currentEditId, poipointid, curpos, waypointlist, color);
					m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
					m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, poipointid);
				}
			}
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));

		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "updatePoint",
				Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))),
				Q_ARG(QVariant, QVariant::fromValue(lng)),
				Q_ARG(QVariant, QVariant::fromValue(lat)),
				Q_ARG(QVariant, QVariant::fromValue(alt)));


			//// 获取 QML 中的 ListModel 对象
			//QObject* listModelObject = pRoot->findChild<QObject*>("fence_points_list_listModel");
			//if (listModelObject) {
			//	std::list<waypointinfo> waypointlist;
			//	// 从 ListModel 中获取数据并打印
			//	QVariantList dataList = listModelObject->property("data").toList();
			//	for (const QVariant& data : dataList)
			//	{
			//		QVariantMap item = data.toMap();
			//		QString pointid = item.value("pointid").toString();
			//		uint64_t poipointid = pointid.toULongLong();
			//		if (poipointid == waypointid)
			//		{
			//			QMetaObject::invokeMethod(pRoot, "updatePoint",
			//				Q_ARG(QVariant, QVariant::fromValue(pointid)),
			//				Q_ARG(QVariant, QVariant::fromValue(lng)),
			//				Q_ARG(QVariant, QVariant::fromValue(lat)),
			//				Q_ARG(QVariant, QVariant::fromValue(alt)));
			//			waypointinfo val{ poipointid, lng, lat, alt, 1, (int)(waypointlist.size()) * 10 };
			//			waypointlist.push_back(val);
			//		}
			//		else
			//		{
			//			double lngold = item.value("lng").toDouble();
			//			double latold = item.value("lat").toDouble();
			//			double altold = item.value("alt").toDouble();
			//			waypointinfo val{ poipointid, lngold, latold, altold, 1, (int)(waypointlist.size()) * 10 };
			//			waypointlist.push_back(val);
			//		}
			//	}

			//	QColor color = QColor(255, 0, 0, 255);
			//	LAT_LNG curpos{ lat,lng };
			//	m_pMapEditWidget->add_entity_waypoint(m_currentEditId, waypointid, curpos, waypointlist, color);
			//	m_pMapEditWidget->locate_entity_tracking(m_currentEditId);;
			//	m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, waypointid);
			//}
		}
	}
	break;
	default:
		break;
	}
}

void QtModelWidget::selectWaypoint_slot(quint64 waypointid)
{
	bool bExist = false;
	QQuickWidget* qmlWidget = nullptr;
	switch (m_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			if (pRoot)
			{
				QVariant value = pRoot->property("mpointid");
				QString mpointstr = value.toString();

				if (!mpointstr.isEmpty())
				{
					uint64_t poipointid = mpointstr.toULongLong();
					if (poipointid == waypointid)
					{
						QObject* pRoot = (QObject*)qmlWidget->rootObject();
						QMetaObject::invokeMethod(pRoot, "selectPoint",
							Q_ARG(QVariant, QVariant::fromValue(mpointstr)));

						m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
						m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, poipointid);
						bExist = true;
					}
				}
			}
		}
		if (!bExist)
		{
			m_pMapEditWidget->locate_entity_tracking(0);
			m_pMapEditWidget->locate_entity_waypoint(0, 0);
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));

		if (qmlWidget)
		{
			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "selectPoint",
				Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))));

			//// 获取 QML 中的 ListModel 对象
			//QObject* listModelObject = pRoot->findChild<QObject*>("fence_points_list_listModel");
			//if (listModelObject) {
			//	std::list<waypointinfo> waypointlist;
			//	// 从 ListModel 中获取数据并打印
			//	QVariantList dataList = listModelObject->property("data").toList();
			//	for (const QVariant& data : dataList)
			//	{
			//		QVariantMap item = data.toMap();
			//		QString pointid = item.value("pointid").toString();
			//		uint64_t poipointid = pointid.toULongLong();
			//		if (poipointid == waypointid)
			//		{
			//			QObject* pRoot = (QObject*)qmlWidget->rootObject();
			//			QMetaObject::invokeMethod(pRoot, "selectPoint",
			//				Q_ARG(QVariant, QVariant::fromValue(pointid)));

			//			m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
			//			m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, poipointid);
			//			bExist = true;
			//			break;
			//		}
			//	}
			//}
		}
	}
	break;
	default:
		break;
	}

}

#else

void QtModelWidget::appendWaypoint_slot(quint64 waypointid, double lng, double lat)
{
	appendWaypoint(m_currenttype,m_currentEditId, waypointid, lng, lat);
}

void QtModelWidget::updateWaypoint_slot(quint64 waypointid, double lng, double lat)
{
	if (m_currentEditId == 0)
	{
		return;
	}
	double alt = 0.0;
	std::unordered_map<quint64, geometry_points_info>* _pgeometrypointmap = nullptr;
	QQuickWidget* qmlWidget = nullptr;
	switch (m_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		_pgeometrypointmap = &m_agentData.m_poi_geometrypointmap;
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));
		_pgeometrypointmap = &m_agentData.m_fence_geometrypointmap;
	}
	break;
	default:
		break;
	}

	if (qmlWidget && _pgeometrypointmap)
	{
		uint64_t currentedit = 0;
		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		if (pRoot)
		{
			auto _entitywaypointmap_itor = _pgeometrypointmap->begin();
			while (_entitywaypointmap_itor != _pgeometrypointmap->end())
			{
				quint64 _currententityid = _entitywaypointmap_itor->first;
				std::list<waypointinfo>& waypointlist = _entitywaypointmap_itor->second.m_pts;
				auto waypointlist_itor = std::find_if(waypointlist.begin(),
					waypointlist.end(),
					[&](const std::list<waypointinfo>::value_type& vt) {
					return vt.waypointid == waypointid;
				});
				if (waypointlist_itor != waypointlist.end())
				{
					waypointinfo& info = *waypointlist_itor;
					info.lng = lng;
					info.lat = lat;
					m_currentEditId = _currententityid;

					QMetaObject::invokeMethod(pRoot, "updatePoint",
						Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))),
						Q_ARG(QVariant, QVariant::fromValue(lng)),
						Q_ARG(QVariant, QVariant::fromValue(lat)),
						Q_ARG(QVariant, QVariant::fromValue(alt)));

					LAT_LNG curpos{ lat,lng };

					m_add_waypoints.insert(std::make_pair(waypointid, m_currentEditId));
					m_pMapEditWidget->add_entity_waypoint(m_currentEditId, waypointid, curpos, waypointlist, _entitywaypointmap_itor->second.cl);
					m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
					m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, waypointid);

					break;
				}
				_entitywaypointmap_itor++;
			}
		}
	}
}

void QtModelWidget::selectWaypoint_slot(quint64 waypointid)
{
	//if (m_currentEditId == 0)
	//{
	//	return;
	//}
	double alt = 0.0;
	std::unordered_map<quint64, geometry_points_info>* _pgeometrypointmap = nullptr;
	QQuickWidget* qmlWidget = nullptr;
	switch (m_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		_pgeometrypointmap = &m_agentData.m_poi_geometrypointmap;
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));
		_pgeometrypointmap = &m_agentData.m_fence_geometrypointmap;
	}
	break;
	default:
		break;
	}

	if (qmlWidget && _pgeometrypointmap)
	{
		uint64_t currentedit = 0;
		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		if (pRoot)
		{
			//QVariant value = pRoot->property("edit_id");
			//QString edit_idstr = value.toString();
			//currentedit = edit_idstr.toULongLong();
			bool bExist = false;
			auto _entitywaypointmap_itor = _pgeometrypointmap->begin();
			while (_entitywaypointmap_itor != _pgeometrypointmap->end())
			{
				quint64 _currententityid = _entitywaypointmap_itor->first;
				std::list<waypointinfo>& waypointlist = _entitywaypointmap_itor->second.m_pts;
				auto waypointlist_itor = std::find_if(waypointlist.begin(),
					waypointlist.end(),
					[&](const std::list<waypointinfo>::value_type& vt) {
					return vt.waypointid == waypointid;
				});
				if (waypointlist_itor != waypointlist.end())
				{
					waypointinfo& info = *waypointlist_itor;
					m_currentEditId = _currententityid;

					QMetaObject::invokeMethod(pRoot, "selectPoint",
						Q_ARG(QVariant, QVariant::fromValue(QString::number(waypointid))));

					m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
					m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, waypointid);
					bExist = true;
					break;
				}
				_entitywaypointmap_itor++;
			}
			if (!bExist)
			{
				m_pMapEditWidget->locate_entity_tracking(0);
				m_pMapEditWidget->locate_entity_waypoint(0, 0);
			}
		}
	}
}

#endif

void QtModelWidget::addModelFinished(QPair<uint64_t, QString>& agentInfo, const  QModelIndex& index, bool bLoad)
{
	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (bLoad)
	{
		if (pTreeModel)
		{
			QModelIndex addmodelindex = pTreeModel->appendSubAgentChild(index, agentInfo);

			QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
			QMetaObject::invokeMethod(pRoot, "expand",
				Q_ARG(QVariant, QVariant::fromValue(index)));

			QMetaObject::invokeMethod(pRoot, "selectIndex",
				Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));
		}
		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
                m_pModelWidget->pOSGManager()->getModelSenceData()->setModelInstanceSelect(0, agentInfo.first, agentInfo.first, true);

				QJsonObject jsondata;
                if (m_pModelWidget->pOSGManager()->getModelSenceData()->getModelProfileData(agentInfo.first, jsondata))
				{
					m_agentData.m_modelAbstracted.insert(std::make_pair(agentInfo.first, std::move(jsondata)));
				}
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}
	}
	else
	{
		if (pTreeModel)
		{
			QModelIndex parentModelindex;
			if (pTreeModel->getSubModelIndex(TreeNode::E_NODE_TYPE_SUBMODELS, parentModelindex, 0))
			{
				QModelIndex addmodelindex = pTreeModel->appendSubAgentChild(parentModelindex, agentInfo, false);

				//            QObject *pRoot = (QObject*)m_qmlpanelWidget->rootObject();
				//            QMetaObject::invokeMethod(pRoot, "expand",
				//                                      Q_ARG(QVariant, QVariant::fromValue(parentModelindex)));

				//            QMetaObject::invokeMethod(pRoot, "selectIndex",
				//                                      Q_ARG(QVariant, QVariant::fromValue(addmodelindex)));
			}

		}

		switch (m_eAgentType)
		{
		case E_AGENT_TYPE_INSTAGENT:
		{
			if (m_pModelWidget)
			{
				QJsonObject jsondata;
                if (m_pModelWidget->pOSGManager()->getModelSenceData()->getModelProfileData(agentInfo.first, jsondata))
				{
					m_agentData.m_modelAbstracted.insert(std::make_pair(agentInfo.first, std::move(jsondata)));
				}
			}
		}
		break;
		case E_AGENT_TYPE_SCENE:
			break;
		default:
			break;
		}

		//edit_type_slot(TreeNode::E_NODE_TYPE_CONFIGS, 0, true, false);
		//this->m_pOSGManager->getModelSenceData()->setModelInstanceSelect(0, agentInfo.first, agentInfo.first, true);
	}
}

void QtModelWidget::showSubPage(QQuickWidget* qmlWidget, uint64_t id, QJsonObject& datacontext)
{
	if (qmlWidget)
	{
		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		QMetaObject::invokeMethod(pRoot, "setContextDataId",
			Q_ARG(QVariant, QVariant::fromValue(QString::number(id))));
		QMetaObject::invokeMethod(pRoot, "setContextData",
			Q_ARG(QVariant, QVariant::fromValue(datacontext)));
	}
}

void QtModelWidget::updateFieldNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context)
{
	auto itor = std::find_if(m_agentData.m_field.begin(),
		m_agentData.m_field.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == id;
	});
	if (itor != m_agentData.m_field.end())
	{
		QJsonObject& datacontext = itor->second;
		datacontext["fldmdName"] = context;

        m_treeViewController->updateNodeContext(type, id, context);
    }
}

void QtModelWidget::updatePoiNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString &context)
{
    auto itor = std::find_if(m_agentData.m_pois.begin(),
                             m_agentData.m_pois.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
                                 return vt.first == id;
                             });
    if (itor != m_agentData.m_pois.end())
    {
        QJsonObject& datacontext = itor->second;
        datacontext["poiKeyword"] = context;

        m_treeViewController->updateNodeContext(type, id, context);
    }
}

void QtModelWidget::updateActionNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context)
{
	auto itor = std::find_if(m_agentData.m_actions.begin(),
		m_agentData.m_actions.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == id;
	});
	if (itor != m_agentData.m_actions.end())
	{
		QJsonObject& datacontext = itor->second;
		datacontext["axnName"] = context;

		m_treeViewController->updateNodeContext(type, id, context);
	}
}

void QtModelWidget::updateOODANodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context)
{
	auto itor = std::find_if(m_agentData.m_oodas.begin(),
		m_agentData.m_oodas.end(), [&](const std::unordered_map<uint64_t, QJsonObject>::value_type& vt) {
		return vt.first == id;
	});
	if (itor != m_agentData.m_oodas.end())
	{
		QJsonObject& datacontext = itor->second;
		datacontext["oodaKeyword"] = context;

		m_treeViewController->updateNodeContext(type, id, context);
	}
}

void QtModelWidget::appendWaypoint(TreeNode::E_NODE_TYPE _currenttype,quint64 editid, quint64 waypointid, double lng, double lat)
{
	if (editid == 0)
	{
		return;
	}
	double alt = 0.0;
	std::unordered_map<quint64, geometry_points_info>* _pgeometrypointmap = nullptr;
	QQuickWidget* qmlWidget = nullptr;
	switch (_currenttype)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
		break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(8));
		_pgeometrypointmap = &m_agentData.m_poi_geometrypointmap;
	}
	break;
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		qmlWidget = dynamic_cast<QQuickWidget*>(m_listwidgets.at(9));
		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		_pgeometrypointmap = &m_agentData.m_fence_geometrypointmap;
	}
	break;
	default:
		break;
	}

	if (qmlWidget && _pgeometrypointmap)
	{
		uint64_t currentedit = 0;
		quint64 usepointid = waypointid;
		QObject* pRoot = (QObject*)qmlWidget->rootObject();
		if (pRoot)
		{
			QVariant value = pRoot->property("edit_id");
			QString edit_idstr = value.toString();
			currentedit = edit_idstr.toULongLong();

			if (currentedit == editid)
			{

			}
			auto _entitywaypointmap_itor = std::find_if(_pgeometrypointmap->begin(),
				_pgeometrypointmap->end(), [&](const std::unordered_map<quint64, geometry_points_info>::value_type& vt) {
				return vt.first == editid && vt.second.m_geometry_id == editid;
			});
			if (_entitywaypointmap_itor != _pgeometrypointmap->end())
			{
				std::list<waypointinfo>& waypointlist = _entitywaypointmap_itor->second.m_pts;
				if (_currenttype == TreeNode::E_NODE_TYPE_POIS)
				{
					usepointid = editid;
					if (waypointlist.empty())
					{
						waypointinfo val{ usepointid, lng, lat, 0, 1,(int)(waypointlist.size()) * 10 };
						waypointlist.push_back(val);

						QMetaObject::invokeMethod(pRoot, "appendPoint",
							Q_ARG(QVariant, QVariant::fromValue(QString::number(usepointid))),
							Q_ARG(QVariant, QVariant::fromValue(lng)),
							Q_ARG(QVariant, QVariant::fromValue(lat)),
							Q_ARG(QVariant, QVariant::fromValue(alt)));

						LAT_LNG curpos{ lat,lng };
						m_add_waypoints.insert(std::make_pair(usepointid, editid));
						m_pMapEditWidget->add_entity_waypoint(editid, usepointid, curpos, waypointlist, _entitywaypointmap_itor->second.cl);

						m_pMapEditWidget->locate_entity_tracking(editid);
					}
					else
					{
						LAT_LNG curpos{ lat,lng };
						m_add_waypoints.insert(std::make_pair(usepointid, editid));
						m_pMapEditWidget->add_entity_waypoint(editid, usepointid, curpos, waypointlist, _entitywaypointmap_itor->second.cl);

						m_pMapEditWidget->locate_entity_tracking(editid);
						//auto waypointlist_itor = std::find_if(waypointlist.begin(),
						//	waypointlist.end(),
						//	[&](const std::list<waypointinfo>::value_type& vt) {
						//		return vt.waypointid == usepointid;
						//	});
						//if (waypointlist_itor != waypointlist.end())
						//{
						//	waypointinfo& info = *waypointlist_itor;
						//	info.lng = lng;
						//	info.lat = lat;

						//	QMetaObject::invokeMethod(pRoot, "updatePoint",
						//		Q_ARG(QVariant, QVariant::fromValue(QString::number(usepointid))),
						//		Q_ARG(QVariant, QVariant::fromValue(lng)),
						//		Q_ARG(QVariant, QVariant::fromValue(lat)),
						//		Q_ARG(QVariant, QVariant::fromValue(alt)));

						//	LAT_LNG curpos{ lat,lng };
						//	m_pMapEditWidget->add_entity_waypoint(m_currentEditId, usepointid, curpos, waypointlist, _entitywaypointmap_itor->second.cl);
						//	m_pMapEditWidget->locate_entity_tracking(m_currentEditId);
						//	//m_pMapEditWidget->locate_entity_waypoint(m_currentEditId, usepointid);
						//}						
					}
				}
				else if (_currenttype == TreeNode::E_NODE_TYPE_FENCES)
				{
					waypointinfo val{ usepointid, lng, lat, 0, 1,(int)(waypointlist.size()) * 10 };
					waypointlist.push_back(val);

					QMetaObject::invokeMethod(pRoot, "appendPoint",
						Q_ARG(QVariant, QVariant::fromValue(QString::number(usepointid))),
						Q_ARG(QVariant, QVariant::fromValue(lng)),
						Q_ARG(QVariant, QVariant::fromValue(lat)),
						Q_ARG(QVariant, QVariant::fromValue(alt)));

					LAT_LNG curpos{ lat,lng };
					m_add_waypoints.insert(std::make_pair(usepointid, editid));
					m_pMapEditWidget->add_entity_waypoint(editid, usepointid, curpos, waypointlist, _entitywaypointmap_itor->second.cl);

					m_pMapEditWidget->locate_entity_tracking(editid);
				}
			}
		}
	}
}

void QtModelWidget::initTreemodel()
{
	auto addTreeNode = [&](const QString& title, TreeNode::E_NODE_TYPE treeNodeType, TreeNode* pParent) -> TreeNode* {

		uint64_t treenodeid = FunctionAssistant::generate_random_positive_uint64();
		QList<QVariant> list_sub;
		list_sub.append(title);
		list_sub.append(treeNodeType);
		list_sub.append(treenodeid);
		TreeNode* pTreeNodesub = new TreeNode(treeNodeType, list_sub, pParent, treenodeid);
		if (pParent)
		{
			pParent->appendChild(pTreeNodesub);
		}
		return pTreeNodesub;
	};
	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		auto pTreeNode = addTreeNode(tr("agent name"), TreeNode::E_NODE_TYPE_NULL, nullptr);
		pTreeModel->init(pTreeNode);

		addTreeNode(tr("GeneralConfigs"), TreeNode::E_NODE_TYPE_CONFIGS, pTreeNode);
		addTreeNode(tr("SubModels"), TreeNode::E_NODE_TYPE_SUBMODELS, pTreeNode);
		addTreeNode(tr("SubAgents"), TreeNode::E_NODE_TYPE_SUBAGENTS, pTreeNode);
		addTreeNode(tr("Variables"), TreeNode::E_NODE_TYPE_VAEDEFS, pTreeNode);
		addTreeNode(tr("Actions"), TreeNode::E_NODE_TYPE_ACTIONS, pTreeNode);
		addTreeNode(tr("FieldMedias"), TreeNode::E_NODE_TYPE_FIELDMEDIAS, pTreeNode);
		addTreeNode(tr("Waypoints"), TreeNode::E_NODE_TYPE_WAYPOINTS, pTreeNode);
		addTreeNode(tr("OODAs"), TreeNode::E_NODE_TYPE_OODAS, pTreeNode);
		addTreeNode(tr("POIs"), TreeNode::E_NODE_TYPE_POIS, pTreeNode);
		addTreeNode(tr("Fences"), TreeNode::E_NODE_TYPE_FENCES, pTreeNode);
	}
}

void QtModelWidget::updateAgentDesc(const QString& desc)
{
	QObject* pRoot = (QObject*)m_qmlpanelWidget->rootObject();
	QMetaObject::invokeMethod(pRoot, "updateAgentDesc",
		Q_ARG(QVariant, QVariant::fromValue((int32_t)m_eAgentType)),
		Q_ARG(QVariant, QVariant::fromValue(desc)));

	TreeModel* pTreeModel = dynamic_cast<TreeModel*>(m_treeViewController->getTreeModel());
	if (pTreeModel)
	{
		pTreeModel->setRootNodeName(desc);
	}
}


void QtModelWidget::keyPressEvent(QKeyEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->keyPressEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->keyPressEvent(event);
	}
	QWidget::keyPressEvent(event);
}

void QtModelWidget::keyReleaseEvent(QKeyEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->keyReleaseEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->keyReleaseEvent(event);
	}
	QWidget::keyReleaseEvent(event);
}

void QtModelWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->mouseMoveEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->mouseMoveEvent(event);
	}
	QWidget::mouseMoveEvent(event);
}

void QtModelWidget::mousePressEvent(QMouseEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->mousePressEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->mousePressEvent(event);
	}
	QWidget::mousePressEvent(event);
}

void QtModelWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->mouseReleaseEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->mouseReleaseEvent(event);
	}
	QWidget::mouseReleaseEvent(event);
}

void QtModelWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->mouseDoubleClickEvent(event);
	}

	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->mouseDoubleClickEvent(event);
	}
	QWidget::mouseDoubleClickEvent(event);
}

void QtModelWidget::wheelEvent(QWheelEvent* event)
{
	if (m_pModelWidget && m_pModelWidget->isVisible())
	{
		m_pModelWidget->wheelEvent(event);
	}
	if (m_pMapEditWidget && m_pMapEditWidget->isVisible())
	{
		m_pMapEditWidget->wheelEvent(event);
	}
	QWidget::wheelEvent(event);
}


void QtModelWidget::resizeEvent(QResizeEvent* event)
{
	m_qmlpanelWidget->setGeometry(this->geometry());
	m_pQStackedWidget->widget(E_AGENT_TYPE_INSTAGENT)->setGeometry(this->geometry());
	m_pQStackedWidget->widget(E_AGENT_TYPE_SCENE)->setGeometry(this->geometry());
	QWidget::resizeEvent(event);
}

void QtModelWidget::showEvent(QShowEvent* event)
{
	m_qmlpanelWidget->setVisible(true);

	m_pQStackedWidget->currentWidget()->setVisible(true);


	QWidget::showEvent(event);
}

void QtModelWidget::hideEvent(QHideEvent* event)
{
	m_qmlpanelWidget->setVisible(false);
	m_pQStackedWidget->currentWidget()->setVisible(false);
	QWidget::hideEvent(event);
}


