#include "ConcurrentHashMapManager.h"
#include "mainwindow.h"

#include <QTimer>
#include <QtMath>
#include <QDateTime>

#include <QDir>
#include <QFileInfoList>
#ifdef USING_GUI_SHOW
#include "ui_mainwindow.h"
#include "./widget2d/map2dwidget.h"
#include "widget3d/QtOsgWidget.h"
#else
#endif
#include "OriginalThread.h"
#include "OriginalDateTime.h"
#include "httpserver/httpserver.h"


#define USING_DRAW_LINE_SAMPLING

#ifdef USING_GUI_SHOW
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_pMapWidget(nullptr)
    , m_pModelWidget(nullptr)
#else
MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
#endif
    , m_phttpserver(nullptr)
    , m_pHttpServerRunningThread(nullptr)
    , m_pConcurrentHashMapManager(nullptr)
{

#ifdef USING_GUI_SHOW
    ui->setupUi(this);
    this->resize(1600,1200);
#endif

    qRegisterMetaType<tagLineInfo>("tagLineInfo");
    if(nullptr == m_pConcurrentHashMapManager)
    {
        m_pConcurrentHashMapManager = new ConcurrentHashMapManager();
    }

    m_phttpserver = new HttpServer(this);
    m_phttpserver->setDataCallback(std::bind(&MainWindow::httpdatareceive_callback, this, std::placeholders::_1, std::placeholders::_2));

    m_pHttpServerRunningThread = new stdutils::OriThread(std::bind(&MainWindow::thread_httpserver_callback_Loop, this, std::placeholders::_1), this);

#ifdef USING_GUI_SHOW
    //    m_pMapWidget = new Map2dWidget(Map2dWidget::E_MAP_MODE_DISPLAY, this);

    m_pModelWidget = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MAP, this);

    double lon = 103.94037755013733;
    double lat = 30.56454233609911;
    lon = 113.31034109441714;
    lat =  23.39858771117966;
    double hgt = 600;

    QWidget * pQWidget = new QWidget(this);
    m_pLayout = new QHBoxLayout(this);
    if(m_pModelWidget)
    {
        m_pLayout->addWidget(m_pModelWidget);
    }
    if(m_pMapWidget)
    {
        m_pLayout->addWidget(m_pMapWidget);
    }
    m_pLayout->setSpacing(1);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    pQWidget->setLayout(m_pLayout);
    this->setCentralWidget(pQWidget);

    if(m_pMapWidget)
    {
        m_pMapWidget->updateViewCenter(lon, lat);
    }
    if(m_pModelWidget)
    {
        m_pModelWidget->updateViewPoint(lon, lat);
    }
#endif
    connect(m_pConcurrentHashMapManager, &ConcurrentHashMapManager::deal_result_sig, this, &MainWindow::deal_result_slot);
    connect(m_pConcurrentHashMapManager, &ConcurrentHashMapManager::draw_linestring_sig, this, &MainWindow::draw_linestring_slot);

#ifdef USING_GUI_SHOW
    m_pConcurrentHashMapManager->show_building();
#endif
}

MainWindow &MainWindow::getInstance()
{
    static MainWindow w;
    return w;
}

MainWindow::~MainWindow()
{
#ifdef USING_GUI_SHOW
    delete ui;
#endif
    stop_HttpServer();
    if(m_pHttpServerRunningThread)
    {

        delete m_pHttpServerRunningThread;
    }

//    if(m_phttpserver)
//    {
//        delete m_phttpserver;
//    }

    if(m_pConcurrentHashMapManager)
    {
        delete m_pConcurrentHashMapManager;
    }
}

void MainWindow::start_HttpServer()
{
    if (m_pHttpServerRunningThread)
    {
        m_pHttpServerRunningThread->start();
    }
}

void MainWindow::stop_HttpServer()
{
    if (m_pHttpServerRunningThread)
    {
        m_pHttpServerRunningThread->stop();
    }
}

bool MainWindow::httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject &obj)
{
    return true;
}


void MainWindow::thread_httpserver_callback_Loop(void *param)
{
    m_phttpserver->run();
    stdutils::OriDateTime::sleep(1);
}

ConcurrentHashMapManager *MainWindow::pConcurrentHashMapManager() const
{
    return m_pConcurrentHashMapManager;
}


void MainWindow::drawHex_ex(const TYPE_ULID &uildsrc, const HEXIDX_HGT_ARRAY &hexidxslist, const std::vector<QColor> &clstmp)
{

#ifdef USING_GUI_SHOW
#if 0
    if (m_pModelWidget)
    {
        static std::unordered_map<TYPE_ULID, std::unordered_set<H3INDEX>> polygonlist_;
        if(uildsrc == 0 && hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.begin();
            while(polygonlistitor != polygonlist_.end())
            {
                m_pModelWidget->clearEntityTracking(polygonlistitor->first);
                polygonlistitor->second.clear();
                polygonlistitor = polygonlist_.erase(polygonlistitor);
            }
        }
        else
        {

            if (hexidxslist.empty())
            {
                auto polygonlistitor = polygonlist_.find(uildsrc);
                if (polygonlistitor != polygonlist_.end())
                {
                    std::unordered_set<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        m_pModelWidget->clearEntityTracking(uildsrc+*itor);
                        itor++;
                    }
                    polygonlist_[uildsrc].clear();
                    polygonlist_.erase(polygonlistitor);
                }
            }
            else
            {
                QVector<H3INDEX> _existindex;
                if (polygonlist_.find(uildsrc) != polygonlist_.end())
                {
                    std::unordered_set<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        H3INDEX oldh3index = *itor;
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(), [&](const HEXIDX_HGT_ARRAY::value_type& val) {
                            return oldh3index == val.PARAM_seq_hexidx_element;
                        });
                        if (h3itor == hexidxslist.end())
                        {
                            m_pModelWidget->clearEntityTracking(uildsrc+oldh3index);
                            itor = polygonlist_[uildsrc].erase(itor);
                            continue;
                        }
                        else
                        {
                            _existindex.push_back(oldh3index);
                        }
                        itor++;
                    }
                }
                else
                {
                    polygonlist_.insert(std::make_pair(uildsrc, std::unordered_set<H3INDEX>()));
                }
                HEXIDX_ARRAY hexidxslistTMP;
                hexidxslistTMP.resize(1);
                for(int index= 0; index < hexidxslist.size(); index++)
                {
                    hexidxslistTMP[0] = hexidxslist.at(index).PARAM_seq_hexidx_element;
                    if (!_existindex.contains(hexidxslistTMP[0]))
                    {
                        auto &hgt = hexidxslist.at(index).PARAM_seq_hexidx_hgt;

                        std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                        LocationHelper::getIndexBoundary(boundarys, hexidxslistTMP);
                        for (int m = 0; m < boundarys.size(); m++)
                        {
                            auto boundaryitem = boundarys.at(m);

                            H3INDEX h3index = std::get<0>(boundaryitem);

                            auto polygonlist_itor = polygonlist_.find(uildsrc);
                            if (polygonlist_itor != polygonlist_.end())
                            {
                                auto h3index_itor = polygonlist_itor->second.find(h3index);
                                if (h3index_itor == polygonlist_itor->second.end())
                                {
                                    polygonlist_itor->second.insert(h3index);
                                }
                            }
                            else
                            {
                                polygonlist_[uildsrc].insert(h3index);
                            }
                            CellBoundary boundary = std::get<1>(boundaryitem);
                            std::vector<std::tuple<double, double, double> > poslist;
                            std::vector<std::tuple<QColor,QColor>> cls;
                            std::vector<double> llaList_down;
#if 0
                        poslist.resize(boundary.numVerts+1);
                        llaList_down.resize(1);
                        if(!clstmp.empty())
                        {
                            cls.resize(1);
                        }
                        for (int v = 0; v < boundary.numVerts; v++)
                        {
                            poslist[v] = std::make_tuple(
                                LocationHelper::radianToDegree(boundary.verts[v].lng),
                                LocationHelper::radianToDegree(boundary.verts[v].lat),
                                hgt.PARAM_seq_hexidx_hgt0);
                        }
                        poslist[boundary.numVerts] = std::make_tuple(
                            LocationHelper::radianToDegree(boundary.verts[0].lng),
                            LocationHelper::radianToDegree(boundary.verts[0].lat),
                            hgt.PARAM_seq_hexidx_hgt0);

                        llaList_down[0] = abs(hgt.PARAM_seq_hexidx_hgtn-hgt.PARAM_seq_hexidx_hgt0);
                        if(!clstmp.empty() && index < clstmp.size())
                        {
                            cls[0]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                        }
                        LatLng ret;
                        LocationHelper::doCell(ret, hexidxslistTMP[0]);
                        m_pModelWidget->initNode(uildsrc+h3index,"",2, "", LocationHelper::radianToDegree(ret.lng),LocationHelper::radianToDegree(ret.lat),(hgt.PARAM_seq_hexidx_hgt0+hgt.PARAM_seq_hexidx_hgtn)/2,0,0,0);
#else
                            poslist.resize(boundary.numVerts+1);
                            llaList_down.resize(boundary.numVerts+1);
                            if(!clstmp.empty())
                            {
                                cls.resize(boundary.numVerts+1);
                            }
                            for (int v = 0; v < boundary.numVerts; v++)
                            {
                                poslist[v] = std::make_tuple(
                                    LocationHelper::radianToDegree(boundary.verts[v].lng),
                                    LocationHelper::radianToDegree(boundary.verts[v].lat),
                                    hgt.PARAM_seq_hexidx_hgtn);
                                llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0;
                                if(!clstmp.empty() && index < clstmp.size())
                                {
                                    cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                                }
                            }
                            poslist[boundary.numVerts] = std::make_tuple(
                                LocationHelper::radianToDegree(boundary.verts[0].lng),
                                LocationHelper::radianToDegree(boundary.verts[0].lat),
                                hgt.PARAM_seq_hexidx_hgtn);
                            llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0;

                            if(!clstmp.empty()&& index < clstmp.size())
                            {
                                cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }

                            LatLng ret;
                            LocationHelper::doCell(ret, hexidxslistTMP[0]);
                            m_pModelWidget->initNode(uildsrc+h3index,"",5, "", LocationHelper::radianToDegree(ret.lng),LocationHelper::radianToDegree(ret.lat),(hgt.PARAM_seq_hexidx_hgt0+hgt.PARAM_seq_hexidx_hgtn)/2,0,0,0);

#endif
                            m_pModelWidget->updatePosList(uildsrc+h3index,poslist,std::move(llaList_down),cls);
                        }
                    }
                }
            }
        }
    }
#else

    if (m_pModelWidget)
    {
#if 1
        static std::unordered_map<TYPE_ULID, std::unordered_set<H3INDEX>> polygonlist_;
        if(uildsrc == 0 && hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.begin();
            while(polygonlistitor != polygonlist_.end())
            {
                m_pModelWidget->clearEntityTracking(polygonlistitor->first);
                polygonlistitor->second.clear();
                polygonlistitor = polygonlist_.erase(polygonlistitor);
            }
        }
        else
        {
            if (hexidxslist.empty())
            {
                auto polygonlistitor = polygonlist_.find(uildsrc);
                if (polygonlistitor != polygonlist_.end())
                {
                    m_pModelWidget->clearEntityTracking(uildsrc);
                    polygonlist_[uildsrc].clear();
                    polygonlist_.erase(polygonlistitor);
                }
            }
            else
            {
                bool bChanged = false;
                QVector<H3INDEX> _existindex;
                if (polygonlist_.find(uildsrc) != polygonlist_.end())
                {
                    std::unordered_set<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        H3INDEX oldh3index = *itor;
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(), [&](const HEXIDX_HGT_ARRAY::value_type& val) {
                            return oldh3index == val.PARAM_seq_hexidx_element;
                        });
                        if (h3itor == hexidxslist.end())
                        {
                            bChanged = true;
                            itor = polygonlist_[uildsrc].erase(itor);
                            continue;
                        }
                        else
                        {
                            _existindex.push_back(oldh3index);
                        }
                        itor++;
                    }
                    if(!bChanged)
                    {
                        auto itor = hexidxslist.begin();
                        while (itor != hexidxslist.end())
                        {
                            auto newh3index = *itor;
                            if (!_existindex.contains(newh3index.PARAM_seq_hexidx_element))
                            {
                                bChanged = true;
                                break;
                            }
                            itor++;
                        }
                    }
                }
                else
                {
                    polygonlist_.insert(std::make_pair(uildsrc, std::unordered_set<H3INDEX>()));
                    m_pModelWidget->initNode(uildsrc,"",6, "", 0,0,0,0,0,0);
                    bChanged = true;
                }
                if(!bChanged)
                {
                    return;
                }
                m_pModelWidget->updatePosList(uildsrc,std::vector<std::tuple<double, double, double> > ());
                HEXIDX_ARRAY hexidxslistTMP;
                hexidxslistTMP.resize(1);
                for(int index= 0; index < hexidxslist.size(); index++)
                {
                    hexidxslistTMP[0] = hexidxslist.at(index).PARAM_seq_hexidx_element;
                    if (!_existindex.contains(hexidxslistTMP[0]))
                    {
                        auto &hgt = hexidxslist.at(index).PARAM_seq_hexidx_hgt;

                        std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                        LocationHelper::getIndexBoundary(boundarys, hexidxslistTMP);
                        for (int m = 0; m < boundarys.size(); m++)
                        {
                            auto boundaryitem = boundarys.at(m);

                            H3INDEX h3index = std::get<0>(boundaryitem);

                            auto polygonlist_itor = polygonlist_.find(uildsrc);
                            if (polygonlist_itor != polygonlist_.end())
                            {
                                auto h3index_itor = polygonlist_itor->second.find(h3index);
                                if (h3index_itor == polygonlist_itor->second.end())
                                {
                                    polygonlist_itor->second.insert(h3index);
                                }
                            }
                            else
                            {
                                polygonlist_[uildsrc].insert(h3index);
                            }
                            CellBoundary boundary = std::get<1>(boundaryitem);
                            std::vector<std::tuple<double, double, double> > poslist;

                            std::vector<std::tuple<QColor,QColor>> cls;
                            std::vector<double> llaList_down;
                            poslist.resize(boundary.numVerts+1);
                            llaList_down.resize(boundary.numVerts+1);
                            if(!clstmp.empty())
                            {
                                cls.resize(boundary.numVerts+1);
                            }
                            for (int v = 0; v < boundary.numVerts; v++)
                            {
                                poslist[v] = std::make_tuple(
                                    LocationHelper::radianToDegree(boundary.verts[v].lng),
                                    LocationHelper::radianToDegree(boundary.verts[v].lat),
                                    hgt.PARAM_seq_hexidx_hgtn);
                                llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0;
                                if(!clstmp.empty() && index < clstmp.size())
                                {
                                    cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                                }
                            }
                            poslist[boundary.numVerts] = std::make_tuple(
                                LocationHelper::radianToDegree(boundary.verts[0].lng),
                                LocationHelper::radianToDegree(boundary.verts[0].lat),
                                hgt.PARAM_seq_hexidx_hgtn);
                            llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0;

                            if(!clstmp.empty() && index < clstmp.size())
                            {
                                cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }

                            m_pModelWidget->updatePosList(uildsrc,poslist,std::move(llaList_down),cls);
                        }
                    }
                }
            }
        }
#else

        static std::unordered_map<TYPE_ULID, std::unordered_map<H3INDEX, UINT64>> polygonlist_;
        if(uildsrc == 0 && hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.begin();
            while(polygonlistitor != polygonlist_.end())
            {
                m_pModelWidget->clearEntityTracking(polygonlistitor->first);
                polygonlistitor->second.clear();
                polygonlistitor = polygonlist_.erase(polygonlistitor);
            }
        }
        else
        {
            if (hexidxslist.empty())
            {
                auto polygonlistitor = polygonlist_.find(uildsrc);
                if (polygonlistitor != polygonlist_.end())
                {
                    std::unordered_map<H3INDEX, UINT64>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        m_pModelWidget->clearEntityTracking(uildsrc+itor->first);
                        itor++;
                    }
                    polygonlist_[uildsrc].clear();
                    polygonlist_.erase(polygonlistitor);
                }
            }
            else
            {
                QVector<H3INDEX> _existindex;
                if (polygonlist_.find(uildsrc) != polygonlist_.end())
                {
                    std::unordered_map<H3INDEX, UINT64>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        H3INDEX oldh3index = itor->first;
                        QColor oldh3indexcl = itor->second;
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(), [&](const HEXIDX_HGT_ARRAY::value_type& val) {
                            return oldh3index == val.PARAM_seq_hexidx_element;
                        });
                        if (h3itor == hexidxslist.end())
                        {
                            m_pModelWidget->clearEntityTracking(uildsrc+oldh3index);
                            itor = polygonlist_[uildsrc].erase(itor);
                            continue;
                        }
                        else
                        {
                            _existindex.push_back(oldh3index);
                        }
                        itor++;
                    }
                }
                else
                {
                    polygonlist_.insert(std::make_pair(uildsrc, std::unordered_map<H3INDEX, UINT64>()));
                    m_pModelWidget->initNode(uildsrc,"",6, "", 0,0,0,0,0,0);

                }
                HEXIDX_ARRAY hexidxslistTMP;
                hexidxslistTMP.resize(1);
                for(int index= 0; index < hexidxslist.size(); index++)
                {
                    hexidxslistTMP[0] = hexidxslist.at(index).PARAM_seq_hexidx_element;
                    auto &hgt = hexidxslist.at(index).PARAM_seq_hexidx_hgt;

                    std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                    LocationHelper::getIndexBoundary(boundarys, hexidxslistTMP);
                    for (int m = 0; m < boundarys.size(); m++)
                    {
                        auto boundaryitem = boundarys.at(m);

                        H3INDEX h3index = std::get<0>(boundaryitem);
                        if (!_existindex.contains(h3index))
                        {
                            UINT64 ID = FunctionAssistant::generate_random_positive_uint64();
                            auto polygonlist_itor = polygonlist_.find(uildsrc);
                            if (polygonlist_itor != polygonlist_.end())
                            {
                                auto h3index_itor = polygonlist_itor->second.find(h3index);
                                if (h3index_itor != polygonlist_itor->second.end())
                                {
                                    h3index_itor->second = ID;
                                }
                                else
                                {
                                    polygonlist_itor->second.insert(std::make_pair(h3index,ID));
                                }
                            }
                            else
                            {
                                polygonlist_[uildsrc].insert(std::make_pair(h3index,ID));
                            }
                            CellBoundary boundary = std::get<1>(boundaryitem);
                            std::vector<std::tuple<double, double, double> > poslist;

                            std::vector<std::tuple<QColor,QColor>> cls;
                            std::vector<double> llaList_down;
                            poslist.resize(boundary.numVerts+1);
                            llaList_down.resize(boundary.numVerts+1);
                            if(!clstmp.empty())
                            {
                                cls.resize(boundary.numVerts+1);
                            }
                            for (int v = 0; v < boundary.numVerts; v++)
                            {
                                poslist[v] = std::make_tuple(
                                    LocationHelper::radianToDegree(boundary.verts[v].lng),
                                    LocationHelper::radianToDegree(boundary.verts[v].lat),
                                    hgt.PARAM_seq_hexidx_hgtn);
                                llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0;
                                if(!clstmp.empty() && index < clstmp.size())
                                {
                                    cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                                }
                            }
                            poslist[boundary.numVerts] = std::make_tuple(
                                LocationHelper::radianToDegree(boundary.verts[0].lng),
                                LocationHelper::radianToDegree(boundary.verts[0].lat),
                                hgt.PARAM_seq_hexidx_hgtn);
                            llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0;

                            if(!clstmp.empty() && index < clstmp.size())
                            {
                                cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }

                            m_pModelWidget->updatePosList(uildsrc,poslist,std::move(llaList_down),cls);
                        }
                    }
                }
            }
        }
#endif
    }
#endif
#endif
}



#include "gaeactor_processor_interface_instance.h"

void MainWindow::dreaw_result(UINT64 task_id, GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance )
{
#ifdef USING_GUI_SHOW
#ifdef USING_GENERATE_SHOWRESULT_INDEPENDENT
    if(_pGaeactorProcessorInterfaceInstance)
    {
        auto buggrt = _pGaeactorProcessorInterfaceInstance->getCellbuffersSensorInfo(false);

#ifdef USING_GENERATE_GEOPOINTS_CONCURRENT
        tbb::parallel_for(tbb::blocked_range<size_t>(0, buggrt.size()),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for (UINT32 index = r.begin(); index != r.end(); ++index)
                              {
                                  auto hexinfo = buggrt.at(index);
#ifdef USING_SHOW_RESULT
                                  if(m_pModelWidget)
                                  {
                                      auto taskinfo = m_pConcurrentHashMapManager->get_task(task_id);
                                      if(taskinfo)
                                      {
                                          m_pConcurrentHashMapManager->prepare_deal_3d(*taskinfo,hexinfo);
                                      }
                                  }

                                  if(m_pMapWidget)
                                  {
                                      auto taskinfo = m_pConcurrentHashMapManager->get_task(task_id);
                                      if(taskinfo)
                                      {
                                          m_pConcurrentHashMapManager->prepare_deal_2d(*taskinfo,hexinfo);
                                      }
                                  }
#endif
                              }
                          });
#else
        for(int index = 0;index < buggrt.size();index++)
        {
            auto hexinfo = buggrt.at(index);
#ifdef USING_SHOW_RESULT
            m_pConcurrentHashMapManager->prepare_deal_3d(hexinfo);

            if(m_pMapWidget)
            {
                m_pConcurrentHashMapManager->prepare_deal_2d(hexinfo);
            }
#endif
        }
#endif
    }
#endif


    if(m_pMapWidget)
    {        
        HEXIDX_HGT_ARRAY _hexidxslisttmp;
        std::unordered_map<H3INDEX, QColor> _HEXIDX_COLOR_LIST;
        auto iitor = m_pConcurrentHashMapManager->m_hexidxslisttmp_concurrent_hash_map.begin();
        while(iitor != m_pConcurrentHashMapManager->m_hexidxslisttmp_concurrent_hash_map.end())
        {
            _HEXIDX_COLOR_LIST.insert(std::make_pair(iitor->first, std::get<1>(iitor->second)));
            _hexidxslisttmp.push_back(std::get<0>(iitor->second));
            iitor++;
        }
        POLYGON_LIST polygonlisttmp;
        TYPE_ULID uildtest = 8888;
        TYPE_ULID uilddsttest = 7777;
        m_pMapWidget->drawDataToHex(uildtest, uilddsttest, uilddsttest, _hexidxslisttmp, polygonlisttmp, Map2dWidget::E_DRAW_TYPE_HEX_POLYGON, _HEXIDX_COLOR_LIST, true);
    }
    if(m_pModelWidget)
    {
        if(m_pConcurrentHashMapManager->m_bshowhex3d)
        {
            auto iitor = m_pConcurrentHashMapManager->m_hexidxslisttmp3d_concurrent_hash_map.begin();
            while(iitor != m_pConcurrentHashMapManager->m_hexidxslisttmp3d_concurrent_hash_map.end())
            {
                drawHex_ex(iitor->first, std::get<0>(iitor->second),std::get<1>(iitor->second));
                iitor++;
            }
        }
        else
        {
            auto iitor = m_pConcurrentHashMapManager->m_hexidxslisttmp3d_concurrent_hash_map.begin();
            while(iitor != m_pConcurrentHashMapManager->m_hexidxslisttmp3d_concurrent_hash_map.end())
            {
                UINT64 ID1 = FunctionAssistant::generate_random_positive_uint64();
                UINT64 ID2 = FunctionAssistant::generate_random_positive_uint64();
                m_pModelWidget->initNode(ID2,"",14, "", 0,0,0,0,0,0,500,QColor(255,0,0,255));
                m_pModelWidget->initNode(ID1,"",15, "", 0,0,0,0,0,0,500,QColor(255,0,0,255));

                std::vector<std::tuple<double, double, double> > poslist;
                std::vector<std::tuple<QColor,QColor>> cls;
                const HEXIDX_HGT_ARRAY &hexidxslist = std::get<0>(iitor->second);
                const std::vector<QColor> &cllist = std::get<1>(iitor->second);

                for(int index= 0; index < hexidxslist.size(); index++)
                {
                    H3INDEX _h3Index = hexidxslist.at(index).PARAM_seq_hexidx_element;

                    LatLng ret;
                    LocationHelper::doCell(ret, _h3Index);

                    LatLng location;
                    location.lng = LocationHelper::radianToDegree(ret.lng);
                    location.lat = LocationHelper::radianToDegree(ret.lat);
                    poslist.push_back(std::make_tuple(location.lng,location.lat,  hexidxslist.at(index).PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgt0));
                    poslist.push_back(std::make_tuple(location.lng,location.lat,  hexidxslist.at(index).PARAM_seq_hexidx_hgt.PARAM_seq_hexidx_hgtn));

                    cls.push_back(std::make_tuple(cllist.at(index), cllist.at(index)));
                    cls.push_back(std::make_tuple(cllist.at(index), cllist.at(index)));
                }

                std::vector<double> llaList_down;

                m_pModelWidget->updatePosList(ID2,poslist,std::move(llaList_down),cls);

                m_pModelWidget->updatePosList(ID1,poslist,std::move(llaList_down),cls);
                iitor++;
            }
        }
    }
    m_pConcurrentHashMapManager->m_hexidxslisttmp_concurrent_hash_map.clear();
    m_pConcurrentHashMapManager->m_hexidxslisttmp3d_concurrent_hash_map.clear();
#endif
}

void MainWindow::deal_result_slot(const QString &jsobj,void * ptr)
{
//    std::cout<<" deal_result_slot "<<jsobj.toStdString()<<std::endl;
    if(jsobj == "resetDisplay")
    {
#ifdef USING_GUI_SHOW
        auto itor = m_drawlineids.begin();
        while(itor != m_drawlineids.end())
        {
            if(m_pMapWidget)
            {
                m_pMapWidget->drawTrackingLine(*itor,std::vector<LAT_LNG>(),QColor());
            }
            if(m_pModelWidget)
            {
                m_pModelWidget->clearEntityTracking(*itor);
            }
            itor++;
        }
        m_drawlineids.clear();

        if(m_pMapWidget)
        {
            HEXIDX_HGT_ARRAY hexidxslisttmp;
            std::unordered_map<H3INDEX, QColor> HEXIDX_COLOR_LIST;
            POLYGON_LIST polygonlisttmp;
            TYPE_ULID uildtest = 8888;
            TYPE_ULID uilddsttest = 7777;
            m_pMapWidget->drawDataToHex(uildtest, uilddsttest, uilddsttest, hexidxslisttmp, polygonlisttmp, Map2dWidget::E_DRAW_TYPE_HEX_POLYGON, HEXIDX_COLOR_LIST, true);
        }
        if(m_pModelWidget)
        {
            if(m_pConcurrentHashMapManager->m_bshowhex3d)
            {
                drawHex_ex(0, HEXIDX_HGT_ARRAY());
            }
            else
            {
                m_pModelWidget->clearAll();
            }
        }
#endif
    }
    else if(jsobj.startsWith("showclearresult"))
    {
        GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance = reinterpret_cast<GAEAPROCESSORINTERFACEINSTANCE_PTR>(ptr);
#ifdef USING_GUI_SHOW
        UINT64 task_id = jsobj.split("_").at(1).toULongLong();
        dreaw_result(task_id, _pGaeactorProcessorInterfaceInstance);
#endif
        if(_pGaeactorProcessorInterfaceInstance)
        {
            m_pConcurrentHashMapManager->reset_processor(_pGaeactorProcessorInterfaceInstance);
        }
//        m_pConcurrentHashMapManager->show_building();

    }
    else if(jsobj.startsWith("showresult"))
    {
        GAEAPROCESSORINTERFACEINSTANCE_PTR _pGaeactorProcessorInterfaceInstance = reinterpret_cast<GAEAPROCESSORINTERFACEINSTANCE_PTR>(ptr);
#ifdef USING_GUI_SHOW
        UINT64 task_id = jsobj.split("_").at(1).toULongLong();
        dreaw_result(task_id, _pGaeactorProcessorInterfaceInstance);
#endif
//        m_pConcurrentHashMapManager->show_building();

    }
    else if(jsobj == "show_building")
    {
//        QString shppath = QCoreApplication::applicationDirPath() + "/data/gzsczt_fwjz_pro_cgcs2000/gz.shp";
//        m_pModelWidget->initNode(7777,"",4, shppath, 0,0,0,0,0,0,1.0f, QColor(ConcurrentHashMapManager::generateRandomNumber(),ConcurrentHashMapManager::generateRandomNumber(),ConcurrentHashMapManager::generateRandomNumber(),255),QColor(ConcurrentHashMapManager::generateRandomNumber(),ConcurrentHashMapManager::generateRandomNumber(),ConcurrentHashMapManager::generateRandomNumber(),32),2);
    }
}

void MainWindow::draw_linestring_slot(tagLineInfo jsobj)
{

#ifdef USING_GUI_SHOW

    double _point_extend_metres = 500.0;
    auto appendline = [&](int index,const std::vector<tagPtLatLngHgtInfo>& _line_latlnghgts,bool bLine = true)
    {
        std::vector<std::tuple<double, double, double> > poslist;
        std::vector<double> llaList_down;
        auto generateExtendSamplingEx = [&](const tagPtLatLngHgtInfo& start, const tagPtLatLngHgtInfo& end)
        {
            LAT_LNG startLatLng = LAT_LNG{start._latlng.lat, start._latlng.lng};
            LAT_LNG endLatLng = LAT_LNG{end._latlng.lat, end._latlng.lng};

            const FLOAT64 &hgt_start = start._hgt;
            const FLOAT64 &hgt_end = end._hgt;

            const FLOAT64 &hgt_up_start = start._hgt_up;
            const FLOAT64 &hgt_up_end = end._hgt_up;

            const FLOAT64 &hgt_down_start = start._hgt_down;
            const FLOAT64 &hgt_down_end = end._hgt_down;

            const FLOAT64 hgt_diff = hgt_end-hgt_start;
            const FLOAT64 hgt_diff_up = hgt_up_end-hgt_up_start;
            const FLOAT64 hgt_diff_down = hgt_down_end-hgt_down_start;
            poslist.push_back(std::make_tuple(start._latlng.lng,
                                              start._latlng.lat,
                                              start._hgt_up));
            llaList_down.push_back(start._hgt_down);
            double dis = FunctionAssistant::calc_dist(startLatLng, endLatLng);
            if (dis > _point_extend_metres)
            {
                int step = dis / _point_extend_metres;
                glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(startLatLng, endLatLng);
                LAT_LNG lstextendpt = startLatLng;
                for (int i = 1; i < step + 1; i++)
                {
                    LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, _point_extend_metres);
                    double step_dif = (i*1.0f / (step+1)*1.0f);
                    FLOAT64 hgt_ = hgt_start + hgt_diff * step_dif;
                    FLOAT64 hgt_up = hgt_up_start + hgt_diff_up * step_dif;
                    FLOAT64 hgt_down = hgt_down_start + hgt_diff_down * step_dif;

                    poslist.push_back(std::make_tuple(currentextendpt.lng,
                                                      currentextendpt.lat,
                                                      hgt_up));
                    llaList_down.push_back(hgt_down);


                    lstextendpt = currentextendpt;
                }
            }
            poslist.push_back(std::make_tuple(end._latlng.lng,
                                              end._latlng.lat,
                                              end._hgt_up));
            llaList_down.push_back(end._hgt_down);
        };

        auto generateExtendSampling = [&](const tagPtLatLngHgtInfo& cur, const tagPtLatLngHgtInfo& prev)
        {

            LAT_LNG latlng = LAT_LNG{cur._latlng.lat, cur._latlng.lng};
            LAT_LNG lstlatlng = LAT_LNG{prev._latlng.lat, prev._latlng.lng};


            const FLOAT64 &hgt_start = prev._hgt;
            const FLOAT64 &hgt_end = cur._hgt;

            const FLOAT64 &hgt_up_start = prev._hgt_up;
            const FLOAT64 &hgt_up_end = cur._hgt_up;

            const FLOAT64 &hgt_down_start = prev._hgt_down;
            const FLOAT64 &hgt_down_end = cur._hgt_down;

            const FLOAT64 hgt_diff = hgt_end-hgt_start;
            const FLOAT64 hgt_diff_up = hgt_up_end-hgt_up_start;
            const FLOAT64 hgt_diff_down = hgt_down_end-hgt_down_start;

            double dis = FunctionAssistant::calc_dist(latlng, lstlatlng);
            if (dis > _point_extend_metres)
            {
                int step = dis / _point_extend_metres;
                glm::dvec3 directionVectorArr = FunctionAssistant::calculateVector(lstlatlng, latlng);
                LAT_LNG lstextendpt = lstlatlng;
                for (int i = 1; i < step + 1; i++)
                {
                    LAT_LNG currentextendpt = FunctionAssistant::calculateDirectionExtendPoint(lstextendpt, directionVectorArr, _point_extend_metres);
                    double step_dif = (i*1.0f / (step+1)*1.0f);
                    FLOAT64 hgt_ = hgt_start + hgt_diff * step_dif;
                    FLOAT64 hgt_up = hgt_up_start + hgt_diff_up * step_dif;
                    FLOAT64 hgt_down = hgt_down_start + hgt_diff_down * step_dif;

                    poslist.push_back(std::make_tuple(currentextendpt.lng,
                                                      currentextendpt.lat,
                                                      hgt_up));
                    llaList_down.push_back(hgt_down);

                    lstextendpt = currentextendpt;
                }
                double rdis = dis - _point_extend_metres * step;


                poslist.push_back(std::make_tuple(cur._latlng.lng,
                                                  cur._latlng.lat,
                                                  cur._hgt_up));
                llaList_down.push_back(cur._hgt_down);
            }
            else
            {
                poslist.push_back(std::make_tuple(cur._latlng.lng,
                                                  cur._latlng.lat,
                                                  cur._hgt_up));
                llaList_down.push_back(cur._hgt_down);
            }
        };

        quint64 trackingid = jsobj._id;
        QColor color = FunctionAssistant::randColor(255);
        color = QColor(255,255,0,255);
        double linewidthScale = 1.0;
        std::vector<LAT_LNG> waypts;
        waypts.reserve(_line_latlnghgts.size());
#ifndef USING_DRAW_LINE_SAMPLING
        poslist.reserve(_line_latlnghgts.size());
        llaList_down.reserve(_line_latlnghgts.size());
#endif
        for(int i = 0; i < _line_latlnghgts.size(); i++)
        {
            waypts.push_back(LAT_LNG{_line_latlnghgts.at(i)._latlng.lat, _line_latlnghgts.at(i)._latlng.lng});
#ifndef USING_DRAW_LINE_SAMPLING
            poslist.push_back(std::make_tuple(_line_latlnghgts.at(i)._latlng.lng,
                                              _line_latlnghgts.at(i)._latlng.lat,
                                              _line_latlnghgts.at(i)._hgt_up));
            llaList_down.push_back(_line_latlnghgts.at(i)._hgt_down);
#endif
        }

#ifdef USING_DRAW_LINE_SAMPLING

        poslist.push_back(std::make_tuple(_line_latlnghgts.at(0)._latlng.lng,
                                          _line_latlnghgts.at(0)._latlng.lat,
                                          _line_latlnghgts.at(0)._hgt_up));
        llaList_down.push_back(_line_latlnghgts.at(0)._hgt_down);



#if 0
        generateExtendSamplingEx(runway_pts.front(), runway_pts.back(), runway_retlist_1);
#else
        for (int i = 1; i < _line_latlnghgts.size(); i++)
        {
            generateExtendSampling(_line_latlnghgts.at(i), _line_latlnghgts.at(i - 1));
        }
#endif
#endif

        if(m_drawlineids.find(trackingid+index) == m_drawlineids.end())
        {
            m_drawlineids.insert(trackingid+index);
        }
        if(m_pMapWidget)
        {
            m_pMapWidget->drawTrackingLine(trackingid+index,waypts,color,linewidthScale);
        }
        if(m_pModelWidget)
        {
            if(bLine)
            {
                //        m_pModelWidget->initNode(trackingid+index,"",12, "", 0,0,0,0,0,0,1.0f, QColor(255,255,0,255),2);
                m_pModelWidget->initNode(trackingid+index,"",11, "", 0,0,0,0,0,0,1.0f, QColor(255,255,0,255),2);
                //        m_pModelWidget->visibleEntityTracking(trackingid+index,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TRACKING_PLANE);
            }
            else
            {
                m_pModelWidget->initNode(trackingid+index,"",6, "", 0,0,0,0,0,0,1.0f, QColor(255,255,0,255),2);
            }
            m_pModelWidget->updatePosList(trackingid+index,poslist,std::move(llaList_down));
        }
    };

    if(jsobj._line_latlnghgts.empty() && !jsobj._polygon_latlnghgts.empty())
    {
        for(int index = 0; index < jsobj._polygon_latlnghgts.size(); index++)
        {
            if(!jsobj._polygon_latlnghgts.at(index).empty())
            {
                appendline(index, jsobj._polygon_latlnghgts.at(index),false);
            }
        }
    }
    else
    {
        appendline(0,jsobj._line_latlnghgts);
    }
#endif
}

