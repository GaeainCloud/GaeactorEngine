#pragma execution_character_set("utf-8")

#include "gaeactor_processor_interface.h"
#include "../components/eventdriver/eventdriver.h"
#include "mapwidget.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QKeyEvent>
#include <QDateTime>
#include <QTimer>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <QOpenGLContext>
#include <QSurface>
#include <QDir>
#include "components/function.h"

#include "./src/OriginalDateTime.h"
#include "../components/gaeactormanager.h"
#include "LocationHelper.h"
#include <h3Index.h>
#include "components/Triangluation.h"
#include <QCoreApplication>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <iostream>
#include <glm/fwd.hpp>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QRect>
//#include "widget2d/maptexturebuffer.h"
//#include "widget2d/maprendererfbo.h"
#include "components/Triangluation.h"
#include "../components/configmanager.h"
#include "settingsconfig.h"
#include "widget3d/QtOsgWidget.h"



#include "src/OriginalThread.h"
#include "runningmodeconfig.h"
//#define DRAW_ARROW
//#define DRAW_HEX
namespace
{
    float vertices[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
    };
}


MapRenderThread::MapRenderThread(QSurface* surface, QOpenGLContext* mainContext, MapWidget* parent)
    : QThread(parent)
    , m_mainContext(mainContext)
    , m_surface(surface)
    , _setDefaultFboId(false)
    , _pause(true)//默认暂停渲染，接收到场景准备好信号之后再启动
    , m_pMapWidget(parent)
{
    m_renderContext = new QOpenGLContext;
    m_renderContext->setFormat(m_mainContext->format());
    m_renderContext->setShareContext(m_mainContext);
    m_renderContext->create();
    m_renderContext->moveToThread(this);

}

QOpenGLContext* MapRenderThread::renderContext() const
{
    return m_renderContext;
}

MapRenderThread::~MapRenderThread()
{
    m_running = false;
    wait();
}

QMutex* MapRenderThread::getRendererMutex()
{
    return &_renderOSGSceneMutex;
}

// called in UI thread
void MapRenderThread::setNewSize(int width, int height)
{
    QMutexLocker lock(&_resizeMutex);
    m_width = width;
    m_height = height;
}


void MapRenderThread::stop()
{
    m_running = false;
}


// called in render thread
void MapRenderThread::run()
{
    //    m_renderContext->makeCurrent(m_surface);

    //    TextureBuffer::instance()->createTexture(m_renderContext);

    //    Renderer renderer(m_renderContext);

    while (m_running)
    {
        //        int width = 512;
        //        int height = 512;
        //        {
        //            QMutexLocker lock(&_resizeMutex);
        //            width = m_width;
        //            height = m_height;
        //        }
        //        renderer.begin();
        //        renderer.render(width, height, m_pMapWidget,m_renderContext);
        //        renderer.end();
        //        renderer.readPixel(0,0);
        //        TextureBuffer::instance()->updateTexture(m_renderContext, width, height);
        //通知GLWidget::update
        emit imageReady();
        stdutils::OriDateTime::sleep(16);
    }

    //TextureBuffer::instance()->deleteTexture(m_renderContext);
}

enum E_HEXINDEX_STATUS:BYTE
{
    E_HEXINDEX_STATUS_FREE = 0x00,
    E_HEXINDEX_STATUS_ENTITY = 0x01,
    E_HEXINDEX_STATUS_SENSOR = 0x02,
    E_HEXINDEX_STATUS_ALL = E_HEXINDEX_STATUS_ENTITY | E_HEXINDEX_STATUS_SENSOR
};

MapWidget::MapWidget(E_MAP_MODE eMapMode, QWidget* parent)
    : QOpenGLWidget(parent),
#ifdef USING_PATH_GENERATE
    m_srccombox(nullptr),
    m_dstcombox(nullptr),
    m_textLabel(nullptr),
#endif
    m_airportombox(nullptr),
    m_maprender(nullptr),
    m_eMapMode(eMapMode),
    m_thread(nullptr),
    m_pGaeactorManager(nullptr),
    m_GaeactorManager_thread(nullptr),
    m_peventDriver(nullptr),
    m_pModelWidget2(nullptr)
{

    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap = false;
    }

    m_arr_text_id = FunctionAssistant::generate_random_positive_uint64();
    m_dep_text_id = FunctionAssistant::generate_random_positive_uint64();
    MapRender::E_RENDER_MODE eRenderMode = MapRender::E_RENDER_MODE_DISPLAY;
    switch (m_eMapMode)
    {
    case E_MAP_MODE_DISPLAY:eRenderMode = MapRender::E_RENDER_MODE_DISPLAY; break;
    case E_MAP_MODE_DISPLAY_REVIEW:eRenderMode = MapRender::E_RENDER_MODE_DISPLAY_REVIEW; break;
    case E_MAP_MODE_SELECT:eRenderMode = MapRender::E_RENDER_MODE_SELECT; break;
    case E_MAP_MODE_SELECT_PATH:eRenderMode = MapRender::E_RENDER_MODE_SELECT_PATH; break;
    default:
        break;
    }
    m_maprender = new MapRender(eRenderMode);
    //    qRegisterMetaType<LAT_LNG>("LAT_LNG");
    setWindowTitle("Gaeactor-Display");
    setFocusPolicy(Qt::StrongFocus);
    QSurfaceFormat format = this->format();
    format.setSamples(8);
    format.setStencilBufferSize(8);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    this->setFormat(format);

    m_pUpdateTimer = new QTimer(this);
    connect(m_pUpdateTimer, &QTimer::timeout, this, [&] {
#ifdef DRAW_HEX
        if (m_eMapMode == E_MAP_MODE_DISPLAY)
        {
#if 0
            auto buggrt = gaeactorenvironment::GaeactorProcessorInterface::getInstance().getCellbuffers();
            HEXIDX_ARRAY hexidxslisttmp;
            int cc1 = 0;
            int cc2 = 0;
            int cc3 = 0;
            std::unordered_map<H3INDEX, QColor> HEXIDX_COLOR_LIST;
            auto itor3 = buggrt.begin();
            while (itor3 != buggrt.end())
            {
                auto hexinfo = *itor3;

                H3INDEX _h3Index = std::get<0>(hexinfo);
                bool _bValid = std::get<1>(hexinfo);
                uint32_t _eHexidexStatus = std::get<2>(hexinfo);
                if (_bValid)
                {
                    QColor cl = QColor(128, 128, 128, 128);
                    switch (_eHexidexStatus)
                    {
                    case E_HEXINDEX_STATUS_FREE:
                    {
                        cl = QColor(128, 128, 128, 128);
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_ENTITY:
                    {
                        cl = QColor(0, 255, 0, 128);
                        cc1++;
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_SENSOR:
                    {
                        cl = QColor(255, 0, 0, 128);
                        cc2++;
                    }
                    break;
                    case E_HEXINDEX_STATUS_ALL:
                    {
                        cl = QColor(255, 255, 0, 128);
                        cc3++;
                    }break;
                    default:
                        break;
                    }
                    HEXIDX_COLOR_LIST.insert(std::make_pair(_h3Index, cl));
                    hexidxslisttmp.push_back(_h3Index);
                }
                itor3++;
            }
            //std::cout << " count " << HEXIDX_COLOR_LIST.size()<<" entity "<< cc1<<" sensor "<< cc2 << " all " << cc3 << std::endl;
#else

            auto buggrt = gaeactorenvironment::GaeactorProcessorInterface::getInstance().getCellbuffersInfo();
            HEXIDX_HGT_ARRAY hexidxslisttmp;
            std::unordered_map<H3INDEX, QColor> HEXIDX_COLOR_LIST;
            auto itor3 = buggrt.begin();
            while (itor3 != buggrt.end())
            {
                auto hexinfo = itor3->second;
                H3INDEX _h3Index = std::get<0>(hexinfo);
                bool _bValid = std::get<1>(hexinfo);
                uint32_t _eHexidexStatus = std::get<2>(hexinfo);
                auto sensorlists = std::get<3>(hexinfo);
                auto entitylists = std::get<4>(hexinfo);
                if (_bValid)
                {
                    QColor cl = QColor(128, 128, 128, 128);
                    switch (_eHexidexStatus)
                    {
                    case E_HEXINDEX_STATUS_FREE:
                    {
                        cl = QColor(128, 128, 128, 128);
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_ENTITY:
                    {
                        cl = QColor(0, 255, 0, 128);
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_SENSOR:
                    {
                        cl = QColor(0, 255, 0, 128);
                        if (sensorlists.size() > 1)
                        {
                            cl = QColor(0, 0, 255, 128);
                            struct tagSensorConflictInfo{
                                std::tuple<TYPE_ULID,TYPE_ULID> m_sensorid;
                                std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> m_sensorlist;
                                transdata_param_seq_hexidx_hgt m_hgtrange;
                            };
                            std::vector<tagSensorConflictInfo> _tagSensorConflictInfos;
                            _tagSensorConflictInfos.reserve(sensorlists.size());
                            for(int j = 0; j < sensorlists.size(); j++)
                            {
                                const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                                const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                                const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                                tagSensorConflictInfo _tagSensorConflictInfo;
                                _tagSensorConflictInfo.m_sensorid = std::make_tuple(agentid, sensingmediaid);
                                _tagSensorConflictInfo.m_sensorlist.push_back(std::make_tuple(agentid, sensingmediaid));
                                _tagSensorConflictInfo.m_hgtrange = hgt;

                                for(int m = 0; m < _tagSensorConflictInfos.size(); m++)
                                {
                                    const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(m).m_sensorid;
                                    std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> &exist_sensorlist = _tagSensorConflictInfos.at(m).m_sensorlist;
                                    const transdata_param_seq_hexidx_hgt& exist_hgt = _tagSensorConflictInfos.at(m).m_hgtrange;
                                    if(!((hgt.PARAM_seq_hexidx_hgt0 < exist_hgt.PARAM_seq_hexidx_hgt0 && hgt.PARAM_seq_hexidx_hgtn < exist_hgt.PARAM_seq_hexidx_hgt0) ||
                                          (hgt.PARAM_seq_hexidx_hgt0 > exist_hgt.PARAM_seq_hexidx_hgtn && hgt.PARAM_seq_hexidx_hgtn > exist_hgt.PARAM_seq_hexidx_hgtn)))
                                    {
                                        /////////////////////////////////////////////////////////////////////
                                        exist_sensorlist.push_back(_tagSensorConflictInfo.m_sensorid);
                                        _tagSensorConflictInfo.m_sensorlist.push_back(exist_sensorid);
                                        /////////////////////////////////////////////////////////////////////
                                        cl = QColor(255, 0, 0, 128);
                                        /////////////////////////////////////////////////////////////////////
                                    }
                                }
                                _tagSensorConflictInfos.push_back(std::move(_tagSensorConflictInfo));
                            }
                        }
                    }
                    break;
                    case E_HEXINDEX_STATUS_ALL:
                    {
                        cl = QColor(255, 255, 0, 128);
                    }break;
                    default:
                        break;
                    }

                    HEXIDX_COLOR_LIST.insert(std::make_pair(_h3Index, cl));
                    hexidxslisttmp.push_back(transdata_param_seq_hexidx{_h3Index, transdata_param_seq_hexidx_hgt{0,0, 0, 0}});
                }
                itor3++;
            }
#endif
            POLYGON_LIST polygonlisttmp;
            TYPE_SENSORINFO sensorinfotmp;
            sensorinfotmp.PARAM_wave_usage = 0x01;
            E_DISPLAY_MODE eDdisplayModetest = E_DISPLAY_MODE_WAVE;
            TYPE_ULID uildtest = 8888;
            TYPE_ULID uilddsttest = 7777;
            drawDataToHex(uildtest, uilddsttest, uilddsttest, hexidxslisttmp, polygonlisttmp, sensorinfotmp, E_DRAW_TYPE_HEX_POLYGON, HEXIDX_COLOR_LIST, true);
        }
        //if (m_ieventcount != 0 ||
        //    m_ieventcount_plus != 0 ||
        //    m_ieventcount_sub != 0)
        //{
        //    std::cout << " eventcount " << m_ieventcount << " add " << m_ieventcount_plus << " sub " << m_ieventcount_sub << " diff " << (m_ieventcount_plus - m_ieventcount_sub) << std::endl;
        //}
#endif
        this->update();
        static uint64_t icc = 0;
        icc++;
        icc %= 60;
        if (icc == 0)
        {
            //std::cout << " event count " << m_ieventcount<<" plus count "<< m_ieventcount_plus<<" sub count "<< m_ieventcount_sub << std::endl;
        }
    });

    m_pUpdateTimer2 = new QTimer(this);
    connect(m_pUpdateTimer2, &QTimer::timeout, this, [&] {
#ifdef DRAW_HEX
        if (m_eMapMode == E_MAP_MODE_DISPLAY)
        {
            auto buggrt = gaeactorenvironment::GaeactorProcessorInterface::getInstance().getCellbuffersInfo();
            std::unordered_map<UINT64,std::tuple<HEXIDX_HGT_ARRAY,std::vector<QColor>>> hexidxslisttmp;
            auto itor3 = buggrt.begin();
            while (itor3 != buggrt.end())
            {
                auto hexinfo = itor3->second;
                H3INDEX _h3Index = std::get<0>(hexinfo);
                bool _bValid = std::get<1>(hexinfo);
                uint32_t _eHexidexStatus = std::get<2>(hexinfo);
                auto sensorlists = std::get<3>(hexinfo);
                auto entitylists = std::get<4>(hexinfo);
                if (_bValid)
                {
                    QColor cl = QColor(128, 128, 128, 128);
                    switch (_eHexidexStatus)
                    {
                    case E_HEXINDEX_STATUS_FREE:
                    {
                        cl = QColor(128, 128, 128, 128);
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_ENTITY:
                    {
                        cl = QColor(0, 255, 0, 128);
                        itor3++;
                        continue;
                    }break;
                    case E_HEXINDEX_STATUS_SENSOR:
                    {
                        cl = QColor(0, 255, 0, 128);
                        if (sensorlists.size() > 1)
                        {
                            cl = QColor(0, 0, 255, 128);
                        }


                        auto appenditem =[&hexidxslisttmp](const TYPE_ULID& uildsrc,const H3INDEX& h3Index, const transdata_param_seq_hexidx_hgt& hgt, const QColor& cl)
                        {
                            if(hexidxslisttmp.find(uildsrc) == hexidxslisttmp.end())
                            {
                                hexidxslisttmp.insert(std::make_pair(uildsrc,std::make_tuple(HEXIDX_HGT_ARRAY(),std::vector<QColor>())));
                            }
                            HEXIDX_HGT_ARRAY &hexidxslist = std::get<0>(hexidxslisttmp.at(uildsrc));
                            std::vector<QColor> &vcl = std::get<1>(hexidxslisttmp.at(uildsrc));
                            hexidxslist.push_back(transdata_param_seq_hexidx{h3Index, hgt});
                            vcl.push_back(cl);
                        };

                        if(sensorlists.size() == 1)
                        {
                            const TYPE_ULID& agentid = std::get<0>(sensorlists.at(0));
                            const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(0));
                            const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(0));
                            TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(agentid) ^ (std::hash<TYPE_ULID>()(sensingmediaid) << 1);
                            appenditem(uildsrc, _h3Index, hgt, cl);
                        }
                        else
                        {
                            struct tagSensorConflictInfo{
                                std::tuple<TYPE_ULID,TYPE_ULID> m_sensorid;
                                std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> m_sensorlist;
                                transdata_param_seq_hexidx_hgt m_hgtrange;
                                QColor m_cl;
                            };
                            std::vector<tagSensorConflictInfo> _tagSensorConflictInfos;
                            _tagSensorConflictInfos.reserve(sensorlists.size());
                            for(int j = 0; j < sensorlists.size(); j++)
                            {
                                const TYPE_ULID& agentid = std::get<0>(sensorlists.at(j));
                                const TYPE_ULID& sensingmediaid = std::get<1>(sensorlists.at(j));
                                const transdata_param_seq_hexidx_hgt& hgt = std::get<2>(sensorlists.at(j));

                                tagSensorConflictInfo _tagSensorConflictInfo;
                                _tagSensorConflictInfo.m_sensorid = std::make_tuple(agentid, sensingmediaid);
                                _tagSensorConflictInfo.m_sensorlist.push_back(std::make_tuple(agentid, sensingmediaid));
                                _tagSensorConflictInfo.m_hgtrange = hgt;
                                _tagSensorConflictInfo.m_cl = QColor(0, 0, 255, 128);

                                for(int m = 0; m < _tagSensorConflictInfos.size(); m++)
                                {
                                    const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(m).m_sensorid;
                                    QColor & exist_cl = _tagSensorConflictInfos.at(m).m_cl;
                                    std::vector<std::tuple<TYPE_ULID,TYPE_ULID>> &exist_sensorlist = _tagSensorConflictInfos.at(m).m_sensorlist;
                                    const transdata_param_seq_hexidx_hgt& exist_hgt = _tagSensorConflictInfos.at(m).m_hgtrange;
                                    if(!((hgt.PARAM_seq_hexidx_hgt0 < exist_hgt.PARAM_seq_hexidx_hgt0 && hgt.PARAM_seq_hexidx_hgtn < exist_hgt.PARAM_seq_hexidx_hgt0) ||
                                        (hgt.PARAM_seq_hexidx_hgt0 > exist_hgt.PARAM_seq_hexidx_hgtn && hgt.PARAM_seq_hexidx_hgtn > exist_hgt.PARAM_seq_hexidx_hgtn)))
                                    {
                                        /////////////////////////////////////////////////////////////////////
                                        exist_sensorlist.push_back(_tagSensorConflictInfo.m_sensorid);
                                        _tagSensorConflictInfo.m_sensorlist.push_back(exist_sensorid);
                                        /////////////////////////////////////////////////////////////////////
                                        exist_cl = QColor(255, 0, 0, 128);
                                        _tagSensorConflictInfo.m_cl = exist_cl;
                                        cl = QColor(255, 0, 0, 128);
                                        /////////////////////////////////////////////////////////////////////
                                    }
                                }
                                _tagSensorConflictInfos.push_back(std::move(_tagSensorConflictInfo));
                            }

                            for(int j = 0; j < _tagSensorConflictInfos.size(); j++)
                            {
                                const std::tuple<TYPE_ULID,TYPE_ULID>& exist_sensorid = _tagSensorConflictInfos.at(j).m_sensorid;
                                QColor & cl = _tagSensorConflictInfos.at(j).m_cl;
                                const transdata_param_seq_hexidx_hgt& hgt = _tagSensorConflictInfos.at(j).m_hgtrange;
                                TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(std::get<0>(exist_sensorid)) ^ (std::hash<TYPE_ULID>()(std::get<1>(exist_sensorid)) << 1);
                                appenditem(uildsrc, _h3Index, hgt, cl);
                            }
                        }

                    }
                    break;
                    case E_HEXINDEX_STATUS_ALL:
                    {
                        cl = QColor(255, 255, 0, 128);
                    }break;
                    default:
                        break;
                    }
                }
                itor3++;
            }
            auto iitor = hexidxslisttmp.begin();
            while(iitor != hexidxslisttmp.end())
            {
                drawHex_ex(iitor->first, std::get<0>(iitor->second),std::get<1>(iitor->second));
                iitor++;
            }
        }
#endif
    });
    m_pUpdateTimer2->start(5000);
    bPressed = false;


    setMouseTracking(true);

    srand(time(0));

    m_snowflake.setHostId(0x999);
    m_snowflake.setWorkerId(0x999);
#ifdef USING_PATH_GENERATE
    m_routeid = m_snowflake.GetId();
    m_routesrcid = m_snowflake.GetId();
    m_routedstid = m_snowflake.GetId();
#endif


    m_hDataRequsetThread = new stdutils::OriThread(std::bind(&MapWidget::data_deal_thread_func, this, std::placeholders::_1), nullptr);

    m_hDataRequsetThread->start();

    if (m_eMapMode == E_MAP_MODE_DISPLAY)
    {
        m_pGaeactorManagerHelper = new GaeactorManagerHelper(this);
        m_pGaeactorManager = new GaeactorManager(GaeactorManager::E_RUNNING_MODE_REALTIME);
        m_GaeactorManager_thread = new QThread();
        m_pGaeactorManager->moveToThread(m_GaeactorManager_thread);
        if (m_pGaeactorManager)
        {

            m_pGaeactorManagerHelper->registDisplayCallback(std::bind(&MapWidget::displayHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
            m_pGaeactorManagerHelper->registDisplayPosCallback(std::bind(&MapWidget::displayHexidxPosCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            m_pGaeactorManagerHelper->registIntersectionDisplayCallback(std::bind(&MapWidget::displayIntersectionHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            m_pGaeactorManagerHelper->registEchoWaveDisplayCallback(std::bind(&MapWidget::displayEchoWaveHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
            m_pGaeactorManagerHelper->registEventlistUpdateCallback(std::bind(&MapWidget::dealeventlist_update_callback, this, std::placeholders::_1, std::placeholders::_2));
            m_pGaeactorManagerHelper->registPathUpdateCallback(std::bind(&MapWidget::dealpath_update_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            m_pGaeactorManagerHelper->registSensorUpdateCallback(std::bind(&MapWidget::dealsensor_update_callback, this, std::placeholders::_1, std::placeholders::_2));
            m_pGaeactorManagerHelper->registAgentrelationUpdateCallback(std::bind(&MapWidget::dealAgentRelationInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registAgentCommSnrUpdateCallback(std::bind(&MapWidget::dealAgentCommSnrInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registAgentCommStackUpdateCallback(std::bind(&MapWidget::dealAgentCommStackInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registSmdInfoUpdateCallback(std::bind(&MapWidget::dealSmdInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registPrejdugementUpdateCallback(std::bind(&MapWidget::dealPrejdugement_update_callback, this, std::placeholders::_1));

            connect(m_pGaeactorManager, &GaeactorManager::dealtransformSig, m_pGaeactorManagerHelper, &GaeactorManagerHelper::dealtransformSlot);
        }

#ifdef USING_PATH_GENERATE
        m_srccombox = new QComboBox(this);
        m_dstcombox = new QComboBox(this);

        m_textLabel = new QLabel(this);
        m_textLabel->setStyleSheet("color: white;font-size:10pt");
        m_textLabel->setGeometry(0, 10, 2480, 32);
        m_srccombox->setGeometry(m_textLabel->geometry().left(), m_textLabel->geometry().bottom() + 10, 240, 32);
        m_dstcombox->setGeometry(m_textLabel->geometry().left(), m_srccombox->geometry().bottom() + 10, 240, 32);

        connect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
        connect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
#endif
        m_GaeactorManager_thread->start(QThread::HighestPriority);
    }
    else if (m_eMapMode == E_MAP_MODE_DISPLAY_REVIEW)
    {

        m_pGaeactorManagerHelper = new GaeactorManagerHelper(this);
        m_pGaeactorManager = new GaeactorManager(GaeactorManager::E_RUNNING_MODE_REVIEW);
        m_GaeactorManager_thread = new QThread();
        m_pGaeactorManager->moveToThread(m_GaeactorManager_thread);
        if (m_pGaeactorManager)
        {
            m_pGaeactorManagerHelper->registDisplayCallback(std::bind(&MapWidget::displayHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
            m_pGaeactorManagerHelper->registDisplayPosCallback(std::bind(&MapWidget::displayHexidxPosCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            m_pGaeactorManagerHelper->registIntersectionDisplayCallback(std::bind(&MapWidget::displayIntersectionHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
            m_pGaeactorManagerHelper->registEchoWaveDisplayCallback(std::bind(&MapWidget::displayEchoWaveHexidxCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
            m_pGaeactorManagerHelper->registEventlistUpdateCallback(std::bind(&MapWidget::dealeventlist_update_callback, this, std::placeholders::_1, std::placeholders::_2));
            m_pGaeactorManagerHelper->registPathUpdateCallback(std::bind(&MapWidget::dealpath_update_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            m_pGaeactorManagerHelper->registSensorUpdateCallback(std::bind(&MapWidget::dealsensor_update_callback, this, std::placeholders::_1, std::placeholders::_2));
            m_pGaeactorManagerHelper->registAgentrelationUpdateCallback(std::bind(&MapWidget::dealAgentRelationInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registAgentCommSnrUpdateCallback(std::bind(&MapWidget::dealAgentCommSnrInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registAgentCommStackUpdateCallback(std::bind(&MapWidget::dealAgentCommStackInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registSmdInfoUpdateCallback(std::bind(&MapWidget::dealSmdInfo_update_callback, this, std::placeholders::_1));
            m_pGaeactorManagerHelper->registPrejdugementUpdateCallback(std::bind(&MapWidget::dealPrejdugement_update_callback, this, std::placeholders::_1));

            connect(m_pGaeactorManager, &GaeactorManager::dealtransformSig, m_pGaeactorManagerHelper, &GaeactorManagerHelper::dealtransformSlot);
        }
        m_GaeactorManager_thread->start(QThread::HighestPriority);
    }


    m_airportombox = new QComboBox(this);
    m_airportombox->setGeometry(10, 10, 480, 32);

    auto airport_codes = DataManager::getInstance().getAirPortList();

    QStringList airportshownamelist;
    for (auto item : airport_codes)
    {
        auto it = std::find_if(DataManager::getInstance().getAirPortNameList().begin(),
            DataManager::getInstance().getAirPortNameList().end(), [&item](const std::unordered_map<QString, std::tuple<QString, QString>>::value_type& vt) {
            return vt.first == item;
        });
        if (it != DataManager::getInstance().getAirPortNameList().end())
        {
            airportshownamelist.push_back(std::get<1>(it->second) + "_" + item);
        }
    }

    m_airportombox->addItems(airportshownamelist);
    connect(m_airportombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, [&](const QString& selectcontext) {

        QString context = selectcontext.split("_").at(0);
        QString airport_codecontext;
        auto it = std::find_if(DataManager::getInstance().getAirPortNameList().begin(),
            DataManager::getInstance().getAirPortNameList().end(), [&context](const std::unordered_map<QString, std::tuple<QString, QString>>::value_type& vt) {
            return std::get<1>(vt.second) == context;
        });
        if (it != DataManager::getInstance().getAirPortNameList().end())
        {
            airport_codecontext = it->first;
            tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo(airport_codecontext);
            if (ptagAirPortInfo)
            {
                emit select_airport_sig(airport_codecontext);
                m_maprender->rtesetZoom();
                projectionmercator::GeoLatLngRect targetArea(ptagAirPortInfo->topleft,ptagAirPortInfo->bottomright);
                m_maprender->scaleTo(targetArea);

                //m_maprender->moveTo(LAT_LNG(ptagAirPortInfo->center.lat, ptagAirPortInfo->center.lng));
                //auto lat = m_maprender->screenCenterToLatLng();
                m_maprender->requestTiles();
            }
        }
    });
    //	m_airportombox->setCurrentIndex(0);
    //	m_airportombox->setCurrentText(DataManager::getInstance().getCurrentAirport());
}

MapWidget::~MapWidget()
{
    if (mapshader)
    {
        delete mapshader;
    }

    if (m_lineelementshader)
    {
        delete m_lineelementshader;
    }

    if (m_trielementshader)
    {
        delete m_trielementshader;
    }

    //    if (m_thread&&m_thread->isRunning())
    //    {
    //        m_thread->stop();
    //        m_thread->quit();
    //        m_thread->wait();
    //        //因为线程用到了QWaitCondition阻塞，所以这里不能wait，否则程序无法完全退出
    //        m_thread->terminate();
    //        //m_thread->wait();
    //        delete m_thread;
    //    }

    if (m_GaeactorManager_thread)
    {
        m_GaeactorManager_thread->quit();
        m_GaeactorManager_thread->wait();
        m_GaeactorManager_thread->deleteLater();
    }

    if (m_pGaeactorManager)
    {
        m_pGaeactorManager->deleteLater();
    }
}

void MapWidget::render(QOpenGLContext* _renderContext)
{
    m_maprender->render(_renderContext, m_lineelementshader, m_trielementshader, mapshader, m_pTextshader, m_lineubouniform, m_triubouniform);
}

void MapWidget::clearGeoData()
{
    auto itor = m_geoDrawItems.begin();
    while (itor != m_geoDrawItems.end())
    {
        if (m_maprender)
        {
            m_maprender->clearElementData(itor->second, itor->first);
        }
        itor++;
    }
    m_geoDrawItems.clear();

    auto _geoDrawEntityItemsitor = m_geoDrawEntityItems.begin();
    while (_geoDrawEntityItemsitor != m_geoDrawEntityItems.end())
    {
        if (m_maprender)
        {
            m_maprender->clearEntityItem(*_geoDrawEntityItemsitor);
        }
        _geoDrawEntityItemsitor++;
    }
    m_geoDrawEntityItems.clear();


}

#define DRAW_AIRPORT_POLYGONS
#define DRAW_AIRPORT_LINES
#define DRAW_AIRPORT_POINTS


void MapWidget::drawLatLngToHex(const std::vector<LAT_LNG>& originLatLngs, const QColor& color1, bool bDrawDynamic, int32_t res)
{
    uint64_t item_id;
    bool bNeedTrans84GC = true;
    std::vector <LatLng> latlngs_boundary;
    latlngs_boundary.resize(originLatLngs.size());
    for (int i = 0; i < originLatLngs.size(); i++)
    {
        latlngs_boundary[i].lat = LocationHelper::degreeToRadian(originLatLngs.at(i).lat);
        latlngs_boundary[i].lng = LocationHelper::degreeToRadian(originLatLngs.at(i).lng);
    }

    HEXIDX_ARRAY hexidxslist;
    if (res == -1)
    {
        LocationHelper::getPolygonIndex(hexidxslist, latlngs_boundary, INDEX_MAX_SIZE);
    }
    else
    {
        LocationHelper::getPolygonResulutionIndex(hexidxslist, latlngs_boundary, res);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
    QVector<H3INDEX> _existindex;
    std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys = LocationHelper::getIndexBoundary(hexidxslist);
    for (auto boundaryitem : boundarys)
    {
        H3INDEX h3index = std::get<0>(boundaryitem);
        if (!_existindex.contains(h3index))
        {
            //polygonlist_[uildsrc].push_back(h3index);
            CellBoundary boundary = std::get<1>(boundaryitem);
            std::vector<LAT_LNG> geoPosList;
            for (int v = 0; v < boundary.numVerts; v++)
            {
                geoPosList.push_back(LAT_LNG{ LocationHelper::radianToDegree(boundary.verts[v].lat),LocationHelper::radianToDegree(boundary.verts[v].lng) });
            }
            if (!geoPosList.empty())
            {
                QColor color = color1;
                item_id = FunctionAssistant::generate_random_positive_uint64();
                if (bDrawDynamic)
                {
                    m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_POLYGON_MULTI));
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, item_id, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                color.setAlpha(128);
                item_id = FunctionAssistant::generate_random_positive_uint64();
                if (bDrawDynamic)
                {
                    m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_STRIP_MULTI));
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, item_id, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
            }
        }
    }
#else

    if (hexidxslist.empty())
    {
        if (bDrawDynamic)
        {
            auto itor = m_geoDrawItems.begin();
            while (itor != m_geoDrawItems.end())
            {
                if (m_maprender && itor->second == DrawItem::ENUM_TYPE_POINTS_MULTI)
                {
                    m_maprender->clearElementData(itor->second, itor->first);
                    itor = m_geoDrawItems.erase(itor);
                    continue;
                }
                itor++;
            }
        }
    }
    std::vector<LAT_LNG> geoPosList;
    geoPosList.resize(hexidxslist.size());
    for (int i = 0; i < hexidxslist.size(); i++)
    {
        H3INDEX h3index = hexidxslist.at(i);
        LatLng ptt;
        LocationHelper::doCell(ptt, h3index);
        geoPosList[i].lat = LocationHelper::radianToDegree(ptt.lat);
        geoPosList[i].lng = LocationHelper::radianToDegree(ptt.lng);
    }
    if (!geoPosList.empty())
    {
        QColor color(255, 0, 255, 255);
        item_id = FunctionAssistant::generate_random_positive_uint64();
        if (bDrawDynamic)
        {
            m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_POINTS_MULTI));
        }
        m_maprender->updateElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, item_id, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
    }
#endif
}

void MapWidget::data_deal_thread_func(void* pParam)
{
    m_dealfullCond.wait(&m_dealmutex);
    QList<TYPE_ULID> _requestList;
    {
        QMutexLocker locker(&m_requestListmutex);
        _requestList = std::move(m_requestList);
    }
    for (auto item : _requestList)
    {
        DataManager::getInstance().requestEntityIcon(item);
    }
    stdutils::OriDateTime::sleep(1);
}

void MapWidget::drawGeoData(const GeoJsonInfos& geoinfos, bool bDrawExtendArea, bool bDetail, bool bDrawDynamic)
{
    QColor cl = FunctionAssistant::randColor(64);
    QColor cl2(0, 0, 255, 255);
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }

    cl = FunctionAssistant::randColor(128);
    //		cl2 = randColor(128);
    cl2 = cl;
    uint64_t item_id;
    for (auto subcoordinates : geoinfos.subItem)
    {
        switch (geoinfos.type) {
        case E_GEOTYPE_POINT:
        {
#ifdef DRAW_AIRPORT_POINTS
            for (auto subcoordinatessub : subcoordinates.coordinates)
            {
                for (auto subcoordinatesitem : subcoordinatessub)
                {
                    item_id = FunctionAssistant::generate_random_positive_uint64();
                    if (bDrawDynamic)
                    {
                        m_geoDrawEntityItems.push_back(item_id);
                    }
                    m_maprender->appendEntityItem(item_id, subcoordinatesitem, 0, 0, 0, AGENT_ENTITY_PROPERTY_SENSOR_SPACE, 10.0, bNeedTrans84GC);
                    //                        std::vector<LAT_LNG> trackinglines;
                    //                        trackinglines.push_back(subcoordinatesitem);
                    //                        m_maprender->updateElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, cc++, trackinglines, subcoordinates.m_color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE));

                    if (bDetail)
                    {
                        HEXIDX_ARRAY hexidxslist;
                        H3INDEX h3index;
                        LocationHelper::getIndexInfo(h3index, subcoordinatesitem.lat, subcoordinatesitem.lng, INDEX_MAPPING_RESOLUTION_POINT_POS);
                        hexidxslist.push_back(h3index);

                        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        QVector<H3INDEX> _existindex;
                        std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                        LocationHelper::getIndexBoundary(boundarys, hexidxslist);
                        for (auto boundaryitem : boundarys)
                        {
                            H3INDEX h3index = std::get<0>(boundaryitem);
                            if (!_existindex.contains(h3index))
                            {
                                CellBoundary boundary = std::get<1>(boundaryitem);
                                std::vector<LAT_LNG> geoPosList;
                                geoPosList.resize(boundary.numVerts);
                                for (int v = 0; v < boundary.numVerts; v++)
                                {
                                    geoPosList[v].lat = LocationHelper::radianToDegree(boundary.verts[v].lat);
                                    geoPosList[v].lng = LocationHelper::radianToDegree(boundary.verts[v].lng);
                                }
                                if (!geoPosList.empty())
                                {
                                    QColor color(0, 0, 255, 64);
                                    item_id = FunctionAssistant::generate_random_positive_uint64();
                                    if (bDrawDynamic)
                                    {
                                        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_POLYGON_MULTI));
                                    }
                                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, item_id, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);

                                    color = QColor(0, 0, 255, 128);
                                    item_id = FunctionAssistant::generate_random_positive_uint64();
                                    if (bDrawDynamic)
                                    {
                                        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_STRIP_MULTI));
                                    }
                                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, item_id, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                                }
                            }
                        }
                    }
                }
            }
#endif
        }
        break;
        case E_GEOTYPE_LINE:
        {
#ifdef DRAW_AIRPORT_LINES

            {
                std::vector<LAT_LNG> trackinglines;
                for (auto subcoordinatessub : subcoordinates.coordinates)
                {
                    trackinglines.reserve(trackinglines.size() + subcoordinatessub.size());
                    for (auto subcoordinatesitem : subcoordinatessub)
                    {
                        trackinglines.push_back(subcoordinatesitem);
                    }
                }

                if (!trackinglines.empty())
                {
                    item_id = FunctionAssistant::generate_random_positive_uint64();
                    if (bDrawDynamic)
                    {
                        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_STRIP_MULTI));
                    }
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, item_id, trackinglines, subcoordinates.m_color, DrawElements::E_ELEMENT_TYPE_ROUTE, false, bNeedTrans84GC);
                }
            }
            {
                bool bShowArea = false;
                bool bdisplaced_threshold = false;
                if (bDrawExtendArea)
                {
                    if (subcoordinates.m_tags.contains("aeroway"))
                    {
                        if (subcoordinates.m_tags.value("aeroway") == "runway" ||
                            subcoordinates.m_tags.value("aeroway") == "stopbar" ||
                            subcoordinates.m_tags.value("aeroway") == "landingpoint" ||
                            subcoordinates.m_tags.value("aeroway") == "holdingpoint" ||
                            subcoordinates.m_tags.value("aeroway") == "transrunway" ||
                            subcoordinates.m_tags.value("aeroway") == "vacaterunway" ||
                            subcoordinates.m_tags.value("aeroway") == "tr_stopbar" ||
                            subcoordinates.m_tags.value("aeroway") == "tr_checkbar" ||
                            subcoordinates.m_tags.value("aeroway") == "tr_checkbar_stopbar")
                        {
                            if (subcoordinates.m_tags.value("aeroway") == "runway" && subcoordinates.m_tags.contains("runway") && subcoordinates.m_tags.value("runway") == "displaced_threshold")
                            {
                                bdisplaced_threshold = true;
                            }
                            bShowArea = true;
                        }
                        else if (subcoordinates.m_tags.value("aeroway") == "runway_extend")
                        {
                            bShowArea = true;
                        }
                        else if (subcoordinates.m_tags.value("aeroway") == "taxiway")
                        {
                        }
                        else if (subcoordinates.m_tags.value("aeroway") == "parking_position")
                        {
                        }
                    }
                }
                if (bShowArea)
                {
                    for (auto coordinatesExtenditem : subcoordinates.coordinatesExtend)
                    {
                        std::vector<LAT_LNG>& subcoordinates = std::get<0>(coordinatesExtenditem);

                        if (bDetail)
                        {
                            QColor color1 = FunctionAssistant::randColor(64);
                            drawLatLngToHex(subcoordinates, color1, bDrawDynamic, INDEX_MAPPING_RESOLUTION_AREA_POS);
                        }
                        QColor& clarea = std::get<1>(coordinatesExtenditem);
                        QColor areacl = QColor(clarea.red(), clarea.green(), clarea.blue(), 128);

                        if (bdisplaced_threshold)
                        {
                            areacl = QColor(clarea.red(), clarea.green(), clarea.blue(), 64);
                        }

                        std::tuple< std::vector<TRIANGLE>, std::vector<LAT_LNG>> vals;
                        if (subcoordinates.size() >= 5)
                        {
                            vals = TriangluationHelper::generateTriangleByTriangle(subcoordinates);
                        }

                        std::vector<TRIANGLE>& out_triangles_index = std::get<0>(vals);
                        std::vector<LAT_LNG>& out_points_data = std::get<1>(vals);
                        //                        std::vector< std::vector<LAT_LNG>> result;

#if 0
                        std::vector<LAT_LNG> out_points;
                        out_points.reserve(out_triangles_index.size() * 3);
                        for (size_t i = 0; i < out_triangles_index.size(); i++)
                        {
                            out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                            out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                            out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                            //                    std::vector<LAT_LNG> out_pointsTmp;
                            //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                            //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                            //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                            //                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, cc++, out_pointsTmp, cl3,(DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE));
                        }
                        //                        result.emplace_back(std::move(out_points));
                        //                        for (auto it : result)
                        {
                            item_id = FunctionAssistant::generate_random_positive_uint64();
                            if (bDrawDynamic)
                            {
                                m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_TRIGAGLES_MULTI));
                            }
                            //							m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, it, areacl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                            m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, out_points, areacl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                        }
#else
                        item_id = FunctionAssistant::generate_random_positive_uint64();
                        if (bDrawDynamic)
                        {
                            m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_TRIGAGLES_MULTI));
                        }
                        m_maprender->updateElementIndexAndData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, out_triangles_index, out_points_data, areacl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);

                        //for (size_t i = 0; i < out_triangles_index.size(); i++)
                        //{
                        //    std::vector<LAT_LNG> out_pointsTmp;
                        //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                        //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                        //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                        //    item_id = FunctionAssistant::generate_random_positive_uint64();
                        //    if (bDrawDynamic)
                        //    {
                        //        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_LOOP_MULTI));
                        //    }
                        //    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, item_id, out_pointsTmp, cl3, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                        //}
#endif
                        item_id = FunctionAssistant::generate_random_positive_uint64();
                        if (bDrawDynamic)
                        {
                            m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_STRIP_MULTI));
                        }
                        m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, item_id, subcoordinates, areacl, DrawElements::E_ELEMENT_TYPE_ROUTE, false, bNeedTrans84GC);
                    }
                }
            }
#endif
        }
        break;
        case E_GEOTYPE_POLYGON:
        case E_GEOTYPE_MULITPOLYGON:
        {
#ifdef DRAW_AIRPORT_POLYGONS
            if (!subcoordinates.name.isEmpty())
            {
                //m_maprender->drawText(cc++,subcoordinates.center, QColor(0, 0, 255), subcoordinates.name, 1, false);
            }
            for (auto listitem : subcoordinates.coordinates)
            {
                if (bDetail)
                {
                    //QColor color1 =  QColor(0, 0, 255, 64);
                    QColor color1 = FunctionAssistant::randColor(64);
                    drawLatLngToHex(listitem, color1, bDrawDynamic);
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 1
                std::tuple< std::vector<TRIANGLE>, std::vector<LAT_LNG>> vals;
                if (listitem.size() >= 5)
                {
                    vals = TriangluationHelper::generateTriangleByTriangle(listitem);
                }

                std::vector<TRIANGLE>& out_triangles_index = std::get<0>(vals);
                std::vector<LAT_LNG>& out_points_data = std::get<1>(vals);
                std::vector< std::vector<LAT_LNG>> result;
                cl = subcoordinates.m_color;//FunctionAssistant::randColor(128);
#if 0
                std::vector<LAT_LNG> out_points;
                out_points.reserve(out_triangles_index.size() * 3);
                for (size_t i = 0; i < out_triangles_index.size(); i++)
                {
                    out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                    out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                    out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                    //                    std::vector<LAT_LNG> out_pointsTmp;
                    //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                    //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                    //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                    //                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, item_id, out_pointsTmp, cl3,(DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE),false,bNeedTrans84GC);
                }
                //result.emplace_back(std::move(out_points));
                //for (auto it : result)
                {
                    item_id = FunctionAssistant::generate_random_positive_uint64();
                    if (bDrawDynamic)
                    {
                        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_TRIGAGLES_MULTI));
                    }
                    //m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, it, cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, out_points, cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                }
#else
                item_id = FunctionAssistant::generate_random_positive_uint64();
                if (bDrawDynamic)
                {
                    m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_TRIGAGLES_MULTI));
                }
                m_maprender->updateElementIndexAndData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, item_id, out_triangles_index, out_points_data, cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);

                //for (size_t i = 0; i < out_triangles_index.size(); i++)
                //{
                //    std::vector<LAT_LNG> out_pointsTmp;
                //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                //    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                //    item_id = FunctionAssistant::generate_random_positive_uint64();
                //    if (bDrawDynamic)
                //    {
                //        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_LOOP_MULTI));
                //    }
                //    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, item_id, out_pointsTmp, cl3, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                //}
#endif
#else

                auto vals = TriangluationHelper::generateTriangleByGPC(listitem);

                for (auto it : vals)
                {
                    item_id = FunctionAssistant::generate_random_positive_uint64();
                    if (bDrawDynamic)
                    {
                        m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_TRIANGLE_STRIP_MULTI));
                    }
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIANGLE_STRIP_MULTI, item_id, it, cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                    //item_id = FunctionAssistant::generate_random_positive_uint64();
                    //if (bDrawDynamic)
                    //{
                    //	m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_LOOP_MULTI));
                    //}
                    //m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, item_id, it, cl3,(DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE),false,bNeedTrans84GC);
                }
#endif
                item_id = FunctionAssistant::generate_random_positive_uint64();
                if (bDrawDynamic)
                {
                    m_geoDrawItems.insert(std::make_pair(item_id, DrawItem::ENUM_TYPE_LINES_STRIP_MULTI));
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, item_id, listitem, cl2, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
            }
#endif
        }
        break;
        default:
            break;
        }
    }
}

void MapWidget::drawRader(const UINT64& item_id,const LAT_LNG& center, const double& startangle_, const double& spanangle_, const double& radius, const double& rotationSpeed, QColor color)
{
    double startangle_c = int(startangle_) / 360;
    double startangle = startangle_ - startangle_c * 360;

    double x_spanangle = spanangle_;
    if (spanangle_ > 360)
    {
        double spanangle_c = int(spanangle_) / 360;
        x_spanangle = spanangle_ - spanangle_c * 360;
    }
    double spanangle = x_spanangle /2;
    if (fabs(x_spanangle - 360.0) < EPSILON_7)
    {
        spanangle= x_spanangle;
    }

    bool bNeedTrans84GC = false;
    const int numSegments = 360;
    QVector<QPointF> _verts;
    _verts.push_back(QPointF(0, 0));
    float angleStep = spanangle / numSegments;
    for (int i = 0; i < numSegments; ++i)
    {
        float angle = 180-startangle + angleStep * i;
        _verts.push_back(QPointF(sin(angle * M_PI / 180.0f), cos(angle * M_PI / 180.0f)));
    }

    Mercator _centerpt = FunctionAssistant::latLng2WebMercator(center);
    std::vector<LAT_LNG> geoPosList;
    geoPosList.resize(_verts.size());
    for (int i = 0; i < _verts.size(); i++)
    {
        Mercator h3index = Mercator{ _verts[i].x() * radius + _centerpt.x, _verts[i].y() * radius + _centerpt.y };
        geoPosList[i] = FunctionAssistant::webMercator2LatLng(h3index);
    }
    m_maprender->updateElementDataRadar(item_id, geoPosList, color, startangle, spanangle, rotationSpeed, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
}


void MapWidget::prepare()
{
    bool bNeedTrans84GC = false;

    auto airport_codes = DataManager::getInstance().getAirPortList();
    for (auto airport_code_item : airport_codes)
    {
        tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo(airport_code_item);
        if (ptagAirPortInfo)
        {
            auto itor = ptagAirPortInfo->m_GeoJsonInfos.begin();
            while (itor != ptagAirPortInfo->m_GeoJsonInfos.end())
            {
                drawGeoData(itor.value(), false, false, false);
                itor++;
            }

            auto it = std::find_if(DataManager::getInstance().getAirPortNameList().begin(),
                DataManager::getInstance().getAirPortNameList().end(), [&airport_code_item](const std::unordered_map<QString, std::tuple<QString, QString>>::value_type& vt) {
                return vt.first == airport_code_item;
            });
            if (it != DataManager::getInstance().getAirPortNameList().end())
            {
                QString  name = std::get<1>(it->second) + "_" + airport_code_item;

                this->drawPoint(FunctionAssistant::generate_random_positive_uint64(), ptagAirPortInfo->center, name, 22, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT);
            }

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

            auto itor2 = ptagAirPortInfo->m_WPSRunwayInfos.begin();
            while (itor2 != ptagAirPortInfo->m_WPSRunwayInfos.end())
            {
                const GeoJsonInfos& geoinfos = itor2.value();
                drawGeoData(geoinfos, true, false, false);

                for (auto subcoordinates : geoinfos.subItem)
                {
                    auto& _tags = subcoordinates.m_tags;
                    auto center = getCenterPt(subcoordinates.coordinatesExtend);
                    QString content = _tags.value("ref");
                    int fontsize = 20;
                    if (_tags.value("aeroway") == "runway" && _tags.contains("runway") && _tags.value("runway") == "displaced_threshold")
                    {
                        content = content + "_" + "displaced_threshold";
                        fontsize = 18;
                    }
                    this->drawPoint(FunctionAssistant::generate_random_positive_uint64(), center, content, fontsize, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT);
                }
                itor2++;
            }

        }
    }


	//drawRader(FunctionAssistant::generate_random_positive_uint64(), LAT_LNG{ 23.404025554223054 ,113.29410288836715 }, 99, 30, 500000, 1.0);

    //QColor cl = FunctionAssistant::randColor(255);
    //uint64_t item_id;
    //auto itor2 = DataManager::getInstance().getPoiitems().begin();
    //while (itor2 != DataManager::getInstance().getPoiitems().end())
    //{
    //	auto& subcoordinatesitem = itor2->second.poipoint;
    //	item_id = FunctionAssistant::generate_random_positive_uint64();
    //	auto _poisinfo_itor = m_poisinfo.find(itor2->first);
    //	if (_poisinfo_itor != m_poisinfo.end())
    //	{
    //		_poisinfo_itor->second = item_id;
    //	}
    //	else
    //	{
    //		m_poisinfo.insert(std::make_pair(itor2->first, item_id));
    //	}
    //	drawPoint(item_id, subcoordinatesitem, itor2->first);
    //	itor2++;
    //}


//    if (m_pModelWidget2)
//    {
//        m_pModelWidget2->updateEntityTracking(0,
//            109.004610,
//                34.032505,
//                600,
//                0,
//                0,
//                0);

//        m_pModelWidget2->updateEntityTracking(0,
//            104.058,
//                30.66,
//                600,
//            0,
//                0,
//                0);
//    }



//    auto deal_line = [&](const TYPE_ULID& sensorulid, const TYPE_ULID& sensingmediaid, const std::vector<LatLng> & _line)
//    {
//        HEXIDX_ARRAY hexidxslist;
//        LocationHelper::getPathIndex(hexidxslist, _line, 11);
//        std::cout << " size --------------------------------------- " << hexidxslist.size() << "\n";
//        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_sensor_ex(sensorulid, sensingmediaid, hexidxslist);
//    };

//    std::vector<LatLng> _line_latlngs;
//    _line_latlngs.push_back(LatLng{ 23.41060781206609,113.30787954517166 });
//    _line_latlngs.push_back(LatLng{ 23.391249794261242, 113.25178908481467 });
//    _line_latlngs.push_back(LatLng{ 23.380750979362404, 113.2680023235896 });
//    _line_latlngs.push_back(LatLng{ 23.384025928332548, 113.29066937585776 });
//    _line_latlngs.push_back(LatLng{ 23.394042916813305, 113.30425911321225 });
//    _line_latlngs.push_back(LatLng{ 23.39775093677629, 113.31611733962961 });
//    _line_latlngs.push_back(LatLng{ 23.41060781206609,113.30787954517166 });
//    deal_line(0, 0, _line_latlngs);



//    std::vector<LatLng> _line_latlngs2;
//    _line_latlngs2.push_back(LatLng{ 23.412334730622064,113.28307902715665 });
//    _line_latlngs2.push_back(LatLng{ 23.379485815185504, 113.30417859063584 });
//    deal_line(0, 1, _line_latlngs2);

//    //gaeactorenvironment::GaeactorProcessorInterface::getInstance().reset();

//    std::vector<LatLng> _line_latlngs3;
//    _line_latlngs3.push_back(LatLng{ 23.379112655940006, 113.32088803095672 });
//    _line_latlngs3.push_back(LatLng{ 23.35199278340491, 113.26776949555409 });
//    deal_line(0, 2, _line_latlngs3);

//    std::vector<LatLng> _line_latlngs4;
//    _line_latlngs4.push_back(LatLng{ 23.37231713261484,113.28006130442168 });
//    _line_latlngs4.push_back(LatLng{ 23.356150164508662, 113.32103474906694 });
//    deal_line(0, 3, _line_latlngs4);

//    std::vector<LatLng> _line_latlngs5;
//    _line_latlngs5.push_back(LatLng{ 23.38597478830097,113.31154491378896 });
//    _line_latlngs5.push_back(LatLng{ 23.352586638110395, 113.31003623548611 });
//    deal_line(0, 4, _line_latlngs5);

//    std::vector<LatLng> _line_latlngs6;
//    _line_latlngs6.push_back(LatLng{ 23.403212265641386,113.27231214106081 });
//    _line_latlngs6.push_back(LatLng{ 23.34715202009808,113.29155140920045});
//    _line_latlngs6.push_back(LatLng{ 23.364961407312308,113.32404738349999});
//    deal_line(0, 5, _line_latlngs6);

}


bool MapWidget::bTransfer() const
{
    if (m_maprender)
    {
        return m_maprender->bTransfer();
    }
    return false;
}

void MapWidget::updateText(const QString& context)
{
#ifdef USING_PATH_GENERATE
    if (m_textLabel)
    {
        m_textLabel->setText(context);
    }
#endif
}

void MapWidget::drawPoint(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, bool bClear)
{
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
    if (bClear)
    {
        m_maprender->clearEntityItem(item_id);
        m_maprender->clearText(item_id);
    }
    else
    {
        m_maprender->appendEntityItem(item_id, pos, 0, 0, 0, AGENT_ENTITY_PROPERTY_SENSOR_SPACE, 10.0, bNeedTrans84GC);
        drawText(item_id, pos, context, textsize, eDrawType, bClear);
    }
}

void MapWidget::drawText(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, bool bClear)
{
    if (!context.isEmpty() || bClear)
    {
        ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
        QColor txtcl;
        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        {
            txtcl = QColor(0, 0, 255);
        }
        else
        {
            txtcl = QColor(0, 255, 255);
        }
        m_maprender->drawTextPos(item_id, pos, txtcl, context, textsize, eDrawType, true, alignType);
    }
    else
    {
        m_maprender->clearText(item_id);
    }
}

void MapWidget::updateTextColor(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, const QColor& color)
{
    ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
    m_maprender->drawTextPos(item_id, pos, color, context, textsize, eDrawType, true, alignType);
}

void MapWidget::drawTrackingLine(const quint64& trackingid, const  std::vector<LAT_LNG>& waypts, const QColor& color, float linewidthScale)
{
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
    if (waypts.empty())
    {
        m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, trackingid);
    }
    else
    {
        m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, trackingid, waypts, color, DrawElements::E_ELEMENT_TYPE_TRACKING, false, bNeedTrans84GC, linewidthScale);
    }
}

void MapWidget::drawTrackingDashedLine(const quint64& trackingid, const std::vector<LAT_LNG>& waypts, const QColor& color, float linewidthScale /*= 1.0f*/)
{
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
    if (waypts.empty())
    {
        m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI, trackingid);
    }
    else
    {
        m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI, trackingid, waypts, color, DrawElements::E_ELEMENT_TYPE_TRACKING, false, bNeedTrans84GC, linewidthScale);
    }
}

void MapWidget::drawTrackingPoints(const quint64& trackingid, const  std::vector<LAT_LNG>& waypts, const QColor& color, float linewidthScale /*= 1.0f*/)
{
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
    if (waypts.empty())
    {
        m_maprender->clearElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, trackingid);
    }
    else
    {
        m_maprender->updateElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, trackingid, waypts, color, DrawElements::E_ELEMENT_TYPE_TRACKING, false, bNeedTrans84GC, linewidthScale);
    }
}

void MapWidget::updateEntityItemSelect(UINT64 id, bool bSelect)
{
    m_maprender->updateEntityItemSelect(id, bSelect);
}

void MapWidget::updateElementSelect(UINT64 id, bool bSelect, bool bClearOld)
{
    m_maprender->updateElementSelect(id, bSelect, bClearOld);
}

GaeactorManager* MapWidget::getGaeactorManager()
{
    return m_pGaeactorManager;
}

void MapWidget::setEventDriver(EventDriver* peventDriver)
{
    m_peventDriver = peventDriver;
}

void MapWidget::drawHex_ex(const TYPE_ULID &uildsrc, const HEXIDX_HGT_ARRAY &hexidxslist, const std::vector<QColor> &clstmp)
{
#if 0
    if (m_pModelWidget2)
    {
        static std::unordered_map<TYPE_ULID, std::unordered_set<H3INDEX>> polygonlist_;
        if (hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.find(uildsrc);
            if (polygonlistitor != polygonlist_.end())
            {
                std::unordered_set<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                while (itor != polygonlist_[uildsrc].end())
                {
                    m_pModelWidget2->clearEntityTracking(uildsrc+*itor);
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
                        m_pModelWidget2->clearEntityTracking(uildsrc+oldh3index);
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
                                hgt.PARAM_seq_hexidx_hgt0/1000.0f);
                        }
                        poslist[boundary.numVerts] = std::make_tuple(
                            LocationHelper::radianToDegree(boundary.verts[0].lng),
                            LocationHelper::radianToDegree(boundary.verts[0].lat),
                            hgt.PARAM_seq_hexidx_hgt0/1000.0f);

                        llaList_down[0] = abs(hgt.PARAM_seq_hexidx_hgtn-hgt.PARAM_seq_hexidx_hgt0)/1000.0f;
                        if(!clstmp.empty() && index < clstmp.size())
                        {
                            cls[0]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                        }
                        LatLng ret;
                        LocationHelper::doCell(ret, hexidxslistTMP[0]);
                        m_pModelWidget2->initNode(uildsrc+h3index,"",2, "", LocationHelper::radianToDegree(ret.lng),LocationHelper::radianToDegree(ret.lat),(hgt.PARAM_seq_hexidx_hgt0+hgt.PARAM_seq_hexidx_hgtn)/2,0,0,0);
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
                                hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                            llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;
                            if(!clstmp.empty() && index < clstmp.size())
                            {
                                cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }
                        }
                        poslist[boundary.numVerts] = std::make_tuple(
                            LocationHelper::radianToDegree(boundary.verts[0].lng),
                            LocationHelper::radianToDegree(boundary.verts[0].lat),
                            hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                        llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;

                        if(!clstmp.empty()&& index < clstmp.size())
                        {
                            cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                        }

                        LatLng ret;
                        LocationHelper::doCell(ret, hexidxslistTMP[0]);
                        m_pModelWidget2->initNode(uildsrc+h3index,"",5, "", LocationHelper::radianToDegree(ret.lng),LocationHelper::radianToDegree(ret.lat),(hgt.PARAM_seq_hexidx_hgt0+hgt.PARAM_seq_hexidx_hgtn)/2,0,0,0);

#endif
                        m_pModelWidget2->updatePosList(uildsrc+h3index,poslist,std::move(llaList_down),cls);
                    }
                }
            }
        }
    }
#else

    if (m_pModelWidget2)
    {
#if 1
        static std::unordered_map<TYPE_ULID, std::unordered_set<H3INDEX>> polygonlist_;
        if (hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.find(uildsrc);
            if (polygonlistitor != polygonlist_.end())
            {
                m_pModelWidget2->clearEntityTracking(uildsrc);
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
                m_pModelWidget2->initNode(uildsrc,"",6, "", 0,0,0,0,0,0);
                bChanged = true;
            }
            if(!bChanged)
            {
                return;
            }
            m_pModelWidget2->updatePosList(uildsrc,std::vector<std::tuple<double, double, double> > ());
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
                                hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                            llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;
                            if(!clstmp.empty() && index < clstmp.size())
                            {
                                cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }
                        }
                        poslist[boundary.numVerts] = std::make_tuple(
                            LocationHelper::radianToDegree(boundary.verts[0].lng),
                            LocationHelper::radianToDegree(boundary.verts[0].lat),
                            hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                        llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;

                        if(!clstmp.empty() && index < clstmp.size())
                        {
                            cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                        }

                        m_pModelWidget2->updatePosList(uildsrc,poslist,std::move(llaList_down),cls);
                    }
                }
            }
        }
#else

        static std::unordered_map<TYPE_ULID, std::unordered_map<H3INDEX, UINT64>> polygonlist_;
        if (hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.find(uildsrc);
            if (polygonlistitor != polygonlist_.end())
            {
                std::unordered_map<H3INDEX, UINT64>::iterator itor = polygonlist_[uildsrc].begin();
                while (itor != polygonlist_[uildsrc].end())
                {
                    m_pModelWidget2->clearEntityTracking(uildsrc+itor->first);
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
                        m_pModelWidget2->clearEntityTracking(uildsrc+oldh3index);
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
                m_pModelWidget2->initNode(uildsrc,"",6, "", 0,0,0,0,0,0);

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
                                hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                            llaList_down[v] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;
                            if(!clstmp.empty() && index < clstmp.size())
                            {
                                cls[v]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                            }
                        }
                        poslist[boundary.numVerts] = std::make_tuple(
                            LocationHelper::radianToDegree(boundary.verts[0].lng),
                            LocationHelper::radianToDegree(boundary.verts[0].lat),
                            hgt.PARAM_seq_hexidx_hgtn/1000.0f);
                        llaList_down[boundary.numVerts] = hgt.PARAM_seq_hexidx_hgt0/1000.0f;

                        if(!clstmp.empty() && index < clstmp.size())
                        {
                            cls[boundary.numVerts]=std::make_tuple(clstmp.at(index), clstmp.at(index));
                        }

                        m_pModelWidget2->updatePosList(uildsrc,poslist,std::move(llaList_down),cls);
                    }
                }
            }
        }
#endif
    }
#endif
}

void MapWidget::sim_displayHexidxPosCallback_slot(const TYPE_ULID& uildsrc, const transdata_entityposinfo& eninfo)
{
    displayHexidxPosCallback(uildsrc, 0, eninfo, E_DISPLAY_MODE::E_DISPLAY_MODE_ENTITY);
}

void MapWidget::initializeGL()
{
    m_w = this->width();
    m_h = this->height();
    //initRenderThread();

    initializeOpenGLFunctions();
    QOpenGLExtraFunctions* funcs = this->context()->extraFunctions();
    //    QOpenGLExtraFunctions  *funcs = m_thread->renderContext()->extraFunctions();
    mapshader = new Shader(funcs);
    m_lineelementshader = new Shader(funcs);
    m_trielementshader = new Shader(funcs);
    m_pTextshader = new Shader(funcs);

    QString path = QCoreApplication::applicationDirPath();

    mapshader->load((path + "./shaders/map.vs").toStdString().c_str(), (path + "./shaders/map.fs").toStdString().c_str());
#ifdef USING_DEAL_WORLD_TO_SCREEN
    QString vs_path = (path + "./shaders/graph.vs");
#else
    QString vs_path = (path + "./shaders/graph_ex_screen.vs");
#endif

#ifdef DRAW_ARROW
    m_lineelementshader->load(vs_path.toStdString().c_str(), (path + "./shaders/graph.frag").toStdString().c_str(), (path + "./shaders/graph.gs").toStdString().c_str());
#else
    m_lineelementshader->load(vs_path.toStdString().c_str(), (path + "./shaders/graph.frag").toStdString().c_str());
#endif // DRAW_ARROW
    m_trielementshader->load(vs_path.toStdString().c_str(), (path + "./shaders/graph.frag").toStdString().c_str());

    m_pTextshader->load((path + "./shaders/text.vs").toStdString().c_str(), (path + "./shaders/text.frag").toStdString().c_str());



    m_lineelementshader->use();
    GLuint uniformBlockIndex = glGetUniformBlockIndex(m_lineelementshader->program(), "MVPMatrix");
    funcs->glUniformBlockBinding(m_lineelementshader->program(), uniformBlockIndex, 0);
    funcs->glGenBuffers(1, &m_lineubouniform);
    funcs->glBindBuffer(GL_UNIFORM_BUFFER, m_lineubouniform);
    funcs->glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    funcs->glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //定义绑定点为 0 buffer 的范围
    funcs->glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_lineubouniform, 0, 3 * sizeof(glm::mat4));
    m_lineelementshader->unuse();


    m_trielementshader->use();
    GLuint uniformBlockIndex2 = glGetUniformBlockIndex(m_trielementshader->program(), "MVPMatrix");
    funcs->glUniformBlockBinding(m_trielementshader->program(), uniformBlockIndex2, 0);
    funcs->glGenBuffers(1, &m_triubouniform);
    funcs->glBindBuffer(GL_UNIFORM_BUFFER, m_triubouniform);
    funcs->glBufferData(GL_UNIFORM_BUFFER, 3 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    funcs->glBindBuffer(GL_UNIFORM_BUFFER, 0);
    //定义绑定点为 0 buffer 的范围
    funcs->glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_triubouniform, 0, 3 * sizeof(glm::mat4));
    m_trielementshader->unuse();

    m_maprender->setLineelementshader(m_lineelementshader);
    m_maprender->setTrielementshader(m_trielementshader);

    m_maprender->setTriubouniform(m_triubouniform);
    m_maprender->setUbouniform(m_lineubouniform);
    m_maprender->initOpenglFunc(funcs);
    m_maprender->initailize(m_w, m_h);
    m_maprender->setInitFlag(true);
    m_maprender->setInitWindowSize(m_w, m_h);
    m_maprender->requestTiles();

    m_maprender->renderMatrix(m_lineelementshader,m_trielementshader,m_lineubouniform, m_triubouniform);

    if (m_airportombox)
    {


        auto item = DataManager::getInstance().getCurrentAirport();
        auto it = std::find_if(DataManager::getInstance().getAirPortNameList().begin(),
            DataManager::getInstance().getAirPortNameList().end(), [&item](const std::unordered_map<QString, std::tuple<QString, QString>>::value_type& vt) {
            return vt.first == item;
        });
        if (it != DataManager::getInstance().getAirPortNameList().end())
        {
            m_airportombox->setCurrentText(std::get<1>(it->second) + "_" + item);
        }


        tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo(DataManager::getInstance().getCurrentAirport());
        if (ptagAirPortInfo)
        {
            //emit select_airport_sig(DataManager::getInstance().getCurrentAirport());
            m_maprender->rtesetZoom();
            projectionmercator::GeoLatLngRect targetArea(ptagAirPortInfo->topleft, ptagAirPortInfo->bottomright);
            m_maprender->scaleTo(targetArea);

            //m_maprender->moveTo(LAT_LNG(ptagAirPortInfo->center.lat, ptagAirPortInfo->center.lng));
            //auto lat = m_maprender->screenCenterToLatLng();
            m_maprender->requestTiles();
        }
    }
    double lat = 25.6436306575074;
    double lon = 119.06700448708392;
    m_maprender->moveTo(LAT_LNG{lat,lon});
    m_maprender->setCenter(LAT_LNG{lat,lon});
    m_maprender->requestTiles();


    //m_pUpdateTimer->start(16);


    //    char vertexShaderSource[] =
    //        "#version 330\n"
    //        "layout (location = 0) in vec2 vPos;\n"
    //        "layout (location = 1) in vec2 texCoord;\n"
    //        "out vec2 TexCoord;\n"
    //        "void main()\n"
    //        "{\n"
    //        "   gl_Position = vec4(vPos, 0.0, 1.0);\n"
    //        "   TexCoord = texCoord;\n"
    //        "}\n";
    //    char fragmentShaderSource[] =
    //        "#version 330\n"
    //        "out vec4 FragColor;\n"
    //        "in vec2 TexCoord;\n"
    //        "uniform sampler2D ourTexture;\n"
    //        "void main()\n"
    //        "{\n"
    //        "   FragColor = texture(ourTexture, TexCoord);\n"
    //        "}\n";
    //    /*char fragmentShaderSource[] =
    //        "#version 120\n"
    //        "void main()\n"
    //        "{\n"
    //        "   gl_FragColor = vec4(1,0,0,1);\n"
    //        "}\n";*/

    //    m_program.reset(new QOpenGLShaderProgram);
    //    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    //    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    //    bool berr = m_program->link();

    ////    glGenVertexArrays(1, &m_vao);
    ////    glBindVertexArray(m_vao);

    //    glGenBuffers(1, &m_vbo);
    //    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(0));
    //    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    //    glEnableVertexAttribArray(0);
    //    glEnableVertexAttribArray(1);

    //    glBindVertexArray(0);


    if (m_eMapMode == E_MAP_MODE_DISPLAY ||
        m_eMapMode == E_MAP_MODE_DISPLAY_REVIEW ||
        m_eMapMode == E_MAP_MODE_SELECT ||
        m_eMapMode == E_MAP_MODE_SELECT_PATH)
    {
        prepare();
    }
    if (m_eMapMode == E_MAP_MODE_DISPLAY ||
        m_eMapMode == E_MAP_MODE_DISPLAY_REVIEW)
    {
        if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
        {
            m_maprender->drawTextScreen(m_arr_text_id, QPoint(-540, -80), QColor(0, 255, 0, 255), QString("进港航班数:%1").arg(m_arr.size()), 48, DrawElements::E_ELEMENT_TYPE_SCREENCONTEXT, false, (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER));
            m_maprender->drawTextScreen(m_dep_text_id, QPoint(-540, -160), QColor(0, 255, 255, 255), QString("离港航班数:%1").arg(m_dep.size()), 48, DrawElements::E_ELEMENT_TYPE_SCREENCONTEXT, false, (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER));
        }
    }
    if (m_eMapMode == E_MAP_MODE_DISPLAY)
    {
        std::unordered_map<QString, UINT64> m_poisinfo;
        QStringList vallist2;
        vallist2.append("all");

        auto airport_codes = DataManager::getInstance().getAirPortList();
        for (auto airport_code_item : airport_codes)
        {
            tagAirPortInfo* ptagAirPortInfo = DataManager::getInstance().getAirportInfo(airport_code_item);
            if (ptagAirPortInfo)
            {
                std::map<QString, ARR_DEP_RUNWAY_PATH>& pathplans = ptagAirPortInfo->m_Path_Plans;

                auto pathplans_itor = pathplans.begin();
                while (pathplans_itor != pathplans.end())
                {
                    const QString& parkingpoint = pathplans_itor->first;
                    vallist2.append(parkingpoint);
                    pathplans_itor++;
                }

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
                    this->drawPoint(item_id, subcoordinatesitem, itor2->first, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
                    //m_mapWidget->drawText(item_id, subcoordinatesitem, itor2->first,DrawElements::E_ELEMENT_TYPE_POI_CONTEXT);
                    itor2++;
                }
            }
        }
    }

    if (m_eMapMode == E_MAP_MODE_SELECT_PATH)
    {
        std::function<void(const QString& path)> readGeoDirPath = [&](const QString& path) {
            QFileInfo fileInfo(path);
            if (fileInfo.isDir())
            {
                QDir dir(path);
                dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
                QFileInfoList list = dir.entryInfoList();
                for (int i = 0; i < list.size(); i++)
                {
                    readGeoDirPath(list.at(i).filePath());
                }
            }
            else if (fileInfo.isFile()) // �ҵ���Ƭ
            {
                GeoJsonInfos geoinfos;
                if (DataManager::getInstance().readGeoJsonData(path.toStdString(), geoinfos))
                {
                    drawGeoData(geoinfos, true, true, true);
                }
            }
        };
        QString dirinfo = QCoreApplication::applicationDirPath() + "/data/baiyun/WPS/analysis";
        readGeoDirPath(dirinfo);

    }
}

void MapWidget::keyPressEvent(QKeyEvent* event)
{
    QOpenGLWidget::keyPressEvent(event);
}


void MapWidget::resizeGL(int w, int h)
{
    //m_thread->setNewSize(w, h);
    m_w = w;
    m_h = h;
    glViewport(0, 0, (GLint)w, (GLint)h);               //重置当前的视口
    glMatrixMode(GL_PROJECTION);                        //选择投影矩阵
    glLoadIdentity();                                   //重置投影矩阵


    m_TransformModelMatrix = glm::mat4(1.0f);//创建一个单位矩阵
    m_TransformViewMatrix = glm::mat4(1.0f);//创建一个单位矩阵

    glLoadMatrixf(glm::value_ptr<float>(m_TransformProjectionMatrix));
    glMatrixMode(GL_MODELVIEW);                         //选择模型观察矩阵
    glLoadIdentity();                                   //重置模型观察矩阵

    if (m_maprender)
    {
        auto rc = this->geometry();
        m_maprender->reSize(rc.width(), rc.height());

        m_maprender->resize(rc.width(), rc.height());

        m_maprender->requestTiles();
    }
}


void MapWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT/* | GL_STENCIL_BUFFER_BIT*/); //清除屏幕和深度缓存
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    }
    //glViewport(0, 0, m_w, m_h);

//    auto a = QDateTime::currentMSecsSinceEpoch();

    //    m_maprender->updateFrame(QOpenGLContext::currentContext(), mapshader);
    m_maprender->render(this->context(), m_lineelementshader, m_trielementshader, mapshader, m_pTextshader, m_lineubouniform, m_triubouniform);

    this->context()->swapBuffers(this->context()->surface());
    //auto b = QDateTime::currentMSecsSinceEpoch();
    //std::cout << "cost time " << b - a << std::endl;

    //    glViewport(0, 0, m_w, m_h);


    //    glEnable(GL_TEXTURE_2D);
    //    m_program->bind();
    //    glBindVertexArray(m_vao);

    //    if (TextureBuffer::instance()->ready())
    //    {
    //        TextureBuffer::instance()->drawTexture(QOpenGLContext::currentContext(), sizeof(vertices) / sizeof(float) / 4);
    //    }

    //    glBindVertexArray(0);
    //    m_program->release();
    //    glDisable(GL_TEXTURE_2D);
}

void MapWidget::keyReleaseEvent(QKeyEvent* event)
{
    if ((event->modifiers() == Qt::ControlModifier))
    {
        bool bSet = false;
        DrawElements::E_ELEMENT_TYPES _show_ele_type;
        switch (event->key())
        {
        case Qt::Key_Q:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_SENSOR;
            bSet = false;
        }
        break;

        case Qt::Key_W:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_SENSOR;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_E:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_ENTITY;
            bSet = false;
        }
        break;

        case Qt::Key_R:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_ENTITY;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_T:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_ROUTE;
            bSet = false;
        }
        break;

        case Qt::Key_Y:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_ROUTE;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_U:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_TRACKING;
            bSet = false;
        }
        break;

        case Qt::Key_I:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_TRACKING;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_O:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT;
            bSet = false;
        }
        break;

        case Qt::Key_P:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_K:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_POI_CONTEXT;
            bSet = false;
        }
        break;

        case Qt::Key_L:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_POI_CONTEXT;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_H:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_ID_CONTEXT;
            bSet = false;
        }
        break;

        case Qt::Key_J:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_ID_CONTEXT;
            bSet = true;
        }
        break;
        ////////////////////////////////////////////////////////////////////////////////////////

        case Qt::Key_F:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_WAVE;
            bSet = false;
        }
        break;

        case Qt::Key_G:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_WAVE;
            bSet = true;
        }
        break;

        case Qt::Key_S:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT;
            bSet = false;
        }
        break;

        case Qt::Key_D:
        {
            _show_ele_type = DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT;
            bSet = true;
        }
        break;
        }

        m_maprender->setShowProperty(_show_ele_type, bSet);
    }

    QOpenGLWidget::keyReleaseEvent(event);
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint curpt = event->pos();
    if (bPressed)
    {
        m_maprender->moveDrag(curpt, m_lastpt);
        m_lastpt = event->pos();
        m_maprender->requestTiles();
    }
    QOpenGLWidget::mouseMoveEvent(event);
}




void MapWidget::mousePressEvent(QMouseEvent* event)
{
    auto localPos = event->pos();

    if (event->button() == Qt::LeftButton)
    {
        bPressed = true;
        m_lastpt = event->pos();
        m_maprender->resetOffset();
    }
    QOpenGLWidget::mousePressEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    QPoint curpt = event->pos();
    if (event->button() == Qt::LeftButton && bPressed)
    {
        bPressed = false;
        m_maprender->resetOffset();
        if (curpt.x() != m_lastpt.x() || curpt.y() != m_lastpt.y())
        {
            m_maprender->move(curpt, m_lastpt);
            m_maprender->requestTiles();
        }
    }
    QOpenGLWidget::mouseReleaseEvent(event);
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

//#define BOTH_SHOW
//#define SHOW_HEXIDX

enum WAVE_PTY_USAGE_TYPE :uint8_t {

    WAVE_PTY_USAGE_UNDEFINED = 0x00,
    WAVE_PTY_USAGE_ECHOSENSE = 0x01,
    WAVE_PTY_USAGE_INTFERNCE = 0x02,
    WAVE_PTY_USAGE_SUPRESSOR = 0x03,
    WAVE_PTY_USAGE_LASERBURN = 0x04,
    WAVE_PTY_USAGE_VIBRATION = 0x05,
    WAVE_PTY_USAGE_MCRWVBURN = 0x06,
    WAVE_PTY_USAGE_SHOCKWAVE = 0x07,
    WAVE_PTY_USAGE_DETECTION = 0x08,
    WAVE_PTY_USAGE_EXPLOSION = 0x09,
    WAVE_PTY_USAGE_DETECTION_EX = 0xFB,
    WAVE_PTY_USAGE_SPACEOVLP = 0xFD,
    WAVE_PTY_USAGE___COMM___ = 0xFE,
    WAVE_PTY_USAGE__INFRARED = 0xFF

};

void MapWidget::drawDataToUpdateColor(const E_EVENT_MODE& eventmode, const TYPE_ULID& uildsrc, const TYPE_SENSORINFO& sensorinfo, E_DRAW_TYPE drawType)
{
    auto usagetype = (WAVE_PTY_USAGE_TYPE)sensorinfo.PARAM_wave_usage;
    QColor polygon_cl = QColor(0, 255, 255, 92);
    switch (usagetype)
    {
    case WAVE_PTY_USAGE_UNDEFINED:
        break;
    case WAVE_PTY_USAGE_ECHOSENSE:
        break;
    case WAVE_PTY_USAGE_INTFERNCE:
        break;
    case WAVE_PTY_USAGE_SUPRESSOR:
        break;
    case WAVE_PTY_USAGE_LASERBURN:
        break;
    case WAVE_PTY_USAGE_VIBRATION:
        break;
    case WAVE_PTY_USAGE_MCRWVBURN:
        break;
    case WAVE_PTY_USAGE_SHOCKWAVE:
    {
        polygon_cl = QColor(128, 128, 128, 64);
    }
    break;
    case WAVE_PTY_USAGE_DETECTION:
    {
        polygon_cl = QColor(128, 128, 0, 192);
    }
    break;
    case WAVE_PTY_USAGE_EXPLOSION:
        break;
    case WAVE_PTY_USAGE_SPACEOVLP:
    {
        polygon_cl = QColor(255, 255, 0, 64);
    }
    break;
    case WAVE_PTY_USAGE___COMM___:
        break;
    case WAVE_PTY_USAGE__INFRARED:
    {
        polygon_cl = QColor(255, 0, 0, 64);
    }
    break;
    default:
        break;
    }
    switch (eventmode)
    {
    case E_EVENT_MODE_ADD:
    case E_EVENT_MODE_UPDATE:
    {
        polygon_cl = QColor(255, 0, 0, 92);
    }break;
    case E_EVENT_MODE_REMOVE:
    {
        polygon_cl = QColor(0, 255, 0, 92);
    }break;

    default:
    {
    }break;
    }
    switch (drawType)
    {
    case MapWidget::E_DRAW_TYPE_POINTS:
    {
        if (m_maprender)
        {
            m_maprender->updateElementDataColor(DrawItem::ENUM_TYPE_POINTS_MULTI, uildsrc, polygon_cl);
        }
    }
    break;
    case MapWidget::E_DRAW_TYPE_BOUNDARY_POLYGON:
    {
        if (m_maprender)
        {
            m_maprender->updateElementDataColor(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc, polygon_cl);
            m_maprender->updateElementDataColor(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc, polygon_cl);
            m_maprender->updateElementDataColor(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, uildsrc, polygon_cl);
        }
    }
    break;
    case MapWidget::E_DRAW_TYPE_HEX_POLYGON:
    {
    }
    break;
    default:
        break;
    }
}

void MapWidget::drawDataToHex(const TYPE_ULID& uildsrc, const TYPE_ULID& agentid, const TYPE_ULID& sensingmediaid, const HEXIDX_HGT_ARRAY& hexidxslistTMP, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DRAW_TYPE drawType, const std::unordered_map<H3INDEX, QColor>& HEXIDX_COLOR_LIST, bool bFiill)
{
    auto usagetype = (WAVE_PTY_USAGE_TYPE)sensorinfo.PARAM_wave_usage;
    QColor polygon_cl = QColor(0, 255, 255, 64);
    QColor line_cl = QColor(0, 255, 255, 255);
    QColor point_cl = QColor(0, 255, 255, 255);
    switch (usagetype)
    {
    case WAVE_PTY_USAGE_UNDEFINED:
        break;
    case WAVE_PTY_USAGE_ECHOSENSE:
    {
        polygon_cl = QColor(128, 255, 128, 64);
        line_cl = QColor(128, 255, 128, 255);
        point_cl = QColor(128, 255, 128, 255);

        // static std::unordered_map<TYPE_ULID, bool> _sensor_echo;
        // if (!hexidxslist.empty())
        // {
        // 	if (_sensor_echo.find(sensingmediaid) == _sensor_echo.end())
        // 	{
        // 		_sensor_echo.insert(std::make_pair(sensingmediaid, true));
        // 		std::cout << "append echo sensor ++++++++++++ " << sensingmediaid << " size " << _sensor_echo.size() << std::endl;
        // 	}
        // }
        // else
        // {
        // 	auto ittor = _sensor_echo.find(sensingmediaid);
        // 	if (ittor != _sensor_echo.end())
        // 	{
        // 		_sensor_echo.erase(ittor);
        // 		std::cout << "clear echo sensor ------------ " << sensingmediaid << " size " << _sensor_echo.size() << std::endl;
        // 	}
        // }
        // if (!polygonlist.empty())
        // {
        // 	auto pos = LAT_LNG{ polygonlist.at(0).PARAM_latitude / LON_LAT_ACCURACY, polygonlist.at(0).PARAM_longitude / LON_LAT_ACCURACY };
        // 	if (m_maprender)
        // 	{
        // 		ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
        // 		QColor txtcl;
        // 		if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        // 		{
        // 			txtcl = QColor(0, 0, 255);
        // 		}
        // 		else
        // 		{
        // 			txtcl = QColor(0, 255, 255);
        // 		}
        // 		QString txt = (QString::number(sensingmediaid));
        // 		m_maprender->drawTextPos(uildsrc, pos, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT, false, alignType);
        // 	}
        // }
        // else
        // {
        // 	m_maprender->clearText(uildsrc);
        // }
    }
    break;
    break;
    case WAVE_PTY_USAGE_INTFERNCE:
    {
        polygon_cl = QColor(0, 128, 0, 64);
        line_cl = QColor(0, 128, 0, 255);
        point_cl = QColor(0, 128, 0, 255);
    }
    break;
    case WAVE_PTY_USAGE_SUPRESSOR:
    {
        polygon_cl = QColor(128, 0, 128, 64);
        line_cl = QColor(128, 0, 128, 255);
        point_cl = QColor(128, 0, 128, 255);
    }
    break;
    case WAVE_PTY_USAGE_LASERBURN:
        break;
    case WAVE_PTY_USAGE_VIBRATION:
        break;
    case WAVE_PTY_USAGE_MCRWVBURN:
        break;
    case WAVE_PTY_USAGE_SHOCKWAVE:
    {
        polygon_cl = QColor(128, 128, 128, 64);
        line_cl = QColor(128, 128, 128, 255);
        point_cl = QColor(128, 128, 128, 255);
    }
    break;
    case WAVE_PTY_USAGE_DETECTION:
    {
        polygon_cl = QColor(128, 128, 0, 255);
        line_cl = QColor(128, 128, 0, 255);
        point_cl = QColor(128, 128, 0, 255);

        //if (!polygonlist.empty())
        //{
        //	auto pos = LAT_LNG{ polygonlist.at(0).PARAM_latitude / LON_LAT_ACCURACY, polygonlist.at(0).PARAM_longitude / LON_LAT_ACCURACY };
        //	if (m_maprender)
        //	{
        //		ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
        //		QColor txtcl;
        //		if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        //		{
        //			txtcl = QColor(0, 0, 255);
        //		}
        //		else
        //		{
        //			txtcl = QColor(0, 255, 255);
        //		}
        //		QString txt = (QString::number(sensingmediaid));
        //		m_maprender->drawTextPos(uildsrc, pos, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT, false, alignType);
        //	}
        //}
        //else
        //{
        //	m_maprender->clearText(uildsrc);
        //}
    }
    break;
    case WAVE_PTY_USAGE_DETECTION_EX:
    {
        polygon_cl = QColor(255, 0, 0, 192);
        line_cl = QColor(255, 0, 0, 255);
        point_cl = QColor(255, 0, 0, 255);

        //if (!polygonlist.empty())
        //{
        //	auto pos = LAT_LNG{ polygonlist.at(0).PARAM_latitude / LON_LAT_ACCURACY, polygonlist.at(0).PARAM_longitude / LON_LAT_ACCURACY };
        //	if (m_maprender)
        //	{
        //		ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
        //		QColor txtcl;
        //		if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        //		{
        //			txtcl = QColor(0, 0, 255);
        //		}
        //		else
        //		{
        //			txtcl = QColor(0, 255, 255);
        //		}
        //		QString txt = (QString::number(sensingmediaid));
        //		m_maprender->drawTextPos(uildsrc, pos, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT, false, alignType);
        //	}
        //}
        //else
        //{
        //	m_maprender->clearText(uildsrc);
        //}
    }
    break;
    case WAVE_PTY_USAGE_EXPLOSION:
    {
        polygon_cl = QColor(128, 0, 0, 64);
        line_cl = QColor(128, 0, 0, 255);
        point_cl = QColor(128, 0, 0, 255);
    }
    break;
    case WAVE_PTY_USAGE_SPACEOVLP:
    {
        polygon_cl = QColor(255, 255, 0, 64);
        line_cl = QColor(255, 255, 0, 255);
        point_cl = QColor(255, 255, 0, 255);
    }
    break;
    case WAVE_PTY_USAGE___COMM___:
        break;
    case WAVE_PTY_USAGE__INFRARED:
    {
        polygon_cl = QColor(255, 0, 0, 64);
        line_cl = QColor(255, 0, 0, 255);
        point_cl = QColor(255, 0, 0, 255);
    }
    break;
    default:
        break;
    }


    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
    switch (drawType)
    {
    case MapWidget::E_DRAW_TYPE_POINTS:
    {
        if (m_maprender)
        {
            if (hexidxslistTMP.empty() && polygonlist.empty())
            {
                m_maprender->clearElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, uildsrc);
            }
            std::vector<LAT_LNG> geoPosList;
            geoPosList.resize(hexidxslistTMP.size());
            for (int i = 0; i < hexidxslistTMP.size(); i++)
            {
                LatLng ptt;
                LocationHelper::doCell(ptt, hexidxslistTMP.at(i).PARAM_seq_hexidx_element);
                geoPosList[i].lat = LocationHelper::radianToDegree(ptt.lat);
                geoPosList[i].lng = LocationHelper::radianToDegree(ptt.lng);
            }
            if (!geoPosList.empty())
            {
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_POINTS_MULTI, uildsrc, geoPosList, point_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
            }
        }
    }
    break;
    case MapWidget::E_DRAW_TYPE_BOUNDARY_POLYGON:
    {
        if (hexidxslistTMP.empty() && polygonlist.empty())
        {
            if (m_maprender)
            {
#if 0
                m_maprender->clearElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc);
#else
                if (bFiill)
                {
#if 1
                    m_maprender->clearElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc);
#else
                    m_maprender->clearElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc);
#endif
                }
#endif
                m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc);
            }
        }
        else
        {
            if (m_maprender)
            {
#if 0
                QVector<LAT_LNG> geoposlist;
                //LocationHelper::getHexidxsBoundaryConvexHull(geoposlist,hexidxslist);
                LocationHelper::getHexidxsConvexHull(geoposlist, hexidxslist);
                std::vector<LAT_LNG> geoPosList;
                geoPosList.resize(geoposlist.size());
                for (int i = 0; i < geoposlist.size(); i++)
                {
                    geoPosList[i].lat = geoposlist.at(i).latitude();
                    geoPosList[i].lng = geoposlist.at(i).longitude();
                }
                if (bFiill)
                {
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc, geoPosList, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#else

                std::vector<LAT_LNG> geoPosList;
#if 1
                geoPosList.resize(polygonlist.size());
                for (int i = 0; i < polygonlist.size(); i++)
                {
                    geoPosList[i].lat = ((double)(polygonlist.at(i).PARAM_latitude) / LON_LAT_ACCURACY);
                    geoPosList[i].lng = ((double)(polygonlist.at(i).PARAM_longitude) / LON_LAT_ACCURACY);
                }
                //QColor color(255, 255, 0, 255);
                //m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc, geoPosList, line_cl,(DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE),false);
                //m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI, uildsrc, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE),false);

                //bNeedTrans84GC = true;

                if (bFiill)
                {
#if 1
                    std::tuple< std::vector<TRIANGLE>, std::vector<LAT_LNG>> vals;
                    if (geoPosList.size() >= 5)
                    {
                        vals = TriangluationHelper::generateTriangleByTriangle(geoPosList);
                    }

                    std::vector<TRIANGLE>& out_triangles_index = std::get<0>(vals);
                    std::vector<LAT_LNG>& out_points_data = std::get<1>(vals);

#if 0
                    std::vector<LAT_LNG> out_points;
                    out_points.reserve(out_triangles_index.size() * 3);
                    for (size_t i = 0; i < out_triangles_index.size(); i++)
                    {
                        out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                        out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                        out_points.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                        //                    std::vector<LAT_LNG> out_pointsTmp;
                        //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                        //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                        //                    out_pointsTmp.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                        //                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, cc++, out_pointsTmp, cl3,(DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE));
                    }
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc, out_points, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#else
                    m_maprender->updateElementIndexAndData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc, out_triangles_index, out_points_data, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#endif
#else
                    m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc, geoPosList, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#endif
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#else
                std::vector<LAT_LNG> geoPosList_points;

                geoPosList_points.resize(polygonlist.size());
                for (int i = 0; i < polygonlist.size(); i++)
                {
                    geoPosList_points[i].lat = ((double)(polygonlist.at(i).PARAM_latitude) / LON_LAT_ACCURACY);
                    geoPosList_points[i].lng = ((double)(polygonlist.at(i).PARAM_longitude) / LON_LAT_ACCURACY);
                }

                //auto vals = TriangluationHelper::generateTriangleByGPC(geoPosList_points);

                //for (auto it : vals)
                //{
                //	for (auto item : it)
                //	{
                //		geoPosList.push_back(item);
                //	}
                //}
                std::tuple< std::vector<TRIANGLE>, std::vector<LAT_LNG>> vals;
                if (geoPosList_points.size() >= 5)
                {
                    vals = TriangluationHelper::generateTriangleByTriangle(geoPosList_points);
                }

                std::vector<TRIANGLE>& out_triangles_index = std::get<0>(vals);
                std::vector<LAT_LNG>& out_points_data = std::get<1>(vals);

#if 0
                std::vector< std::vector<LAT_LNG>> result;
                geoPosList.reserve(out_triangles_index.size() * 3);
                for (size_t i = 0; i < out_triangles_index.size(); i++)
                {
                    geoPosList.push_back(out_points_data.at(out_triangles_index.at(i).m_a));
                    geoPosList.push_back(out_points_data.at(out_triangles_index.at(i).m_b));
                    geoPosList.push_back(out_points_data.at(out_triangles_index.at(i).m_c));
                }
                m_maprender->updateElementData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc, geoPosList, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                //m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#else
                m_maprender->updateElementIndexAndData(DrawItem::ENUM_TYPE_TRIGAGLES_MULTI, uildsrc, out_triangles_index, out_points_data, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
#endif
#endif
#endif
            }
        }
    }
    break;
    case MapWidget::E_DRAW_TYPE_HEX_POLYGON:
    {
        static std::unordered_map<TYPE_ULID, std::unordered_map<H3INDEX, QColor>> polygonlist_;
        if (hexidxslistTMP.empty() && polygonlist.empty())
        {
            auto polygonlistitor = polygonlist_.find(uildsrc);
            if (polygonlistitor != polygonlist_.end())
            {
                std::unordered_map<H3INDEX, QColor>::iterator itor = polygonlist_[uildsrc].begin();
                while (itor != polygonlist_[uildsrc].end())
                {
                    H3INDEX oldh3index = itor->first;
                    if (m_maprender)
                    {
                        if (bFiill)
                        {
                            m_maprender->clearElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + oldh3index);
                        }
                        m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc + oldh3index);
                    }
                    itor++;
                }
                polygonlist_[uildsrc].clear();
                polygonlist_.erase(polygonlistitor);
            }
        }
        else
        {
            if (m_maprender)
            {
                QVector<H3INDEX> _existindex;
                if (polygonlist_.find(uildsrc) != polygonlist_.end())
                {
                    std::unordered_map<H3INDEX, QColor>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        H3INDEX oldh3index = itor->first;
                        QColor oldh3indexcl = itor->second;
                        auto h3itor = std::find_if(hexidxslistTMP.begin(), hexidxslistTMP.end(), [&](const HEXIDX_HGT_ARRAY::value_type& val) {
                            return oldh3index == val.PARAM_seq_hexidx_element;
                        });
                        if (h3itor == hexidxslistTMP.end())
                        {
                            if (bFiill)
                            {
                                m_maprender->clearElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + oldh3index);
                            }
                            m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc + oldh3index);

                            itor = polygonlist_[uildsrc].erase(itor);
                            continue;
                        }
                        else
                        {
                            QColor clnew = polygon_cl;

                            auto itor = HEXIDX_COLOR_LIST.find(oldh3index);
                            if (itor != HEXIDX_COLOR_LIST.end())
                            {
                                clnew = itor->second;
                            }

                            if (clnew == oldh3indexcl)
                            {
                                _existindex.push_back(oldh3index);
                            }
                        }
                        itor++;
                    }
                }
                else
                {
                    polygonlist_.insert(std::make_pair(uildsrc, std::unordered_map<H3INDEX, QColor>()));
                }

                HEXIDX_ARRAY hexidxslist;
                hexidxslist.resize(hexidxslistTMP.size());
                for(int index= 0; index < hexidxslistTMP.size(); index++)
                {
                    hexidxslist[index] = hexidxslistTMP.at(index).PARAM_seq_hexidx_element;
                }

                std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                LocationHelper::getIndexBoundary(boundarys, hexidxslist);
                //				for (auto boundaryitem : boundarys)
                for (int m = 0; m < boundarys.size(); m++)
                {
                    auto boundaryitem = boundarys.at(m);

                    H3INDEX h3index = std::get<0>(boundaryitem);

                    auto itor = HEXIDX_COLOR_LIST.find(h3index);
                    if (itor != HEXIDX_COLOR_LIST.end())
                    {
                        polygon_cl = itor->second;
                        line_cl = itor->second;
                    }

                    if (!_existindex.contains(h3index))
                    {
                        auto polygonlist_itor = polygonlist_.find(uildsrc);
                        if (polygonlist_itor != polygonlist_.end())
                        {
                            auto h3index_itor = polygonlist_itor->second.find(h3index);
                            if (h3index_itor != polygonlist_itor->second.end())
                            {
                                h3index_itor->second = polygon_cl;
                            }
                            else
                            {
                                polygonlist_itor->second.insert(std::make_pair(h3index, polygon_cl));
                            }
                        }
                        else
                        {
                            polygonlist_[uildsrc].insert(std::make_pair(h3index, polygon_cl));
                        }
                        CellBoundary boundary = std::get<1>(boundaryitem);
                        std::vector<LAT_LNG> geoPosList;
                        geoPosList.resize(boundary.numVerts);
                        for (int v = 0; v < boundary.numVerts; v++)
                        {
                            geoPosList[v].lat = LocationHelper::radianToDegree(boundary.verts[v].lat);
                            geoPosList[v].lng = LocationHelper::radianToDegree(boundary.verts[v].lng);
                        }
                        if (!geoPosList.empty())
                        {
                            if (bFiill)
                            {
                                m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + h3index, geoPosList, polygon_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                            }
                            m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_LOOP_MULTI, uildsrc + h3index, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                        }
                    }
                }
            }
        }
    }
    break;
    default:
        break;
    }
}
void MapWidget::setQtOSGWidget(QtOSGWidget* pModelWidget2)
{
    m_pModelWidget2 = pModelWidget2;
}

void MapWidget::displayHexidxCallback(const TYPE_ULID& agentid, const TYPE_ULID& sensingmediaid, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode)
{
    auto usage = (int)sensorinfo.PARAM_wave_usage;
    QString cctrace;
    switch (usage)
    {
    case WAVE_PTY_USAGE_UNDEFINED:cctrace = "UNDEFINED"; break;
    case WAVE_PTY_USAGE_ECHOSENSE:cctrace = "ECHOSENSE"; break;
    case WAVE_PTY_USAGE_INTFERNCE:cctrace = "INTFERNCE"; break;
    case WAVE_PTY_USAGE_SUPRESSOR:cctrace = "SUPRESSOR"; break;
    case WAVE_PTY_USAGE_LASERBURN:cctrace = "LASERBURN"; break;
    case WAVE_PTY_USAGE_VIBRATION:cctrace = "VIBRATION"; break;
    case WAVE_PTY_USAGE_MCRWVBURN:cctrace = "MCRWVBURN"; break;
    case WAVE_PTY_USAGE_SHOCKWAVE:cctrace = "SHOCKWAVE"; break;
    case WAVE_PTY_USAGE_DETECTION:cctrace = "DETECTION"; break;
    case WAVE_PTY_USAGE_EXPLOSION:cctrace = "EXPLOSION"; break;
    case WAVE_PTY_USAGE_SPACEOVLP - 1:cctrace = "SPACEOVLP-1"; break;
    case WAVE_PTY_USAGE_SPACEOVLP:cctrace = "SPACEOVLP"; break;
    case WAVE_PTY_USAGE___COMM___:cctrace = "__COMM___"; break;
    case WAVE_PTY_USAGE__INFRARED:cctrace = "_INFRARED"; break;
    default:
        int cc = 0;
        cc = 1;
        break;
    }


    //UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    //static UINT64 lasttimestampmap = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    //UINT64 interval = fabs(currentTimeStamp - lasttimestampmap);
    ////if (interval > 1000)
    //{
    //	lasttimestampmap = currentTimeStamp;
    //	std::cout <<"usage "<< cctrace.toStdString() << " show sensor index " << sensorinfo.PARAM_sensor_pack_index << " hexindex size:" << hexidxslist.size() << " polygonlist size:" << polygonlist.size() <<" "<< currentTimeStamp  - sensorinfo.PARAM_timestamp << std::endl;
    //}
    //if (!this->isVisible())
    //{
    //	m_hexidxcallbackdata.insert(std::make_pair(qMakePair(uild, uilddst), std::make_tuple(hexidxslist, polygonlist, sensorinfo, eDdisplayMode)));
    //	return;
    //}
 //   std::cout.precision(17);
    //if (!hexidxslist.empty())
    //{
    //	std::cout << cctrace.toStdString() << " " << uild << " " << uilddst << " hexindex size:" << hexidxslist.size() << " polygonlist size:" << polygonlist.size() << " res " << LocationHelper::getresolution(hexidxslist.at(0)) << std::endl;

    //	//if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_DETECTION)
    //	//{
    //	//	std::cout << "========================================================" << std::endl;
    //	//	for (auto ii : polygonlist)
    //	//	{
    //	//		std::cout << "{" << (double)ii.PARAM_latitude / LON_LAT_ACCURACY << "," << (double)ii.PARAM_longitude / LON_LAT_ACCURACY << "}" << std::endl;
    //	//	}
    //	//	std::cout << "========================================================" << std::endl;
    //	//}

    //}
    if (m_maprender == nullptr)
    {
        return;
    }
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }
#ifdef DRAW_HEX
    //	gaeactorenvironment::GaeactorProcessorInterface::getInstance().deal_hexindex_sensor(uild, uilddst, hexidxslist, 0);


    IDENTIFI_EVENT_INFO eventinfo;
    if ((sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SPACEOVLP))
    {

        std::cout <<"-------------- recv :"<< agentid << " " << sensingmediaid << " hexidxslist size:" << hexidxslist.size() <<" polygon size :"<<polygonlist.size()<< "\n";
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_sensor(agentid, sensingmediaid, hexidxslist,transdata_sensorposinfo(),POLYGON_LIST(),eventinfo);
    }
    if ((sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SPACEOVLP - 1))
    {
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_sensor(agentid, sensingmediaid, hexidxslist,transdata_sensorposinfo(),POLYGON_LIST(),eventinfo);
    }
    if (/*(sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_ECHOSENSE) ||*/
//        (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_DETECTION) ||
        (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_INTFERNCE) ||
        (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SUPRESSOR) ||
        (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE__INFRARED))
    {
        //std::cout.precision(17);
        //std::cout << "11112 " << uild << " " << uilddst << " hexindex size:" << hexidxslist.size() << " polygonlist size:" << polygonlist.size() << std::endl;
        gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_sensor(agentid, sensingmediaid, hexidxslist,transdata_sensorposinfo(),POLYGON_LIST(),eventinfo);
    }
#endif

    //if ((sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_ECHOSENSE))
    //{
    //	if (hexidxslist.empty() || polygonlist.empty())
    //	{
    //		std::cout << "+++++++++++++++++++++++++++++++++++++++++++" << uild << " " << uilddst << " " << usage << " " << hexidxslist.size() << " " << polygonlist.size() << std::endl;
    //	}
    //}

    //    UINT64 currentTimeStamp = stdutils::OriDateTime::getCurrentUTCTimeStampMSecs();
    //    static std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>, UINT64> timestampmap;

    //    QPair<TYPE_ULID,TYPE_ULID>  pp = qMakePair(uild, uilddst);
    //    auto timestampmap_itor = timestampmap.find(pp);
    //    if(timestampmap_itor != timestampmap.end())
    //    {
    //        std::cout <<uilddst<< " ++++++++++++++++deal display sensingmedia "<<fabs(currentTimeStamp - timestampmap_itor->second)<<" ms "/*<< hexidxslist.size() */<<std::endl;
    //        timestampmap_itor->second = currentTimeStamp;
    //    }
    //    else
    //    {
    //        timestampmap.insert(std::make_pair(pp, currentTimeStamp));
    //    }

    //此处是为了避免一个实体 多个感知域
    TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(agentid) ^ (std::hash<TYPE_ULID>()(sensingmediaid) << 1);
    if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SPACEOVLP && sensingmediaid == agentid)
    {
        uildsrc = agentid;
    }
    switch (eDdisplayMode)
    {
    case E_DISPLAY_MODE_WAVE:
    {
#ifdef BOTH_SHOW
        drawDataToHex(uildsrc, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON);

        drawDataToHex(uildsrc, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_HEX_POLYGON);
#else
#ifndef SHOW_HEXIDX

        if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE__INFRARED ||
            sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_VIBRATION)
        {
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON);
            drawHex_ex(uildsrc, hexidxslist);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_ECHOSENSE)
        {
            //			std::cout << "ECHOSENSE " << uild << " " << std::endl;
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), false);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_DETECTION)
        {
            //			std::cout  << "DETECTION " << uild << " " << std::endl;
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), false);
//            drawHex_ex(uildsrc, hexidxslist);
        }
        else if (sensorinfo.PARAM_wave_usage == 251)
        {
            //			std::cout  << "DETECTION " << uild << " " << std::endl;
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), false);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_EXPLOSION)
        {
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), true);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_INTFERNCE)
        {
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), false);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SUPRESSOR)
        {
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON, std::unordered_map<H3INDEX, QColor>(), false);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SPACEOVLP)
        {
            m_spacesensorlist.push_back(uildsrc);
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_BOUNDARY_POLYGON);
        }
        else if (sensorinfo.PARAM_wave_usage == WAVE_PTY_USAGE_SPACEOVLP - 1)
        {
            drawDataToHex(uildsrc, agentid, sensingmediaid, hexidxslist, polygonlist, sensorinfo, E_DRAW_TYPE_POINTS);
        }

//        if (hexidxslist.empty())
//        {
//            if (m_maprender)
//            {
//                m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_MULTI, uildsrc + 500);
//            }
//        }
//        else
//        {
//            QColor color(0, 255, 0, 255);
//            std::vector<LAT_LNG> geoLinePosList;

//            auto _entitySpeedLines_tor2 = m_entityPos.find(agentid);
//            if (_entitySpeedLines_tor2 != m_entityPos.end())
//            {
//                LAT_LNG cur_pos = _entitySpeedLines_tor2->second;

//                double distance = 10.0f; // 距离，单位为公里

//                LAT_LNG northLatLng = FunctionAssistant::calculateDestination(cur_pos, distance);
//                LAT_LNG pt = FunctionAssistant::rotatePoint(northLatLng, cur_pos, -sensorinfo.PARAM_wave_direction_azimuth);

//                std::vector<LAT_LNG> geoLinePosList;
//                geoLinePosList.push_back(cur_pos);
//                geoLinePosList.push_back(pt);
//                m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_MULTI, uildsrc + 500, geoLinePosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);

//            }
//        }
#else
        static std::unordered_map<TYPE_ULID, QList<H3INDEX>> polygonlist_;
        if (hexidxslist.empty())
        {
            auto polygonlistitor = polygonlist_.find(uildsrc);
            if (polygonlistitor != polygonlist_.end())
            {
                QList<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                while (itor != polygonlist_[uildsrc].end())
                {
                    H3INDEX oldh3index = *itor;
                    m_maprender->clearElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + oldh3index);
                    itor++;
                }
                polygonlist_[uildsrc].clear();
                polygonlist_.erase(polygonlistitor);
            }
        }
        else
        {
            if (m_maprender)
            {
                QVector<H3INDEX> _existindex;
                if (polygonlist_.find(uildsrc) != polygonlist_.end())
                {
                    QList<H3INDEX>::iterator itor = polygonlist_[uildsrc].begin();
                    while (itor != polygonlist_[uildsrc].end())
                    {
                        H3INDEX oldh3index = *itor;
                        auto h3itor = std::find_if(hexidxslist.begin(), hexidxslist.end(), [&](const uint64_t& val) {
                            return oldh3index == val;
                        });
                        if (h3itor == hexidxslist.end())
                        {
                            m_maprender->clearElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + oldh3index);
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
                    polygonlist_.insert(std::make_pair(uildsrc, QList<H3INDEX>()));
                }

                std::vector<std::tuple<H3INDEX, CellBoundary>> boundarys;
                LocationHelper::getIndexBoundary(boundarys, hexidxslist);
                for (auto boundaryitem : boundarys)
                {
                    H3INDEX h3index = std::get<0>(boundaryitem);
                    if (!_existindex.contains(h3index))
                    {
                        polygonlist_[uildsrc].push_back(h3index);
                        CellBoundary boundary = std::get<1>(boundaryitem);
                        std::vector<LAT_LNG> geoPosList;
                        geoPosList.resize(boundary.numVerts);
                        for (int v = 0; v < boundary.numVerts; v++)
                        {
                            geoPosList[v].lat = LocationHelper::radianToDegree(boundary.verts[v].lat);
                            geoPosList[v].lng = LocationHelper::radianToDegree(boundary.verts[v].lng);
                        }
                        if (!geoPosList.empty())
                        {
                            QColor color(255, 255, 0, 64);
                            switch (sensorinfo.PARAM_wave_usage)
                            {
                            case WAVE_PTY_USAGE_VIBRATION: color = QColor(0, 255, 0, 64);
                                break;
                            case WAVE_PTY_USAGE__INFRARED: color = QColor(255, 0, 0, 64);
                                break;
                            case WAVE_PTY_USAGE_SPACEOVLP: color = QColor(0, 0, 255, 64);
                                break;
                            default:break;
                            }
                            m_maprender->updateElementData(DrawItem::ENUM_TYPE_POLYGON_MULTI, uildsrc + h3index, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false);
                        }
                    }
                }
            }
        }
#endif
#endif
    }break;
    default:break;
    }

}


#define  TEXTLINE_SIZE (13)

#define DSIPLAY_TRACKING_LINE

//#define  DISPLAY_ICON
void MapWidget::displayHexidxPosCallback(const TYPE_ULID& uildsrc, const TYPE_ULID& uilddst, const transdata_entityposinfo& eninfo, E_DISPLAY_MODE eDdisplayMode)
{
//    if ((PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_NORMAL ||
//         PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR) && !eninfo.PARAM_attached)
//    {
//        if (m_pModelWidget2)
//        {
//            if (eninfo.PARAM_pos_hexidx != 0)
//            {
//                LAT_LNG poss = LAT_LNG{ (double)eninfo.PARAM_latitude / LON_LAT_ACCURACY,(double)eninfo.PARAM_longitude / LON_LAT_ACCURACY };
////                QString path = DataManager::getInstance().getModelPath(uildsrc);
//                QString path = QCoreApplication::applicationDirPath() + "/res/model/A350_1000.fbx";
////                double hgt_level = m_pModelWidget2->getPositionElevation(poss.lng,poss.lat)+15;

//                if(!path.isEmpty())
//                {
//                    QColor lineColor = QColor(0,255,0,255);
//                    if (PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE2)
//                    {
//                        lineColor = QColor(255,255,0,255);
//                    }
//                    m_pModelWidget2->initNode(uildsrc,"",0,path,
//                                              poss.lng,
//                                              poss.lat,
//                                              (double)eninfo.PARAM_amsl / 1000.0f,
//                                              eninfo.PARAM_roll,
//                                              eninfo.PARAM_pitch,
//                                              eninfo.PARAM_yaw,0.01,lineColor, QColor(0,255,0,255),1);

//                    m_pModelWidget2->visibleEntityTracking(uildsrc,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL |
//                                                                               (UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_LABEL);
////                    m_pModelWidget2->visibleEntityTracking(uildsrc,false,(UINT64)QtOSGWidget::E_VISIABLE_ATTRIBUTE_TAIL );
//                    //                m_pModelWidget2->updateLineColor(uildsrc,lineColor);
//                }
//                m_pModelWidget2->updateEntityTracking(uildsrc,
//                                                      poss.lng,
//                                                      poss.lat,
//                                                      (double)eninfo.PARAM_amsl / 1000.0f,
//                                                      eninfo.PARAM_roll,
//                                                      eninfo.PARAM_pitch,
//                                                      eninfo.PARAM_yaw);
//            }
//            else
//            {
//                m_pModelWidget2->clearEntityTracking(uildsrc);
//            }
//        }
//    }

    if (m_maprender == nullptr)
    {
        return;
    }

    int c = 0;
    if (PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE1)
    {
        c = 1;
    }
    else if (PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE2)
    {
        c = 2;
    }

    static std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, int> m_entity;
    auto ckk = qMakePair(uildsrc, c);
    if (c == 1 || c == 2)
    {
        if (m_entity.find(ckk) == m_entity.end())
        {
            m_entity.insert(std::make_pair(ckk, 0));
        }
    }

    QString iconPatah;
    if (m_eMapMode == E_MAP_MODE_DISPLAY_REVIEW)
    {
        QString iconPatahtmp = getEntityIcon(uildsrc);
        if (iconPatahtmp.isEmpty())
        {
            DataManager::getInstance().setEntityIconName(uildsrc, "default");
            iconPatahtmp = getEntityIcon(uildsrc);
        }
        iconPatah = QString("./res/icon/userdefined/%1.png").arg(iconPatahtmp);
    }
    else
    {
        if (!(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE))
        {
            QString iconPatahtmp = getEntityIcon(uildsrc);
            if (iconPatahtmp.isEmpty())
            {
                if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SPACE_ENITY)
                {
                    DataManager::getInstance().setEntityIconName(uildsrc, "landinggear");
                }
                else if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES)
                {
                    DataManager::getInstance().setEntityIconName(uildsrc, "flag_");
                }
                else
                {
                    DataManager::getInstance().setEntityIconName(uildsrc, "default");
                }
                iconPatahtmp = getEntityIcon(uildsrc);
            }

            if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES)
            {
                UINT64 itype;
                memcpy(&itype, (BYTE*)eninfo.PARAM_reserved, sizeof(UINT64));

                if (itype == 0)
                {
                    iconPatahtmp += "free";
                }
                else if (itype == 2)
                {
                    iconPatahtmp += "locked";
                }
                else
                {
                    iconPatahtmp += "locking";
                }
                //std::cout << " " << iconPatahtmp.toStdString() << std::endl;
            }
            else
            {
                if ((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE1))
                {
                    iconPatahtmp += "_departure";
                }
                else if ((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE2))
                {
                    iconPatahtmp += "_arrival";
                }
            }
            iconPatah = QString("./res/icon/userdefined/%1.png").arg(iconPatahtmp);
        }
        else
        {
            iconPatah = QString("./res/icon/mark-waypoints.png");

        }
    }
    if (iconPatah.isEmpty())
    {
        return;
    }
    //iconPatah = "./res/icon/userdefined/tank.png";


    //std::cout << uildsrc  <<" "<< iconPatah.toStdString() << std::endl;
    ///////////////////////////////////////////////////////////////////////////////////////////
#ifdef DRAW_HEX
    IDENTIFI_EVENT_INFO eventinfo;
    gaeactorenvironment::GaeactorProcessorInterface::getInstance().update_hexindex_entity(uildsrc, eninfo.PARAM_pos_hexidx, eninfo.PARAM_amsl, transdata_entityposinfo(),eventinfo);
#endif
    /////////////////////////////////////////////////////////////////////////////////////////
    bool bNeedTrans84GC = false;
    if (!SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
    {
        bNeedTrans84GC = true;
    }
    else
    {
        bNeedTrans84GC = m_maprender->bTransfer();
    }

    switch (eDdisplayMode)
    {
    case E_DISPLAY_MODE_ENTITY:
    {
        bool bAdd = false;
        if (eninfo.PARAM_pos_hexidx == 0)
        {
            if (m_maprender)
            {
#ifdef DISPLAY_ICON
                m_maprender->clearEntityItem(uildsrc);
                m_maprender->clearTrackingLineData(uildsrc);
#else
                m_maprender->clearEntityItem(uildsrc);
                auto _entitySpeedLines_tor = m_entitySpeedLines.find(uildsrc);
                if (_entitySpeedLines_tor != m_entitySpeedLines.end())
                {
                    TYPE_ULID uildlinesrc = _entitySpeedLines_tor->second.speedline_id;
                    m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_MULTI, uildlinesrc);
                    m_entitySpeedLines.erase(_entitySpeedLines_tor);
                }
#ifdef DSIPLAY_TRACKING_LINE
                m_maprender->clearElementData(DrawItem::ENUM_TYPE_LINES_MULTI, uildsrc);
#endif
                m_maprender->clearTrackingLineData(uildsrc);
#endif
                m_maprender->clearText(uildsrc);
            }
        }
        else
        {
            //if (eninfo.PARAM_longitude != 0 && eninfo.PARAM_latitude != 0 && eninfo.PARAM_longitude != -2147483648 && eninfo.PARAM_latitude != -2147483648 /*&& pos_eninfo.PARAM_longitude != eninfo.PARAM_longitude && pos_eninfo.PARAM_latitude != eninfo.PARAM_latitude*/)
            {
                std::vector<LAT_LNG> ptlist;
                LAT_LNG pos = LAT_LNG{ (double)eninfo.PARAM_latitude / (LON_LAT_ACCURACY*1.0f),(double)eninfo.PARAM_longitude / (LON_LAT_ACCURACY*1.0f) };
                ptlist.push_back(pos);
//                std::cout.precision(17);
//                std::cout<<uildsrc<<" ---sss--- "<<eninfo.PARAM_longitude<<"-->"<<pos.lng<<" "<<eninfo.PARAM_latitude <<"-->"<<pos.lat<<" \n";

                double speed_v = eninfo.PARAM_speed / 1000.0f;
                if (m_maprender)
                {
#ifdef DISPLAY_ICON
                    m_maprender->appendEntityItem(uildsrc, pos, eninfo.PARAM_pitch, eninfo.PARAM_roll, eninfo.PARAM_yaw, eninfo.PARAM_sensor_property, bNeedTrans84GC);
                    m_maprender->appendTrackingLineData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, uildsrc, ptlist, QColor(255, 255, 0, 255), DrawElements::E_ELEMENT_TYPE_TRACKING, false, bNeedTrans84GC);

#else

                    if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE)
                    {
                        //m_maprender->appendEntityIconPathItem(uildsrc, "./res/icon/mark-waypoints.png", pos, eninfo.PARAM_pitch, eninfo.PARAM_roll, eninfo.PARAM_yaw, eninfo.PARAM_sensor_property,1.0, bNeedTrans84GC);
                    }
                    else
                    {
                        float fsize = 1.0;
                        LAT_LNG posTmp = pos;
                        if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SPACE_ENITY)
                        {
                            fsize = 5;
                        }
                        else if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES)
                        {
                            fsize = 20;
                        }
                        else
                        {
//                            fsize = 25;
                            fsize = 70;
//                            fsize = 100;
//                            fsize = 5000;
                        }
                        m_maprender->appendEntityIconPathItem(uildsrc, iconPatah, posTmp, eninfo.PARAM_pitch, eninfo.PARAM_roll, eninfo.PARAM_yaw, eninfo.PARAM_sensor_property, fsize, bNeedTrans84GC);
#ifdef DSIPLAY_TRACKING_LINE
                        if (!(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SPACE_ENITY))
                        {
                            m_maprender->appendTrackingLineData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, uildsrc, ptlist, QColor(255, 255, 0, 255), DrawElements::E_ELEMENT_TYPE_TRACKING, false, bNeedTrans84GC);
                        }
#endif

                        if (!(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE) &&
                            !(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SPACE_ENITY) &&
                            !(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES))
                        {
                            QColor color(0, 255, 0, 255);
                            std::vector<LAT_LNG> geoLinePosList;

                            auto _entitySpeedLines_tor = m_entitySpeedLines.find(uildsrc);
                            if (_entitySpeedLines_tor == m_entitySpeedLines.end())
                            {
                                tagEntityRunningInfo entityrunninginfo = tagEntityRunningInfo{ FunctionAssistant::generate_random_positive_uint64(),pos ,eninfo.PARAM_timestamp ,pos ,eninfo.PARAM_timestamp, 0, 0, speed_v, 0 };
                                m_entitySpeedLines.insert(std::make_pair(uildsrc, std::move(entityrunninginfo)));
                            }
                            else
                            {
                                tagEntityRunningInfo& entityrunninginfo = _entitySpeedLines_tor->second;

                                double curdis = FunctionAssistant::calc_dist(entityrunninginfo.lstpos, pos);

                                UINT64 timeinterval = eninfo.PARAM_timestamp - entityrunninginfo.lsttimetamp;
                                if (eninfo.PARAM_reserved[3] == 0xff)
                                {
                                    timeinterval = 100;
                                }

                                double sec = (double)(timeinterval) / 1000.0f;
                                double speed_calc = curdis / sec;

                                if (m_peventDriver)
                                {
                                    if (eninfo.PARAM_reserved[3] != 0xff)
                                    {
                                        speed_calc = speed_calc / m_peventDriver->speed_coeff();
                                    }
                                }

                                entityrunninginfo.runningdistance = eninfo.PARAM_reserved[0] / 1000.0;
                                entityrunninginfo.totaldistance = eninfo.PARAM_reserved[1] / 1000.0;

                                entityrunninginfo.lstpos = pos;
                                entityrunninginfo.updateinterval = timeinterval;
                                entityrunninginfo.lsttimetamp = eninfo.PARAM_timestamp;
                                entityrunninginfo.runningspeed = speed_v;
                                entityrunninginfo.calcspeed = speed_calc;
                                entityrunninginfo.calcdistance += curdis;
                            }

                            double distance = speed_v * 10 / 1000.0f; // 距离，单位为公里

                            LAT_LNG northLatLng = FunctionAssistant::calculateDestination(pos, distance);
                            LAT_LNG pt = FunctionAssistant::rotatePoint(northLatLng, pos, -eninfo.PARAM_yaw);


                            geoLinePosList.push_back(pos);
                            geoLinePosList.push_back(pt);

                            TYPE_ULID uildlinesrc = m_entitySpeedLines.at(uildsrc).speedline_id;
                            m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_MULTI, uildlinesrc, geoLinePosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
                        }
                        auto _entitySpeedLines_tor2 = m_entityPos.find(uildsrc);
                        if (_entitySpeedLines_tor2 == m_entityPos.end())
                        {
                            m_entityPos.insert(std::make_pair(uildsrc, pos));
                        }
                        else
                        {
                            _entitySpeedLines_tor2->second = pos;
                        }
                    }
#endif
                }

                if (!(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE) &&
                    !(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SPACE_ENITY) &&
                    !(PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES))
                {
                    QStringList flightInfo;

                    BYTE* PARAM_reserved_PTR = (BYTE*)(eninfo.PARAM_reserved);
                    UINT64* PARAM_agentEntityId = (UINT64*)(PARAM_reserved_PTR + sizeof(INT32) * 2);

                    auto usingflightid = *PARAM_agentEntityId;

                    switch (m_eMapMode)
                    {
                    case E_MAP_MODE_DISPLAY_REVIEW:
                    {
                        auto flightinfos = DataManager::getInstance().get_flightInfo_from_db(usingflightid);

                        {


                            if (flightinfos.flightid == QString::number(usingflightid).toStdString() &&
                                (((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE1) && (flightinfos.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)) ||
                                    ((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE2) && (flightinfos.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR))))
                            {
                                QString timestr = QString::fromStdString(flightinfos.m_PlanDateTimeTakeOff);

                                if (flightinfos.m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
                                {
                                    timestr = QString::fromStdString(flightinfos.m_PlanDateTimeLanding);

                                    if (m_arr.find(uildsrc) == m_arr.end())
                                    {
                                        m_arr.insert(std::make_pair(uildsrc, QString::fromStdString(flightinfos.m_FilghtNumber)));
                                    }
                                }
                                else
                                {
                                    if (m_dep.find(uildsrc) == m_dep.end())
                                    {
                                        m_dep.insert(std::make_pair(uildsrc, QString::fromStdString(flightinfos.m_FilghtNumber)));
                                    }
                                }

                                std::unordered_map<TYPE_ULID, QString> m_arr;

                                std::unordered_map<TYPE_ULID, QString> m_dep;
                                flightInfo.push_back(QString::number(*PARAM_agentEntityId));
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_FilghtNumber));
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_DepArrFlag));
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_PlaneNum));
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_PlaneType));
                                flightInfo.push_back(timestr);
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_Seat) + "-->" + QString::fromStdString(flightinfos.m_Runway));
                                flightInfo.push_back(QString::fromStdString(flightinfos.m_Terminal));

                                flightInfo.push_back(QString::fromStdString(flightinfos.alloc_parkingpoint) + "==>" + QString::fromStdString(flightinfos.alloc_runway));
                            }
                        }
                    }break;
                    case E_MAP_MODE_DISPLAY:
                    {
                        auto flightinfos = DataManager::getInstance().getflightInfo(usingflightid);

                        {
                            tagPath_Plan* path_plan = flightinfos._tagPath_Plan;
                            FlightPlanConf* pFlightPlanConf = flightinfos.pflightdata;


                            if (pFlightPlanConf && path_plan &&
                                (((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE1) && (pFlightPlanConf->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)) ||
                                    ((PROPERTY_GET_RESERVED(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_RESERVED_TYPE2) && (pFlightPlanConf->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR))))
                            {
                                QString timestr = pFlightPlanConf->m_PlanDateTimeTakeOff;

                                if (pFlightPlanConf->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
                                {
                                    timestr = pFlightPlanConf->m_PlanDateTimeLanding;

                                    if (m_arr.find(uildsrc) == m_arr.end())
                                    {
                                        m_arr.insert(std::make_pair(uildsrc, pFlightPlanConf->m_FilghtNumber));
                                    }
                                }
                                else
                                {
                                    if (m_dep.find(uildsrc) == m_dep.end())
                                    {
                                        m_dep.insert(std::make_pair(uildsrc, pFlightPlanConf->m_FilghtNumber));
                                    }
                                }

                                std::unordered_map<TYPE_ULID, QString> m_arr;

                                std::unordered_map<TYPE_ULID, QString> m_dep;
                                flightInfo.push_back(QString::number(*PARAM_agentEntityId));
                                flightInfo.push_back(pFlightPlanConf->m_FilghtNumber);
                                flightInfo.push_back(pFlightPlanConf->m_DepArrFlag);
                                flightInfo.push_back(pFlightPlanConf->m_PlaneNum);
                                flightInfo.push_back(pFlightPlanConf->m_PlaneType);
                                flightInfo.push_back(timestr);
                                flightInfo.push_back(pFlightPlanConf->m_Seat + "-->" + pFlightPlanConf->m_Runway);
                                flightInfo.push_back(pFlightPlanConf->m_Terminal);

                                if (path_plan)
                                {
                                    flightInfo.push_back(path_plan->m_parkingpoint + "==>" + path_plan->m_runway);
                                }
                            }
                        }
                    }; break;
                    default:
                        break;
                    }



                    flightInfo.push_back("===========================");
                    flightInfo.push_back(QString::number(uildsrc));


                    flightInfo.push_back(QString("[经度:%1, 纬度:%2, 高度:%3 m ]")
                        .arg(QString::number(pos.lng, 'f', 5))
                        .arg(QString::number(pos.lat, 'f', 5))
                        .arg(QString::number(eninfo.PARAM_amsl / 1000.0f, 'f', 5)));
                    flightInfo.push_back(QString("速度:%1 m/s").arg(QString::number(speed_v, 'f', 5)));
                    flightInfo.push_back(QString("油量:%1 %").arg(QString::number(eninfo.PARAM_reserved[4] / 1000.0f, 'f', 5)));
                    flightInfo.push_back(QString("成本:%1 元").arg(QString::number(eninfo.PARAM_reserved[5] / 1000.0f, 'f', 5)));
                    //					flightInfo.push_back(QString("计算速度:%1 m/s").arg(QString::number(m_entitySpeedLines.at(uildsrc).calcspeed, 'f', 5)));
                    flightInfo.push_back(QString("运行距离:%1 m").arg(QString::number(m_entitySpeedLines.at(uildsrc).runningdistance, 'f', 5)));
                    //					flightInfo.push_back(QString("计算运行距离:%1 m").arg(QString::number(m_entitySpeedLines.at(uildsrc).calcdistance, 'f', 5)));
                                        flightInfo.push_back(QString("总距离:%1 m").arg(QString::number(m_entitySpeedLines.at(uildsrc).totaldistance, 'f', 5)));
                    //					flightInfo.push_back(QString("剩余距离:%1 m").arg(QString::number(m_entitySpeedLines.at(uildsrc).totaldistance - m_entitySpeedLines.at(uildsrc).runningdistance, 'f', 5)));
                                        //flightInfo.push_back(QString("interval:%1 ms").arg(QString::number(m_entitySpeedLines.at(uildsrc).updateinterval)));

                    double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
                    offset = offset > 0.02 ? 0.02 : offset;
                    if (m_maprender)
                    {
                        ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
                        LAT_LNG offsetpt = LAT_LNG{ pos.lat + offset,pos.lng + 0.0005 };
                        QColor txtcl;
                        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
                        {
                            txtcl = QColor(0, 0, 255);
                        }
                        else
                        {
                            txtcl = QColor(0, 255, 255);
                        }
                        QString txt = flightInfo.join("\n");
                        //						if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                        {
                                m_maprender->drawTextPos(uildsrc, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT, false, alignType);
                        }
                    }
                    if (runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                    {
                        m_maprender->drawTextScreen(m_arr_text_id, QPoint(-540, -80), QColor(0, 255, 0, 255), QString("进港航班数:%1").arg(m_arr.size()), 48, DrawElements::E_ELEMENT_TYPE_SCREENCONTEXT, false, (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER));
                        m_maprender->drawTextScreen(m_dep_text_id, QPoint(-540, -160), QColor(0, 255, 255, 255), QString("离港航班数:%1").arg(m_dep.size()), 48, DrawElements::E_ELEMENT_TYPE_SCREENCONTEXT, false, (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER));
                    }
                }
                else if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_RES)
                {
                    UINT64 iocuppy;
                    memcpy(&iocuppy, (BYTE*)(eninfo.PARAM_reserved) + sizeof(UINT64), sizeof(UINT64));
                    QStringList flightInfo;
                    flightInfo.append(QString::number(iocuppy));
                    double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
                    offset = offset > 0.02 ? 0.02 : offset;

                    if (m_maprender)
                    {
                        ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
                        LAT_LNG offsetpt = LAT_LNG{ pos.lat + offset,pos.lng + 0.0005 };
                        QColor txtcl;
                        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
                        {
                            txtcl = QColor(0, 0, 255);
                        }
                        else
                        {
                            txtcl = QColor(0, 255, 255);
                        }
                        QString txt = flightInfo.join("\n");
                        m_maprender->drawTextPos(uildsrc, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT, false, alignType);
                    }
                }
                else
                {
                    QStringList flightInfo;
                    flightInfo.push_back(QString::number(uildsrc));

                    flightInfo.push_back(QString("[经度:%1, 纬度:%2, 高度:%3 m ]")
                        .arg(QString::number(pos.lng, 'f', 5))
                        .arg(QString::number(pos.lat, 'f', 5))
                        .arg(QString::number(eninfo.PARAM_amsl / 1000.0f, 'f', 5)));
                    flightInfo.push_back(QString("速度:%1 m/s").arg(QString::number(speed_v, 'f', 5)));
                    double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
                    offset = offset > 0.02 ? 0.02 : offset;

                    if (m_maprender)
                    {
                        ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
                        LAT_LNG offsetpt = LAT_LNG{ pos.lat ,pos.lng };
                        QColor txtcl;
                        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
                        {
                            txtcl = QColor(0, 0, 255);
                        }
                        else
                        {
                            txtcl = QColor(0, 255, 255);
                        }

                        QString txt = flightInfo.join("\n");
                        m_maprender->drawTextPos(uildsrc, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT, false, alignType);
                    }
                }
                bAdd = true;
            }

        }
#ifdef USING_PATH_GENERATE
        if (PROPERTY_GET_TYPE(eninfo.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR)
        {
            auto itor = m_node.find(uildsrc);
            if (itor != m_node.end())
            {
                if (!bAdd)
                {

                    if (m_srccombox && m_dstcombox)
                    {
                        disconnect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot);
                        disconnect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot);

                        QString nodename = QString::number(uildsrc);
                        int index1 = m_srccombox->findText(nodename);
                        m_srccombox->removeItem(index1);
                        int index2 = m_dstcombox->findText(nodename);
                        m_dstcombox->removeItem(index2);

                        connect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
                        connect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
                    }
                }
            }
            else
            {
                if (bAdd)
                {
                    LAT_LNG pos = LAT_LNG{ (double)eninfo.PARAM_latitude / LON_LAT_ACCURACY, (double)eninfo.PARAM_longitude / LON_LAT_ACCURACY };
                    uint64_t _index = m_node.size();
                    QString nodename = QString::number(uildsrc);
                    m_node.insert(std::make_pair(uildsrc, std::make_tuple(_index, pos)));
                    if (m_srccombox && m_dstcombox)
                    {
                        disconnect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot);
                        disconnect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot);
                        m_srccombox->addItem(nodename, QVariant(uildsrc));
                        m_dstcombox->addItem(nodename, QVariant(uildsrc));
                        connect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
                        connect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MapWidget::currentIndexChangedSlot, Qt::UniqueConnection);
                    }
                }
            }
        }
#endif

    }break;
    default:break;
    }

}

void MapWidget::displayIntersectionHexidxCallback(const TYPE_ULID& uildsrc, const TYPE_ULID& uilddst, const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX> >& hexidxslistinfo, E_DISPLAY_MODE eDdisplayMode)
{
    if (m_maprender == nullptr)
    {
        return;
    }
}

void MapWidget::displayEchoWaveHexidxCallback(const TYPE_ULID& uildval, const EVENT_TUPLE& echowaveinfo, const HEXIDX_HGT_ARRAY& hexidxslist, const QVector<LAT_LNG>& geolatlnglist, bool bEchoWave)
{
    if (m_maprender == nullptr)
    {
        return;
    }
    const TYPE_ULID& uildsrc = std::get<0>(echowaveinfo);
    const TYPE_ULID& uilddst = std::get<1>(echowaveinfo);
    const TYPE_ULID& sensingmediaid = std::get<2>(echowaveinfo);
    std::vector<LAT_LNG> geoPosList;
    geoPosList.reserve(geolatlnglist.size());
    for (auto geolatlnglistitem : geolatlnglist)
    {
        geoPosList.push_back(geolatlnglistitem);
    }

    if (hexidxslist.empty() && geoPosList.empty())
    {
        if (m_maprender)
        {
            m_maprender->clearElementPairData(DrawItem::ENUM_TYPE_LINES_MULTI, uildsrc, uilddst, sensingmediaid);
            //std::cout << "clear  EchoWaveHexidx " << std::endl;
        }
    }
    else
    {
        if (m_maprender)
        {
            QColor color(255, 0, 255, 32);
            if (bEchoWave)
            {
                color = QColor(0, 0, 255, 64);
            }
            bool bNeedTrans84GC = true;
            m_maprender->updateElementPairData(DrawItem::ENUM_TYPE_LINES_MULTI, uildsrc, uilddst, sensingmediaid, geoPosList, color, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);
        }
    }
}

void MapWidget::dealeventlist_update_callback(const E_EVENT_MODE& eventmode, const std::vector<EVENT_INFO>& eventlist)
{

//    if (!runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
//    {
        return;
//    }
    if (m_maprender == nullptr)
    {
        return;
    }
    bool bNeedTrans84GC = true;

    static std::map<QPair<TYPE_ULID,TYPE_ULID>,std::map<TYPE_ULID,UINT64>> _EVENT_ICON;

    auto updateEventIcon=[&](const EVENT_INFO& eventinfo)
    {
        QString iconPatah = "";
        QPair<TYPE_ULID,TYPE_ULID> _sensorid = qMakePair(eventinfo.m_sensorid, eventinfo.m_sensingmediaid);
        bool bRemove = false;
        switch (eventmode)
        {
        case E_EVENT_MODE_ADD:
        {
            iconPatah = QString("./res/icon/userdefined/flag_free.png");
        }
        break;
        case E_EVENT_MODE_UPDATE:
        {

            iconPatah = QString("./res/icon/userdefined/flag_locking.png");
        }
        break;
        case E_EVENT_MODE_REMOVE:
        {
            iconPatah = QString("./res/icon/userdefined/flag_locked.png");
            bRemove = true;
        }
        break;
        default:
        {

        }break;
        }

        UINT64 _uildsrc = 0;
        auto itor = _EVENT_ICON.find(_sensorid);
        if(itor == _EVENT_ICON.end())
        {
            _uildsrc = FunctionAssistant::generate_random_positive_uint64();
            std::map<TYPE_ULID,UINT64> entitys;
            entitys.insert(std::make_pair(eventinfo.m_entityid, _uildsrc));
            _EVENT_ICON.insert(std::make_pair(_sensorid, entitys));
        }
        else
        {
            std::map<TYPE_ULID,UINT64> &entitys = itor->second;
            auto entity_itor=entitys.find(eventinfo.m_entityid);
            if(entity_itor == entitys.end())
            {
                _uildsrc = FunctionAssistant::generate_random_positive_uint64();
                entitys.insert(std::make_pair(eventinfo.m_entityid, _uildsrc));
            }
            else
            {
                _uildsrc = entity_itor->second;
            }
        }

        float fsize = 3200.0;
        LAT_LNG posTmp{ (double)eventinfo.m_entityposinfo.PARAM_latitude / LON_LAT_ACCURACY, (double)eventinfo.m_entityposinfo.PARAM_longitude / LON_LAT_ACCURACY };
        m_maprender->appendEntityIconPathItem(_uildsrc, iconPatah, posTmp, eventinfo.m_entityposinfo.PARAM_pitch, eventinfo.m_entityposinfo.PARAM_roll, eventinfo.m_entityposinfo.PARAM_yaw, eventinfo.m_entityposinfo.PARAM_sensor_property, fsize, bNeedTrans84GC);

//        if(bRemove)
//        {
//            m_maprender->clearEntityItem(_uildsrc);
//        }
    };
    switch (eventmode)
    {
    case E_EVENT_MODE_ADD:
    case E_EVENT_MODE_UPDATE:
    {
        for (auto item : eventlist)
        {
            updateEventIcon(item);
//            const TYPE_ULID& sensoruild = item.m_sensorid;
//            const TYPE_ULID& entityuilddst = item.m_entityid;
//            const TYPE_ULID& sensingmediaid = item.m_sensingmediaid;
//            const transdata_entityposinfo& sensorpos = item.m_sensorposinfo;
//            const transdata_entityposinfo& entitypos = item.m_entityposinfo;

//            std::vector<LAT_LNG> geoPosList;
//            geoPosList.push_back(LAT_LNG{ (double)sensorpos.PARAM_latitude / LON_LAT_ACCURACY, (double)sensorpos.PARAM_longitude / LON_LAT_ACCURACY });
//            geoPosList.push_back(LAT_LNG{ (double)entitypos.PARAM_latitude / LON_LAT_ACCURACY, (double)entitypos.PARAM_longitude / LON_LAT_ACCURACY });

//            if (m_maprender)
//            {
//                if (m_spacesensorlist.contains(sensoruild) && PROPERTY_GET_TYPE(sensorpos.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE)
//                {
//                    drawDataToUpdateColor(eventmode, sensoruild, item.m_sensorproprety, E_DRAW_TYPE_BOUNDARY_POLYGON);
//                }
//                QColor color = QColor(0, 0, 255, 255);
//                DrawItem::ENUM_TYPE type = DrawItem::ENUM_TYPE_LINES_MULTI;

//                DrawElements::E_ELEMENT_TYPE elementType = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_ENTITY;
//                if (item.m_entityisSensorProprety)
//                {
//                    color = QColor(0, 255, 255, 128);
//                    type = DrawItem::ENUM_TYPE_LINES_STRIP_MULTI;
//                    elementType = DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_SENSOR;
//                }
//                m_maprender->updateElementPairData(type, sensoruild, entityuilddst, sensingmediaid, geoPosList, color, elementType, item.m_entityisSensorProprety, false, bNeedTrans84GC);
//                //if (eventmode == E_EVENT_MODE_ADD && item.m_entityisSensorProprety)
//                //{
//                //	std::pair<uint64_t, uint64_t> srcid_dstid_pairkey = std::make_pair(sensoruild, entityuilddst);
//                //	std::pair<uint64_t, uint64_t> dstid_srcid_pairkey = std::make_pair(entityuilddst, sensoruild);

//                //	int offset = 0;
//                //	auto srcid_dstid_itor = m_linepair.find(srcid_dstid_pairkey);
//                //	auto dstid_srcid_itor = m_linepair.find(dstid_srcid_pairkey);
//                //	//自己(正向 a->b) 和 反向(b->a) 均不存在

//                //	if (srcid_dstid_itor == m_linepair.end() && dstid_srcid_itor == m_linepair.end())
//                //	{
//                //		offset = -1;
//                //	}
//                //	//自己(正向 a->b)不存在 和 反向(b->a) 存在
//                //	else if ((srcid_dstid_itor == m_linepair.end() && dstid_srcid_itor != m_linepair.end()))
//                //	{
//                //		offset = 1;
//                //	}
//                //	else if ((srcid_dstid_itor != m_linepair.end() && dstid_srcid_itor != m_linepair.end()))
//                //	{
//                //	}

//                //	auto itor = m_linepair.find(srcid_dstid_pairkey);
//                //	if (itor != m_linepair.end())
//                //	{
//                //		int &posoffset = std::get<0>(itor->second);
//                //		uint64_t &id = std::get<1>(itor->second);
//                //
//                //		QString nodename = QString::number(sensoruild) + "->" + QString::number(entityuilddst);
//                //		m_maprender->drawText(id, LAT_LNG{(geoPosList.at(0).lat + geoPosList.at(1).lat) / 2 + 0.001 * posoffset,  (geoPosList.at(0).lng + geoPosList.at(1).lng) / 2 + 0.001 * posoffset }, QColor(255, 255, 0), nodename, 0.5, true);
//                //	}
//                //	else
//                //	{
//                //		uint64_t id = m_snowflake.GetId();
//                //		m_linepair.insert(std::make_pair(srcid_dstid_pairkey, std::make_tuple(offset, id)));
//                //		QString nodename = QString::number(sensoruild) + "->" + QString::number(entityuilddst);
//                //		m_maprender->drawText(id, LAT_LNG{(geoPosList.at(0).lat + geoPosList.at(1).lat) / 2 + 0.001 * offset, (geoPosList.at(0).lng + geoPosList.at(1).lng) / 2 + 0.001 * offset }, QColor(255, 255, 0), nodename, 0.5, true);
//                //	}
//                //}

//            }
//            if (eventmode == E_EVENT_MODE_ADD)
//            {
//                m_ieventcount++;
//                m_ieventcount_plus++;
//            }
        }
    }
    break;
    case E_EVENT_MODE_REMOVE:
    {
        for (auto item : eventlist)
        {            
            updateEventIcon(item);
//            const TYPE_ULID& sensoruild = item.m_sensorid;
//            const TYPE_ULID& entityuilddst = item.m_entityid;
//            const TYPE_ULID& sensingmediaid = item.m_sensingmediaid;

//            const transdata_entityposinfo& sensorpos = item.m_sensorposinfo;
//            if (m_maprender)
//            {
//                DrawItem::ENUM_TYPE type = DrawItem::ENUM_TYPE_LINES_MULTI;
//                if (item.m_entityisSensorProprety)
//                {
//                    type = DrawItem::ENUM_TYPE_LINES_STRIP_MULTI;
//                }
//                m_maprender->clearElementPairData(type, sensoruild, entityuilddst, sensingmediaid);
//            }

//            if (m_spacesensorlist.contains(sensoruild) && PROPERTY_GET_TYPE(sensorpos.PARAM_sensor_property) == AGENT_ENTITY_PROPERTY_SENSOR_SPACE)
//            {
//                drawDataToUpdateColor(eventmode, sensoruild, item.m_sensorproprety, E_DRAW_TYPE_BOUNDARY_POLYGON);
//            }

//            if (item.m_entityisSensorProprety)
//            {

//                EVENT_KEY_TYPE srcid_dstid_pairkey = EVENT_KEY_TYPE{ sensoruild, entityuilddst, sensingmediaid };

//                auto itor = m_linepair.find(srcid_dstid_pairkey);
//                if (itor != m_linepair.end())
//                {
//                    uint64_t& id = std::get<1>(itor->second);
//                    if (m_maprender)
//                    {
//                        m_maprender->clearText(id);
//                    }
//                }
//            }

//            m_ieventcount--;
//            m_ieventcount_sub++;
        }
    }
    break;
    default:break;
    }
}

void MapWidget::dealpath_update_callback(const TYPE_ULID& src, const TYPE_ULID& dst, const tagPathInfo& path)
{
    if (m_maprender == nullptr)
    {
        return;
    }
#ifdef USING_PATH_GENERATE
    QColor color(255, 255, 0, 255);
    if (!path.m_bValid)
    {
        color = QColor(255, 0, 0, 255);
    }
    std::vector<LAT_LNG> geoPosList;
    QString tracestr;

    if (!path.m_path.empty())
    {
        tracestr = "start: v" + QString::number(path.m_path.at(0).m_index) + "_" + QString::number(path.m_path.at(0).m_id) + "---> dest:" + "v" + QString::number(path.end.m_index) + "_" + QString::number(path.end.m_id) + " ptah:";
    }
    geoPosList.reserve(path.m_path.size());
    for (auto geoposlistitem : path.m_path)
    {
        tracestr += "-->v" + QString::number(geoposlistitem.m_index) + "_" + QString::number(geoposlistitem.m_id);
        geoPosList.push_back(LAT_LNG{ geoposlistitem.m_pos.lat - 0.001, geoposlistitem.m_pos.lng });
    }

    tracestr += " value ";
    if (!path.m_bValid)
    {
        tracestr += " oo ";
        geoPosList.clear();
    }
    else
    {
        tracestr += QString::number(path.value);
    }

    if (geoPosList.size() <= 2)
    {
        if (m_maprender)
        {
            m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_MULTI, m_routeid, geoPosList, color, DrawElements::E_ELEMENT_TYPE_ROUTE, false);
        }
    }
    else
    {
        if (m_maprender)
        {
            m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, m_routeid, geoPosList, color, DrawElements::E_ELEMENT_TYPE_ROUTE, false);
        }
    }

    m_textLabel->setText(tracestr);
#endif
}

void MapWidget::dealsensor_update_callback(const TYPE_ULID& src, const E_EVENT_MODE& type)
{
    if (m_maprender == nullptr)
    {
        return;
    }
    //auto itor = m_node.find(src);
    //if (itor != m_node.end())
    //{
    //	disconnect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWGT::currentIndexChangedSlot);
    //	disconnect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWGT::currentIndexChangedSlot);

    //	QString nodename = "v" + QString::number(std::get<0>(itor->second)) + "_" + QString::number(itor->first);
    //	switch (type)
    //	{
    //	case E_EVENT_MODE_ADD:
    //	{
    //		static int32_t cc = 0;
    //		cc++;
    //		std::cout << "count " << cc << std::endl;
    //		m_ordername.insert(std::get<0>(itor->second), std::make_tuple(src, nodename));
    //		//m_srccombox->addItem(nodename, QVariant(itor->first));
    //		//m_dstcombox->addItem(nodename, QVariant(itor->first));

    //	}break;
    //	case E_EVENT_MODE_REMOVE:
    //	{
    //		m_ordername.remove(std::get<0>(itor->second));
    //		//int index1 = m_srccombox->findText(nodename);
    //		//m_srccombox->removeItem(index1);

    //		//int index2 = m_dstcombox->findText(nodename);
    //		//m_dstcombox->removeItem(index2);
    //	}
    //	break;
    //	default:
    //		break;
    //	}

    //	QStringList lisval;
    //	for (auto item : m_ordername.keys())
    //	{
    //		lisval.push_back(std::get<1>(m_ordername.value(item)));
    //	}
    //	m_srccombox->clear();
    //	m_dstcombox->clear();
    //
    //	m_srccombox->addItems(lisval);
    //	m_dstcombox->addItems(lisval);

    //	connect(m_srccombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWGT::currentIndexChangedSlot, Qt::UniqueConnection);
    //	connect(m_dstcombox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &MainWGT::currentIndexChangedSlot, Qt::UniqueConnection);
    //}
}

void MapWidget::dealAgentRelationInfo_update_callback(const msg::AgentRelationInfo::msg_AgentRelationInfo* agentrelationdata)
{
    if (m_maprender == nullptr || agentrelationdata == nullptr)
    {
        return;
    }
    bool bNeedTrans84GC = true;
    if (m_maprender)
    {
        uint64_t srcid = QString::fromStdString(agentrelationdata->src_agent().agentid()).toULongLong();
        uint64_t dstid = QString::fromStdString(agentrelationdata->dst_agent().agentid()).toULongLong();
        uint64_t sensingmediaid = QString::fromStdString(agentrelationdata->src_agent().sensingmediaid()).toULongLong();

        TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(srcid) ^ (std::hash<TYPE_ULID>()(dstid) << 1);
        switch (agentrelationdata->refreshtype()) {
        case msg::AgentRelationInfo::E_REFRESHTYPE_ADD:
        case msg::AgentRelationInfo::E_REFRESHTYPE_UPDATE:
        {
            std::vector<LAT_LNG> geoPosList;
            geoPosList.push_back(LAT_LNG{ agentrelationdata->src_agent().lat(), agentrelationdata->src_agent().lon() });
            geoPosList.push_back(LAT_LNG{ agentrelationdata->dst_agent().lat(), agentrelationdata->dst_agent().lon() });

            bool drawline = true;
            float fsize = 30.0;
            QString iconPatah;
            LAT_LNG pos;
            bool showtext = false;
            QColor color = QColor(0, 0, 255, 255);
            DrawItem::ENUM_TYPE type = DrawItem::ENUM_TYPE_LINES_MULTI;
            float linewidthScale = 1.0f;
            switch (agentrelationdata->relationtype()) {
            case msg::AgentRelationInfo::E_RELATIONTYPE_COMM:
            {
//                return;
                color = QColor(0, 255, 0, 255);
            }break;
            case msg::AgentRelationInfo::E_RELATIONTYPE_TRACK:
            {
//                return;
                color = QColor(255, 0, 0, 255);
            }break;
            case msg::AgentRelationInfo::E_RELATIONTYPE_DETECT:
            {
//                return;
                switch (agentrelationdata->usage()) {
                case 0x01:color = QColor(0, 255, 255, 255); break;
                case 0x08:color = QColor(255, 0, 255, 255); break;
                default:
                    break;
                }
                showtext = true;
            } break;
            case msg::AgentRelationInfo::E_RELATIONTYPE_IFF_REQUEST:
            {
                return;
                type = DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI;
                if (m_maprender->isContainsPair(type, dstid, srcid, msg::AgentRelationInfo::E_RELATIONTYPE_IFF_RESPONSE))
                {
                    color = QColor(0, 255, 0, 255);
                    m_maprender->clearEntityItem(uildsrc);
                }
                else
                {
                    color = QColor(255, 0,0, 255);
                    pos.lat = geoPosList.at(1).lat;
                    pos.lng = geoPosList.at(1).lng;
                    iconPatah = "./res/icon/userdefined/threat.png";
                    fsize = 50.0 * 30;
                    m_maprender->appendEntityIconPathItem(uildsrc, iconPatah, pos, 0, 0, 0, false, fsize, bNeedTrans84GC);
                }
                linewidthScale = 20.0;
                drawline = false;
            }break;
            case msg::AgentRelationInfo::E_RELATIONTYPE_IFF_RESPONSE:
            {
                return;
                type = DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI;
                if (m_maprender->isContainsPair(type, dstid, srcid, msg::AgentRelationInfo::E_RELATIONTYPE_IFF_REQUEST))
                {
                    color = QColor(0, 255, 0, 255);
                }
                else 
                {
                    color = QColor(255, 0, 255, 255);
                }
                linewidthScale = 15.0;
                drawline = false;
            }break;

            case msg::AgentRelationInfo::E_RELATIONTYPE_IFF_THREAT:
            {
                return;
                //type = DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI;
                //if (m_maprender->isContainsPair(type, dstid, srcid, msg::AgentRelationInfo::E_RELATIONTYPE_IFF_RESPONSE))
                //{
                //    drawline = false;
                //    fsize = 50.0*30;
                //    iconPatah = "./res/icon/userdefined/threat.png";
                //}
                //else
                //{
                //    drawline = false;
                //    fsize = 50.0 * 30;
                //    iconPatah = "./res/icon/userdefined/threat.png";
                //}

                //pos.lat = geoPosList.at(1).lat;
                //pos.lng = geoPosList.at(1).lng;

                //m_maprender->appendEntityIconPathItem(uildsrc, iconPatah, pos, 0, 0, 0, false, fsize, bNeedTrans84GC);

                drawline = false;
            }break;
            case msg::AgentRelationInfo::E_RELATIONTYPE_IFF_NOTHREAT:
            {
                return;

                drawline = false;
                fsize = 50.0 * 35;
                iconPatah = "./res/icon/userdefined/nothreat.png";

                pos.lat = geoPosList.at(0).lat;
                pos.lng = geoPosList.at(0).lng;

                m_maprender->appendEntityIconPathItem(uildsrc, iconPatah, pos, 0, 0, 0, false, fsize, bNeedTrans84GC);
            }break;
            default:
                drawline = false;
                break;
            }
            if (drawline)
            {
                m_maprender->updateElementPairData(type, srcid, dstid, sensingmediaid, geoPosList, color, DrawElements::E_ELEMENT_TYPE_INTERSECTION_SENSOR_ENTITY, false, false, bNeedTrans84GC, linewidthScale);
            }


            if (showtext)
            {
                EVENT_KEY_TYPE srcid_dstid_pairkey = EVENT_KEY_TYPE{ srcid, dstid, sensingmediaid };

                uint64_t id = FunctionAssistant::generate_random_positive_uint64();
                auto itor = m_linepair.find(srcid_dstid_pairkey);
                if (itor != m_linepair.end())
                {
                    id = std::get<1>(itor->second);
                }
                else
                {
                    m_linepair.insert(std::make_pair(srcid_dstid_pairkey, std::make_tuple(0, id)));
                }

                LAT_LNG pos;
                pos.lat = (geoPosList.at(0).lat + geoPosList.at(1).lat) / 2;
                pos.lng = (geoPosList.at(0).lng + geoPosList.at(1).lng) / 2;
                double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
                offset = offset > 0.02 ? 0.02 : offset;

                ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
                LAT_LNG offsetpt = LAT_LNG{ pos.lat + offset,pos.lng + 0.0005 };
                QColor txtcl;
                if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
                {
                    txtcl = QColor(0, 0, 255);
                }
                else
                {
                    txtcl = QColor(0, 255, 255);
                }
                QString txt = QString("距离:%1 m").arg(QString::number(agentrelationdata->distance(), 'f', 5));
                m_maprender->drawTextPos(id, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT, false, alignType);
            }
        }
        break;
        case msg::AgentRelationInfo::E_REFRESHTYPE_REMOVE:
        {
            if (m_maprender)
            {
                DrawItem::ENUM_TYPE type = DrawItem::ENUM_TYPE_LINES_MULTI;
                m_maprender->clearElementPairData(type, srcid, dstid, sensingmediaid);

                EVENT_KEY_TYPE srcid_dstid_pairkey = EVENT_KEY_TYPE{ srcid, dstid, sensingmediaid };

                auto itor = m_linepair.find(srcid_dstid_pairkey);
                if (itor != m_linepair.end())
                {
                    uint64_t& id = std::get<1>(itor->second);
                    m_maprender->clearText(id);
                }
                m_maprender->clearElementPairData(DrawItem::ENUM_TYPE_LINES_STIPPLE_MULTI, srcid, dstid, sensingmediaid);
                m_maprender->clearEntityItem(uildsrc);
            }

        }break;
        default:
            break;
        }
    }
}

void MapWidget::dealAgentCommSnrInfo_update_callback(const msg::AgentCommSnrInfo::msg_AgentCommSnrInfo* agentrelationdata)
{
    if (m_maprender == nullptr || agentrelationdata == nullptr)
    {
        return;
    }
    bool bNeedTrans84GC = true;
    if (m_maprender)
    {

        std::vector<LAT_LNG> geoPosList;


        auto a_entitySpeedLines_tor = m_entitySpeedLines.find(QString::fromStdString(agentrelationdata->src_agent()).toULongLong());
        if (a_entitySpeedLines_tor != m_entitySpeedLines.end())
        {
            geoPosList.push_back(a_entitySpeedLines_tor->second.lstpos);
        }
        auto b_entitySpeedLines_tor = m_entitySpeedLines.find(QString::fromStdString(agentrelationdata->dst_agent()).toULongLong());
        if (b_entitySpeedLines_tor != m_entitySpeedLines.end())
        {
            geoPosList.push_back(b_entitySpeedLines_tor->second.lstpos);
        }

        if (geoPosList.size() == 2)
        {

            QColor color = QColor(0, 0, 255, 255);

            EVENT_KEY_TYPE srcid_dstid_pairkey;
            if (agentrelationdata->positive())
            {
                srcid_dstid_pairkey = EVENT_KEY_TYPE{ QString::fromStdString(agentrelationdata->src_agent()).toULongLong(), QString::fromStdString(agentrelationdata->dst_agent()).toULongLong(), QString::fromStdString(agentrelationdata->sensingmediaid()).toULongLong() };
            }
            else
            {
                srcid_dstid_pairkey = EVENT_KEY_TYPE{ QString::fromStdString(agentrelationdata->dst_agent()).toULongLong(), QString::fromStdString(agentrelationdata->src_agent()).toULongLong(), QString::fromStdString(agentrelationdata->sensingmediaid()).toULongLong() };
            }

            uint64_t id = FunctionAssistant::generate_random_positive_uint64();
            auto itor = m_linepair.find(srcid_dstid_pairkey);
            if (itor != m_linepair.end())
            {
                id = std::get<1>(itor->second);
            }
            else
            {
                m_linepair.insert(std::make_pair(srcid_dstid_pairkey, std::make_tuple(0, id)));
            }

            LAT_LNG pos;
            pos.lat = (geoPosList.at(0).lat + geoPosList.at(1).lat) / 2;
            pos.lng = (geoPosList.at(0).lng + geoPosList.at(1).lng) / 2;
            double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
            offset = offset > 0.02 ? 0.02 : offset;

            ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
            LAT_LNG offsetpt = LAT_LNG{ pos.lat + offset,pos.lng + 0.0005 };
            QColor txtcl;
            if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
            {
                txtcl = QColor(0, 0, 255);
            }
            else
            {
                txtcl = QColor(0, 255, 255);
            }
            QStringList flightInfo;
            flightInfo.append(QString("snr:%1 ").arg(QString::number(agentrelationdata->snr(), 'f', 5)));
            flightInfo.append(QString("sinr:%1 ").arg(QString::number(agentrelationdata->sinr(), 'f', 5)));
            flightInfo.append(QString("freqb:%1 ").arg(QString::number(agentrelationdata->freqhzb(), 'f', 5)));
            flightInfo.append(QString("biffcode:%1 ").arg(QString::fromStdString(agentrelationdata->b_iffcode())));
            flightInfo.append(QString("aiffcode:%1 ").arg(QString::fromStdString(agentrelationdata->a_iffcode())));
            flightInfo.append(QString("index:%1 ").arg(agentrelationdata->packindex()));

            QString txt = flightInfo.join("\n");
            m_maprender->drawTextPos(id, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_TRACKING_CONTEXT, false, alignType);
        }
    }
}

void MapWidget::dealSmdInfo_update_callback(const ::msg::AgentPositionInfo::msg_transdata_smd * smdinfodata)
{
    if (m_maprender == nullptr || smdinfodata == nullptr)
    {
        return;
    }
    UINT64 agentid = QString::fromStdString(smdinfodata->m_sensorid()).toULongLong();
    UINT64 sensingmediaid = QString::fromStdString(smdinfodata->m_sensingmediaid()).toULongLong();

    TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(agentid) ^ (std::hash<TYPE_ULID>()(sensingmediaid) << 1);

    if(smdinfodata->bvalid())
    {
        LAT_LNG center{smdinfodata->lat(), smdinfodata->lon()};
        double xx = smdinfodata->direction_azimuth();
        double startangle_= xx + smdinfodata->dazm()/2;
        double spanangle_ = smdinfodata->dazm();
        double radius = smdinfodata->radius()*1.2;
        double rotationSpeed = 10.0;

            QColor color= QColor(0, 255, 0, 255);

        switch (smdinfodata->usage_type())
        {
        case WAVE_PTY_USAGE_UNDEFINED:break;
        case WAVE_PTY_USAGE_ECHOSENSE:color =  QColor(255, 255, 0, 255); break;
        case WAVE_PTY_USAGE_INTFERNCE:color = QColor(192, 0, 192, 255); break;
        case WAVE_PTY_USAGE_SUPRESSOR:color = QColor(128, 0, 128, 255); break;
        case WAVE_PTY_USAGE_LASERBURN:break;
        case WAVE_PTY_USAGE_VIBRATION:break;
        case WAVE_PTY_USAGE_MCRWVBURN:break;
        case WAVE_PTY_USAGE_SHOCKWAVE:break;
        case WAVE_PTY_USAGE_DETECTION:color= QColor(0, 255, 0, 255); break;
        case WAVE_PTY_USAGE_EXPLOSION:color = QColor(255, 0, 255, 255); break;
        case WAVE_PTY_USAGE_SPACEOVLP - 1:break;
        case WAVE_PTY_USAGE_SPACEOVLP:break;
        case WAVE_PTY_USAGE___COMM___:break;
        case WAVE_PTY_USAGE__INFRARED:break;
        default:
            break;
        }
        drawRader(uildsrc,center,startangle_,spanangle_,radius,rotationSpeed,color);
    }
    else
    {
        m_maprender->clearElementData(DrawItem::ENUM_TYPE_RADAR, uildsrc);
    }
}

void MapWidget::dealPrejdugement_update_callback(const msg::AgentPositionInfo::msg_transprejusdgmentline *smdinfodata)
{
    if (m_maprender == nullptr || smdinfodata == nullptr)
    {
        return;
    }

    bool bNeedTrans84GC = true;
    std::vector<LAT_LNG> geoPosList;
    geoPosList.resize(smdinfodata->prejudmentline_size());
    for (int i = 0; i < smdinfodata->prejudmentline_size(); i++)
    {
        geoPosList[i].lat = smdinfodata->prejudmentline().at(i).param_latitude();
        geoPosList[i].lng = smdinfodata->prejudmentline().at(i).param_longitude();
    }

    QColor line_cl = QColor(smdinfodata->cl_r(), smdinfodata->cl_g(), smdinfodata->cl_b(), 255);

    line_cl = QColor(255, 0, 255, 255);

    TYPE_ULID uildsrc = std::hash<TYPE_ULID>()(QString::fromStdString(smdinfodata->targetindex()).toULongLong()) ^ (std::hash<TYPE_ULID>()(QString::fromStdString(smdinfodata->handlekey()).toULongLong()) << 1);

    m_maprender->updateElementData(DrawItem::ENUM_TYPE_LINES_STRIP_MULTI, uildsrc, geoPosList, line_cl, (DrawElements::E_ELEMENT_TYPE)(DrawElements::E_ELEMENT_TYPE_SENSOR | DrawElements::E_ELEMENT_TYPE_WAVE), false, bNeedTrans84GC);

}

void MapWidget::dealAgentCommStackInfo_update_callback(const msg::CommStackFrameResultElement::msg_CommStackFrameResultElement* agentrelationdata)
{
    if (m_maprender == nullptr || agentrelationdata == nullptr)
    {
        return;
    }
    bool bNeedTrans84GC = true;

    std::vector<LAT_LNG> geoPosList;


    auto a_entitySpeedLines_tor = m_entitySpeedLines.find(QString::fromStdString(agentrelationdata->src_agent()).toULongLong());
    if (a_entitySpeedLines_tor != m_entitySpeedLines.end())
    {
        geoPosList.push_back(a_entitySpeedLines_tor->second.lstpos);
    }
    auto b_entitySpeedLines_tor = m_entitySpeedLines.find(QString::fromStdString(agentrelationdata->dst_agent()).toULongLong());
    if (b_entitySpeedLines_tor != m_entitySpeedLines.end())
    {
        geoPosList.push_back(b_entitySpeedLines_tor->second.lstpos);
    }

    if (geoPosList.size() == 2)
    {

        QColor color = QColor(0, 0, 255, 255);

        EVENT_KEY_TYPE srcid_dstid_pairkey;
        srcid_dstid_pairkey = EVENT_KEY_TYPE{ QString::fromStdString(agentrelationdata->src_agent()).toULongLong(), QString::fromStdString(agentrelationdata->dst_agent()).toULongLong(), 0 };


        uint64_t id = FunctionAssistant::generate_random_positive_uint64();
        auto itor = m_linepair2.find(srcid_dstid_pairkey);
        if (itor != m_linepair2.end())
        {
            id = std::get<1>(itor->second);
        }
        else
        {
            m_linepair2.insert(std::make_pair(srcid_dstid_pairkey, std::make_tuple(0, id)));
        }

        LAT_LNG pos;
        pos.lat = geoPosList.at(1).lat;
        pos.lng = geoPosList.at(1).lng;
        double offset = 0.02 * (0.015 / 1 / m_maprender->getScale());
        offset = offset > 0.02 ? 0.02 : offset;

        ENUM_TEXT_ALIGN alignType = (ENUM_TEXT_ALIGN)(ENUM_TEXT_ALIGN_HOR_LEFT | ENUM_TEXT_ALIGN_VER_CENTER);
        LAT_LNG offsetpt = LAT_LNG{ pos.lat + offset,pos.lng - 0.001 };
        QColor txtcl;
        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        {
            txtcl = QColor(0, 0, 255);
        }
        else
        {
            txtcl = QColor(0, 255, 255);
        }
        QStringList flightInfo;
        flightInfo.append(QString("ber:%1 ").arg(QString::number(agentrelationdata->ber(), 'f', 5)));
        flightInfo.append(QString("fer:%1 ").arg(QString::number(agentrelationdata->fer(), 'f', 5)));

        QString txt = flightInfo.join("\n");
        m_maprender->drawTextPos(id, offsetpt, txtcl, txt, DEFAULT_FONT_SIZE, DrawElements::E_ELEMENT_TYPE_NORMALCONTEXT, false, alignType);
    }
}

void MapWidget::initRenderThread()
{
    auto context = QOpenGLContext::currentContext();
    auto mainSurface = context->surface();

    auto renderSurface = new QOffscreenSurface(nullptr, this);
    renderSurface->setFormat(context->format());
    renderSurface->create();

    context->doneCurrent();
    m_thread = new MapRenderThread(renderSurface, context, this);
    context->makeCurrent(mainSurface);

    connect(m_thread, &MapRenderThread::imageReady, this, [this]() {
        update();
    }, Qt::QueuedConnection);
    m_thread->start();

}

QString MapWidget::getEntityIcon(const TYPE_ULID& uildsrc)
{
    auto icon = DataManager::getInstance().getEntityIcon(uildsrc);
    if (icon.isEmpty())
    {
        requestEntityIconSlot(uildsrc);
    }
    return icon;
}
#ifdef USING_PATH_GENERATE
void MapWidget::currentIndexChangedSlot(const QString& index)
{
    if (m_eMapMode == E_MAP_MODE_DISPLAY)
    {
        if (nullptr == m_srccombox || nullptr == m_dstcombox || m_srccombox->count() == 0 || m_dstcombox->count() == 0)
        {
            return;
        }
        QString srcstr = m_srccombox->currentText();
        QString dststr = m_dstcombox->currentText();
        uint64_t src = srcstr.toULongLong();
        uint64_t dst = dststr.toULongLong();

        auto srcitor = m_node.find(src);

        QColor txtcl;
        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_showmap)
        {
            txtcl = QColor(255, 255, 0);
        }
        else
        {
            txtcl = QColor(0, 255, 255);
        }

        if (srcitor != m_node.end())
        {
            m_maprender->drawText(m_routesrcid, LAT_LNG{ std::get<1>(srcitor->second).lat + 0.02,std::get<1>(srcitor->second).lng }, QColor(255, 255, 0), m_srccombox->currentText(), 1.0, DrawElements::E_ELEMENT_TYPE_ID_CONTEXT, true);
        }

        auto dstitor = m_node.find(dst);
        if (dstitor != m_node.end())
        {
            m_maprender->drawText(m_routedstid, LAT_LNG{ std::get<1>(dstitor->second).lat + 0.02,std::get<1>(dstitor->second).lng }, QColor(255, 255, 0), m_dstcombox->currentText(), 1.0, DrawElements::E_ELEMENT_TYPE_ID_CONTEXT, true);
        }
        std::cout << "requset path " << src << " to " << dst << std::endl;
        if (m_pGaeactorManager)
        {
            m_pGaeactorManager->dealSensorPath(src, dst);
        }
    }
}
#endif

void MapWidget::requestEntityIconSlot(const TYPE_ULID& uildsrc)
{
    {
        QMutexLocker locker(&m_requestListmutex);
        m_requestList.push_back(uildsrc);
    }
    m_dealfullCond.wakeAll();
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    int delta = event->delta();
    QPoint curptv = event->pos();
    bool    flag = delta > 0 ? true : false;
    m_maprender->zoomByPosition(flag, curptv);
    m_maprender->requestTiles();
    QOpenGLWidget::wheelEvent(event);
}

void setsH3Index(H3INDEX* hp, int res, int baseCell, Direction initDigit) {
    H3INDEX h = H3_INIT;
    H3_SET_MODE(h, H3_CELL_MODE);
    H3_SET_RESOLUTION(h, res);
    H3_SET_BASE_CELL(h, baseCell);
    for (int r = 1; r <= res; r++) H3_SET_INDEX_DIGIT(h, r, initDigit);
    *hp = h;
}


void MapWidget::showEvent(QShowEvent* event)
{
    if (m_pUpdateTimer && !m_pUpdateTimer->isActive())
    {
        //		m_pUpdateTimer->start(16);
        m_pUpdateTimer->start(33);
    }
    QOpenGLWidget::showEvent(event);
}

void MapWidget::hideEvent(QHideEvent* event)
{
    if (m_pUpdateTimer && m_pUpdateTimer->isActive())
    {
        m_pUpdateTimer->stop();
    }
    QOpenGLWidget::hideEvent(event);
}
