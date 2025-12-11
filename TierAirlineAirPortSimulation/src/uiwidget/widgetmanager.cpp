#include "widgetmanager.h"

#include "src/storage/OriginalDataInputManager.h"
#include "src/storage/OriginalDataOutputManager.h"
#include "../httpserver/httpserver.h"
#include "./src/OriginalThread.h"
#include "./src/OriginalDateTime.h"
#include "../components/configmanager.h"
#include "settingsconfig.h"

#include <QDir>

#include <QtXlsx/QtXlsx>
#include "runningmodeconfig.h"

WidgetManager::WidgetManager(QObject *parent)
    :QObject(parent),
    m_peventDriver(nullptr),
    m_pOriginalDataInputManager(nullptr)
{
    qRegisterMetaType<TYPE_ULID>("TYPE_ULID");
    qRegisterMetaType<uint64_t>("uint64_t");
    m_phttpserver = new HttpServer(this);
    m_phttpserver->setDataCallback(std::bind(&WidgetManager::httpdatareceive_callback, this, std::placeholders::_1, std::placeholders::_2));

    m_pHttpServerRunningThread = new stdutils::OriThread(std::bind(&WidgetManager::thread_httpserver_callback_Loop, this, std::placeholders::_1), this);

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

    connect(this, &WidgetManager::deal_sim_ctrl_sig, this, &WidgetManager::deal_sim_ctrl_slot);
    connect(this, &WidgetManager::deal_sim_review_ctrl_sig, this, &WidgetManager::deal_sim_review_ctrl_slot);
    connect(this, &WidgetManager::deal_sim_data_sig, this, &WidgetManager::deal_sim_data_slot);
    connect(this, &WidgetManager::deal_sim_review_sig, this, &WidgetManager::deal_sim_review_slot);
    connect(this, &WidgetManager::deal_record_ctrl_sig, this, &WidgetManager::deal_record_ctrl_slot);

    connect(this, &WidgetManager::environment_init_succeed_sig, this, &WidgetManager::environment_init_succeed_slot);
    connect(this, &WidgetManager::sendDataSignal, this, &WidgetManager::sendDataSlot);


    QString airportcode = "CAN";
    QStringList allowRunway;
    allowRunway << "2L";
    allowRunway << "2R";
    allowRunway << "1";
    this->setAirportInfos(airportcode, allowRunway);

    request_agentruntimedata();
}

WidgetManager::~WidgetManager()
{
    runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
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

void WidgetManager::eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype)
{
    switch (eventtype) {
    case E_EVENT_TYPE_ID_HEARTBEAT:
    {
        if (DataManager::getInstance().m_play_cur >= DataManager::getInstance().m_play_min && DataManager::getInstance().m_play_cur <= DataManager::getInstance().m_play_max)
        {
             DataManager::getInstance().m_play_cur += m_peventDriver->get_heartbeat_second();
        }
    }
    break;
    case E_EVENT_TYPE_ID_SIM_TRACKING:
    {

    }
    break;
    case E_EVENT_TYPE_ID_PARKINGPOINT_RELEASE:
    {
        this->deal_eventdriver_callback(event_id, eventtype);
    }
    break;
    case E_EVENT_TYPE_ID_RUNWAY_RELEASE:
    {
        this->deal_eventdriver_callback(event_id, eventtype);
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



void getXlsxDocumentVal(const QXlsx::Document &worksheet, const QString& title, QString& val)
{
    val = worksheet.read(title).toString();
}
void WidgetManager::importexcel(const QString &fileName)
{
    QXlsx::Document _Xlsx(fileName);
    int lastColumn = _Xlsx.dimension().columnCount();
    int lastRow = _Xlsx.dimension().rowCount();

    std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> >& flightdata = DataManager::getInstance().m_flightdata;
    std::map<uint64_t, tagFlightEventTime> &total_flightEventTimedata = DataManager::getInstance().total_flightEventTimedata;
    std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> &total_flightdata = DataManager::getInstance().total_flightdata;
    std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> > flighttimedata;

    std::map<uint64_t, std::map<uint64_t, tagFlightEventTime>> flightEventTimedata;


    total_flightEventTimedata.clear();
    total_flightdata.clear();
    flightdata.clear();
    flighttimedata.clear();
    flightEventTimedata.clear();
    for (int i = 1; i < lastRow; i++)
    {
        FlightPlanConf fightplanConf;

        getXlsxDocumentVal(_Xlsx, "A" + QString::number(i + 1), fightplanConf.m_Date);
        if (!fightplanConf.m_Date.isEmpty())
        {
            getXlsxDocumentVal(_Xlsx, "B" + QString::number(i + 1), fightplanConf.m_FilghtNumber);
            getXlsxDocumentVal(_Xlsx, "C" + QString::number(i + 1), fightplanConf.m_DepArrFlag);
            getXlsxDocumentVal(_Xlsx, "D" + QString::number(i + 1), fightplanConf.m_PlaneNum);
            getXlsxDocumentVal(_Xlsx, "E" + QString::number(i + 1), fightplanConf.m_PlaneType);
            getXlsxDocumentVal(_Xlsx, "F" + QString::number(i + 1), fightplanConf.m_FlightClass);
            getXlsxDocumentVal(_Xlsx, "G" + QString::number(i + 1), fightplanConf.m_FlightLeg);
            getXlsxDocumentVal(_Xlsx, "H" + QString::number(i + 1), fightplanConf.m_FlightStartPlace);
            getXlsxDocumentVal(_Xlsx, "I" + QString::number(i + 1), fightplanConf.m_FlightEndPlace);
            getXlsxDocumentVal(_Xlsx, "J" + QString::number(i + 1), fightplanConf.m_PlanDateTimeTakeOff);
            getXlsxDocumentVal(_Xlsx, "K" + QString::number(i + 1), fightplanConf.m_ExpectedDateTimeTakeOff);
            getXlsxDocumentVal(_Xlsx, "L" + QString::number(i + 1), fightplanConf.m_RealityDateTimeTakeOff);

            getXlsxDocumentVal(_Xlsx, "M" + QString::number(i + 1), fightplanConf.m_PlanDateTimeLanding);
            getXlsxDocumentVal(_Xlsx, "N" + QString::number(i + 1), fightplanConf.m_ExpectedDateTimeLanding);
            getXlsxDocumentVal(_Xlsx, "O" + QString::number(i + 1), fightplanConf.m_RealityDateTimeLanding);
            getXlsxDocumentVal(_Xlsx, "P" + QString::number(i + 1), fightplanConf.m_Delay);

            getXlsxDocumentVal(_Xlsx, "Q" + QString::number(i + 1), fightplanConf.m_Seat);
            getXlsxDocumentVal(_Xlsx, "R" + QString::number(i + 1), fightplanConf.m_Terminal);
            getXlsxDocumentVal(_Xlsx, "S" + QString::number(i + 1), fightplanConf.m_Runway);
            QString flightid;
            getXlsxDocumentVal(_Xlsx, "T" + QString::number(i + 1), flightid);
            if(flightid.isEmpty())
            {
                fightplanConf.flightid = QString::number(i);
            }
            else
            {
                fightplanConf.flightid = flightid;
            }
            QString wps;
            getXlsxDocumentVal(_Xlsx, "U" + QString::number(i + 1), wps);
            if(!wps.isEmpty())
            {
                fightplanConf.wps = wps.remove(QRegExp("\\s+")).remove("\\");
            }
            if (fightplanConf.m_DepArrFlag == "到达")
            {
                fightplanConf.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_ARR;
            }
            else
            {
                fightplanConf.m_flight_dep_arr_type = E_FLIGHT_DEP_ARR_TYPE_DEP;
            }

            if(fightplanConf.flightid.isEmpty())
            {
                fightplanConf.flightid = QString::number(FunctionAssistant::generate_random_positive_uint64());
            }

            if (fightplanConf.m_PlaneNum.isEmpty())
            {
                std::stringstream ss;
                ss << fightplanConf.printf();


                std::stringstream sstmp;
                sstmp << " 航班号 为空 ";

                DataManager::getInstance().trans_log("读取航班时刻表：错误 ", ss, sstmp);
            }

            QDateTime dateTime;
            QString date_time = fightplanConf.m_Date;

            dateTime = QDateTime::fromString(fightplanConf.m_Date, "yyyyMMdd");

            FlightPlanConf *pData = nullptr;
            auto flightdata_itor = std::find_if(flightdata.begin(),
                flightdata.end(), [&](const std::unordered_map<QString, std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>>::value_type & vt) {
                return vt.first == fightplanConf.m_Date;
            });
            if (flightdata_itor != flightdata.end())
            {
                std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString> _key = std::make_tuple(fightplanConf.m_FilghtNumber, fightplanConf.m_flight_dep_arr_type, fightplanConf.m_Runway);
                std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf>& day_flights = flightdata_itor->second;
                day_flights.insert(std::make_pair(_key, std::move(fightplanConf)));
                pData = &day_flights.at(_key);
            }
            else
            {
                std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString> _key = std::make_tuple(fightplanConf.m_FilghtNumber, fightplanConf.m_flight_dep_arr_type, fightplanConf.m_Runway);
                std::unordered_map<std::tuple<QString, E_FLIGHT_DEP_ARR_TYPE, QString>, FlightPlanConf> day_flights;
                day_flights.insert(std::make_pair(_key, std::move(fightplanConf)));
                flightdata.insert(std::make_pair(date_time, std::move(day_flights)));
                pData = &(flightdata.at(date_time).at(_key));
            }

            if (pData)
            {
                uint64_t timedate = dateTime.toSecsSinceEpoch();
                uint64_t timestamp = 0;
                switch (fightplanConf.m_flight_dep_arr_type)
                {
                case E_FLIGHT_DEP_ARR_TYPE_ARR:
                {
                    QDateTime dateTimestamp;
                    dateTimestamp = QDateTime::fromString(pData->m_PlanDateTimeLanding, "yyyy-MM-dd hh:mm:ss");
                    pData->updatePlanDateTimeLanding_timestamp(dateTimestamp.toSecsSinceEpoch());
                    //dateTimestamp.addSecs(-pData->m_aheadtimelen);
                    timestamp = pData->m_PlanDateTimeLanding_ahead_timestamp;
                    //pData->m_PlanDateTimeLanding_ahead_timestamp = timestamp;

                }
                break;
                case E_FLIGHT_DEP_ARR_TYPE_DEP:
                {
                    QDateTime dateTimestamp;
                    dateTimestamp = QDateTime::fromString(pData->m_PlanDateTimeTakeOff, "yyyy-MM-dd hh:mm:ss");
                    pData->updatePlanDateTimeTakeOff_timestamp(dateTimestamp.toSecsSinceEpoch());
                    //dateTimestamp.addSecs(-pData->m_aheadtimelen);
                    timestamp = pData->m_PlanDateTimeTakeOff_ahead_timestamp;
                    //pData->m_PlanDateTimeTakeOff_ahead_timestamp = timestamp;
                }
                break;
                default:
                    break;
                }

                auto flighttimedata_itor = std::find_if(flighttimedata.begin(),
                    flighttimedata.end(), [&](const std::map<uint64_t, std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> >::value_type & vt) {
                    return vt.first == timedate;
                });
                if (flighttimedata_itor != flighttimedata.end())
                {
                    std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*>& day_flights = flighttimedata_itor->second;
                    if (day_flights.find(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber)) != day_flights.end())
                    {
                        auto &existval = day_flights.at(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber));
                        std::stringstream ss;
                        ss  << existval->printf() << " 与 " << pData->printf();
                        std::stringstream sstmp;
                        sstmp << " 航班号重复 ";

                        DataManager::getInstance().trans_log("读取航班时刻表：错误 ", ss, sstmp);
                    }
                    else
                    {
                        day_flights.insert(std::make_pair(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber), /*std::move*/(pData)));

                        std::map<uint64_t, tagFlightEventTime> &day_flight_events = flightEventTimedata.at(timedate);

                        auto day_flight_events_itor = std::find_if(day_flight_events.begin(),
                            day_flight_events.end(), [&](const std::map<uint64_t, tagFlightEventTime>::value_type & vt) {
                            return vt.first == timestamp;
                        });
                        if (day_flight_events_itor != day_flight_events.end())
                        {
                            tagFlightEventTime &flightevent = day_flight_events_itor->second;
                            flightevent.appendFlightCfg(pData);
                        }
                        else
                        {
                            tagFlightEventTime flightevent(timestamp, FunctionAssistant::generate_random_positive_uint64(), timestamp - timedate);
                            flightevent.appendFlightCfg(pData);
                            day_flight_events.insert(std::make_pair(timestamp, std::move(flightevent)));
                        }
                    }
                }
                else
                {
                    std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*> day_flights;
                    day_flights.insert(std::make_pair(std::make_tuple(timestamp, pData->m_flight_dep_arr_type, pData->m_Runway, pData->m_FilghtNumber), /*std::move*/(pData)));
                    flighttimedata.insert(std::make_pair(timedate, std::move(day_flights)));

                    ////////////////////////////////////////////////////////////////////////
                    std::map<uint64_t, tagFlightEventTime> day_flight_events;
                    tagFlightEventTime flightevent(timestamp, FunctionAssistant::generate_random_positive_uint64(), timestamp - timedate);
                    flightevent.appendFlightCfg(pData);

                    day_flight_events.insert(std::make_pair(timestamp, std::move(flightevent)));
                    flightEventTimedata.insert(std::make_pair(timedate, std::move(day_flight_events)));
                    ////////////////////////////////////////////////////////////////////////
                }
            }
        }
    }


    auto flightEventTimedata_itor = flightEventTimedata.begin();
    while (flightEventTimedata_itor != flightEventTimedata.end())
    {
        std::map<uint64_t, tagFlightEventTime> &day_flight_events = flightEventTimedata_itor->second;
        auto day_flight_events_itor = day_flight_events.begin();
        while (day_flight_events_itor != day_flight_events.end())
        {
            auto day_flight_events_itor_tmp = day_flight_events_itor;
            auto day_flight_events_itor_prev = --day_flight_events_itor_tmp;
            day_flight_events_itor_tmp = day_flight_events_itor;
            auto day_flight_events_itor_next = ++day_flight_events_itor_tmp;
            tagFlightEventTime &flightevent = day_flight_events_itor->second;
            if (day_flight_events_itor == day_flight_events.begin())
            {
                if (day_flight_events_itor_next != day_flight_events.end())
                {
                    flightevent.setNext(&day_flight_events_itor_next->second);
                }
            }
            else if (day_flight_events_itor_next == day_flight_events.end())
            {
                flightevent.setPrev(&day_flight_events_itor_prev->second);
            }
            else
            {
                flightevent.setPrev(&day_flight_events_itor_prev->second);
                if (day_flight_events_itor_next != day_flight_events.end())
                {
                    flightevent.setNext(&day_flight_events_itor_next->second);
                }
            }
            day_flight_events_itor++;
        }
        flightEventTimedata_itor++;
    }

    //////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
    auto tmp_flightEventTimedata_itor = flightEventTimedata.begin();
    while (tmp_flightEventTimedata_itor != flightEventTimedata.end())
    {
        std::map<uint64_t, tagFlightEventTime> &day_flight_events = tmp_flightEventTimedata_itor->second;
        auto day_flight_events_itor = day_flight_events.begin();
        while (day_flight_events_itor != day_flight_events.end())
        {
            auto tmpitor = total_flightEventTimedata.find(day_flight_events_itor->first);
            if (tmpitor == total_flightEventTimedata.end())
            {
                total_flightEventTimedata.insert(std::make_pair(day_flight_events_itor->first, day_flight_events_itor->second));
            }
            else
            {
                tmpitor->second.m_flightCfgs.insert(tmpitor->second.m_flightCfgs.end(), day_flight_events_itor->second.m_flightCfgs.begin(), day_flight_events_itor->second.m_flightCfgs.end());
            }
            day_flight_events_itor++;
        }
        tmp_flightEventTimedata_itor++;
    }



    auto total_flightEventTimedata_itor = total_flightEventTimedata.begin();
    while (total_flightEventTimedata_itor != total_flightEventTimedata.end())
    {
        auto total_flightEventTimedata_itor_tmp = total_flightEventTimedata_itor;
        auto total_flightEventTimedata_itor_prev = --total_flightEventTimedata_itor_tmp;
        total_flightEventTimedata_itor_tmp = total_flightEventTimedata_itor;
        auto total_flightEventTimedata_itor_next = ++total_flightEventTimedata_itor_tmp;
        tagFlightEventTime &flightevent = total_flightEventTimedata_itor->second;
        if (total_flightEventTimedata_itor == total_flightEventTimedata.begin())
        {
            if (total_flightEventTimedata_itor_next != total_flightEventTimedata.end())
            {
                flightevent.setNext(&total_flightEventTimedata_itor_next->second);
            }
        }
        else if (total_flightEventTimedata_itor_next == total_flightEventTimedata.end())
        {
            flightevent.setPrev(&total_flightEventTimedata_itor_prev->second);
        }
        else
        {
            flightevent.setPrev(&total_flightEventTimedata_itor_prev->second);
            if (total_flightEventTimedata_itor_next != total_flightEventTimedata.end())
            {
                flightevent.setNext(&total_flightEventTimedata_itor_next->second);
            }
        }
        total_flightEventTimedata_itor++;
    }


    std::stringstream ss;
    ss << "************* 时间驱动事件个数: " << total_flightEventTimedata.size();
    DataManager::getInstance().trans_log("读取航班时刻表： ", ss, std::stringstream(),false);

    /////////////////////////////////////////////////////////////////////////////
    auto flighttimedata_itor = flighttimedata.begin();
    while (flighttimedata_itor != flighttimedata.end())
    {
        std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& day_flights = flighttimedata_itor->second;
        total_flightdata.insert(day_flights.begin(), day_flights.end());
        flighttimedata_itor++;
    }
    dealTotalFlightData_ex(total_flightdata);
}

void WidgetManager::dealTotalFlightData_ex(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *> &total_flightdata)
{
    std::unordered_map<QString, std::unordered_map<QString, QString> >&  _InstagentInstance = DataManager::getInstance().m_InstagentInstance;
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

    auto _InstagentInstance_itor = _InstagentInstance.begin();
    while (_InstagentInstance_itor != _InstagentInstance.end())
    {
        const std::unordered_map<QString, QString>& instance = _InstagentInstance_itor->second;
        auto _instance_itor = instance.begin();
        while (_instance_itor != instance.end())
        {
            auto agentInstances_itor = std::find_if(agentInstances.begin(),
                                                    agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
                                                        return vt.first == _instance_itor->second;
                                                    });
            if (agentInstances_itor != agentInstances.end())
            {
                agentInstances.erase(agentInstances_itor);
            }
            _instance_itor++;
        }
        _InstagentInstance_itor++;
    }

    _InstagentInstance.clear();
    auto day_flights_itor = total_flightdata.begin();
    while (day_flights_itor != total_flightdata.end())
    {
        const FlightPlanConf * pflighlt = day_flights_itor->second;
        if (!pflighlt)
        {
            day_flights_itor++;
            continue;
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////

        auto _InstagentInstanceitor = std::find_if(_InstagentInstance.begin(),
                                                   _InstagentInstance.end(), [&](const std::unordered_map<QString, std::unordered_map<QString, QString>>::value_type &vt) {
                                                       return vt.first == pflighlt->m_PlaneType;
                                                   });
        if (_InstagentInstanceitor != _InstagentInstance.end())
        {
            std::unordered_map<QString, QString>& planeinstancelist = _InstagentInstanceitor->second;

            auto _planeinstancelistitor = std::find_if(planeinstancelist.begin(),
                                                       planeinstancelist.end(), [&](const std::unordered_map<QString, QString>::value_type& vt) {
                                                           return vt.first == pflighlt->m_PlaneNum;
                                                       });
            if (_planeinstancelistitor == planeinstancelist.end())
            {
                planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
            }
        }
        else
        {
            std::unordered_map<QString, QString> planeinstancelist;
            planeinstancelist.insert(std::make_pair(pflighlt->m_PlaneNum, QString::number(FunctionAssistant::generate_random_positive_uint64())));
            _InstagentInstance.insert(std::make_pair(pflighlt->m_PlaneType, std::move(planeinstancelist)));
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        day_flights_itor++;
    }

    tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getAirportInfo(m_airport_code);
    if (nullptr == ptagAirPortInfo)
    {
        return;
    }

    /////////////////////////////////////////////////////////////////////////////
    deal_instagentData_slot(m_airport_code, m_allowRunway);
}

void WidgetManager::setAirportInfos(const QString &airport_code, const QStringList &allowRunway)
{
    m_airport_code = airport_code;
    m_allowRunway = allowRunway;
}


void WidgetManager::refreshAgents()
{
    std::unordered_map<QString, AgentKeyItemInfo>& agentKeys = DataManager::getInstance().agentKeyMaps();

    agentKeys.clear();
    QJsonObject jsobj;

    if (DataManager::getInstance().pHttpClient() && DataManager::getInstance().pHttpClient()->requeset_agent_data(jsobj))
    {
        auto dataarray = jsobj.value("data").toArray();
        for (auto dataarrayitem : dataarray)
        {
            auto dataarrayitemobj = dataarrayitem.toObject();
            AgentKeyItemInfo agentkeyitem;
            agentkeyitem.fromJson(dataarrayitemobj);
            agentkeyitem.modelUrlFat = "qrc:/res/img/undefined.jpg";
            QJsonObject subjs;
            agentkeyitem.toJson(subjs);

            auto agentKeys_itor = std::find_if(agentKeys.begin(),
                                               agentKeys.end(), [&](const std::unordered_map<QString, AgentKeyItemInfo>::value_type& vt) {
                                                   return vt.first == agentkeyitem.agentKey;
                                               });
            if (agentKeys_itor == agentKeys.end())
            {
                agentKeys.insert(std::make_pair(agentkeyitem.agentKey, std::move(agentkeyitem)));
            }
        }
    }
}

void WidgetManager::deal_instagentData_slot(const QString &airport_code, const QStringList &allowRunway)
{

    refreshAgents();
    std::unordered_map<QString, AgentKeyItemInfo>& agentKeys = DataManager::getInstance().agentKeyMaps();

    std::unordered_map<QString, std::unordered_map<QString, QString> >& _InstagentInstance = DataManager::getInstance().m_InstagentInstance;

    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

    QJsonArray jsarr;
    QJsonObject jsobj;

    auto _InstagentInstance_itor = _InstagentInstance.begin();
    while (_InstagentInstance_itor != _InstagentInstance.end())
    {
        const std::unordered_map<QString, QString>& instance = _InstagentInstance_itor->second;
        auto _instance_itor = instance.begin();
        while (_instance_itor != instance.end())
        {
            AgentKeyItemInfo targetAgent;
            QString targetName = _instance_itor->first;
            //            targetName = "a350-1000";
            //            targetName = "A350-1000";
            targetName = "a";
            auto agentKeys_itor = std::find_if(agentKeys.begin(),
                                               agentKeys.end(), [&](const std::unordered_map<QString, AgentKeyItemInfo>::value_type& vt) {
                                                   return vt.second.agentName == targetName;
                                               });
            if (agentKeys_itor != agentKeys.end())
            {
                targetAgent = agentKeys_itor->second;
            }
            if (!targetAgent.modelUrlSymbols.isEmpty())
            {
                QJsonObject modelUrlSymbols;

                modelUrlSymbols.insert("modelUrlSymbols", targetAgent.modelUrlSymbols);
                DataManager::getInstance().setEntityIcon(_instance_itor->second.toULongLong(), modelUrlSymbols);
            }

            AgentInstanceInfo agentinstanceinfo;
            agentinstanceinfo.m_agentinfo.agentKeyItem = targetAgent;
            agentinstanceinfo.m_agentinfo.agentKeyItem.agentKey = "AGENTKEY_10503387614339227422";
            agentinstanceinfo.m_agentinfo.agentKeyItem.agentId = _instance_itor->second;
            agentinstanceinfo.m_agentinfo.agentInstId = _instance_itor->first;
            agentinstanceinfo.m_agentinfo.agentKeyItem.agentName = tr("Aircraft Registration:") + _instance_itor->first + "\n" + tr("Aircraft Type:") + _InstagentInstance_itor->first;
            agentinstanceinfo.m_agentinfo.agentKeyItem.agentNameI18n = tr("Aircraft Registration:") + _instance_itor->first + "\n" + tr("Aircraft Type:") + _InstagentInstance_itor->first;
            agentinstanceinfo.m_agentinfo.agentKeyItem.modelUrlFat = "qrc:/res/img/undefined.jpg";

            QJsonObject jsitem;
            agentinstanceinfo.toJson(jsitem);

            jsarr.push_back(jsitem);

            auto agentInstances_itor = std::find_if(agentInstances.begin(),
                                                    agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
                                                        return vt.first == _instance_itor->second;
                                                    });
            if (agentInstances_itor == agentInstances.end())
            {
                agentInstances.insert(std::make_pair(_instance_itor->second, std::move(agentinstanceinfo)));
            }
            _instance_itor++;
        }
        _InstagentInstance_itor++;
    }


    deal_instagentData_slot_runtime(airport_code, allowRunway);
}

#define RUN_SPEED (0.0)

void WidgetManager::deal_instagentData_slot_runtime(const QString &airport_code, const QStringList &allowRunway)
{
    //飞机实体信息
    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();
    m_runfilghtId.clear();

    auto appendInfo = [&](const std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>& flightinfo, const AgentInstanceInfo& agentinstanceinfo, FlightPlanConf* pflighltData) {
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
//        patternAgentsitem.insert("agentLabel", agentinstanceinfo.m_agentinfo.agentKeyItem.agentName);
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
            [&](const std::unordered_map<QString, std::tuple<QJsonObject, bool>>::value_type& vt) {
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
                std::unordered_map<QString, tagTriggerFlightInfo>& idlst = _runfilghtId_itor->second;
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
    std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf*>& day_flights = DataManager::getInstance().total_flightdata;
    auto day_flights_itor = day_flights.begin();
    while (day_flights_itor != day_flights.end())
    {
        FlightPlanConf* pflighltData = day_flights_itor->second;
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
            const AgentInstanceInfo& agentinstanceinfo = agentInstances_itor->second;
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

//            DataManager::getInstance().updatereviewsrecorddir("TierAirlineAirPortSimulation_custom_"+dateStr);
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

        {
            EventDriver* pEventDriver = DataManager::getInstance().peventDriver();
            if(pEventDriver && pEventDriver->running_status() != E_RUNNING_STATUS_TYPE_STOP)
            {
                pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP);
                if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
                {
                    runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
                }
                runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");

//                {
//                    DataManager::getInstance().exportexcel();
//                }
                m_runtime_playstatus = false;
                DataManager::getInstance().trans_log("操作：仿真终止",std::stringstream(),std::stringstream());
            }
        }

        std::stringstream val;
        std::stringstream val2;
        val<<" "<<airportcode.toStdString()<<" "<<url.toStdString();
        DataManager::getInstance().trans_log("操作：仿真数据",val,val2);

        QString dt_st = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh_mm_ss_zzz");
        QString filename = QString("%1/data/plan/%2.xlsx").arg(QCoreApplication::applicationDirPath()).arg(dt_st);
        QString dir = QString("%1/data/plan/").arg(QCoreApplication::applicationDirPath());


        if(!url.contains(dir))
        {
            val.str("");
            val2.str("");
            val<< filename.toStdString();
            val2 << "开始下载";
            DataManager::getInstance().trans_log("操作：仿真数据 ", val, val2);
            if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->requeset_sim_data(url.toStdString(),filename.toStdString()))
            {
                val.str("");
                val2.str("");
                val<< filename.toStdString();
                val2 << "下载失败";
                DataManager::getInstance().trans_log("操作：仿真数据 ",val, val2);
            }
            val.str("");
            val2.str("");
            val<< filename.toStdString();
            val2 << "下载成功 开始解析";
            DataManager::getInstance().trans_log("操作：仿真数据 ", val, val2);
        }
        else
        {
            filename = url;
        }

        if (QFileInfo(filename).exists())
        {
            this->setAirportInfos(airportcode, allowrunwaylist);
            this->importexcel(filename);
            val.str("");
            val2.str("");
            val<< filename.toStdString();
            val2 << "解析完成";
            DataManager::getInstance().trans_log("操作：仿真数据 ",val, val2);
            DataManager::getInstance().trans_sim_data_ok();
        }
        else
        {
            val.str("");
            val2.str("");
            val<< filename.toStdString();
            val2 << "数据导入失败，请重新导入";
            DataManager::getInstance().trans_log("操作：仿真数据 ",val, val2);
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
            auto& reviewdatamap = DataManager::getInstance().m_reviewdata;
            auto itor = reviewdatamap.find(reviewdata);
            if (itor != reviewdatamap.end())
            {
                this->initializeReadFileSlot(&itor->second);
            }
        }
        else
        {
            if ("review" == operate)
            {
                auto& reviewdatamap = DataManager::getInstance().m_reviewdata;
                auto itor = reviewdatamap.find(reviewdata);
                if (itor != reviewdatamap.end())
                {
                    this->initializeReadFileSlot(&itor->second);
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

void WidgetManager::updateRecordstatus(bool bRedord)
{
    if (bRedord)
    {
        runningmode::RunningModeConfig::getInstance().start_process_detached("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
    }
    else
    {
       runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
    }
    SettingsConfig::getInstance().updateRecordstatus(bRedord);
}


void WidgetManager::deal_record_ctrl_slot(const QJsonObject &obj)
{
    if(obj.contains("ctrl") && obj.value("ctrl").isBool())
    {
        bool bEnable = obj.value("ctrl").toBool();
        this->updateRecordstatus(bEnable);
    }
}

void WidgetManager::run_agent_data(const QJsonObject &jsobj)
{
    if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_agent_data(jsobj))
    {
        std::cout << "http error run_agent_data" << std::endl;
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
            std::cout << "http error prgctrl" << std::endl;
        }
    }
}

void WidgetManager::request_agentruntimedata()
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
void WidgetManager::append_agentruntime_data(const QJsonObject &jsobj)
{
    //qDebug() << jsobj;
    if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->append_agentruntime_data(jsobj))
    {
        std::cout << "http error agentruntime_data" << std::endl;
    }
}

void WidgetManager::add_runtime_style_slot(const QJsonObject &runtimestyle)
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

        m_runtimedatas.insert(std::make_pair(runtimeid, std::make_tuple(runtimestyle, false)));


        append_agentruntime_data(runtimestyle);
    }
    else
    {
        std::get<0>(_runtimedatas_itor->second) = runtimestyle;

        QString item_name = "undefined name";
        QString item_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString item_founder = "undefined user";


        update_agentruntime_data(runtimestyle);
    }
    //m_pRuntimeEditWidget->resetData();
}

void WidgetManager::run_agentruntime_data(const QJsonObject &jsobj)
{
    if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->execute_agentruntime_data(jsobj))
    {
        m_peventDriver->stop();
        std::cout << "http error agentruntime_data" << std::endl;
    }
}

void WidgetManager::update_agentruntime_data(const QJsonObject &jsobj)
{
    if (DataManager::getInstance().pHttpClient() && !DataManager::getInstance().pHttpClient()->update_agentruntime_data(jsobj))
    {
        std::cout << "http error agentruntime_data" << std::endl;
    }
}

void WidgetManager::runRuntimeStyle(const QVariant &runtiemstyleid)
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
//        static int cc = 0;
//        //qDebug() << senddata;
//        std::cout << "running runtimestyle " << runtimestyleobj.value("patternSig").toString().toStdString() << "------------ count ------------ " << cc << std::endl;
//        cc++;
        run_agentruntime_data(senddata);
    }
}
void WidgetManager::qml_send_agent_data_slot()
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
    std::stringstream val;
    std::stringstream val2;
    val<<title.toStdString();
    DataManager::getInstance().trans_log("初始化仿真机场数据： ", val, val2);
}

void WidgetManager::environment_init_succeed_slot()
{
    if (m_peventDriver)
    {
        m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_START);
    }


    qml_send_agent_data_slot();

    m_runtime_playstatus = true;

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

void WidgetManager::trigger_runtime_event_end_slot()
{
    if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
    {
        runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
    }
//    DataManager::getInstance().kill_process("gaeactor-hub");
    if (m_peventDriver)
    {
        m_peventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP,false);
    }
    DataManager::getInstance().exportexcel();

    m_runtime_playstatus = false;

    std::stringstream ss;
    ss << "---------------------------------------仿真结束---------------------------------------";
    DataManager::getInstance().trans_log("操作： ", ss, std::stringstream());

    DataManager::getInstance().trans_sim_runtime_end();
}

void WidgetManager::trigger_review_event_end_slot()
{
    if(m_pOriginalDataInputManager->getBRunning())
    {
        m_pOriginalDataInputManager->stop();
    }
    m_review_pausestatus = false;
    DataManager::getInstance().m_review_status = "stop";
    DataManager::getInstance().trans_log("操作：回放结束 ", std::stringstream(), std::stringstream());
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

        this->trigger_flight_to_run_slot(triggertimestamp, triggerflights);
    }
}



#define TARGET_HEIGHT_METER (300.0f)
#define HOLDPOINT_EXTEND_METER (25)
#define HOLDPOINT_EXTEND_AREA_METER (150)
#define TAKEOFF_METER (1750)
#define LANDINGPOINT_METER (15)

//#define USING_NEW_RELLOC_PARKING_POINT

quint64 WidgetManager::getPath(FlightPlanConf *pflighltData, quint64 flightid, QJsonArray &wps,const QString& runtimeid)
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

#else
        LATLNGS_VECTOR &_extendwpslatlng_tmp = _ptagPath_Plan->m_extendwpslatlng;
        LATLNGS_VECTOR &_runway_total = _ptagPath_Plan->m_runway_total;
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


void WidgetManager::trigger_flight_to_run_slot(uint64_t triggertimestamp, const QList<tagTriggerFlightInfo>& triggerflights)
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
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
                            wpsCore.append(virtual_t+10*icount);
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
//                            std::cout << "skip ------------------ wps empty" << std::endl;
                        }

                    }
                }
            }
            idlst_itor++;
        }
    }
}


void WidgetManager::deal_eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype)
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


quint64 WidgetManager::jumpToDataMillisecondPosOffsetPercent(double percent)
{
    if (m_pOriginalDataInputManager)
    {
        m_pOriginalDataInputManager->stop();

        qint64 totalMSecs = DataManager::getInstance().m_currentreview.getTotalMSecs();
        quint64 iJumpDataMillisecondPos = totalMSecs *percent;
        m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPos) - 1;
        m_pOriginalDataInputManager->start();

        /////////////////////////////////////////////////////////////
        auto formatTimeStr=[](qint64 msecs)->std::string{
            qint64 dd =  msecs / (3600 * 1000 * 24);
            qint64 hh = (msecs - dd * (3600 * 1000 * 24)) / (3600 * 1000);
            qint64 mm = (msecs - hh * (3600 * 1000)) / (60 * 1000);
            qint64 ss = (msecs - hh * (3600 * 1000) - mm * (60 * 1000)) / 1000;
            qint64 ms = (msecs - hh * (3600 * 1000) - mm * (60 * 1000) - ss * 1000) % 1000;
            QString timestr = QString("%1 %2:%3:%4.%5").arg(QString::number(dd), 2, '0').arg(QString::number(hh), 2, '0').arg(QString::number(mm), 2, '0').arg(QString::number(ss), 2, '0').arg(QString::number(ms), 3, '0');
            return timestr.toStdString();
        };
        std::cout<<"trigger jump to "<<percent<<" "<<iJumpDataMillisecondPos<<"-->"<<formatTimeStr(iJumpDataMillisecondPos)<<" / "<<totalMSecs<<"-->"<<formatTimeStr(totalMSecs)<<"\n";

        /////////////////////////////////////////////////////////////
    }
    return m_iDataFrames;

}

bool WidgetManager::initializeReadFileSlot(tagReplayItemInfo* _currentitem)
{
    if (nullptr == _currentitem)
    {
        return false;
    }

    DataManager::getInstance().setCurrentReviewItem(*_currentitem);
    //    m_bInitialize = false;

    if (m_pOriginalDataInputManager != nullptr)
    {
        m_pOriginalDataInputManager->clear();
        delete m_pOriginalDataInputManager;
        m_pOriginalDataInputManager = nullptr;
    }

    try
    {
        m_pOriginalDataInputManager = new originaldatastoragestd::OriginalDataInputManager();
    }
    catch (...)
    {
        m_pOriginalDataInputManager = nullptr;

        return false;
    }


    if (!m_pOriginalDataInputManager->initialize(DataManager::getInstance().m_currentreview.m_absoluteFilePath.toStdString().c_str(), nullptr, std::bind(&WidgetManager::data_callback, this,
                                                                                                                              std::placeholders::_1,
                                                                                                                              std::placeholders::_2,
                                                                                                                              std::placeholders::_3,
                                                                                                                              std::placeholders::_4,
                                                                                                                              std::placeholders::_5)))
    {
        return false;
    }
    m_review_playstatus = false;
    m_review_pausestatus = false;
    DataManager::getInstance().m_review_speed = m_pOriginalDataInputManager->getReadSpeed();
    return true;
}

bool WidgetManager::data_callback(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp)
{
    auto inputstorageType = m_pOriginalDataInputManager->getInputStorageType();
    //    inputstorageType = originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS;
    switch (inputstorageType)
    {
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
    {
        //        QDateTime datetime = QDateTime::fromMSecsSinceEpoch(iTimeStamp);
        //        QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();;
        //        qDebug() << formatTimeStr(iDataSendTimeStamp - m_pCurrentitem->m_iTimeStampStart) << "_" << formatTimeStr(m_payStart.msecsTo(currentDateTime)) << "_" << formatTimeStr(m_payStart.msecsTo(datetime));
        //        qDebug() << datetime.toString("yyyy-MM-dd hh:mm:ss.zzz") << "len " << iDataLen /*<< ":" << array.toHex(' ')*/;
        emit this->sendDataSignal(iGlobeFileReadValidDataPos, iDataSendTimeStamp);


        DataManager::getInstance().deal_review(pData, iDataLen, iTimeStamp, iGlobeFileReadValidDataPos, iDataSendTimeStamp);
    }
    break;
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
    case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
    {
        //        this->decodestateData(pData,iDataLen,iTimeStamp,iGlobeFileReadBeginValidDataPos,iDataSendTimeStamp, true);
    }
    break;
    default:
        break;
    }
    return true;
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

void WidgetManager::sendDataSlot(qint64 iGlobeFileReadValidDataPos, qint64 iDataSendTimeStamp)
{
    DataManager::getInstance().getTimestampRange_reviewdata(iPrePackDataSendTime, iDataSendTimeStamp);

    iPrePackDataSendTime = iDataSendTimeStamp;

    if(iGlobeFileReadValidDataPos == DataManager::getInstance().m_currentreview.m_iValidDataTotalLen)
    {
        trigger_review_event_end_slot();
    }
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
                    pEventDriver->set_speed_coeff(speed);
                    std::stringstream ss;
                    ss<<QString::number(speed).toStdString();
                    DataManager::getInstance().trans_log("操作：仿真速度调整",ss,std::stringstream());
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
                    if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
                    {
                        runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
                    }
                    DataManager::getInstance().updateConnecting(false);
                    runningmode::RunningModeConfig::getInstance().restart_process("gaeactor-hub");
                    DataManager::getInstance().trans_log("操作：仿真启动",std::stringstream(),std::stringstream());

                }
                if("pause" == ctrlparam)
                {
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_PAUSE);
                    m_runtime_pausestatus = true;
                    DataManager::getInstance().trans_log("操作：仿真暂停",std::stringstream(),std::stringstream());
                }

                if("resume" == ctrlparam)
                {
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_RESUME);
                    m_runtime_pausestatus = false;
                    DataManager::getInstance().trans_log("操作：仿真继续",std::stringstream(),std::stringstream());
                }

                if("stop" == ctrlparam)
                {
                    if(SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
                    {
                        runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
                    }
                    runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-hub");
                    pEventDriver->setRunning_status(E_RUNNING_STATUS_TYPE_STOP);

                    DataManager::getInstance().exportexcel();

                    m_runtime_playstatus = false;
                    DataManager::getInstance().trans_log("操作：仿真终止",std::stringstream(),std::stringstream());
                }
            }
        }
}


void WidgetManager::deal_review(const QString &ctrltype, const QString &ctrlparam)
{

    if(m_pOriginalDataInputManager)
    {
        if(ctrltype == "prgctrl")
        {
            double speed = ctrlparam.toDouble();

            if(m_pOriginalDataInputManager->getBRunning())
            {
                if(fabs(m_pOriginalDataInputManager->getReadSpeed() - 0.0) < EPSILON_7)
                {
                    //暂停状态
                    m_pausespeed = speed;
                        DataManager::getInstance().m_review_speed = speed;
                }
                else
                {
                    //非暂停状态
                    DataManager::getInstance().m_review_speed = speed;
                    m_pOriginalDataInputManager->setReadSpeed(speed);
                }

                std::stringstream ss;
                ss<<QString::number(speed).toStdString();
                DataManager::getInstance().trans_log("操作：回放速度调整",ss,std::stringstream());
            }
            else
            {

                std::stringstream ss;
                ss<<QString::number(speed).toStdString();
                DataManager::getInstance().trans_log("操作：回放未启动，回放速度调整无效",ss,std::stringstream());
            }
        }
        else if(ctrltype == "processctrl")
        {
            if("start" == ctrlparam)
            {
                m_review_playstatus = true;
                if (!m_pOriginalDataInputManager->getBRunning())
                {
                    m_pOriginalDataInputManager->start();
                }
                DataManager::getInstance().m_review_status = "start";
                DataManager::getInstance().trans_log("操作：回放启动",std::stringstream(),std::stringstream());
            }
            if("pause" == ctrlparam)
            {
                m_review_pausestatus = true;
                if (m_pOriginalDataInputManager->getBRunning())
                {
                    m_pausespeed = m_pOriginalDataInputManager->getReadSpeed();
                    DataManager::getInstance().m_review_speed = 0.0;
                    m_pOriginalDataInputManager->setReadSpeed(0.0);
                }
                DataManager::getInstance().m_review_status = "pause";
                DataManager::getInstance().trans_log("操作：回放暂停",std::stringstream(),std::stringstream());
            }

            if("resume" == ctrlparam)
            {
                m_review_pausestatus = false;
                if (m_pOriginalDataInputManager->getBRunning())
                {
                    DataManager::getInstance().m_review_speed = m_pausespeed;
                    m_pOriginalDataInputManager->setReadSpeed(m_pausespeed);
                }
                DataManager::getInstance().m_review_status = "resume";
                DataManager::getInstance().trans_log("操作：回放继续",std::stringstream(),std::stringstream());
            }

            if("stop" == ctrlparam)
            {
                m_review_playstatus = false;
                if (m_pOriginalDataInputManager->getBRunning())
                {
                    m_pOriginalDataInputManager->stop();
                }
                DataManager::getInstance().m_review_status = "stop";
                DataManager::getInstance().trans_log("操作：回放终止",std::stringstream(),std::stringstream());
            }
        }
        else if(ctrltype == "processjumpctrl")
        {
            double percent = ctrlparam.toDouble();

            m_iDataFrames = this->jumpToDataMillisecondPosOffsetPercent(percent);
        }
    }
}

