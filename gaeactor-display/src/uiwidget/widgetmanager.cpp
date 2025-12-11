#include "widgetmanager.h"

#include <QVBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QStackedWidget>
#include "mapwidget.h"
#include "runtimelistwidget.h"
#include "agentslistwidget.h"
#include "instagentslistwidget.h"
#include "sostepslistwidget.h"
#include "replaywidget.h"
#include "runningwidget.h"
#include "settingswidget.h"
#include "trackingwidget.h"
#include "widget3d/QtOsgWidget.h"
#include "../components/global_variables.h"


#include "src/storage/OriginalDataInputManager.h"
#include "src/storage/OriginalDataOutputManager.h"
#include "../httpserver/httpserver.h"
#include "./src/OriginalThread.h"
#include "./src/OriginalDateTime.h"
#include "../components/configmanager.h"
#include "settingsconfig.h"

#include <QDir>
#include "runningmodeconfig.h"
WidgetManager::WidgetManager(QWidget *parent)
    :QWidget(parent),
    m_qmlWidget(nullptr),
    m_icurrentShowItem(0),
    m_peventDriver(nullptr)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
    }

    m_phttpserver = new HttpServer(this);
    m_phttpserver->setDataCallback(std::bind(&WidgetManager::httpdatareceive_callback, this, std::placeholders::_1, std::placeholders::_2));

    m_pHttpServerRunningThread = new stdutils::OriThread(std::bind(&WidgetManager::thread_httpserver_callback_Loop, this, std::placeholders::_1), this);

    m_qmlWidget = new QQuickWidget();
    m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_qmlWidget->rootContext()->setContextProperty("parentWidget",this);
    QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);

    m_qmlWidget->setSource(QUrl("qrc:/qml/navigatebar.qml"));

    QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
    //QObject *pRoot = (QObject*)pWidget->rootObject();
    if (pRoot != NULL) {
    }
    //     m_qmlWidget->hide();

    this->setStyleSheet("WidgetManager{background-color:#2e2f30;}");


    m_peventDriver = new EventDriver(this);
    connect(m_peventDriver, &EventDriver::trigger_event_to_deal_sig, this, &WidgetManager::trigger_event_to_deal_slot);
    connect(m_peventDriver, &EventDriver::trigger_event_end_sig, this, &WidgetManager::trigger_runtime_event_end_slot);
    connect(this, &WidgetManager::trigger_event_end_sig, this, &WidgetManager::trigger_runtime_event_end_slot);

    DataManager::getInstance().setEnvironment_init_succeed_callback(std::bind(&WidgetManager::environment_init_succeed,this));

    DataManager::getInstance().setPeventDriver(m_peventDriver);


    m_peventDriver->set_event_callback(std::bind(&WidgetManager::eventdriver_callback, this, std::placeholders::_1, std::placeholders::_2));

    tagEventInfo timereventinfo;
    timereventinfo.e_update_type = E_EVENT_UPDATE_TYPE_REPEAT_PERIOD;
    timereventinfo.m_eventId = FunctionAssistant::generate_random_positive_uint64();
    timereventinfo.timeout = 1000;// pflightevent->m_day_senscod_offset_ms;
    timereventinfo.repeattimes = 0;
    timereventinfo.m_eventtype = E_EVENT_TYPE_ID_HEARTBEAT;
    timereventinfo.flightevent = nullptr;
    timereventinfo.bEnableAdjustSpeed = false;
    timereventinfo.bEnablePause = false;
    m_peventDriver->addevent(timereventinfo);

    m_peventDriver->start();
    m_pQStackedWidget = new QStackedWidget(this);
    initWidgets();

    m_pLayout = new QHBoxLayout();
    m_pLayout->addWidget(m_qmlWidget);
    m_pLayout->addWidget(m_pQStackedWidget);
    m_pLayout->setSpacing(0);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_qmlWidget->setFixedWidth(64);
    m_pLayout->setStretchFactor(m_qmlWidget,0);
    m_pLayout->setStretch(0, 1);
    m_pLayout->setStretch(1, 20);
    setLayout(m_pLayout);



}

WidgetManager::~WidgetManager()
{
    if(m_qmlWidget)
    {
        m_qmlWidget->deleteLater();
    }

    if(m_pLayout)
    {
        m_pLayout->deleteLater();
    }

    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
    }
}

void WidgetManager::start_HttpServer()
{
    if (m_pHttpServerRunningThread)
    {
        m_pHttpServerRunningThread->start();
    }
}

void WidgetManager::stop_HttpServer()
{
    if (m_pHttpServerRunningThread)
    {
        m_pHttpServerRunningThread->stop();
    }
}

void WidgetManager::run()
{
    start_HttpServer();
}

void WidgetManager::stop()
{
    stop_HttpServer();
}

void WidgetManager::updateWidget(const QVariant &runtiemstyleid)
{
    m_icurrentShowItem = runtiemstyleid.toUInt();

    int widgetindex = m_btn_widget_id.at(m_icurrentShowItem);
    m_pQStackedWidget->setCurrentIndex(widgetindex);
}

void WidgetManager::eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype)
{
    switch (eventtype) {
    case E_EVENT_TYPE_ID_HEARTBEAT:
    {
        RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
        if (pMapWidget)
        {
            DataManager::getInstance().m_play_cur += m_peventDriver->get_heartbeat_second();
            pMapWidget->addSliderValue(m_peventDriver->get_heartbeat_second());
//			pMapWidget->addSliderValue_dt(m_peventDriver->get_dt());
        }
    }
    break;
    case E_EVENT_TYPE_ID_SIM_TRACKING:
    {
        RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
        if (pMapWidget)
        {
            pMapWidget->driverSimTracking();
        }
    }
    break;
    case E_EVENT_TYPE_ID_PARKINGPOINT_RELEASE:
    {
        RuntimeListWidget * runtimelistWidget = dynamic_cast<RuntimeListWidget*>(m_pQStackedWidget->widget(2));
        if (runtimelistWidget)
        {
            runtimelistWidget->eventdriver_callback(event_id, eventtype);
        }
    }
    break;
    case E_EVENT_TYPE_ID_RUNWAY_RELEASE:
    {
        RuntimeListWidget * runtimelistWidget = dynamic_cast<RuntimeListWidget*>(m_pQStackedWidget->widget(2));
        if (runtimelistWidget)
        {
            runtimelistWidget->eventdriver_callback(event_id, eventtype);
        }
    }
    break;
    case E_EVENT_TYPE_ID_SIM_END:
    {
        emit trigger_event_end_sig();
    }
    break;
    default:break;
    }
}

void WidgetManager::environment_init_succeed()
{
    std::cout << "gaeactor_hub.exe start ok" << std::endl;
    emit environment_init_succeed_sig();
}

void WidgetManager::resizeEvent(QResizeEvent *event)
{
    m_pQStackedWidget->currentWidget()->resize(m_pQStackedWidget->width(), m_pQStackedWidget->height());
    QWidget::resizeEvent(event);
}

void WidgetManager::showEvent(QShowEvent *event)
{
    m_qmlWidget->show();
    m_pQStackedWidget->currentWidget()->setVisible(true);
    QWidget::showEvent(event);
}

void WidgetManager::hideEvent(QHideEvent *event)
{
    m_qmlWidget->hide();
    m_pQStackedWidget->currentWidget()->setVisible(false);
    QWidget::hideEvent(event);
}

void WidgetManager::initWidgets()
{
    AgentsListWidget* pAgentsListWidget = new AgentsListWidget(this);
    InstAgentsListWidget* pInstAgentsListWidget = new InstAgentsListWidget(this);
    RunningWidget *_mapWidget = new RunningWidget(this);
    SoStepListWidget* pSoStepListWidget = new SoStepListWidget(this);
    QtOSGWidget* pModelWidget2 = new QtOSGWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MAP, this);
    RuntimeListWidget* runtimelistWidget = new RuntimeListWidget(this);
    ReplayWidget *pReplayWidget = new ReplayWidget(this);
    SettingsWidget *pSettingsWidget = new SettingsWidget(this);
    TrackingWidget *pTrackingWidget = new TrackingWidget(this);

    _mapWidget->setRuntimeListWidget(runtimelistWidget);
    _mapWidget->setQtOSGWidget(pModelWidget2);
    pAgentsListWidget->hide();
    pInstAgentsListWidget->hide();
    _mapWidget->hide();
    pSoStepListWidget->hide();
    pModelWidget2->hide();
    runtimelistWidget->hide();
    pReplayWidget->hide();

//    _mapWidget->resize(m_pQStackedWidget->width(), m_pQStackedWidget->height());
    _mapWidget->setEventDriver(m_peventDriver);
    runtimelistWidget->setEventDriver(m_peventDriver);

    connect(this, &WidgetManager::deal_sim_ctrl_sig, this, &WidgetManager::deal_sim_ctrl_slot);
    connect(this, &WidgetManager::deal_sim_review_ctrl_sig, this, &WidgetManager::deal_sim_review_ctrl_slot);
    connect(this, &WidgetManager::deal_sim_data_sig, this, &WidgetManager::deal_sim_data_slot);
    connect(this, &WidgetManager::deal_sim_review_sig, this, &WidgetManager::deal_sim_review_slot);
    connect(this, &WidgetManager::deal_record_ctrl_sig, this, &WidgetManager::deal_record_ctrl_slot);

    connect(this, &WidgetManager::environment_init_succeed_sig, this, &WidgetManager::environment_init_succeed_slot);
    


    connect(pSoStepListWidget, &SoStepListWidget::deal_instagentData_sig, pInstAgentsListWidget, &InstAgentsListWidget::deal_instagentData_slot);
    connect(pInstAgentsListWidget, &InstAgentsListWidget::deal_instagentData_sig, runtimelistWidget, &RuntimeListWidget::deal_instagentData_slot);


    connect(runtimelistWidget, &RuntimeListWidget::updatepoicolor_sig, _mapWidget, &RunningWidget::updatepoitextcolor_slot);


    auto pplaywgt = pReplayWidget->getPlayWidget();
    if(pplaywgt)
    {
        connect(pplaywgt, &PlayWidget::trigger_review_event_end_sig, this, &WidgetManager::trigger_review_event_end_slot);
    }


    QString airportcode = "CAN";
    QStringList allowRunway;
    allowRunway << "2L";
    allowRunway << "2R";
    allowRunway << "1";
    pSoStepListWidget->setAirportInfos(airportcode, allowRunway);

    pInstAgentsListWidget->initData();
    pSoStepListWidget->initData();

    auto appendwidget = [&](int indexid, QWidget *w)
    {
        m_btn_widget_id.insert(std::make_pair(indexid, m_pQStackedWidget->count()));
        m_pQStackedWidget->addWidget(w);
    };
    appendwidget(0,pAgentsListWidget);
    appendwidget(1,pInstAgentsListWidget);
    appendwidget(2,runtimelistWidget);
    appendwidget(3,pSoStepListWidget);
    appendwidget(4,_mapWidget);
    appendwidget(5,pModelWidget2);
    appendwidget(6,pReplayWidget);
    appendwidget(7, pTrackingWidget);
    appendwidget(8,pSettingsWidget);
    m_pQStackedWidget->widget(4)->show();
    m_pQStackedWidget->setCurrentIndex(4);
    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
    if (pMapWidget)
    {
        double speed = m_peventDriver->speed_coeff();
        pMapWidget->updateSpeed(speed);
    }
}

void WidgetManager::deal_sim_ctrl_slot(const QJsonObject &obj)
{
    if(obj.contains("ctrltype") && obj.contains("ctrlparam") && obj.value("ctrltype").isString() && obj.value("ctrlparam").isString())
    {
        auto ctrltype = obj.value("ctrltype").toString();
        auto ctrlparam = obj.value("ctrlparam").toString();
        deal_runtime(ctrltype, ctrlparam);
    }
}

void WidgetManager::deal_sim_review_ctrl_slot(const QJsonObject &obj)
{
    if(obj.contains("ctrltype") && obj.contains("ctrlparam") && obj.value("ctrltype").isString() && obj.value("ctrlparam").isString())
    {
        auto ctrltype = obj.value("ctrltype").toString();
        auto ctrlparam = obj.value("ctrlparam").toString();
        deal_review(ctrltype, ctrlparam);
    }
}

void WidgetManager::deal_sim_data_slot(const QJsonObject &obj)
{
    if(obj.contains("airportcode") && obj.contains("airportcode") && obj.value("url").isString() && obj.value("url").isString())
    {
        auto airportcode = obj.value("airportcode").toString();
        auto url = obj.value("url").toString();
        auto simname = obj.value("simname").toString("");
        auto arrrunway = obj.value("arrrunway").toArray();
        auto deprunway = obj.value("deprunway").toArray();
        auto allowrunway = obj.value("allowrunway").toArray();
        QStringList arrrunwaylist;
        QStringList deprunwaylist;
        QStringList allowrunwaylist;
        for(auto item:arrrunway)
        {
            arrrunwaylist.push_back(item.toString());
        }

        for(auto item:deprunway)
        {
            deprunwaylist.push_back(item.toString());
        }
        for(auto item:allowrunway)
        {
            allowrunwaylist.push_back(item.toString());
        }

        if(airportcode.isEmpty())
        {
            airportcode = "CAN";
        }
        if(simname.isEmpty())
        {
//            QDateTime datetime = QDateTime::currentDateTime().toUTC();
//            QString dateStr = datetime.toString("yyyyMMddhhmmsszzz");

//            DataManager::getInstance().updatereviewsrecorddir("lavic_desktop_custom_"+dateStr);
        }
        else
        {
            QDateTime datetime = QDateTime::currentDateTime().toUTC();
            QString dateStr = datetime.toString("yyyyMMddhhmmsszzz");
            DataManager::getInstance().updatereviewsrecorddir(simname+"_"+dateStr);
        }

        if(allowrunwaylist.isEmpty())
        {
            allowrunwaylist << "2L";
            allowrunwaylist << "2R";
            allowrunwaylist << "1";
        }

        DataManager::getInstance().trans_log("操作：仿真数据",std::stringstream()<<" "<<airportcode.toStdString()<<" "<<url.toStdString(),std::stringstream());

        QString dt_st = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss_zzz");
        QString filename = QString("%1/data/plan/%2.xlsx").arg(QCoreApplication::applicationDirPath()).arg(dt_st);
        QString dir = QString("%1/data/plan/").arg(QCoreApplication::applicationDirPath());


        if(!url.contains(dir))
        {
            DataManager::getInstance().trans_log("操作：仿真数据 ", std::stringstream() << filename.toStdString(), std::stringstream() << "开始下载");
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->requeset_sim_data(url.toStdString(),filename.toStdString()))
            {
                DataManager::getInstance().trans_log("操作：仿真数据 ",std::stringstream()<<filename.toStdString(),std::stringstream()<<"下载失败");
            }
            DataManager::getInstance().trans_log("操作：仿真数据 ", std::stringstream() << filename.toStdString(), std::stringstream() << "下载成功 开始解析");
        }
        else
        {
            filename = url;
        }

        if (QFileInfo(filename).exists())
        {
            SoStepListWidget * pMapWidget = dynamic_cast<SoStepListWidget*>(m_pQStackedWidget->widget(3));
            if (pMapWidget)
            {

                pMapWidget->setAirportInfos(airportcode, allowrunwaylist);
                pMapWidget->importexcel(filename);
            }
            DataManager::getInstance().trans_log("操作：仿真数据 ",std::stringstream()<<filename.toStdString(),std::stringstream()<<"解析完成");
            DataManager::getInstance().trans_sim_data_ok();
        }
        else
        {
            DataManager::getInstance().trans_log("操作：仿真数据 ",std::stringstream()<<filename.toStdString(),std::stringstream()<<" 数据导入失败，请重新导入");
        }
    }
}

void WidgetManager::deal_sim_review_slot(const QJsonObject &obj)
{
    if(obj.contains("reviewdata") && obj.value("reviewdata").isString())
    {
        auto reviewdata =  obj.value("reviewdata").toString();
        QString operate = "";
        if (obj.contains("operate") && obj.value("operate").isString())
        {
            operate = obj.value("operate").toString();
        }
        if (operate.isEmpty())
        {
            auto &reviewdatamap = DataManager::getInstance().m_reviewdata;
            auto itor = reviewdatamap.find(reviewdata);
            if(itor != reviewdatamap.end())
            {
                ReplayWidget * pReplayWidget = dynamic_cast<ReplayWidget*>(m_pQStackedWidget->widget(6));
                if (pReplayWidget)
                {
                    auto pplaywgt = pReplayWidget->getPlayWidget();
                    if(pplaywgt)
                    {
                        pplaywgt->initializeReadFileSlot(&itor->second);
                    }
                }
            }
        }
        else
        {
            if ("review" == operate)
            {
                auto &reviewdatamap = DataManager::getInstance().m_reviewdata;
                auto itor = reviewdatamap.find(reviewdata);
                if(itor != reviewdatamap.end())
                {
                    ReplayWidget * pReplayWidget = dynamic_cast<ReplayWidget*>(m_pQStackedWidget->widget(6));
                    if (pReplayWidget)
                    {
                        auto pplaywgt = pReplayWidget->getPlayWidget();
                        if(pplaywgt)
                        {
                            pplaywgt->initializeReadFileSlot(&itor->second);
                        }
                    }
                }
            }
            else if ("delete" == operate)
            {
                auto& reviewdatamap = DataManager::getInstance().m_reviewdata;
                auto itor = reviewdatamap.find(reviewdata);
                if (itor != reviewdatamap.end())
                {
                    DataManager::getInstance().rmdirs(itor->second.m_titledir);
                }
            }
        }
    }
}

void WidgetManager::deal_record_ctrl_slot(const QJsonObject &obj)
{
    if(obj.contains("ctrl") && obj.value("ctrl").isBool())
    {
        bool bEnable = obj.value("ctrl").toBool();

        SettingsWidget * pSettingsWidget = dynamic_cast<SettingsWidget*>(m_pQStackedWidget->widget(8));
        if (pSettingsWidget)
        {
            pSettingsWidget->updateRecordstatus(bEnable);
        }
    }
}

void WidgetManager::environment_init_succeed_slot()
{
    if (m_peventDriver)
    {
        m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_START);
    }

    RuntimeListWidget* runtimelistWidget = dynamic_cast<RuntimeListWidget*>(m_pQStackedWidget->widget(2));
    if (runtimelistWidget)
    {
        runtimelistWidget->qml_send_agent_data_slot();
    }

    RunningWidget* pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
    if (pMapWidget)
    {
        pMapWidget->setPlayClickSlot(true);
        pMapWidget->setSliderRange(DataManager::getInstance().m_play_cur - DataManager::getInstance().m_play_min, DataManager::getInstance().m_play_range);
        pMapWidget->setSliderValue(DataManager::getInstance().m_play_cur);


        DataManager::getInstance().update_SiminfoLog_to_db(SiminfoLog{0,
                                                                      DataManager::getInstance().m_play_min,
                                                                      DataManager::getInstance().m_play_max,
                                                                        ConfigManager::getInstance().step_interval,
                                                                        ConfigManager::getInstance().step_dt,
                                                                        ConfigManager::getInstance().step_freq,
                                                                        ConfigManager::getInstance().one_second_sim_step_second,
                                                                      DataManager::getInstance().m_simname.toStdString(),
                                                                      stdutils::OriDateTime::getCurrentUTCTimeStampMSecs()});

        DataManager::getInstance().m_recordruntimelogs.clear();
    }
}

void WidgetManager::trigger_runtime_event_end_slot()
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
        {
            runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
        }
//    DataManager::getInstance().kill_process("gaeactor-hub");
    }
    if (m_peventDriver)
    {
        m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP,false);
    }
    DataManager::getInstance().exportexcel();

    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
    if (pMapWidget)
    {
        pMapWidget->setPlayClickSlot(false);
    }

    std::stringstream ss;
    ss << "---------------------------------------仿真结束---------------------------------------";
    DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());

    DataManager::getInstance().trans_sim_runtime_end();
}

void WidgetManager::trigger_review_event_end_slot()
{
    ReplayWidget * pReplayWidget = dynamic_cast<ReplayWidget*>(m_pQStackedWidget->widget(6));
    if (pReplayWidget)
    {
        auto pplaywgt = pReplayWidget->getPlayWidget();
        if(pplaywgt)
        {
            auto pOriginalDataInputManager = pplaywgt->pOriginalDataInputManager();
            if(pOriginalDataInputManager)
            {
                pplaywgt->setPlayClickSlot(false);
                DataManager::getInstance().trans_log("操作：回放结束 ", std::stringstream(), std::stringstream());
           }
        }
    }
    DataManager::getInstance().trans_sim_review_end();
}


void WidgetManager::trigger_event_to_deal_slot(uint64_t triggertimestamp, uint64_t trigger_event_id)
{
    tagFlightEventTime *pflightevent = nullptr;

    std::map<uint64_t, tagFlightEventTime> &day_flight_events = DataManager::getInstance().total_flightEventTimedata;
    auto day_flight_events_itor = std::find_if(day_flight_events.begin(),
        day_flight_events.end(), [&triggertimestamp, &trigger_event_id](const std::map<uint64_t, tagFlightEventTime>::value_type &vt) {
        return vt.first == triggertimestamp && vt.second.m_itime == triggertimestamp && vt.second.m_eventid == trigger_event_id;
    });
    if (day_flight_events_itor != day_flight_events.end())
    {
        tagFlightEventTime &flightevent = day_flight_events_itor->second;
        pflightevent = &flightevent;
    }
    if (pflightevent)
    {

        //RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
        //if (pMapWidget)
        //{
        //	pMapWidget->setSliderValue(triggertimestamp);
        //}

        QList<tagTriggerFlightInfo> triggerflights;
        auto flightconfigs_itor = pflightevent->m_flightCfgs.begin();
        while (flightconfigs_itor != pflightevent->m_flightCfgs.end())
        {
            FlightPlanConf* pFlightPlanConf = *flightconfigs_itor;

            tagTriggerFlightInfo triggerFlightInfo;
            triggerFlightInfo.m_FilghtNumber = pFlightPlanConf->m_FilghtNumber;
            triggerFlightInfo.m_flight_dep_arr_type = pFlightPlanConf->m_flight_dep_arr_type;
            triggerFlightInfo.m_Runway = pFlightPlanConf->m_Runway;
            triggerflights << triggerFlightInfo;
            flightconfigs_itor++;
        }

        RuntimeListWidget * pRuntimeListWidget = dynamic_cast<RuntimeListWidget*>(m_pQStackedWidget->widget(2));
        if (pRuntimeListWidget)
        {
            pRuntimeListWidget->trigger_flight_to_run_slot(triggertimestamp, triggerflights);
        }
    }
}

void WidgetManager::thread_httpserver_callback_Loop(void* param)
{
    m_phttpserver->run();
    stdutils::OriDateTime::sleep(1);
}

bool WidgetManager::httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject & obj)
{
    switch(eDataType)
    {
    case E_DATA_TYPE_SIM_DATA:
    {
        emit deal_sim_data_sig(obj);
    }break;
    case E_DATA_TYPE_SIM_CTRL:
    {
        emit deal_sim_ctrl_sig(obj);
    }break;
    case E_DATA_TYPE_SIM_REVIEW_CTRL:
    {
        emit deal_sim_review_ctrl_sig(obj);
    }break;
    case E_DATA_TYPE_SIM_REVIEW_DATA:
    {
        emit deal_sim_review_sig(obj);
    }break;
    case E_DATA_TYPE_RECORD_CTRL:
    {
        emit deal_record_ctrl_sig(obj);
    }break;

    }
    return true;
}

void WidgetManager::deal_runtime(const QString &ctrltype, const QString &ctrlparam)
{
        if(ctrltype == "prgctrl")
        {
            double speed = ctrlparam.toDouble();
            EventDriver* pEventDriver = DataManager::getInstance().peventDriver();
            if(pEventDriver && speed >= 1.0 && speed <= 100.0)
            {
                float ff = pEventDriver->speed_coeff();
                if (fabs(ff - speed) > 1E-7)
                {
                    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
                    if (pMapWidget)
                    {
                        pMapWidget->updateSpeed(speed);
                    }
                    pEventDriver->set_speed_coeff(speed);

                    DataManager::getInstance().trans_log("操作：仿真速度调整",std::stringstream()<< QString::number(speed).toStdString(),std::stringstream());
                }
            }
        }
        else if(ctrltype == "processctrl")
        {
            EventDriver* pEventDriver = DataManager::getInstance().peventDriver();
            if(pEventDriver)
            {
                if("start" == ctrlparam)
                {
                    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                    {
                        if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
                        {
                            runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
                        }

                        DataManager::getInstance().updateConnecting(false);
                        runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-hub");
                    }
                    DataManager::getInstance().trans_log("操作：仿真启动",std::stringstream(),std::stringstream());
                }
                if("pause" == ctrlparam)
                {
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_PAUSE);
                    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
                    if (pMapWidget)
                    {
                        pMapWidget->setPauseClickSlot(true);
                    }
                    DataManager::getInstance().trans_log("操作：仿真暂停",std::stringstream(),std::stringstream());
                }

                if("resume" == ctrlparam)
                {
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_RESUME);
                    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
                    if (pMapWidget)
                    {
                        pMapWidget->setPauseClickSlot(false);
                    }
                    DataManager::getInstance().trans_log("操作：仿真继续",std::stringstream(),std::stringstream());
                }

                if("stop" == ctrlparam)
                {
                    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
                    {
                        if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
                        {
                            runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
                        }
                        runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
                    }
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP);

                    DataManager::getInstance().exportexcel();

                    RunningWidget * pMapWidget = dynamic_cast<RunningWidget*>(m_pQStackedWidget->widget(4));
                    if (pMapWidget)
                    {
                        pMapWidget->setPlayClickSlot(false);
                    }
                    DataManager::getInstance().trans_log("操作：仿真终止",std::stringstream(),std::stringstream());
                }
            }
        }
}


void WidgetManager::deal_review(const QString &ctrltype, const QString &ctrlparam)
{

    ReplayWidget * pReplayWidget = dynamic_cast<ReplayWidget*>(m_pQStackedWidget->widget(6));
    if (pReplayWidget)
    {
        auto pplaywgt = pReplayWidget->getPlayWidget();
        if(pplaywgt)
        {
            auto pOriginalDataInputManager = pplaywgt->pOriginalDataInputManager();
            if(pOriginalDataInputManager)
            {
                if(ctrltype == "prgctrl")
                {
                    double speed = ctrlparam.toDouble();

                    DataManager::getInstance().m_review_speed = speed;
                    pOriginalDataInputManager->setReadSpeed(speed);
                    pplaywgt->setReadSpeedContext(pOriginalDataInputManager->getReadSpeed());

                    DataManager::getInstance().trans_log("操作：回放速度调整",std::stringstream()<< QString::number(speed).toStdString(),std::stringstream());
                }
                else if(ctrltype == "processctrl")
                {
                    if("start" == ctrlparam)
                    {
                        pplaywgt->setPlayClickSlot(true);
                        DataManager::getInstance().trans_log("操作：回放启动",std::stringstream(),std::stringstream());
                    }
                    if("pause" == ctrlparam)
                    {
                        pplaywgt->setPauseClickSlot(true);
                        DataManager::getInstance().trans_log("操作：回放暂停",std::stringstream(),std::stringstream());
                    }

                    if("resume" == ctrlparam)
                    {
                        pplaywgt->setPauseClickSlot(false);
                        DataManager::getInstance().trans_log("操作：回放继续",std::stringstream(),std::stringstream());
                    }

                    if("stop" == ctrlparam)
                    {
                        pplaywgt->setPlayClickSlot(false);
                        DataManager::getInstance().trans_log("操作：回放终止",std::stringstream(),std::stringstream());
                    }
                }
                else if(ctrltype == "processjumpctrl")
                {
                    double percent = ctrlparam.toDouble();

                    qint64 m_iDataFrames = pplaywgt->jumpToDataMillisecondPosOffsetPercent(percent);
                }
            }
        }
    }
}

