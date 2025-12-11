#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "head_define.h"
#include "ui_mainwindow.h"
#include <QGeoView/QGVMap.h>

#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVWidgetText.h>
#include <QGeoView/QGVLayerBing.h>
#include <QGeoView/QGVLayerGoogle.h>
#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVLayerLocal.h>

#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>
#include <QGeoView/QGVMapQGView.h>

#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QDir>
#include "src/ellipseitem.h"
#include "src/rectangleitem.h"
#include "src/polygonitem.h"
#include "src/pieitem.h"
#include "src/lineitem.h"
#include "src/imageitem.h"
#include "src/QGVWidgetTools.h"

#include <constants.h>
#include "gaeactormanager.h"
#include <iostream>
#include "src/listviewitem.h"
#include "testdata.h"
#include "src/OriginalDateTime.h"
#include <QKeyEvent>
#include "ProjectionEPSG3857.h"
#include "loghelper.h"
#include "redismanager.h"
#include "LocationHelper.h"
#include "runningmodeconfig.h"


#define KM_PH_TO_M_S(VAL)   ((VAL)*1000/3600)
#define SHOW_HEXAGON

#ifdef DEBUG_HEX
#ifndef SHOW_HEXAGON
#define SHOW_HEXAGON
#endif
#endif


#ifndef SHOW_HEXAGON
#define SHOW_CONVEXHULL
#endif

#define SPEED (500)
#define SHOW_ECHOWAVE
#ifdef _DEBUG
//#define SLIENT_TIME_GAP (255)
#define SLIENT_TIME_GAP (255)
#else
#define SLIENT_TIME_GAP (100)
#endif
#define GET_CUSTOM_LINE
Mercator latLng2WebMercator(const LAT_LNG &node)
{
    Mercator mercator;
    double earthRad = 6378137.0;
    double x = node.lng * M_PI / 180 * earthRad;
    double a = node.lat * M_PI / 180;
    double y = earthRad / 2 * log((1.0 + sin(a)) / (1.0 - sin(a)));
    mercator.x = x;
    mercator.y = y;
    return mercator; //[12727039.383734727, 3579066.6894065146]
}


Mercator latLng2WebMercator2(const LAT_LNG &node)
{
    Mercator mercator;
    mercator.x = node.lng * 20037508.34 / 180;
    mercator.y = log(tan((90 + node.lat)*M_PI / 360)) / (M_PI / 180);
    mercator.y = mercator.y * 20037508.34 / 180;

    return mercator; //[12727039.383734727, 3579066.6894065146]
}


LAT_LNG webMercator2LatLng(Mercator mercator)
{
    LAT_LNG node;
    node.lng = mercator.x / 20037508.34 * 180;
    node.lat = mercator.y / 20037508.34 * 180;
    node.lat = 180 / M_PI * (2 * atan(exp(node.lat * M_PI / 180)) - M_PI / 2);
    return node; //[114.32894001591471, 30.58574800385281]
}





MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pCurrentEllipse(nullptr)
    , m_pCurrentPie(nullptr)
    , m_pCurrentLine(nullptr)
    , m_pCurrentRectangle(nullptr)
    , m_pCurrentPolygon(nullptr)
    , m_pMoveTrackingItem(nullptr)
    , m_bSelectEnable(false)
    , m_bdoubleclicked(false)
    , m_cellsNum(0)
    , m_showTrackingLine(true)
    , m_showHexLayer(true)
    , m_speedcoeff(256.0)
    , m_pItemLayer(nullptr)
    , m_pItemHexLayer(nullptr)
    , m_pItemHexEchoLayer(nullptr)
    , m_pItemIntersectionLayer(nullptr)
    , m_pItemTmpLayer(nullptr)
    , m_pLineLayer(nullptr)
    , m_pEntityImageLayer(nullptr)
{
    ui->setupUi(this);
    this->resize(1920,1680);
    m_geoMap = new QGVMap(this);
    m_geoMap->setObjectName(QString::fromUtf8("geoMap"));

    m_snowflake.setHostId(SNOWFLAKE_VIEWER_HOST_ID);
    m_snowflake.setWorkerId(SNOWFLAKE_VIEWER_WORK_ID);

    if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_ENABLE_UPDATE_REDIS())
    {
        RedisManager::getInstance().loadConnectCfgToInit();
    }
    ui->horizontalLayout_3->addWidget(m_geoMap);

    m_pDataSrcListViewModel = new DataSrcListViewModel(this);
    m_pDataSrcItemDelegate = new DataSrcItemDelegate(m_pDataSrcListViewModel,this);
    ui->listView->setModel(m_pDataSrcListViewModel);
    ui->listView->setItemDelegate(m_pDataSrcItemDelegate);

    m_snowflake.setHostId(SNOWFLAKE_TEST_HOST_ID);
    m_snowflake.setWorkerId(SNOWFLAKE_TEST_WORK_ID);

    connect(ui->comboBox,static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged),this,&MainWindow::currentIndexChangedSlot);

    connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)),this,SLOT(currentIndexChangedSlot(QString)));


    setMouseTracking(true);
    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    ui->label_4->setText("速度*"+QString::number(m_speedcoeff));
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat(QString("%1%").arg(QString::number(0,'f',6)));//50.43


#if 0
    //RedisManager::getInstance().append_notify_subscribe_handler(std::bind(&MainWindow::redis_callback, this,std::placeholders::_1, std::placeholders::_2));
    //RedisManager::getInstance().subscribe("actoriconinfo");

    std::string actoriconinfo;
    RedisManager::getInstance().get("actoriconinfo",actoriconinfo);

    dealredisdata(actoriconinfo);
#endif
}



void MainWindow::redis_callback(const char *channel, const char *data)
{
    std::string RedisManager(data);
    dealredisdata(RedisManager);
}


#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

QJsonObject string_to_json_object(QString in)
{
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(in.toUtf8());

    // check validity of the document
    if(!doc.isNull())
    {
        if(doc.isObject())
        {
            obj = doc.object();
        }
        else
        {
            return obj;
        }
    }
    else
    {
        return obj;
    }
    return obj;
}

void MainWindow::dealredisdata(std::string &data)
{
    std::cout<<data<<std::endl;
    auto EquinoxServerjson = string_to_json_object(QString::fromStdString(data));
    auto actorinfo = EquinoxServerjson.value("actorinfo").toArray();
    for(auto actorinfoitem:actorinfo)
    {
        auto actors = actorinfoitem.toObject();
        uint64_t entitykey = actors.value("entitykey").toInt();
        auto idsn = actors.value("idsn").toString();
        auto actorSig = actors.value("actorSig").toString();
        auto entitySig = actors.value("entitySig").toString();
        auto kernelKey = actors.value("kernelKey").toString();
        auto displayName = actors.value("displayName").toString();

        if(m_actors.find(entitykey) == m_actors.end())
        {
            m_actors.insert(std::make_pair(entitykey,std::make_tuple(std::move(idsn), std::move(actorSig), std::move(entitySig), std::move(kernelKey),std::move(displayName))));
        }
    }
}

void MainWindow::updateEntityImage(ImageItem *pImage, const TYPE_ULID &uildsrc)
{
    auto ulidiconsitor = m_ulidicons.find(uildsrc);
    if(ulidiconsitor == m_ulidicons.end())
    {
        auto actitor = m_actors.find(uildsrc);
        if(actitor != m_actors.end())
        {
            QString &idsn = std::get<0>(actitor->second);
            QString &entitySig = std::get<2>(actitor->second);
            QString &displayName = std::get<4>(actitor->second);

            pImage->loadImage("./res/mapIcon/"+entitySig+".png");
            pImage->setEntityname(displayName);
        }
        else
        {
            std::string actoriconinfo;
            RedisManager::getInstance().get("actoriconinfo",actoriconinfo);
            dealredisdata(actoriconinfo);
            auto actitor2 = m_actors.find(uildsrc);
            if(actitor2 != m_actors.end())
            {
                QString &entitySig = std::get<2>(actitor2->second);
                QString &displayName = std::get<4>(actitor2->second);
                pImage->loadImage("./res/mapIcon/"+entitySig+".png");
                pImage->setEntityname(displayName);
            }
        }
    }
}


MainWindow::~MainWindow()
{
    for(auto item:m_layers)
    {
        auto name = item.first;
        auto layer = item.second;
        if(layer)
        {
            delete layer;
            layer = nullptr;
        }
    }
    m_layers.clear();

    if(m_pItemLayer)
    {
        delete m_pItemLayer;
    }

    if(m_pItemHexLayer)
    {
        delete m_pItemHexLayer;
    }

    if(m_pItemHexEchoLayer)
    {
        delete m_pItemHexEchoLayer;
    }



    if(m_pItemIntersectionLayer)
    {
        delete m_pItemIntersectionLayer;
    }

    if(m_pEntityImageLayer)
    {
        delete m_pEntityImageLayer;
    }

    if(m_pItemTmpLayer)
    {
        delete m_pItemTmpLayer;
    }

    if(m_pLineLayer)
    {
        delete m_pLineLayer;
    }

    for(auto item:m_widgets)
    {
        auto name = item.first;
        auto layer = item.second;
        if(layer)
        {
            delete layer;
            layer = nullptr;
        }
    }
    m_widgets.clear();
    delete ui;

    LocationHelper::releaseSource();
}

void MainWindow::init()
{
    /*
     * All "online" items required instance of QNetworkAccessManager.
     * Also it is recommended to use QNetworkCache for this manager to reduce
     * network load and speed-up download.
     */
    QDir("cacheDir").removeRecursively();
    mCache = new QNetworkDiskCache(this);
    mCache->setCacheDirectory("cacheDir");
    mManager = new QNetworkAccessManager(this);
    mManager->setCache(mCache);
    QGV::setNetworkManager(mManager);


    mManager2 = new QNetworkAccessManager(this);
    mManager2->setCache(mCache);
    QGV::setNetworkManager2(mManager2);

    initLayers();
    initWidgets();
    initgeomap();

    connect(m_geoMap, &QGVMap::mapMouseMove, this, &MainWindow::onMouseMove);


    connect(m_geoMap, &QGVMap::mapMousePress, this, &MainWindow::onMousePress);
    connect(m_geoMap, &QGVMap::mapMouseRelease, this, &MainWindow::onMouseRelease);
    connect(m_geoMap, &QGVMap::mapMouseDoubleClick, this, &MainWindow::onMouseDoubleClick);
    connect(m_geoMap, &QGVMap::mapKeyPressEvent, this, &MainWindow::onKeyPressEvent);
    connect(m_geoMap, &QGVMap::mapKeyReleaseEvent, this, &MainWindow::onKeyReleaseEvent);
    connect(m_geoMap, &QGVMap::mapwheelEvent, this, &MainWindow::onWheelEvent);


    /*
     * By default geomap started with zoom 1, which is usually a to high
     * resolution for view.
     * With this code we change camera to show "worldwide" area.
     */
    auto target = targetArea();//m_geoMap->getProjection()->boundaryGeoRect();
    m_geoMap->cameraTo(QGVCameraActions(m_geoMap).scaleTo(target));
    m_geoMap->geoView()->cameraScale(0.0025);
    m_pTimer = new QTimer(this);
    m_pTimer->start(TIMER_INTERVAL);
    connect(m_pTimer, &QTimer::timeout,this,&MainWindow::timeout_slot);


    GaeactorManager::getInstance().registDisplayCallback(std::bind(&MainWindow::displayHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    GaeactorManager::getInstance().registDisplayPosCallback(std::bind(&MainWindow::displayHexidxPosCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    GaeactorManager::getInstance().registIntersectionDisplayCallback(std::bind(&MainWindow::displayIntersectionHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    GaeactorManager::getInstance().registEchoWaveDisplayCallback(std::bind(&MainWindow::displayEchoWaveHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
    GaeactorManager::getInstance().registEventlistUpdateCallback(std::bind(&MainWindow::dealeventlist_update_callback, this, std::placeholders::_1, std::placeholders::_2));

    ui->spinBox->setMinimum(0);
    ui->spinBox->setMaximum(360);
    ui->spinBox->setValue(60);
    ui->spinBox_2->setMinimum(0);
    ui->spinBox_2->setMaximum(360);
    ui->spinBox_2->setValue(60);

    connect(ui->spinBox, SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlot(int)));
    connect(ui->spinBox_2, SIGNAL(valueChanged(int)),this,SLOT(valueChangedSlot(int)));

}

void MainWindow::currentIndexChangedSlot(const QString &val)
{
    if(mFooter)
    {
        mFooter->setText("");
    }
    for(auto item:m_layers)
    {
        auto name = item.first;
        auto layer = item.second;
        if(layer)
        {
            if(name == val)
            {
                layer->setVisible(true);
                if (mFooter)
                {
                    mFooter->setText(layer->getName() + ", " + layer->getDescription());
                }
                updateDownloadLevel(layer);
                updateZoomLevel(layer);
            }
            else
            {
                layer->setVisible(false);
            }
        }
    }
}

void MainWindow::onMouseMove(QPointF projPos)
{
    /*
     * Current projection position can be converted to geo-coordinates and
     * printed by corresponding functions
     */
    auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);

    LAT_LNG node{geoPos.latitude(),geoPos.longitude()};
    Mercator mMercator = latLng2WebMercator(node);
    mText->setText(QString("<b>%1, %2</b>")
                   .arg(geoPos.latToString())
                   .arg(geoPos.lonToString()));
    mText2->setText(QString("<b>Mercator:%1, %2</b>")
                   .arg(QString::number(mMercator.x,'f'))
                   .arg(QString::number(mMercator.y,'f')));



    switch (m_pQGVWidgetTools->getSelectType())
    {
    case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
    case QGVWidgetTools::E_SELECT_TYPE_PIE:
    case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
    {
        if(m_bSelectEnable)
        {
            m_distance =  geoMap()->getProjection()->geodesicMeters(m_projPos_start, projPos);

            mText3->setText("distance: "+QString::number(m_distance/1000.0f)+" km ");
            auto geoPos1 = m_geoMap->getProjection()->projToGeo(m_projPos_start);
            QGV::GeoRect geoRect = QGV::GeoRect(geoPos1, geoPos);
            switch (m_pQGVWidgetTools->getSelectType())
            {
            case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
            {
                if(m_pCurrentEllipse)
                {
                    m_pCurrentEllipse->updateRadius(m_distance,false);
//                    auto pts = m_pCurrentEllipse->geoRect();
//                    generateHexTmp(pts);
                }
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_PIE:
            {
                if(m_pCurrentPie)
                {
                    m_pCurrentPie->updateRadius(m_distance,false);
//                    auto pts = m_pCurrentPie->geoRect();
//                    generateHexTmp(pts);
                }
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
            {
                if(m_pCurrentRectangle)
                {
                    m_pCurrentRectangle->updateRect(geoRect,false);

//                    QVector<QGV::GeoPos> pts;
//                    pts.push_back(geoRect.topLeft());
//                    pts.push_back(geoRect.topRight());
//                    pts.push_back(geoRect.bottomRight());
//                    pts.push_back(geoRect.bottomLeft());
//                    pts.push_back(geoRect.topLeft());
//                    generateHexTmp(pts);
                }
            }
                break;
            default:break;
            }

            m_geoMap->geoView()->scene()->update();
        }

    }break;

    case QGVWidgetTools::E_SELECT_TYPE_LINE:
    {
        if(m_pCurrentLine && m_bSelectEnable)
        {
            auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
#ifdef GET_CUSTOM_LINE
            m_pCurrentLine->appendPoint(geoPos,true);
#else
            m_pCurrentLine->addPointMove(geoPos);
#endif
//            auto pts = m_pCurrentLine->geoRect();
//            pts.push_back(geoPos);
//            generateHexTmp(pts);
            m_geoMap->geoView()->scene()->update();
        }
    }
    break;
    case QGVWidgetTools::E_SELECT_TYPE_POLYGON:
    {
        if(m_pCurrentPolygon && m_bSelectEnable)
        {
            auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
            m_pCurrentPolygon->addPointMove(geoPos);
//            auto pts = m_pCurrentPolygon->geoRect();
//            pts.push_back(geoPos);
//            generateHexTmp(pts);
//            m_geoMap->geoView()->scene()->update();
        }
    }break;

    case QGVWidgetTools::E_SELECT_TYPE_MOVE_TRACKING_LINE:
    {
        if(m_pMoveTrackingItem && m_bSelectEnable)
        {
            auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
            m_pMoveTrackingItem->addPointMove(geoPos);
//            auto pts = m_pCurrentLine->geoRect();
//            pts.push_back(geoPos);
//            generateHexTmp(pts);
            m_geoMap->geoView()->scene()->update();
        }
    }break;
    default:
    {
        m_bSelectEnable = false;
    }break;
    }
}


void MainWindow::onMousePress(QPointF projPos)
{
    QColor color(0,255,0,48);
    switch (m_pQGVWidgetTools->getSelectType())
    {
    case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
    case QGVWidgetTools::E_SELECT_TYPE_PIE:
    case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
    {
        if(m_bSelectEnable)
        {
            m_bSelectEnable = false;
        }
        else
        {
            m_bSelectEnable = true;

            switch (m_pQGVWidgetTools->getSelectType())
            {
            case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
            {
                auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
                m_pCurrentEllipse = new EllipseItem(m_snowflake.GetId(),geoPos,KM_PH_TO_M_S(800),10, color);
                m_pCurrentEllipse->setTransfer(false);
                m_pCurrentEllipse->setShowTrackingLine(m_showTrackingLine);
                QString title = QString("预警机_%1\nlon:%2\nlat:%3\n").arg(QString::number(m_pCurrentEllipse->ulid())).arg(geoPos.lonToString()).arg(geoPos.latToString());
                m_ulidicons.insert(std::make_pair(m_pCurrentEllipse->ulid(),std::make_tuple("./res/AWACS_1.png",title,E_ENTITY_DISAPPEAR_TYPE_LOOP)));
                m_pItemLayer->addItem(m_pCurrentEllipse);
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_PIE:
            {
                auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);

                int startAngle = ui->spinBox->value();
                int spanAngle = ui->spinBox_2->value();
                m_pCurrentPie = new PieItem(m_snowflake.GetId(),geoPos,KM_PH_TO_M_S(50*4),10, startAngle, spanAngle, color);
                m_pCurrentPie->setTransfer(false);
                m_pCurrentPie->setShowTrackingLine(m_showTrackingLine);
                QString title = QString("雷达_%1\nlon:%2\nlat:%3\n").arg(QString::number(m_pCurrentPie->ulid())).arg(geoPos.lonToString()).arg(geoPos.latToString());
                m_ulidicons.insert(std::make_pair(m_pCurrentPie->ulid(),std::make_tuple("./res/radar_3.png",title,E_ENTITY_DISAPPEAR_TYPE_LOOP)));
                m_pItemLayer->addItem(m_pCurrentPie);
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
            {                
                int radius = 10;
                QGV::GeoRect geoRect = geoMap()->getProjection()->projToGeo({ projPos, projPos + QPointF(radius, radius) });
                m_pCurrentRectangle = new RectangleItem(m_snowflake.GetId(),geoRect, QGV::ItemFlag::Clickable,KM_PH_TO_M_S(4000), color);
                m_pCurrentRectangle->setTransfer(false);
                m_pCurrentRectangle->setShowTrackingLine(m_showTrackingLine);
//                QString title = QString("卫星_%1\nlon:%1\nlat:%1\n").arg(QString::number(m_pCurrentEllipse->ulid())).arg(geoRect..lonToString()).arg(geoPos.latToString());
                m_ulidicons.insert(std::make_pair(m_pCurrentRectangle->ulid(),std::make_tuple("./res/satellite_2.png","卫星_"+QString::number(m_pCurrentRectangle->ulid()),E_ENTITY_DISAPPEAR_TYPE_LOOP)));

                m_pItemLayer->addItem(m_pCurrentRectangle);
            }
                break;
            default:break;
            }
            m_projPos_start = projPos;
        }
    }break;
    case QGVWidgetTools::E_SELECT_TYPE_LINE:
    {
        auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
        if(!m_bSelectEnable)
        {
            m_bSelectEnable = true;
            m_pCurrentLine = new LineItem(m_snowflake.GetId(),geoPos,20, QColor(0,0,255,255),false);
            m_pCurrentLine->setTransfer(false);
            QString title = QString("无人机_%1\nlon:%2\nlat:%3\n").arg(QString::number(m_pCurrentLine->ulid())).arg(geoPos.lonToString()).arg(geoPos.latToString());
            m_ulidicons.insert(std::make_pair(m_pCurrentLine->ulid(),std::make_tuple("./res/uav.png",title,E_ENTITY_DISAPPEAR_TYPE_LOOP)));
            m_pCurrentLine->updateSpeedCoeff(m_speedcoeff);
            m_TrackingLineItems.push_back(m_pCurrentLine);
            m_pCurrentLine->setShowTrackingLine(m_showTrackingLine);
            m_pLineLayer->addItem(m_pCurrentLine);
            m_projPos_start = projPos;
        }
        else
        {
            if(m_pCurrentLine)
            {
                m_pCurrentLine->appendPoint(geoPos,true);
                m_geoMap->geoView()->scene()->update();
            }
        }
    }
    break;
    case QGVWidgetTools::E_SELECT_TYPE_POLYGON:
    {
        auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
        if(!m_bSelectEnable)
        {
            m_bSelectEnable = true;
            QVector<QGV::GeoPos> geoPosList;
            geoPosList.push_back(geoPos);

            m_pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color);
            m_pCurrentPolygon->setTransfer(false);
            m_pCurrentPolygon->setShowTrackingLine(m_showTrackingLine);
            QString title = QString("数据链_%1\nlon:%2\nlat:%3\n").arg(QString::number(m_pCurrentPolygon->ulid())).arg(geoPos.lonToString()).arg(geoPos.latToString());
            m_ulidicons.insert(std::make_pair(m_pCurrentPolygon->ulid(),std::make_tuple("./res/data_link.png",title,E_ENTITY_DISAPPEAR_TYPE_LOOP)));
            m_pItemLayer->addItem(m_pCurrentPolygon);
            m_projPos_start = projPos;
        }
        else
        {
            if(m_pCurrentPolygon)
            {
                m_pCurrentPolygon->appendPoint(geoPos);
                m_geoMap->geoView()->scene()->update();
            }
        }
    }break;
    case QGVWidgetTools::E_SELECT_TYPE_MOVE_TRACKING_LINE:
    {
        int iCount = m_pItemLayer->countItems();
        for(int index = 0; index < iCount; index++)
        {
            QGVItem* pQGVItem = m_pItemLayer->getItem(index);
            if(pQGVItem->isSelected())
            {
                if(!m_bSelectEnable)
                {
                    m_bSelectEnable = true;
                    m_pMoveTrackingItem = dynamic_cast<BaseItem*>(pQGVItem);
                    m_pMoveTrackingItem->setShowTrackingLine(m_showTrackingLine);
                }
                else
                {
                    if(m_pMoveTrackingItem)
                    {
                        auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
                        m_pMoveTrackingItem->appendPoint(geoPos);
                        m_geoMap->geoView()->scene()->update();
                    }
                }
            }
        }
    }
    break;
    default:
    {
        m_bSelectEnable = false;
    }break;
    }
}

void MainWindow::onMouseRelease(QPointF projPos)
{
    auto geoPos = m_geoMap->getProjection()->projToGeo(projPos);
    switch (m_pQGVWidgetTools->getSelectType())
    {
    case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
    case QGVWidgetTools::E_SELECT_TYPE_PIE:
    case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
    {
        if(!m_bSelectEnable)
        {
            auto geoPos1 = m_geoMap->getProjection()->projToGeo(m_projPos_start);
            QGV::GeoRect geoRect = QGV::GeoRect(geoPos1, geoPos);
            m_distance =  geoMap()->getProjection()->geodesicMeters(m_projPos_start, projPos);
            switch (m_pQGVWidgetTools->getSelectType())
            {
            case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
            {
                if(m_pCurrentEllipse)
                {
                    m_pCurrentEllipse->updateRadius(m_distance,true);
                    generateHex(m_pCurrentEllipse->ulid(), m_pCurrentEllipse->center(), m_pCurrentEllipse->geoRect(),m_pCurrentEllipse);
                }
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_PIE:
            {
                if(m_pCurrentPie)
                {
                    m_pCurrentPie->updateRadius(m_distance,true);
                    generateHex(m_pCurrentPie->ulid(), m_pCurrentPie->center(), m_pCurrentPie->geoRect(),m_pCurrentPie);
                }
            }
                break;
            case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
            {
                if(m_pCurrentRectangle)
                {
                    m_pCurrentRectangle->updateRect(geoRect,true);
                    generateHex(m_pCurrentRectangle->ulid(), m_pCurrentRectangle->center(), m_pCurrentRectangle->geoRect(),m_pCurrentRectangle);
                }
            }
                break;
            default:break;
            }
            m_geoMap->geoView()->scene()->update();

            switch (m_pQGVWidgetTools->getSelectType())
            {
            case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
            {
                if(m_pCurrentEllipse)
                {
                    m_pCurrentEllipse->setTransfer(m_bTransfer);
                }
                m_pCurrentEllipse = nullptr;
            }
            break;
            case QGVWidgetTools::E_SELECT_TYPE_PIE:
            {
                if(m_pCurrentPie)
                {
                    m_pCurrentPie->setTransfer(m_bTransfer);
                }
                m_pCurrentPie = nullptr;
            }
            break;
            case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
            {
                if(m_pCurrentRectangle)
                {
                    m_pCurrentRectangle->setTransfer(m_bTransfer);
                }
                m_pCurrentRectangle = nullptr;
            }
            break;
            default:break;
            }


        }
    }break;
    case QGVWidgetTools::E_SELECT_TYPE_LINE:
    {
        if(m_bdoubleclicked)
        {
            m_bdoubleclicked = false;
            m_bSelectEnable = false;
            if(m_pCurrentLine)
            {
                m_pCurrentLine->appendFinished();
                m_pCurrentLine->updateSpeedCoeff(m_speedcoeff);
                m_TrackingLineItems.push_back(m_pCurrentLine);
            }
            m_geoMap->geoView()->scene()->update();

            m_pCurrentLine->setTransfertmp(m_bTransfer);

            m_pCurrentLine = nullptr;
        }
    }break;
    case QGVWidgetTools::E_SELECT_TYPE_POLYGON:
    {
        if(m_bdoubleclicked)
        {
            m_bdoubleclicked = false;
            m_bSelectEnable = false;
            if(m_pCurrentPolygon)
            {
                m_pCurrentPolygon->appendFinished();
                m_pCurrentPolygon->setBFill(true);
                generateHex(m_pCurrentPolygon->ulid(), m_pCurrentPolygon->center(), m_pCurrentPolygon->geoRect(),m_pCurrentPolygon);
            }
            m_geoMap->geoView()->scene()->update();
            m_pCurrentPolygon->setTransfertmp(m_bTransfer);
            m_pCurrentPolygon = nullptr;
        }

    }break;
    case QGVWidgetTools::E_SELECT_TYPE_MOVE_TRACKING_LINE:
    {
        if(m_bdoubleclicked)
        {
            m_bdoubleclicked = false;
            m_bSelectEnable = false;
            if(m_pMoveTrackingItem)
            {
                m_pMoveTrackingItem->appendFinished();
                m_pMoveTrackingItem->updateSpeedCoeff(m_speedcoeff);
                m_TrackingItems.push_back(m_pMoveTrackingItem);


                m_geoMap->geoView()->scene()->update();

                EllipseItem*_pCurrentEllipse = dynamic_cast<EllipseItem*>(m_pMoveTrackingItem);
                if(_pCurrentEllipse)
                {
                    _pCurrentEllipse->setTransfer(m_bTransfer);
                }

                PieItem*_pPieItem = dynamic_cast<PieItem*>(m_pMoveTrackingItem);
                if(_pPieItem)
                {
                    _pPieItem->setTransfer(m_bTransfer);
                }

                RectangleItem*_pRectangleItem= dynamic_cast<RectangleItem*>(m_pMoveTrackingItem);
                if(_pRectangleItem)
                {
                    _pRectangleItem->setTransfer(m_bTransfer);
                }


                PolygonItem*_pPolygonItem= dynamic_cast<PolygonItem*>(m_pMoveTrackingItem);
                if(_pPolygonItem)
                {
                    _pPolygonItem->setTransfertmp(m_bTransfer);
                }

            }
            m_geoMap->geoView()->scene()->update();

            EllipseItem*_pCurrentEllipse = dynamic_cast<EllipseItem*>(m_pMoveTrackingItem);
            if(_pCurrentEllipse)
            {
                _pCurrentEllipse->setTransfer(m_bTransfer);
            }

            PieItem*_pPieItem = dynamic_cast<PieItem*>(m_pMoveTrackingItem);
            if(_pPieItem)
            {
                _pPieItem->setTransfer(m_bTransfer);
            }

            RectangleItem*_pRectangleItem= dynamic_cast<RectangleItem*>(m_pMoveTrackingItem);
            if(_pRectangleItem)
            {
                _pRectangleItem->setTransfer(m_bTransfer);
            }


            PolygonItem*_pPolygonItem= dynamic_cast<PolygonItem*>(m_pMoveTrackingItem);
            if(_pPolygonItem)
            {
                _pPolygonItem->setTransfertmp(m_bTransfer);
            }



            m_pMoveTrackingItem = nullptr;
        }
    }break;

    default:
    {
        m_bSelectEnable = false;
    }break;
    }
}

void MainWindow::generateHex(TYPE_ULID ulid, const QGV::GeoPos& geopos, const QVector<QGV::GeoPos> & pts, BaseItem *pMoveTrackingItem)
{
    auto wsgeopostmp = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.latitude(), geopos.longitude());
    LAT_LNG pos{wsgeopostmp.lat, wsgeopostmp.lng};
    std::vector<LatLng> data;
    std::vector<transdata_param_seq_polygon> _polygon;
    for(auto item:pts)
    {
        auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(item.latitude(), item.longitude());
        transdata_param_seq_polygon _polygon_pt{(LON_LAT_TYPE)(wsgeo.lat * LON_LAT_ACCURACY), (LON_LAT_TYPE)(wsgeo.lng * LON_LAT_ACCURACY), 0};
        _polygon.emplace_back(std::move(_polygon_pt));
        LatLng latlng{LocationHelper::degreeToRadian(wsgeo.lat),LocationHelper::degreeToRadian(wsgeo.lng)};
        data.emplace_back(std::move(latlng));
    }
    HEXIDX_ARRAY indexlist;
    int res = LocationHelper::getPolygonRes(data);
    LocationHelper::getPolygonResulutionIndex(indexlist, data,res);

    HEXIDX_HGT_ARRAY hexidxslist;
    hexidxslist.resize(indexlist.size());
    for(int i = 0;i < hexidxslist.size(); i++)
    {
        hexidxslist[i].PARAM_seq_hexidx_element = indexlist.at(i);
    }
    GaeactorManager::getInstance().dealHexidex(ulid,pos, hexidxslist, _polygon, pMoveTrackingItem->isTrackingable() ? SLIENT_TIME_GAP : 0);
}

void MainWindow::generateHexTmp(const QVector<QGV::GeoPos> & pts)
{
//    m_pItemTmpLayer->deleteItems();
//    std::vector<LAT_LNG> data;
//    for(auto item:pts)
//    {
//        data.push_back(LAT_LNG{LocationHelper::degreeToRadian(item.latitude()), LocationHelper::degreeToRadian(item.longitude())});
//    }
//    auto boundarys = m_LocationHelper.getPolygonBoundary(data);

//    for(auto boundary : boundarys)
//    {
//        QVector<QGV::GeoPos> geoPosList;
//        for (int v = 0; v < boundary.numVerts; v++)
//        {
//            geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
//        }
//        PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList, QColor(0,0,255,168));
//        m_pItemTmpLayer->addItem(_pCurrentPolygon);
    //    }
}


void MainWindow::displayHexidxPosCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const transdata_entityposinfo &eninfo, E_DISPLAY_MODE eDdisplayMode)
{
    switch(eDdisplayMode)
    {
    case E_DISPLAY_MODE_ENTITY:
    {
        if(eninfo.PARAM_pos_hexidx == 0)
        {
            auto itor = m_imageEntity.find(uildsrc);
            if(itor != m_imageEntity.end())
            {
                QList<std::tuple<ImageItem *, LineItem*>>&polygonlist = itor->second;
                for(auto item : polygonlist)
                {
                    ImageItem * pImage = std::get<0>(item);
                    LineItem * pLine = std::get<1>(item);
                    m_pEntityImageLayer->removeItem(pImage);
                    m_pLineLayer->removeItem(pLine);
                    delete pImage;
                    delete pLine;
                }
                polygonlist.clear();
                m_imageEntity.erase(itor);
            }
        }
        else
        {
            HEXIDX_ARRAY indexlist;
            indexlist.push_back(eninfo.PARAM_pos_hexidx);
            std::vector<std::tuple<H3INDEX,CellBoundary>> boundarys;
            LocationHelper::getIndexBoundary(boundarys, indexlist);

            H3INDEX h3index = 0;


            LAT_LNG origincorretgeo{(double)eninfo.PARAM_latitude/LON_LAT_ACCURACY, (double)eninfo.PARAM_longitude/LON_LAT_ACCURACY};



            auto itor = m_imageEntity.find(uildsrc);
            if(itor != m_imageEntity.end())
            {
                QList<std::tuple<ImageItem *, LineItem*>>&pCurrentPolygon = itor->second;
                if(pCurrentPolygon.size() == 1)
                {
                    ImageItem * pImage = std::get<0>(pCurrentPolygon.at(0));
                    LineItem * pLine = std::get<1>(pCurrentPolygon.at(0));

                    pImage->setTransfer(m_bTransfer);
                    pLine->setTransfer(m_bTransfer);
                    pImage->updatePts(QGV::GeoPos(origincorretgeo.lat, origincorretgeo.lng),eninfo.PARAM_pitch, eninfo.PARAM_roll, eninfo.PARAM_yaw);
                    pLine->appendPoint(QGV::GeoPos(origincorretgeo.lat, origincorretgeo.lng),false);

                    updateEntityImage(pImage,uildsrc);
                }
            }
            else
            {
                QList<std::tuple<ImageItem *, LineItem*>> polygonlist;
                ImageItem *_pCurrentImageItem = new ImageItem(m_snowflake.GetId(),QGV::GeoPos(origincorretgeo.lat, origincorretgeo.lng), QColor(255,255,0,64),false);

                auto ulidiconsitor = m_ulidicons.find(uildsrc);
                if(ulidiconsitor != m_ulidicons.end())
                {
                    _pCurrentImageItem->loadImage(std::get<0>(ulidiconsitor->second));
                    _pCurrentImageItem->setEntityname(std::get<1>(ulidiconsitor->second));
                    _pCurrentImageItem->setEntityulid(qMakePair(ulidiconsitor->first,ulidiconsitor->first));
                }
                updateEntityImage(_pCurrentImageItem,uildsrc);


                _pCurrentImageItem->setH3index(h3index);
                LineItem *pLine = new LineItem(m_snowflake.GetId(),QGV::GeoPos(origincorretgeo.lat, origincorretgeo.lng), 1,QColor(255,255,0,64),false);
                pLine->popfrontpt();
                pLine->setShowTrackingLine(m_showTrackingLine);
                polygonlist.push_back(std::make_tuple(_pCurrentImageItem,pLine));
                m_imageEntity.insert(std::make_pair(uildsrc, std::move(polygonlist)));
                m_pEntityImageLayer->addItem(_pCurrentImageItem);
                m_pLineLayer->addItem(pLine);

                m_geoMap->geoView()->scene()->update();
                if(ulidiconsitor != m_ulidicons.end())
                {
                    _pCurrentImageItem->setTransfer(m_bTransfer);
                    pLine->setTransfertmp(m_bTransfer);
                }
                else
                {
                    _pCurrentImageItem->setTransfer(m_bTransfer);
                    pLine->setTransfer(m_bTransfer);
                }
            }
        }
        m_geoMap->geoView()->scene()->update();

    }break;
    default:
        break;
    }
}

void MainWindow::displayHexidxCallback(const TYPE_ULID &uild,const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY &hexidxslistTMP, const POLYGON_LIST& polygondatalist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDisplayMode)
{
    return;
    QPair<TYPE_ULID,TYPE_ULID> uildsrc = qMakePair(uild,uilddst);

    HEXIDX_ARRAY hexidxslist;
    hexidxslist.resize(hexidxslistTMP.size());
    for(int index= 0; index < hexidxslistTMP.size(); index++)
    {
        hexidxslist[index] = hexidxslistTMP.at(index).PARAM_seq_hexidx_element;
    }

    EVENT_KEY_TYPE event_key_tuple = EVENT_KEY_TYPE{uild,uilddst, sensorinfo.PARAM_source_sensingmediaid};
//    qDebug()<<uildsrc;
    switch(eDisplayMode)
    {
    case E_DISPLAY_MODE_ECHO:
    {
#ifdef SHOW_HEXAGON
        if(hexidxslist.empty())
        {
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                for(auto item : polygonlist)
                {
                    m_pItemHexLayer->removeItem(item);
                    delete item;
                }
                polygonlist.clear();
                m_polygons.erase(itor);
            }
        }
        else
        {
            std::vector<std::tuple<H3INDEX,CellBoundary>> boundarys;
            LocationHelper::getIndexBoundary(boundarys, hexidxslist);

            QColor color(0,0,255,168);
            if(eDisplayMode == E_DISPLAY_MODE_ECHO)
            {
                color=QColor(255,0,255,168);
            }
            auto itor = m_echowavepolygons.find(event_key_tuple);
            if(itor != m_echowavepolygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                if(hexidxslist.empty())
                {
                    QList<PolygonItem *>::iterator itor = polygonlist.begin();
                    while(itor != polygonlist.end())
                    {
                        PolygonItem * pPolygonItem  = *itor;
                        m_pItemHexLayer->removeItem(pPolygonItem);
                        pPolygonItem->deleteLater();
                        itor = polygonlist.erase(itor);
                        continue;
                    }
                    qDebug()<<"clear echo wave item all PolygonItem";
                }
                else
                {
                    QVector<H3INDEX> _existindex;
                    QList<PolygonItem *>::iterator itor = polygonlist.begin();
                    while(itor != polygonlist.end())
                    {
                        PolygonItem * pPolygonItem  = *itor;
                        H3INDEX oldh3index = pPolygonItem->h3index();
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(),[&](const uint64_t & val){
                            return oldh3index==val;
                        });
                        if(h3itor == hexidxslist.end())
                        {
                            m_pItemHexLayer->removeItem(pPolygonItem);
                            itor = polygonlist.erase(itor);
                            continue;
                        }
                        else
                        {
                            _existindex.push_back(oldh3index);
                        }
                        itor++;
                    }

                    for(auto boundaryitem : boundarys)
                    {
                        H3INDEX h3index = std::get<0>(boundaryitem);
                        if(!_existindex.contains(h3index))
                        {
                            CellBoundary boundary = std::get<1>(boundaryitem);
                            QVector<QGV::GeoPos> geoPosList;
                            for (int v = 0; v < boundary.numVerts; v++)
                            {
                                geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
                            }
                            if(!geoPosList.empty())
                            {
                                PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList, KM_PH_TO_M_S(50*4), color,false);
                                _pCurrentPolygon->setEntityulid(uildsrc);
                                _pCurrentPolygon->setH3index(h3index);
                                m_pItemHexLayer->addItem(_pCurrentPolygon);
                                polygonlist.push_back(_pCurrentPolygon);
                                m_geoMap->geoView()->scene()->update();
                                _pCurrentPolygon->setTransfers(m_bTransfer);
                            }
                        }
                    }
                }
            }
            else
            {
                QList<PolygonItem *> polygonlist;
                for(auto boundaryitem : boundarys)
                {
                    H3INDEX h3index = std::get<0>(boundaryitem);
                    CellBoundary boundary = std::get<1>(boundaryitem);
                    QVector<QGV::GeoPos> geoPosList;
                    for (int v = 0; v < boundary.numVerts; v++)
                    {
                        geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
                    }
                    if(!geoPosList.empty())
                    {
                        PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList, KM_PH_TO_M_S(50*4), color,false);
                        _pCurrentPolygon->setEntityulid(uildsrc);
                        _pCurrentPolygon->setH3index(h3index);
                        m_pItemHexLayer->addItem(_pCurrentPolygon);
                        polygonlist.push_back(_pCurrentPolygon);
                        m_geoMap->geoView()->scene()->update();
                        _pCurrentPolygon->setTransfers(m_bTransfer);
                    }
                }
                qDebug()<<"create echo wave item";

                m_echowavepolygons.insert(std::make_pair(event_key_tuple, std::move(polygonlist)));
            }
        }
        m_geoMap->geoView()->scene()->update();
#else
#ifdef SHOW_CONVEXHULL_
        if(hexidxslist.empty())
        {
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                for(auto item : polygonlist)
                {
                    m_pItemHexLayer->removeItem(item);
                    delete item;
                }
                polygonlist.clear();
                m_polygons.erase(itor);
            }
        }
        else
        {
            QVector<LAT_LNG> geoposlist = LocationHelper::getHexidxsConvexHull(hexidxslist);

            QVector<QGV::GeoPos> geoPosList;
            for(auto geoposlistitem : geoposlist)
            {
                geoPosList.push_back(QGV::GeoPos(geoposlistitem.latitude(),geoposlistitem.longitude()));
            }

            QColor color(0,0,255,10);
            auto itor = m_echowavegeopolygons.find(qMakePair(uildsrc,uilddst));
            if(itor != m_echowavegeopolygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                {
                    if(polygonlist.size() == 1)
                    {
                        polygonlist.at(0)->updatePts(geoPosList,false);
                    }
                    else
                    {
                        if(!geoPosList.empty())
                        {
                            PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList, KM_PH_TO_M_S(50*4), color,false);
                            _pCurrentPolygon->setEntityulid(uildsrc);
                            m_pItemHexLayer->addItem(_pCurrentPolygon);
                            polygonlist.push_back(_pCurrentPolygon);
                            m_geoMap->geoView()->scene()->update();
                            _pCurrentPolygon->setTransfers(m_bTransfer);
                        }
                    }
                }
            }
            else
            {
                if(!geoPosList.empty())
                {
                    QList<PolygonItem *> polygonlist;
                    PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList, KM_PH_TO_M_S(50*4), color,false);
                    _pCurrentPolygon->setEntityulid(uildsrc);
                    m_pItemHexLayer->addItem(_pCurrentPolygon);
                    polygonlist.push_back(_pCurrentPolygon);
                    m_echowavegeopolygons.insert(std::make_pair(qMakePair(uildsrc,uilddst), std::move(polygonlist)));
                    m_geoMap->geoView()->scene()->update();
                    _pCurrentPolygon->setTransfers(m_bTransfer);
                }
            }
        }
        m_geoMap->geoView()->scene()->update();
#endif
#endif
    }
    break;
    case E_DISPLAY_MODE_WAVE:
    {
#ifdef SHOW_HEXAGON
        if (sensorinfo.PARAM_wave_usage == 0x05 ||
            sensorinfo.PARAM_wave_usage == 255)
        {
            return;
        }
        if(hexidxslist.empty())
        {
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                for(auto item : polygonlist)
                {
                    m_pItemHexLayer->removeItem(item);
                    delete item;
                }
                polygonlist.clear();
                m_polygons.erase(itor);
            }
        }
        else
        {
            std::vector<std::tuple<H3INDEX,CellBoundary>> boundarys;
            LocationHelper::getIndexBoundary(boundarys, hexidxslist);
            QColor color(0,0,255,168);
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                if(hexidxslist.empty())
                {
                    QList<PolygonItem *>::iterator itor = polygonlist.begin();
                    while(itor != polygonlist.end())
                    {
                        PolygonItem * pPolygonItem  = *itor;
                        m_pItemHexLayer->removeItem(pPolygonItem);
                        pPolygonItem->deleteLater();
                        itor = polygonlist.erase(itor);
                        continue;
                    }
                }
                else
                {
                    QVector<H3INDEX> _existindex;
                    QList<PolygonItem *>::iterator itor = polygonlist.begin();
                    while(itor != polygonlist.end())
                    {
                        PolygonItem * pPolygonItem  = *itor;
                        H3INDEX oldh3index = pPolygonItem->h3index();
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(),[&](const uint64_t & val){
                            return oldh3index==val;
                        });
                        if(h3itor == hexidxslist.end())
                        {
                            m_pItemHexLayer->removeItem(pPolygonItem);
                            pPolygonItem->deleteLater();
                            itor = polygonlist.erase(itor);
                            continue;
                        }
                        else
                        {
                            _existindex.push_back(oldh3index);
                        }
                        itor++;
                    }

                    for(auto boundaryitem : boundarys)
                    {
                        H3INDEX h3index = std::get<0>(boundaryitem);
                        if(!_existindex.contains(h3index))
                        {
                            CellBoundary boundary = std::get<1>(boundaryitem);
                            QVector<QGV::GeoPos> geoPosList;
                            for (int v = 0; v < boundary.numVerts; v++)
                            {
                                geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
                            }
                            if(!geoPosList.empty())
                            {
                                PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
                                _pCurrentPolygon->setEntityulid(uildsrc);
                                _pCurrentPolygon->setH3index(h3index);
                                m_pItemHexLayer->addItem(_pCurrentPolygon);
                                polygonlist.push_back(_pCurrentPolygon);
                                m_geoMap->geoView()->scene()->update();
                                _pCurrentPolygon->setTransfers(m_bTransfer);
                            }
                        }
                    }
                }
            }
            else
            {
                QList<PolygonItem *> polygonlist;
                for(auto boundaryitem : boundarys)
                {
                    H3INDEX h3index = std::get<0>(boundaryitem);
                    CellBoundary boundary = std::get<1>(boundaryitem);
                    QVector<QGV::GeoPos> geoPosList;
                    for (int v = 0; v < boundary.numVerts; v++)
                    {
                        geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
                    }
                    if(!geoPosList.empty())
                    {
                        PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
                        _pCurrentPolygon->setEntityulid(uildsrc);
                        _pCurrentPolygon->setH3index(h3index);
                        m_pItemHexLayer->addItem(_pCurrentPolygon);
                        polygonlist.push_back(_pCurrentPolygon);
                        m_geoMap->geoView()->scene()->update();
                        _pCurrentPolygon->setTransfers(m_bTransfer);
                    }
                }
                m_polygons.insert(std::make_pair(uildsrc, std::move(polygonlist)));
            }
        }
        m_geoMap->geoView()->scene()->update();
#else
#ifdef SHOW_CONVEXHULL
        if(hexidxslist.empty())
        {
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                for(auto item : polygonlist)
                {
                    m_pItemHexLayer->removeItem(item);
                    delete item;
                }
                polygonlist.clear();
                m_polygons.erase(itor);
            }
        }
        else
        {
#if 0
            QVector<LAT_LNG> geoposlist = LocationHelper::getHexidxsConvexHull(hexidxslist);
            QVector<QGV::GeoPos> geoPosList;
            for(auto geoposlistitem : geoposlist)
            {
                geoPosList.push_back(QGV::GeoPos(geoposlistitem.latitude(),geoposlistitem.longitude()));
            }
#else
            QVector<QGV::GeoPos> geoPosList;
            for(auto geoposlistitem : polygondatalist)
            {
                geoPosList.push_back(QGV::GeoPos((double)(geoposlistitem.PARAM_latitude)/LON_LAT_ACCURACY, (double)(geoposlistitem.PARAM_longitude)/LON_LAT_ACCURACY));
            }
#endif
            QColor color(255, 0, 0, 64);
            if (sensorinfo.PARAM_wave_usage == 0x05)
            {
                color = QColor(255, 255, 0, 64);
            }
            auto itor = m_polygons.find(uildsrc);
            if(itor != m_polygons.end())
            {
                QList<PolygonItem *>&polygonlist = itor->second;
                {
                    if(polygonlist.size() == 1)
                    {
                        polygonlist.at(0)->updatePts(geoPosList);
                    }
                    else
                    {
                        if(!geoPosList.empty())
                        {
                            PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
                            _pCurrentPolygon->setEntityulid(uildsrc);
                            m_pItemHexLayer->addItem(_pCurrentPolygon);
                            polygonlist.push_back(_pCurrentPolygon);
                        }
                    }
                }
            }
            else
            {
                if(!geoPosList.empty())
                {
                    QList<PolygonItem *> polygonlist;
                    PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
                    _pCurrentPolygon->setEntityulid(uildsrc);
                    m_pItemHexLayer->addItem(_pCurrentPolygon);
                    polygonlist.push_back(_pCurrentPolygon);
                    m_polygons.insert(std::make_pair(uildsrc, std::move(polygonlist)));

                    m_geoMap->geoView()->scene()->update();
                    _pCurrentPolygon->setTransfers(m_bTransfer);
                }
            }
        }
        m_geoMap->geoView()->scene()->update();
#endif
#endif        
    }break;
    default:break;
    }
}

void MainWindow::displayIntersectionHexidxCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> > &hexidxslistinfo, E_DISPLAY_MODE eDdisplayMode)
{
    return;
    HEXIDX_ARRAY hexindexlist;
    for(auto item:hexidxslistinfo)
    {
        hexindexlist.push_back(std::get<2>(item));
    }

    std::vector<std::tuple<H3INDEX,CellBoundary>> boundarys;
    LocationHelper::getIndexBoundary(boundarys,hexindexlist);

    auto itor = m_polygonsIntersection.find(uildsrc);
    if(itor != m_polygonsIntersection.end())
    {
        QList<PolygonItem *>&polygonlist = itor->second;
        QList<PolygonItem *>::iterator itor = polygonlist.begin();
        while(itor != polygonlist.end())
        {
            PolygonItem * pPolygonItem  = *itor;
            QPair<TYPE_ULID, TYPE_ULID> ppPolygonItemUlidPair = pPolygonItem->getIntersectionUlidPair();

            auto itor2 = std::find_if(hexidxslistinfo.begin(),
                                      hexidxslistinfo.end(),
                                      [&](const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> >::value_type &vt){
                                          return ((std::get<0>(vt) == ppPolygonItemUlidPair.first) && \
                                                  (std::get<1>(vt) == ppPolygonItemUlidPair.second));
                                      });
            if(itor2 == hexidxslistinfo.end())
            {
                m_pItemIntersectionLayer->removeItem(pPolygonItem);
                pPolygonItem->deleteLater();
                itor = polygonlist.erase(itor);
                continue;
            }
            itor++;
        }

        for(auto boundaryitem : boundarys)
        {
            H3INDEX h3index = std::get<0>(boundaryitem);

            auto itor2 = std::find_if(hexidxslistinfo.begin(),
                                      hexidxslistinfo.end(),
                                      [&](const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> >::value_type &vt){
                                          return std::get<2>(vt) == h3index;
                                      });
            QPair<TYPE_ULID, TYPE_ULID> newIntersectionUlidPair;
            if(itor2 != hexidxslistinfo.end())
            {
                newIntersectionUlidPair = qMakePair(std::get<0>(*itor2),std::get<1>(*itor2));
            }

            CellBoundary boundary = std::get<1>(boundaryitem);
            QVector<QGV::GeoPos> geoPosList;
            for (int v = 0; v < boundary.numVerts; v++)
            {
                geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
            }

            auto itor3 = std::find_if(polygonlist.begin(),
                                      polygonlist.end(),
                                      [&](const PolygonItem * vt){

                                          QPair<TYPE_ULID, TYPE_ULID> ppPolygonItemUlidPair = vt->getIntersectionUlidPair();

                                          return ((ppPolygonItemUlidPair.first == newIntersectionUlidPair.first) && \
                                                  (ppPolygonItemUlidPair.second == newIntersectionUlidPair.second));
                                      });
            if(itor3 != polygonlist.end())
            {
                PolygonItem * _pCurrentPolygon  = *itor3;
                _pCurrentPolygon->setH3index(h3index);
                _pCurrentPolygon->updatePts(geoPosList);
            }
            else
            {
                if(!geoPosList.empty())
                {
                    PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), QColor(0,255,255,168),false);
                    _pCurrentPolygon->setH3index(h3index);
                    _pCurrentPolygon->setIntersectionUlidPair(newIntersectionUlidPair);
                    m_pItemIntersectionLayer->addItem(_pCurrentPolygon);
                    polygonlist.push_back(_pCurrentPolygon);
                }
            }
        }
    }
    else
    {
        QList<PolygonItem *> polygonlist;
        for(auto boundaryitem : boundarys)
        {
            H3INDEX h3index = std::get<0>(boundaryitem);
            CellBoundary boundary = std::get<1>(boundaryitem);
            QVector<QGV::GeoPos> geoPosList;
            for (int v = 0; v < boundary.numVerts; v++)
            {
                geoPosList.push_back(QGV::GeoPos(LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng)));
            }
            if(!geoPosList.empty())
            {
                PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), QColor(0,255,255,168),false);
                _pCurrentPolygon->setH3index(h3index);

                auto itor2 = std::find_if(hexidxslistinfo.begin(),
                                            hexidxslistinfo.end(),
                                          [&](const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> >::value_type &vt){
                                              return std::get<2>(vt) == h3index;
                                          });
                QPair<TYPE_ULID, TYPE_ULID> newIntersectionUlidPair;
                if(itor2 != hexidxslistinfo.end())
                {
                    newIntersectionUlidPair = qMakePair(std::get<0>(*itor2),std::get<1>(*itor2));
                }
                _pCurrentPolygon->setIntersectionUlidPair(newIntersectionUlidPair);
                m_pItemIntersectionLayer->addItem(_pCurrentPolygon);
                polygonlist.push_back(_pCurrentPolygon);
            }
        }
        m_polygonsIntersection.insert(std::make_pair(uildsrc, std::move(polygonlist)));
    }
    m_geoMap->geoView()->scene()->update();
}

void MainWindow::displayEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE &echowaveinfo, const HEXIDX_HGT_ARRAY&hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave)
{
#ifdef SHOW_ECHOWAVE
    const TYPE_ULID & uildsrc = std::get<0>(echowaveinfo);
    const TYPE_ULID & uilddst = std::get<1>(echowaveinfo);
    const TYPE_ULID & sensingmediaid = std::get<2>(echowaveinfo);

    EVENT_KEY_TYPE event_key_tuple = EVENT_KEY_TYPE{uildsrc,uilddst, sensingmediaid};

    QVector<QGV::GeoPos> geoPosList;
    for(auto geolatlnglistitem : geolatlnglist)
    {
        geoPosList.push_back(QGV::GeoPos(geolatlnglistitem.lat,geolatlnglistitem.lng));
    }

    QColor color(255,0,255,64);
    if(bEchoWave)
    {
        color = QColor(0,0,255,192);
    }
    if(hexidxslist.empty() && geoPosList.empty())
    {
        auto itor = m_echowavegeopolygons.find(event_key_tuple);
        if(itor != m_echowavegeopolygons.end())
        {
            QList<PolygonItem *>&polygonlist = itor->second;
            for(auto item : polygonlist)
            {
                m_pItemHexEchoLayer->removeItem(item);
                delete item;
            }
            polygonlist.clear();
            m_echowavegeopolygons.erase(itor);
        }
    }
    auto itor = m_echowavegeopolygons.find(event_key_tuple);
    if(itor != m_echowavegeopolygons.end())
    {
        QList<PolygonItem *>&polygonlist = itor->second;
        if(hexidxslist.empty() && geoPosList.empty())
        {
            for(auto item : polygonlist)
            {
                m_pItemHexEchoLayer->removeItem(item);
                delete item;
            }
            polygonlist.clear();
            m_echowavegeopolygons.erase(itor);
        }
        else
        {
            if(polygonlist.size() == 1)
            {
                polygonlist.at(0)->updatePts(geoPosList);
                polygonlist.at(0)->setTransfer(m_bTransfer);
            }
            else
            {
                if(!geoPosList.empty())
                {
                    PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
                    _pCurrentPolygon->setEntityulid(qMakePair(uildval,uildval));
                    m_pItemHexEchoLayer->addItem(_pCurrentPolygon);
                    polygonlist.push_back(_pCurrentPolygon);
                    m_geoMap->geoView()->scene()->update();
                    _pCurrentPolygon->setTransfer(m_bTransfer);
                }
            }
        }
    }
    else
    {
        if(!hexidxslist.empty() && !geoPosList.empty())
        {
            QList<PolygonItem *> polygonlist;
            PolygonItem * _pCurrentPolygon = new PolygonItem(m_snowflake.GetId(),geoPosList,KM_PH_TO_M_S(50*4), color,false);
            _pCurrentPolygon->setEntityulid(qMakePair(uildval,uildval));
            m_pItemHexEchoLayer->addItem(_pCurrentPolygon);
            polygonlist.push_back(_pCurrentPolygon);
            m_echowavegeopolygons.insert(std::make_pair(event_key_tuple, std::move(polygonlist)));

            m_geoMap->geoView()->scene()->update();
            _pCurrentPolygon->setTransfer(m_bTransfer);
        }
    }
    m_geoMap->geoView()->scene()->update();
#endif
}


void MainWindow::dealeventlist_update_callback(const E_EVENT_MODE &eventmode, const std::vector<EVENT_INFO> &eventlist)
{
    auto getEntityName = [&](const TYPE_ULID& entityulid)
    {
        QString entityname;

        auto ulidiconsitor = m_ulidicons.find(entityulid);
        if(ulidiconsitor != m_ulidicons.end())
        {
            entityname = std::get<1>(ulidiconsitor->second);;
        }
        else
        {
            auto actitor = m_actors.find(entityulid);
            if(actitor != m_actors.end())
            {
                QString &displayName = std::get<4>(actitor->second);
                entityname = displayName;
            }
        }
        return entityname;
    };

    switch(eventmode)
    {
    case E_EVENT_MODE_ADD:
    {
        for(auto item : eventlist)
        {
            const TYPE_ULID & sensoruild = item.m_sensorid;
            const TYPE_ULID & entityuilddst = item.m_entityid;
            const TYPE_ULID & sensingmediaid = item.m_sensingmediaid;
            const transdata_entityposinfo & sensorpos = item.m_sensorposinfo;
            const transdata_entityposinfo & entitypos = item.m_entityposinfo;

            HEXIDX_HGT_ARRAY hexidxslist;
            hexidxslist.push_back(transdata_param_seq_hexidx{sensorpos.PARAM_pos_hexidx,transdata_param_seq_hexidx_hgt{0,0,0,0}});
            hexidxslist.push_back(transdata_param_seq_hexidx{entitypos.PARAM_pos_hexidx,transdata_param_seq_hexidx_hgt{0,0,0,0}});
            QVector<LAT_LNG> geolatlnglist;
            geolatlnglist.push_back(LAT_LNG{(double)(sensorpos.PARAM_latitude)/LON_LAT_ACCURACY, (double)(sensorpos.PARAM_longitude)/LON_LAT_ACCURACY});
            geolatlnglist.push_back(LAT_LNG{(double)(entitypos.PARAM_latitude)/LON_LAT_ACCURACY, (double)(entitypos.PARAM_longitude)/LON_LAT_ACCURACY});

            displayEchoWaveHexidxCallback(sensoruild, std::make_tuple(entityuilddst,sensoruild,sensingmediaid, sensorpos.PARAM_pos_hexidx, entitypos.PARAM_pos_hexidx), hexidxslist, geolatlnglist, true);

            m_pDataSrcListViewModel->add(item,getEntityName(sensoruild),getEntityName(entityuilddst), true);
            dealFoundEvent(item);
            m_ieventcount++;
            m_ieventcount_plus++;
        }
    }
    break;
    case E_EVENT_MODE_REMOVE:
    {
        for(auto item : eventlist)
        {
            const TYPE_ULID & sensoruild = item.m_sensorid;
            const TYPE_ULID & entityuilddst = item.m_entityid;
            const TYPE_ULID & sensingmediaid = item.m_sensingmediaid;
            const transdata_entityposinfo & sensorpos = item.m_sensorposinfo;
            const transdata_entityposinfo & entitypos = item.m_entityposinfo;

            displayEchoWaveHexidxCallback(sensoruild, std::make_tuple(entityuilddst, sensoruild,sensingmediaid, sensorpos.PARAM_pos_hexidx, entitypos.PARAM_pos_hexidx), HEXIDX_HGT_ARRAY(), QVector<LAT_LNG>(), true);

            m_pDataSrcListViewModel->add(item,getEntityName(sensoruild),getEntityName(entityuilddst), false);
            m_ieventcount--;
            m_ieventcount_sub++;
        }
    }
    break;

    case E_EVENT_MODE_UPDATE:
    {
        for(auto item : eventlist)
        {
            const TYPE_ULID & sensoruild = item.m_sensorid;
            const TYPE_ULID & entityuilddst = item.m_entityid;            
            const TYPE_ULID & sensingmediaid = item.m_sensingmediaid;
            const transdata_entityposinfo & sensorpos = item.m_sensorposinfo;
            const transdata_entityposinfo & entitypos = item.m_entityposinfo;

            HEXIDX_HGT_ARRAY hexidxslist;
            hexidxslist.push_back(transdata_param_seq_hexidx{sensorpos.PARAM_pos_hexidx,transdata_param_seq_hexidx_hgt{0,0,0,0}});
            hexidxslist.push_back(transdata_param_seq_hexidx{entitypos.PARAM_pos_hexidx,transdata_param_seq_hexidx_hgt{0,0,0,0}});
            QVector<LAT_LNG> geolatlnglist;
            geolatlnglist.push_back(LAT_LNG{(double)(sensorpos.PARAM_latitude)/LON_LAT_ACCURACY, (double)(sensorpos.PARAM_longitude)/LON_LAT_ACCURACY});
            geolatlnglist.push_back(LAT_LNG{(double)(entitypos.PARAM_latitude)/LON_LAT_ACCURACY, (double)(entitypos.PARAM_longitude)/LON_LAT_ACCURACY});

            displayEchoWaveHexidxCallback(sensoruild, std::make_tuple(entityuilddst, sensoruild,sensingmediaid,  sensorpos.PARAM_pos_hexidx, entitypos.PARAM_pos_hexidx), hexidxslist, geolatlnglist, true);

            m_pDataSrcListViewModel->updateItem(item,getEntityName(sensoruild),getEntityName(entityuilddst));

            //std::cout <<"update *** event"<<std::endl;
        }
    }
    break;
    default:break;
    }
}

void MainWindow::updateTrackingItems()
{
    bool bUpdate = false;
    auto itor = m_TrackingItems.begin();
    while(itor!=m_TrackingItems.end())
    {
        BaseItem* item = *itor;
        QVector<QGV::GeoPos> pts;

        bool bNeedAutoRemove = false;
        auto ulidiconsitor = m_ulidicons.find(item->ulid());
        if(ulidiconsitor != m_ulidicons.end())
        {
            E_ENTITY_DISAPPEAR_TYPE & disappeartype = std::get<2>(ulidiconsitor->second);
            if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE)
            {
                bNeedAutoRemove = true;
//                m_ulidicons.erase(ulidiconsitor);
            }
        }

        if(item->updateTrackingPoint() && bNeedAutoRemove)
        {
            int iCount2 = m_pLineLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pLineLayer->getItem(index);

                BaseItem *pEntityItem = dynamic_cast<BaseItem*>(pQGVItem);
                if((pEntityItem->ulid() ==  item->ulid()))
                {
                    m_pLineLayer->removeItem(pEntityItem);

                    LineItem* pLineItem = dynamic_cast<LineItem*>(pEntityItem);
                    if(pLineItem)
                    {
                        if(m_TrackingLineItems.contains(pLineItem))
                        {
                            m_TrackingLineItems.removeAll(pLineItem);
                        }
                    }
                    pEntityItem->deleteLater();
                    break;
                }
            }

            auto ulidiconsitor = m_ulidicons.find(item->ulid());
            if(ulidiconsitor != m_ulidicons.end())
            {
                E_ENTITY_DISAPPEAR_TYPE & disappeartype = std::get<2>(ulidiconsitor->second);
                if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE)
                {
                    m_ulidicons.erase(ulidiconsitor);
                }
            }
            GaeactorManager::getInstance().clearHexidex(item->ulid());
            itor = m_TrackingItems.erase(itor);
            continue;
        }
        else
        {
            pts = item->geoRect();
            generateHex(item->ulid(), item->center(), pts,item);
        }
        itor++;
    }
    if(bUpdate)
    {
        m_geoMap->geoView()->scene()->update();
    }
}

void MainWindow::updateTrackingLineItems()
{
    bool bUpdate = false;
    auto itor = m_TrackingLineItems.begin();
    while(itor!=m_TrackingLineItems.end())
    {
        LineItem* pLineItem = *itor;
        if(pLineItem)
        {
            pLineItem->dealGuidePt();
            QGV::GeoPos  lastgeopostmp;
            bool bgetlast = pLineItem->updateTrackingLastPoints(lastgeopostmp);

            QGV::GeoPos  geopos;
            if(pLineItem->updateTrackingPoints(geopos))
            {
                auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(geopos.latitude(), geopos.longitude());
                INT32 alt = 0.0;
                FLOAT32 roll = 0.0;
                FLOAT32 pitch = 0.0;
                FLOAT32 yaw = 0.0;
                if(bgetlast)
                {
                    auto lastgeopos = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(lastgeopostmp.latitude(), lastgeopostmp.longitude());
                    yaw = std::get<1>(projectionmercator::ProjectionEPSG3857::calculateBraring(LAT_LNG{lastgeopos.lat,lastgeopos.lng} , wsgeo));
                }
                GaeactorManager::getInstance().dealentityHexidex(pLineItem->ulid(),LAT_LNG{wsgeo.lat,wsgeo.lng}, alt,  roll,  pitch,  yaw,false);
                bUpdate = true;
            }
        }
        itor++;
    }
    if(bUpdate)
    {
        m_geoMap->geoView()->scene()->update();
    }

}


void MainWindow::testCode()
{
    step1();
}

void MainWindow::step1()
{
    step1_1();
    step1_2();
}

void MainWindow::step1_1()
{
    QList<std::tuple<LAT_LNG,LAT_LNG*,int,QString,QString, QColor,double,E_ENTITY_DISAPPEAR_TYPE>> _entitys;

    for(int index = 0; index < sizeof(minepos)/sizeof(LAT_LNG); index++)
    {
        _entitys.push_back(std::make_tuple(minepos[index], nullptr, 0, "./res/mine_blue.png",QString("水雷%1").arg(QString::number(index+1)),QColor(0,0,255,64),0,E_ENTITY_DISAPPEAR_TYPE_FOUND_REMOVE));
    }

    for(auto &_entitysitem : _entitys)
    {
        const LAT_LNG &pos = std::get<0>(_entitysitem);
        const QString& iconname = std::get<3>(_entitysitem);
        const QString& entityname = std::get<4>(_entitysitem);
        const QColor & color = std::get<5>(_entitysitem);
        double &fspeed = std::get<6>(_entitysitem);
        E_ENTITY_DISAPPEAR_TYPE &disppeartype = std::get<7>(_entitysitem);
        generateentityFunc(pos, iconname,entityname,color,fspeed,disppeartype);
        stdutils::OriDateTime::sleep(1);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    m_geoMap->geoView()->scene()->update();
}

void MainWindow::step1_2()
{
    QList<std::tuple<LAT_LNG,LAT_LNG*,int,QString,QString, QColor,double, double,E_ENTITY_DISAPPEAR_TYPE>> _entitys3;

    _entitys3.push_back(std::make_tuple(usv1[0], usv11, sizeof(usv11)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇1组1号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv1[1], usv12, sizeof(usv12)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇1组2号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv1[2], usv13, sizeof(usv13)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇1组3号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv1[3], usv14, sizeof(usv14)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇1组4号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));

    _entitys3.push_back(std::make_tuple(usv2[0], usv21, sizeof(usv21)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇2组1号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv2[1], usv22, sizeof(usv22)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇2组2号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv2[2], usv23, sizeof(usv23)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇2组3号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));
    _entitys3.push_back(std::make_tuple(usv2[3], usv24, sizeof(usv24)/sizeof(LAT_LNG), "./res/cruiser1_red.png","侦察无人艇2组4号",QColor(255,0,0,64),KM_PH_TO_M_S(55),2.5*1000,E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE));

    for(auto &_entitysitem : _entitys3)
    {
        const LAT_LNG &pos = std::get<0>(_entitysitem);
        const LAT_LNG* trackingpts = std::get<1>(_entitysitem);
        int &ptcount = std::get<2>(_entitysitem);
        const QString& iconname = std::get<3>(_entitysitem);
        const QString& entityname = std::get<4>(_entitysitem);
        const QColor & color = std::get<5>(_entitysitem);
        double fspeed = std::get<6>(_entitysitem);
        double radius = std::get<7>(_entitysitem);

        E_ENTITY_DISAPPEAR_TYPE &disppeartype = std::get<8>(_entitysitem);
        if(trackingpts && ptcount > 0)
        {
            generateSensorFunc(pos, trackingpts, ptcount, iconname,entityname,color,fspeed,radius,disppeartype);
        }
        stdutils::OriDateTime::sleep(1);
    }
    m_geoMap->geoView()->scene()->update();
}

void MainWindow::step2()
{
    step2_1();
    step2_2();
    step2_3();
}

void MainWindow::step2_1()
{
    QList<std::tuple<LAT_LNG,LAT_LNG*,int,QString,QString, QColor,double,E_ENTITY_DISAPPEAR_TYPE>> _entitys;
    _entitys.push_back(std::make_tuple(testboat[0], testboat1, sizeof(testboat1)/sizeof(LAT_LNG), "./res/destroyer4_blue.png","试验舰1号",QColor(0,0,200,64),KM_PH_TO_M_S(70),E_ENTITY_DISAPPEAR_TYPE_FOUND_GUIDE));
    _entitys.push_back(std::make_tuple(testboat[1], testboat2, sizeof(testboat2)/sizeof(LAT_LNG), "./res/destroyer4_blue.png","试验舰2号",QColor(0,0,255,64),KM_PH_TO_M_S(70),E_ENTITY_DISAPPEAR_TYPE_FOUND_GUIDE));

    for(auto &_entitysitem : _entitys)
    {
        const LAT_LNG &pos = std::get<0>(_entitysitem);
        const LAT_LNG* trackingpts = std::get<1>(_entitysitem);
        int &ptcount = std::get<2>(_entitysitem);
        const QString& iconname = std::get<3>(_entitysitem);
        const QString& entityname = std::get<4>(_entitysitem);
        const QColor & color = std::get<5>(_entitysitem);
        double fspeed = std::get<6>(_entitysitem);
        E_ENTITY_DISAPPEAR_TYPE &disppeartype = std::get<7>(_entitysitem);
        generateTrackingLineFunc(pos, trackingpts, ptcount, iconname,entityname,color,fspeed,disppeartype);
        stdutils::OriDateTime::sleep(1);
    }
    m_geoMap->geoView()->scene()->update();
}

void MainWindow::step2_2()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    QList<std::tuple<LAT_LNG,LAT_LNG*,int,QString,QString, QColor,double,E_ENTITY_DISAPPEAR_TYPE>> _entitys2;
    _entitys2.push_back(std::make_tuple(uav[0], nullptr, 0, "./res/uav3_red.png","攻击无人机1号",QColor(255,0,0,64),KM_PH_TO_M_S(50*2),E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE));
    _entitys2.push_back(std::make_tuple(uav[1], nullptr, 0, "./res/uav3_red.png","攻击无人机2号",QColor(255,0,0,64),KM_PH_TO_M_S(50*4),E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE));
    _entitys2.push_back(std::make_tuple(uav[2], nullptr, 0, "./res/uav3_red.png","攻击无人机3号",QColor(255,0,0,64),KM_PH_TO_M_S(50*6),E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE));
    _entitys2.push_back(std::make_tuple(uav[3], nullptr, 0, "./res/uav3_red.png","攻击无人机4号",QColor(255,0,0,64),KM_PH_TO_M_S(50*8),E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE));


    for(auto &_entitysitem : _entitys2)
    {
        const LAT_LNG &pos = std::get<0>(_entitysitem);
        const QString& iconname = std::get<3>(_entitysitem);
        const QString& entityname = std::get<4>(_entitysitem);
        const QColor & color = std::get<5>(_entitysitem);
        double fspeed = std::get<6>(_entitysitem);
        E_ENTITY_DISAPPEAR_TYPE &disppeartype = std::get<7>(_entitysitem);
        generateTrackingLineFunc(pos, nullptr, 0, iconname,entityname,color,fspeed,disppeartype);
        stdutils::OriDateTime::sleep(1);
    }
    m_geoMap->geoView()->scene()->update();
}

void MainWindow::step2_3()
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    QList<std::tuple<LAT_LNG,LAT_LNG*,int,QString,QString, QColor,double, double,E_ENTITY_DISAPPEAR_TYPE>> _entitys3;

    _entitys3.push_back(std::make_tuple(cruiserboat1[0], cruiserboat11, sizeof(cruiserboat11)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇1组1号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat1[1], cruiserboat12, sizeof(cruiserboat12)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇1组2号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat1[2], cruiserboat13, sizeof(cruiserboat13)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇1组3号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat1[3], cruiserboat14, sizeof(cruiserboat14)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇1组4号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));

    _entitys3.push_back(std::make_tuple(cruiserboat2[0], cruiserboat21, sizeof(cruiserboat21)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇2组1号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat2[1], cruiserboat22, sizeof(cruiserboat22)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇2组2号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat2[2], cruiserboat23, sizeof(cruiserboat23)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇2组3号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));
    _entitys3.push_back(std::make_tuple(cruiserboat2[3], cruiserboat24, sizeof(cruiserboat24)/sizeof(LAT_LNG), "./res/destroyer3_red.png","巡逻艇2组4号",QColor(255,0,0,64),KM_PH_TO_M_S(60),5*1000,E_ENTITY_DISAPPEAR_TYPE_LOOP));

    for(auto &_entitysitem : _entitys3)
    {
        const LAT_LNG &pos = std::get<0>(_entitysitem);
        const LAT_LNG* trackingpts = std::get<1>(_entitysitem);
        int &ptcount = std::get<2>(_entitysitem);
        const QString& iconname = std::get<3>(_entitysitem);
        const QString& entityname = std::get<4>(_entitysitem);
        const QColor & color = std::get<5>(_entitysitem);
        double fspeed = std::get<6>(_entitysitem);
        double radius = std::get<7>(_entitysitem);

        E_ENTITY_DISAPPEAR_TYPE &disppeartype = std::get<8>(_entitysitem);
        if(trackingpts && ptcount > 0)
        {
            generateSensorFunc(pos, trackingpts, ptcount, iconname,entityname,color,fspeed,radius,disppeartype);
        }
        stdutils::OriDateTime::sleep(1);
    }
    m_geoMap->geoView()->scene()->update();
}


void MainWindow::dealFoundEvent(const EVENT_INFO& mapitem)
{
    const TYPE_ULID& entityulidold = mapitem.m_entityid;

    bool bClear = false;
    auto ulidiconsitor = m_ulidicons.find(entityulidold);
    if(ulidiconsitor != m_ulidicons.end())
    {
        E_ENTITY_DISAPPEAR_TYPE & disappeartype = std::get<2>(ulidiconsitor->second);
        if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_FOUND_REMOVE)
        {
            GaeactorManager::getInstance().clearentityHexidex(entityulidold);
            m_ulidicons.erase(ulidiconsitor);
            bClear = true;
        }
        if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_FOUND_GUIDE)
        {
            QGV::GeoPos targetPos((double)(mapitem.m_entityposinfo.PARAM_latitude)/LON_LAT_ACCURACY, (double)(mapitem.m_entityposinfo.PARAM_longitude)/LON_LAT_ACCURACY);
//            const H3INDEX& entityhexidxsrc = std::get<4>(mapitem);
//            LAT_LNG entitylatlng = LocationHelper::doCell(entityhexidxsrc);
//            QGV::GeoPos targetPos(LocationHelper::radianToDegree(entitylatlng.lat), LocationHelper::radianToDegree(entitylatlng.lng));

            auto targetitor = m_targetfllowguide.find(entityulidold);
            if(targetitor != m_targetfllowguide.end())
            {
                QGV::GeoPos & geopos = std::get<0>(targetitor->second);
                geopos = targetPos;
                QList<LineItem*> & followlist = std::get<1>(targetitor->second);

                for(auto pitem:followlist)
                {
                    pitem->appendguidept(targetPos);
                }
            }
            else
            {
                auto getFollowItem=[&]()
                {
                    LineItem* pfllow = nullptr;
                    int range = m_followItem.size();
                    if(range != 0)
                    {
                        int rangeIndex = (stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()+qrand()) % range;
                        pfllow = m_followItem.at(rangeIndex);
                        m_followItem.removeAll(pfllow);
                    }
                    return pfllow;
                };
                QList<LineItem*> followlist;
                LineItem* pfllow1 =  getFollowItem();
                if(pfllow1)
                {
                    pfllow1->appendguidept(targetPos);
                    followlist.push_back(pfllow1);
                }
                LineItem* pfllow2 =  getFollowItem();
                if(pfllow2)
                {
                    pfllow2->appendguidept(targetPos);
                    followlist.push_back(pfllow2);
                }

                m_targetfllowguide.insert(std::make_pair(entityulidold, std::make_tuple(targetPos,followlist)));
            }
        }
    }
    if(bClear)
    {
        auto ulidiconsitor2 = m_ulidicons.begin();

        bool bEnd = true;
        while(ulidiconsitor2 != m_ulidicons.end())
        {
            E_ENTITY_DISAPPEAR_TYPE & disappeartype = std::get<2>(ulidiconsitor2->second);
            if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_FOUND_REMOVE)
            {
                bEnd = false;
                break;
            }
            ulidiconsitor2++;
        }
        if(bEnd)
        {
            m_scriptstep = E_SCRIPT_TYPE_STEP1;
            m_interval = 5;
        }
    }
}

void MainWindow::generateSensorFunc(const LAT_LNG &pos, const LAT_LNG *trackingpts, int ptcount, const QString &iconname, const QString &name, const QColor &color, double fspeed,double radius,E_ENTITY_DISAPPEAR_TYPE &disppeartype)
{
    EllipseItem *pLine = new EllipseItem(m_snowflake.GetId(),QGV::GeoPos(pos.lat, pos.lng),fspeed, 5*1000,color,false);
    m_ulidicons.insert(std::make_pair(pLine->ulid(),std::make_tuple(iconname,name,disppeartype)));
    pLine->setShowTrackingLine(m_showTrackingLine);
    m_pLineLayer->addItem(pLine);
    for(int index = 0; index < ptcount; index++)
    {
        if(index < ptcount - 1)
        {
            pLine->appendPoint(QGV::GeoPos(trackingpts[index].lat, trackingpts[index].lng));
        }
        else
        {
            pLine->appendFinish(QGV::GeoPos(trackingpts[index].lat, trackingpts[index].lng));
            pLine->updateSpeedCoeff(m_speedcoeff);
            m_TrackingItems.push_back(pLine);
        }
    }
}

void MainWindow::generateTrackingLineFunc(const LAT_LNG &pos, const LAT_LNG* trackingpts, int ptcount, const QString& iconname,const QString& name, const QColor &color,double fspeed,E_ENTITY_DISAPPEAR_TYPE &disppeartype)
{
    LineItem *pLine = new LineItem(m_snowflake.GetId(),QGV::GeoPos(pos.lat, pos.lng), fspeed,color,false);
    m_ulidicons.insert(std::make_pair(pLine->ulid(),std::make_tuple(iconname,name,disppeartype)));
    pLine->setShowTrackingLine(m_showTrackingLine);
    if(disppeartype == E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE)
    {
        pLine->setShowTrackingLine(true);
        m_followItem.push_back(pLine);
    }
    m_pLineLayer->addItem(pLine);
    if(nullptr == trackingpts || ptcount == 0)
    {
        pLine->appendFinish(QGV::GeoPos(pos.lat, pos.lng));
        pLine->updateSpeedCoeff(m_speedcoeff);
        m_TrackingLineItems.push_back(pLine);
    }
    else
    {
        for(int index = 0; index < ptcount; index++)
        {
            if(index < ptcount - 1)
            {
                pLine->appendPoint(QGV::GeoPos(trackingpts[index].lat, trackingpts[index].lng),false);
            }
            else
            {
                pLine->appendFinish(QGV::GeoPos(trackingpts[index].lat, trackingpts[index].lng));
                pLine->updateSpeedCoeff(m_speedcoeff);
                m_TrackingLineItems.push_back(pLine);
            }
        }

    }
}

void MainWindow::generateentityFunc(const LAT_LNG &pos, const QString &iconname, const QString &name, const QColor &color, double fspeed,E_ENTITY_DISAPPEAR_TYPE &disppeartype)
{
    LineItem *pLine = new LineItem(m_snowflake.GetId(),QGV::GeoPos(pos.lat, pos.lng), fspeed,color,false);
    m_ulidicons.insert(std::make_pair(pLine->ulid(),std::make_tuple(iconname,name,disppeartype)));
    pLine->setShowTrackingLine(m_showTrackingLine);

    m_pLineLayer->addItem(pLine);

    auto wsgeo = projectionmercator::ProjectionEPSG3857::gcj02_to_wgs84(pos.lat, pos.lng);

    INT32 alt = 0.0;
    FLOAT32 roll = 0.0;
    FLOAT32 pitch = 0.0;
    FLOAT32 yaw = 0.0;

    GaeactorManager::getInstance().dealentityHexidex(pLine->ulid(),wsgeo, alt,  roll,  pitch,  yaw,false);
}

void MainWindow::updateLineLayerShow()
{
    //m_pLineLayer->setVisible(m_showTrackingLine);

    int iCount2 = m_pLineLayer->countItems();
    for(int index = 0; index < iCount2; index++)
    {
        bool bShow = m_showTrackingLine;
        QGVItem* pQGVItem = m_pLineLayer->getItem(index);

        BaseItem *pEntityItem = dynamic_cast<BaseItem*>(pQGVItem);
        auto ulidiconsitor = m_ulidicons.find(pEntityItem->ulid());

        if(ulidiconsitor != m_ulidicons.end())
        {
            E_ENTITY_DISAPPEAR_TYPE & disappeartype = std::get<2>(ulidiconsitor->second);
            if(disappeartype == E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE)
            {
                bShow = true;
            }
        }
        pEntityItem->setShowTrackingLine(bShow);
    }

    int iCount = m_pItemLayer->countItems();
    for(int index = 0; index < iCount; index++)
    {
        QGVItem* pQGVItem = m_pItemLayer->getItem(index);
        BaseItem *pEntityItem = dynamic_cast<BaseItem*>(pQGVItem);
        if(pEntityItem)
        {
            pEntityItem->setShowTrackingLine(m_showTrackingLine);
        }
    }

//    int iCount3 = m_pEntityImageLayer->countItems();
//    for(int index = 0; index < iCount3; index++)
//    {
//        QGVItem* pQGVItem = m_pEntityImageLayer->getItem(index);
//        BaseItem *pEntityItem = dynamic_cast<BaseItem*>(pQGVItem);
//        if(pEntityItem)
//        {
//            pEntityItem->setShowTrackingLine(m_showTrackingLine);
//        }
//    }




    m_geoMap->geoView()->scene()->update();
}

void MainWindow::updateHexLayerShow()
{
    m_pItemHexLayer->setVisible(m_showHexLayer);

    int iCount2 = m_pItemHexLayer->countItems();
    for(int index = 0; index < iCount2; index++)
    {
        bool bShow = m_showHexLayer;
        QGVItem* pQGVItem = m_pItemHexLayer->getItem(index);
        pQGVItem->setVisible(bShow);
    }

    m_geoMap->geoView()->scene()->update();
}

void MainWindow::updateSpeedCoeff()
{
    auto itor = m_TrackingLineItems.begin();
    while(itor!=m_TrackingLineItems.end())
    {
        BaseItem* pLineItem = *itor;
        if(pLineItem)
        {
            pLineItem->updateSpeedCoeff(m_speedcoeff);
        }
        itor++;
    }

    auto itor2 = m_TrackingItems.begin();
    while(itor2!=m_TrackingItems.end())
    {
        BaseItem* pLineItem = *itor2;
        if(pLineItem)
        {
            pLineItem->updateSpeedCoeff(m_speedcoeff);
        }
        itor2++;
    }
}

void MainWindow::updateDownloadLevel(QGVLayer *layer)
{
    QGVLayerTilesOnline *pQGVLayerTilesOnline = dynamic_cast<QGVLayerTilesOnline *>(layer);

    if(pQGVLayerTilesOnline)
    {
        ui->comboBox_2->clear();
        ui->comboBox_2->addItem("所有分辨率",QVariant(-1));
        for(int index = pQGVLayerTilesOnline->minZoomlevel(); index <= pQGVLayerTilesOnline->maxZoomlevel(); index++)
        {
            ui->comboBox_2->addItem(QString::number(index),QVariant(index));
        }
        ui->comboBox_2->setCurrentIndex((pQGVLayerTilesOnline->maxZoomlevel() - pQGVLayerTilesOnline->minZoomlevel())/2);
    }
}

void MainWindow::updateZoomLevel(QGVLayer *layer)
{
    QGVLayerTilesOnline *pQGVLayerTilesOnline = dynamic_cast<QGVLayerTilesOnline *>(layer);

    if(pQGVLayerTilesOnline)
    {
        int currentzoom = pQGVLayerTilesOnline->curZoom();


        m_bTransfer = (currentzoom > 14)? true:false;

        if(m_pLineLayer)
        {
            int iCount2 = m_pLineLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pLineLayer->getItem(index);

                LineItem *pLineItem = dynamic_cast<LineItem*>(pQGVItem);
                if(pLineItem)
                {
                    if(m_ulidicons.find(pLineItem->ulid()) != m_ulidicons.end())
                    {
                        pLineItem->setTransfertmp(m_bTransfer);
                    }
                    else
                    {
                        pLineItem->setPenwidth(2/1/geoMap()->getCamera().scale());
                        pLineItem->setTransfer(m_bTransfer);
                    }
                }
            }
        }

        if(m_pEntityImageLayer)
        {
            int iCount2 = m_pEntityImageLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pEntityImageLayer->getItem(index);

                ImageItem *_pCurrentImageItem = dynamic_cast<ImageItem*>(pQGVItem);
                if(_pCurrentImageItem)
                {
                    _pCurrentImageItem->setTransfer(m_bTransfer);
                }
            }
        }

        if(m_pItemHexLayer)
        {
            int iCount2 = m_pItemHexLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pItemHexLayer->getItem(index);

                PolygonItem *_pCurrentPolygon = dynamic_cast<PolygonItem*>(pQGVItem);
                if(_pCurrentPolygon)
                {
                    _pCurrentPolygon->setPenwidth(2/1/geoMap()->getCamera().scale());
                    _pCurrentPolygon->setTransfers(m_bTransfer);
                }
            }
        }

        if(m_pItemIntersectionLayer)
        {
            int iCount2 = m_pItemIntersectionLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pItemIntersectionLayer->getItem(index);

                PolygonItem *_pCurrentPolygon = dynamic_cast<PolygonItem*>(pQGVItem);
                if(_pCurrentPolygon)
                {
                    _pCurrentPolygon->setPenwidth(2/1/geoMap()->getCamera().scale());
                    //_pCurrentPolygon->setTransfer(m_bTransfer);
                }
            }
        }

        if(m_pItemLayer)
        {
            int iCount2 = m_pItemLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pItemLayer->getItem(index);
                PolygonItem *_pCurrentPolygon = dynamic_cast<PolygonItem*>(pQGVItem);
                if(_pCurrentPolygon)
                {
                    _pCurrentPolygon->setPenwidth(2/1/geoMap()->getCamera().scale());
                    _pCurrentPolygon->setTransfertmp(m_bTransfer);
                }

                EllipseItem *_pEllipseItem = dynamic_cast<EllipseItem*>(pQGVItem);
                if(_pEllipseItem)
                {
                    _pEllipseItem->setTransfer(m_bTransfer);
                }

                PieItem *_pPieItem = dynamic_cast<PieItem*>(pQGVItem);
                if(_pPieItem)
                {
                    _pPieItem->setTransfer(m_bTransfer);
                }

                RectangleItem *_pRectangleItem = dynamic_cast<RectangleItem*>(pQGVItem);
                if(_pRectangleItem)
                {
                    _pRectangleItem->setTransfer(m_bTransfer);
                }
            }
        }

        if(m_pItemHexEchoLayer)
        {
            int iCount2 = m_pItemHexEchoLayer->countItems();
            for(int index = 0; index < iCount2; index++)
            {
                QGVItem* pQGVItem = m_pItemHexEchoLayer->getItem(index);

                PolygonItem *_pCurrentPolygon = dynamic_cast<PolygonItem*>(pQGVItem);
                if(_pCurrentPolygon)
                {
                    _pCurrentPolygon->setPenwidth(2/1/geoMap()->getCamera().scale());
                    _pCurrentPolygon->setTransfer(m_bTransfer);
                }
            }
        }


        mTextzoom->setText(QString("<b>zoom level:%1</b>")
                               .arg(currentzoom));
    }
}

void MainWindow::onMouseDoubleClick(QPointF projPos)
{
    switch (m_pQGVWidgetTools->getSelectType())
    {
    case QGVWidgetTools::E_SELECT_TYPE_ELLIPSE:
    case QGVWidgetTools::E_SELECT_TYPE_PIE:
    {
    }break;

    case QGVWidgetTools::E_SELECT_TYPE_RECTANGLE:
    {
    }break;

    case QGVWidgetTools::E_SELECT_TYPE_LINE:
    case QGVWidgetTools::E_SELECT_TYPE_POLYGON:
    case QGVWidgetTools::E_SELECT_TYPE_MOVE_TRACKING_LINE:
    {
        if(m_bSelectEnable)
        {
            m_bdoubleclicked = true;
        }
    }break;
    default:
    {
        m_bSelectEnable = false;
    }break;
    }

}

void MainWindow::onWheelEvent(QWheelEvent *event)
{
    int index = ui->comboBox->currentIndex();
    updateZoomLevel(m_layers.at(index).second);
}

void MainWindow::onKeyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_I))
    {
        m_showTrackingLine = false;
        updateLineLayerShow();
    }
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_U))
    {
        m_showTrackingLine = true;
        updateLineLayerShow();
    }
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_L))
    {
        m_showHexLayer = false;
        updateHexLayerShow();
    }
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_H))
    {
        m_showHexLayer = true;
        updateHexLayerShow();
    }



}

void MainWindow::onKeyReleaseEvent(QKeyEvent *event)
{

}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}

void MainWindow::timeout_slot()
{
    updateTrackingLineItems();
    updateTrackingItems();
    static int sec = 0;
    sec++;
    if(sec%(m_interval*10) == 0)
    {
        switch (m_scriptstep) {
        case E_SCRIPT_TYPE_STEP1:
        {
            step2_1();
            m_scriptstep = E_SCRIPT_TYPE_STEP2;
            m_interval = 4;
        };break;
        case E_SCRIPT_TYPE_STEP2:{};
            {
                step2_3();
                m_scriptstep = E_SCRIPT_TYPE_STEP3;
                m_interval = 3;
            };break;
        case E_SCRIPT_TYPE_STEP3:
        {
            step2_2();
            m_scriptstep = E_SCRIPT_TYPE_NULL;
            m_interval = 3;
        };break;
        case E_SCRIPT_TYPE_STEP4:{};break;
        case E_SCRIPT_TYPE_STEP5:{};break;
        default:
            break;
        }
    }
    //std::cout <<"event"<<m_ieventcount <<" "<<m_ieventcount_plus<<" "<<m_ieventcount_sub  <<std::endl;
}

void MainWindow::valueChangedSlot(int value)
{
    int count = m_pItemLayer->countItems();
    for(int i = 0; i< count; i++)
    {
        QGVItem* pQGVItem = m_pItemLayer->getItem(i);

        if(pQGVItem->isSelected())
        {
            PieItem* pPieItem = dynamic_cast<PieItem*>(pQGVItem);
            if(pPieItem)
            {
                if(sender() == ui->spinBox)
                {
                    pPieItem->updateStartAngle(value);
                    generateHex(pPieItem->ulid(), pPieItem->center(), pPieItem->geoRect(),pPieItem);
                    m_geoMap->geoView()->scene()->update();
                    pPieItem->setTransfer(m_bTransfer);
                    m_geoMap->geoView()->scene()->update();
                }
                else if(sender() == ui->spinBox_2)
                {
                    pPieItem->updateEndAngle(value);
                    generateHex(pPieItem->ulid(), pPieItem->center(), pPieItem->geoRect(),pPieItem);
                    m_geoMap->geoView()->scene()->update();
                    pPieItem->setTransfer(m_bTransfer);
                    m_geoMap->geoView()->scene()->update();
                }
            }
        }
    }
}



void MainWindow::initLayers()
{
    mText = new QGVWidgetText();
    mText->setAnchor(QPoint(0, 0), { Qt::TopEdge });
    mText->show();
    mText2 = new QGVWidgetText();
    mText2->setAnchor(QPoint(0, 20), { Qt::TopEdge });
    mText2->show();

    mText3 = new QGVWidgetText();
    mText3->setAnchor(QPoint(100, 0), { Qt::TopEdge, Qt::RightEdge});
    mText3->show();


    mText4 = new QGVWidgetText();
    mText4->setAnchor(QPoint(100, 20), { Qt::TopEdge, Qt::RightEdge});
    mText4->show();

    mTextzoom = new QGVWidgetText();
    mTextzoom->setAnchor(QPoint(150, 20), { Qt::BottomEdge, Qt::RightEdge});
    mTextzoom->show();




    m_geoMap->addWidget(mText);
    m_geoMap->addWidget(mText2);
    m_geoMap->addWidget(mText3);
    m_geoMap->addWidget(mText4);
    m_geoMap->addWidget(mTextzoom);
    /*
     * Footer will be used to show fixed text about selected background layer.
     * Widget owned by map.
     */
    mFooter = new QGVWidgetText();
    m_geoMap->addWidget(mFooter);


    mFooter->setStyleSheet("color: white");
    mText->setStyleSheet("color: white");
    mText2->setStyleSheet("color: white");
    mText3->setStyleSheet("color: white;font-size:20pt");
    mText4->setStyleSheet("color: white");
    mTextzoom->setStyleSheet("color: white;font-size:20pt");

    /*
     * List of available tile maps.
     */
    const QString customURI = "http://c.tile.stamen.com/watercolor/${z}/${x}/${y}.jpg";

    m_layers.push_back(qMakePair(QString("OSM"), new QGVLayerOSM()));
    m_layers.push_back(qMakePair(QString("GOOGLE_SATELLITE"), new QGVLayerGoogle(QGV::TilesType::Satellite)));
    m_layers.push_back(qMakePair(QString("GOOGLE_HYBRID"), new QGVLayerGoogle(QGV::TilesType::Hybrid)));
    m_layers.push_back(qMakePair(QString("GOOGLE_SCHEMA"), new QGVLayerGoogle(QGV::TilesType::Schema)));
    m_layers.push_back(qMakePair(QString("BING_SATELLITE"), new QGVLayerBing(QGV::TilesType::Satellite)));
    m_layers.push_back(qMakePair(QString("BING_HYBRID"), new QGVLayerBing(QGV::TilesType::Hybrid)));
    m_layers.push_back(qMakePair(QString("BING_SCHEMA"), new QGVLayerBing(QGV::TilesType::Schema)));
    m_layers.push_back(qMakePair(QString("CUSTOM_OSM"), new QGVLayerOSM(customURI)));
    m_layers.push_back(qMakePair(QString("LOCAL"), new QGVLayerLocal()));

    /*
     * Layers will be owned by map.
     */
    for (auto pair : m_layers) {
        auto name = pair.first;
        auto layer = pair.second;
        QGVLayerTilesOnline *pQGVLayerTilesOnline = dynamic_cast<QGVLayerTilesOnline *>(layer);

        if(pQGVLayerTilesOnline)
        {
            connect(pQGVLayerTilesOnline, &QGVLayerTilesOnline::updateDownloadProgresssig, this, &MainWindow::updateDownloadProgressslot);
        }

        layer->hide();
        ui->comboBox->addItem(name);
        m_geoMap->addItem(layer);
    }
    ui->comboBox->setCurrentIndex(8);

    updateDownloadLevel(m_layers.at(8).second);

    updateZoomLevel(m_layers.at(8).second);

    //////////////////////////////////////////////////////////
    m_pItemLayer = new QGVLayer();
    m_pItemLayer->setName("ItemLayer");
    m_pItemLayer->setDescription("Item Layer");
    geoMap()->addItem(m_pItemLayer);

    //////////////////////////////////////////////////////////
    m_pItemTmpLayer = new QGVLayer();
    m_pItemTmpLayer->setName("ItemLayerTmp");
    m_pItemTmpLayer->setDescription("ItemTmp Layer");
    geoMap()->addItem(m_pItemTmpLayer);

    //////////////////////////////////////////////////////////
    m_pItemHexLayer = new QGVLayer();
    m_pItemHexLayer->setName("ItemHexLayer");
    m_pItemHexLayer->setDescription("ItemHex Layer");
    geoMap()->addItem(m_pItemHexLayer);

    //////////////////////////////////////////////////////////
    m_pItemHexEchoLayer = new QGVLayer();
    m_pItemHexEchoLayer->setName("ItemHexEchoLayer");
    m_pItemHexEchoLayer->setDescription("ItemHexEcho Layer");
    geoMap()->addItem(m_pItemHexEchoLayer);

    //////////////////////////////////////////////////////////
    m_pItemIntersectionLayer = new QGVLayer();
    m_pItemIntersectionLayer->setName("IntersectionLayer");
    m_pItemIntersectionLayer->setDescription("Intersection Layer");
    geoMap()->addItem(m_pItemIntersectionLayer);


    //////////////////////////////////////////////////////////
    m_pLineLayer = new QGVLayer();
    m_pLineLayer->setName("LineLayer");
    m_pLineLayer->setDescription("Line Layer");
    geoMap()->addItem(m_pLineLayer);

    //////////////////////////////////////////////////////////
    m_pEntityImageLayer = new QGVLayer();
    m_pEntityImageLayer->setName("EntityImageLayer");
    m_pEntityImageLayer->setDescription("EntityImage Layer");
    geoMap()->addItem(m_pEntityImageLayer);

    updateLineLayerShow();
    updateHexLayerShow();
}

void MainWindow::initWidgets()
{
    m_widgets.push_back(qMakePair(QString("Compass"), new QGVWidgetCompass()));
    m_widgets.push_back(qMakePair(QString("ZoomButtons"), new QGVWidgetZoom()));
    m_widgets.push_back(qMakePair(QString("ScaleHorizontal"), new QGVWidgetScale(Qt::Horizontal)));
    m_widgets.push_back(qMakePair(QString("ScaleVertical"), new QGVWidgetScale(Qt::Vertical)));

    m_pQGVWidgetTools = new QGVWidgetTools();
    m_widgets.push_back(qMakePair(QString("Tools"), m_pQGVWidgetTools));

    /*
     * Widgets will be owned by map.
     */
    for (auto pair : m_widgets) {
        auto name = pair.first;
        auto widget = pair.second;
        widget->show();
        m_geoMap->addWidget(widget);
    }
}

void MainWindow::initgeomap()
{
    startTracking(true);
    enableAction(QGV::MouseAction::ZoomWheel,true);
    enableAction(QGV::MouseAction::ZoomRect,true);
    enableAction(QGV::MouseAction::Selection,true);
    enableAction(QGV::MouseAction::Move,true);
    enableAction(QGV::MouseAction::ContextMenu,true);
    enableAction(QGV::MouseAction::Tooltip,true);
}

void MainWindow::startTracking(bool start)
{
    m_geoMap->setMouseTracking(start);
}

void MainWindow::enableAction(QGV::MouseAction action, bool enable)
{
    m_geoMap->setMouseAction(action, enable);
}

QGVMap *MainWindow::geoMap() const
{
    return m_geoMap;
}


QGV::GeoPos MainWindow::randPos(const QGV::GeoRect& targetArea)
{
    const double latRange = targetArea.latTop() - targetArea.latBottom();
    const double lonRange = targetArea.lonRigth() - targetArea.lonLeft();
    static const int range = 1000;
    return { targetArea.latBottom() + latRange * (qrand() % range) / range,
             targetArea.lonLeft() + lonRange * (qrand() % range) / range };
}

QGV::GeoRect MainWindow::randRect(const QGV::GeoRect& targetArea, const QSizeF& size)
{
    const auto baseGeo = randPos(targetArea);
    const auto base = geoMap()->getProjection()->geoToProj(baseGeo);
    return geoMap()->getProjection()->projToGeo({ base, base + QPointF(size.width(), size.height()) });
}

QGV::GeoRect MainWindow::randRect(const QGV::GeoRect& targetArea, int baseSize)
{
    const auto size = randSize(baseSize);
    return randRect(targetArea, size);
}

QSizeF MainWindow::randSize(int baseSize)
{
    const int range = -baseSize / 2;
    return QSize(baseSize + (qrand() % range), baseSize + (qrand() % range));
}

QGV::GeoRect MainWindow::targetArea() const
{
    return QGV::GeoRect(QGV::GeoPos(31.525029614929977,103.09173869852884), QGV::GeoPos(29.544029022607717,106.62621662988863));
//    return QGV::GeoRect(QGV::GeoPos(26.06822947982789, 119.60365706952996), QGV::GeoPos(24.41884246847917,122.74837258794958));
}


void MainWindow::on_pushButton_3_clicked()
{
    step1_1();
}


void MainWindow::on_pushButton_2_clicked()
{
    step1_2();
}


void MainWindow::on_pushButton_4_clicked()
{
    m_speedcoeff = m_speedcoeff/2.0;
    ui->label_4->setText("速度*"+QString::number(m_speedcoeff));
    updateSpeedCoeff();
}


void MainWindow::on_pushButton_clicked()
{
    m_speedcoeff = m_speedcoeff*2.0;
    ui->label_4->setText("速度*"+QString::number(m_speedcoeff));
    updateSpeedCoeff();
}


void MainWindow::on_pushButton_5_clicked()
{
    const QGVProjection* projection = geoMap()->getProjection();
    const QGVCameraState camera = geoMap()->getCamera();
    const QRectF areaProjRect = camera.projRect().intersected(projection->boundaryProjRect());
    const QGV::GeoRect areaGeoRect = projection->projToGeo(areaProjRect);


    int index = ui->comboBox->currentIndex();
    if(index != 7 && index != 8)
    {
        QGVLayerTilesOnline *pQGVLayerTilesOnline = dynamic_cast<QGVLayerTilesOnline *>(m_layers.at(index).second);

        if(pQGVLayerTilesOnline)
        {
            int zoommax = ui->comboBox_2->currentData().toInt();

            pQGVLayerTilesOnline->setDoladMaxZoom(zoommax);
            pQGVLayerTilesOnline->downloadAreaData(areaGeoRect);

            ui->progressBar->setValue(0);
            ui->progressBar->setFormat(QString("%1%").arg(QString::number(0,'f',6)));//50.43
            ui->label_5->setText(QString("%1/%2 0--->%3").arg(0).arg(0).arg(pQGVLayerTilesOnline->zoommax()));
        }
    }

}

void MainWindow::updateDownloadProgressslot(int total, int reply, int zoom, int zoommax)
{
    float f = 100.0*reply / total;
    ui->progressBar->setValue(f);
    ui->progressBar->setFormat(QString("%1%").arg(QString::number(f,'f',6)));//50.43
    ui->label_5->setText(QString("%1/%2 %3--->%4").arg(reply).arg(total).arg(zoom).arg(zoommax));
}
