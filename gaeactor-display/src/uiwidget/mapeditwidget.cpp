#pragma execution_character_set("utf-8")
#include "mapeditwidget.h"
#include <QKeyEvent>
#include <QCoreApplication>
#include "widget2d/pickingobject.h"
#include "widget2d/pickingtexture.h"
#include "widget2d/pickidmanager.h"

MapEditWidget::MapEditWidget(E_MAP_MODE eMapMode, QWidget *parent)
	: MapWidget(eMapMode,parent),
	m_pPickingObject(nullptr),
	m_pPickingTexture(nullptr),
	m_pickshader(nullptr),
	m_bPressed(false),
	m_bCtrl(false)
{
    //    qRegisterMetaType<LAT_LNG>("LAT_LNG");
	setWindowTitle("Edit Agent Runtime Style");
	setFocusPolicy(Qt::StrongFocus);
}

MapEditWidget::~MapEditWidget()
{
	if (m_pickshader)
	{
		delete m_pickshader;
	}
}

void MapEditWidget::remove_entity_waypoint(const quint64 &entityid, const quint64 &waypointid, const std::list<waypointinfo> &waypts, const QColor& color)
{
	m_maprender->clearEntityItem(waypointid);
	if (waypts.empty())
	{
		m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, entityid);
	}
	else
	{
        std::vector<LAT_LNG> ptlist;
        ptlist.resize(waypts.size());
        int i = 0;
		for (auto item : waypts)
        {
            ptlist[i].lat = item.lat;
            ptlist[i].lng = item.lng;
            i++;
		}
        m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, entityid, ptlist, color, DrawElements::E_ELEMENT_TYPE_TRACKING,false);
	}
}

void MapEditWidget::clear_entity_waypoint_tracking(const quint64 &entityid)
{
	m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, entityid);

}

void MapEditWidget::add_entity_waypoint_tracking(const quint64 &entityid, const QColor &color)
{
    std::vector<LAT_LNG> ptlist;
    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, entityid, ptlist, color, DrawElements::E_ELEMENT_TYPE_TRACKING,false);
}

void MapEditWidget::add_entity_waypoint(const quint64 &entityid, const quint64 &waypointid, const LAT_LNG& currentpos, const std::list<waypointinfo> &waypts, const QColor &color, bool bNeedTrans84GC)
{
	m_maprender->appendWaypointsItem(waypointid, currentpos,200.0, bNeedTrans84GC);
    std::vector<LAT_LNG> ptlist;
    ptlist.resize(waypts.size());
    int i = 0;
    for (auto item : waypts)
    {
        ptlist[i].lat = item.lat;
        ptlist[i].lng = item.lng;
        i++;
    }

    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, entityid, ptlist, color, DrawElements::E_ELEMENT_TYPE_TRACKING,false);
}

void MapEditWidget::locate_entity_waypoint(const quint64 &entityid, const quint64 &waypointid)
{
	m_seletitemid = waypointid;
	m_maprender->updateEntityItemSelect(waypointid, true);
}

void MapEditWidget::locate_entity_tracking(const quint64 &entityid)
{
	m_maprender->updateElementSelect(entityid, true);
}

GeoJsonInfos MapEditWidget::addJsonFile(const QString& jsfilename)
{
	GeoJsonInfos geoinfos;
	if (DataManager::getInstance().readGeoJsonData(jsfilename.toStdString(), geoinfos))
	{
		drawGeoData(geoinfos, true, true,true);
	}	
	return geoinfos;
}

void MapEditWidget::paintGL()
{
	if (m_bCtrl && m_bPressed)
	{
		PickingPhase(m_select_x, m_select_y);
	}

	MapWidget::paintGL();
}

void MapEditWidget::resizeGL(int w, int h)
{
	MapWidget::resizeGL(w, h);

	m_pPickingObject->Init(w, h);
	m_pPickingTexture->Init(w, h);

}

void MapEditWidget::initializeGL()
{
	MapWidget::initializeGL();
	QOpenGLExtraFunctions  *funcs = this->context()->extraFunctions();
	m_pPickingObject = new PickingObject(funcs);
	m_pPickingTexture = new PickingTexture(funcs);

	m_pickshader = new Shader(funcs);
	QString path = QCoreApplication::applicationDirPath();
	m_pickshader->load((path + "./shaders/map.vs").toStdString().c_str(), (path + "./shaders/picking.fs").toStdString().c_str());
}

void MapEditWidget::keyPressEvent(QKeyEvent *event)
{
	MapWidget::keyPressEvent(event);
	if ((event->key() == Qt::Key_Control))
	{
		m_bCtrl = true;
	}
}


void MapEditWidget::keyReleaseEvent(QKeyEvent *event)
{
	if ((event->key() == Qt::Key_Control))
	{
		m_bCtrl = false;

		//        locate_entity_tracking(0);
		locate_entity_waypoint(0, 0);
	}
	MapWidget::keyReleaseEvent(event);
}

void MapEditWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (m_bPressed)
	{
		m_select_x = event->x();
		m_select_y = /*height() -*/ event->y();
	}
	if (m_bCtrl)
	{
		if (m_seletitemid != 0)
		{
			auto localPos = event->pos();
            LAT_LNG pos = m_maprender->screenToLatLng(localPos);
			emit updateWaypoint_sig(m_seletitemid, pos.lng, pos.lat);

		}
	}

	MapWidget::mouseMoveEvent(event);
}

void MapEditWidget::wheelEvent(QWheelEvent *event)
{
	MapWidget::wheelEvent(event);
}

void MapEditWidget::mousePressEvent(QMouseEvent *event)
{
	auto localPos = event->pos();

	if (event->button() == Qt::RightButton)
	{
		if (m_eMapMode == E_MAP_MODE_SELECT)
		{
            LAT_LNG pos = m_maprender->screenToLatLng(localPos);
			quint64 id = FunctionAssistant::generate_random_positive_uint64();
			emit appendWaypoint_sig(id, pos.lng, pos.lat);
		}
	}
	else  if (event->button() == Qt::LeftButton) {
		m_select_x = event->x();
		m_select_y = /*height() -*/ event->y();
		if (m_bCtrl)
		{
			m_bPressed = true;
		}
	}
	if (!m_bCtrl)
	{
		MapWidget::mousePressEvent(event);
	}
}

void MapEditWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		m_select_x = event->x();
		m_select_y = /*height() -*/ event->y();
		m_bPressed = false;
		if (!m_bCtrl)
		{
			locate_entity_waypoint(0, 0);
			locate_entity_tracking(0);
		}
	}

	if (!m_bCtrl)
	{
        MapWidget::mouseReleaseEvent(event);
    }
}

void MapEditWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    MapWidget::mouseDoubleClickEvent(event);
}


void MapEditWidget::PickingPhase(int x, int y)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/); //清除屏幕和深度缓存
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glViewport(0, 0, m_w, m_h);
#if 0
	m_pPickingObject->EnableWriting();

	m_maprender->renderPick(this->context(), m_lineelementshader, m_trielementshader, m_pRadarshader, m_pickshader, m_pTextshader, m_lineubouniform, m_triubouniform, m_radarubouniform, false);

	m_pPickingObject->DisableWriting();

	uint32_t color = m_pPickingObject->ReadPixel(x, y);
#else
	m_maprender->renderPick(this->context(), m_lineelementshader, m_trielementshader, m_pickshader, m_pTextshader, m_lineubouniform, m_triubouniform, true);
	uint32_t color = m_maprender->pickingPhase(x, y);
	auto ele_id = PickIdManager::getInstance().getId(color);
	switch (std::get<1>(ele_id)) {
	case PickIdManager::E_ITEM_TYPE_NORMAL:
	{
		        //locate_entity_waypoint(0, 0);
		        locate_entity_tracking(std::get<0>(ele_id).sensorid);
	}break;
	case PickIdManager::E_ITEM_TYPE_PAIR: break;
	case PickIdManager::E_ITEM_TYPE_ICON:
	{
		emit selectWaypoint_sig(std::get<0>(ele_id).sensorid);
	}break;
	default:
	{
		//        locate_entity_tracking(0);
		//        locate_entity_waypoint(0, 0);
	}
	break;
	}

	std::cout << std::hex << " select " << std::get<0>(ele_id).sensorid << " color " << color << std::endl;
#endif

}
