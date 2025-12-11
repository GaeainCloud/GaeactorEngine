#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtMath>
#include <QDateTime>

#include <QDir>
#include <QFileInfoList>
#include "./widget2d/map2dwidget.h"
#include "./widget2d/map2deditwidget.h"

#include "widget3d/QtOsgWidget.h"
#include "widget3d/OSGManager.h"
#include "widget3d/ModelSceneData.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(1400,1200);
#if 1
    m_pMapWidget = new Map2dWidget(Map2dWidget::E_MAP_MODE_DISPLAY, this);
    this->setCentralWidget(m_pMapWidget);
#else
    m_pModelWidget2 = new Map2dEditWidget(Map2dWidget::E_MAP_MODE_SELECT, this);
    this->setCentralWidget(m_pModelWidget2);
    connect(m_pModelWidget2,&Map2dEditWidget::appendWaypoint_sig,this,&MainWindow::appendWaypoint_slot);
    connect(m_pModelWidget2,&Map2dEditWidget::updateWaypoint_sig,this,&MainWindow::updateWaypoint_slot);
    connect(m_pModelWidget2,&Map2dEditWidget::selectWaypoint_sig,this,&MainWindow::selectWaypoint_slot);

    m_currententityid = 0;

    entity_waypoints_info en_way_info;

    en_way_info.cl = FunctionAssistant::randColor(255);
    m_entitywaypointmap.insert(std::make_pair(m_currententityid, std::move(en_way_info)));
    m_pModelWidget2->add_entity_waypoint_tracking(m_currententityid, m_entitywaypointmap.at(m_currententityid).cl);
#endif

    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MAP, this);

    double lon = 103.94037755013733;
    double lat = 30.56454233609911;
    double hgt = 600;

    QWidget * pQWidget = new QWidget(this);

    m_pLayout = new QHBoxLayout(this);
    m_pLayout->addWidget(m_pModelWidget);
    m_pLayout->addWidget(m_pMapWidget);
    m_pLayout->setSpacing(1);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    pQWidget->setLayout(m_pLayout);
    this->setCentralWidget(pQWidget);

    m_pMapWidget->updateViewCenter(lon, lat);
    m_pModelWidget->updateViewPoint(lon, lat);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::appendWaypoint_slot(quint64 waypointid, double lng, double lat)
{
    LAT_LNG curpos{ lat,lng };

    //if (m_mapWidget->bTransfer())
    //{
    //  auto corretgeo = projectionmercator::ProjectionEPSG3857::wgs84_to_gcj02(lat, lng);
    //  curpos = LAT_LNG{ corretgeo.latitude(), corretgeo.longitude() };
    //}
    auto _entitywaypointmap_itor = m_entitywaypointmap.find(m_currententityid);
    if(_entitywaypointmap_itor != m_entitywaypointmap.end())
    {
        std::list<waypointinfo> & waypointlist = _entitywaypointmap_itor->second.m_waypts;
        waypointinfo val{waypointid,lng,lat,0,1,(int)(waypointlist.size())*10};
        waypointlist.push_back(val);


        m_pModelWidget2->add_entity_waypoint(m_currententityid,waypointid,curpos, waypointlist, _entitywaypointmap_itor->second.cl);

        m_pModelWidget2->locate_entity_tracking(m_currententityid);
    }
}

void MainWindow::updateWaypoint_slot(quint64 waypointid, double lng, double lat)
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

            LAT_LNG curpos{lat,lng};
            m_pModelWidget2->add_entity_waypoint(m_currententityid,waypointid,curpos, waypointlist, _entitywaypointmap_itor->second.cl);
            m_pModelWidget2->locate_entity_tracking(m_currententityid);
            m_pModelWidget2->locate_entity_waypoint(m_currententityid,waypointid);

            break;
        }
        _entitywaypointmap_itor++;
    }
}

void MainWindow::selectWaypoint_slot(quint64 waypointid)
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
            m_pModelWidget2->locate_entity_tracking(m_currententityid);
            m_pModelWidget2->locate_entity_waypoint(m_currententityid,waypointid);
            bExist = true;
            break;
        }
        _entitywaypointmap_itor++;
    }
    if(!bExist)
    {
        m_pModelWidget2->locate_entity_tracking(0);
        m_pModelWidget2->locate_entity_waypoint(0,0);
    }
}

