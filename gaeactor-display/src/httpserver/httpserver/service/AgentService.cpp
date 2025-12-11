
#include "AgentService.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "../../../components/configmanager.h"
#include <QJsonArray>
#include "components/function.h"
#include "../../../components/eventdriver/eventdriver.h"
#include "../../../datamanager/datamanager.hpp"
#include "settingsconfig.h"

#include <random>
#include "gaeactor_processor_interface.h"
#include "LocationHelper.h"
#include <QTimeZone>


AgentService::AgentService()
{
    m_jsonmapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    m_jsonmapper->getSerializer()->getConfig()->useBeautifier = false;
}

AgentService::~AgentService()
{

}

oatpp::Object<simulationRangeDto> AgentService::outputSimulationRange()
{
    std::cout<<"*************recv query runtime sim range"<<std::endl;
    QJsonObject resultObj = DataManager::getInstance().outputSimulationRange();
    oatpp::String str = FunctionAssistant::json_object_to_string(resultObj).toStdString();
    oatpp::Object<simulationRangeDto> result = m_jsonmapper->readFromString<oatpp::Object<simulationRangeDto>>(str);
    return result;
}

oatpp::Object<simulationReviewRangeDto> AgentService::outputSimulationReviewRange()
{
    std::cout<<"*************recv query review sim range"<<std::endl;
    QJsonObject resultObj = DataManager::getInstance().outputSimulationReviewRange();
    oatpp::String str = FunctionAssistant::json_object_to_string(resultObj).toStdString();
    oatpp::Object<simulationReviewRangeDto> result = m_jsonmapper->readFromString<oatpp::Object<simulationReviewRangeDto>>(str);
    return result;
}

oatpp::Object<recordCtrlDto> AgentService::outputRecordCtrl()
{
    std::cout<<"*************recv query record status"<<std::endl;

    auto _recordCtrlDto = recordCtrlDto::createShared();
    _recordCtrlDto->ctrl = SettingsConfig::getInstance().lavic_desktop_cfg().m_recording;
    return _recordCtrlDto;
}

oatpp::Object<simparamDto> AgentService::outputSimParamCtrl()
{
    std::cout<<"*************recv query sim params"<<std::endl;
    auto _simparamDto = simparamDto::createShared();

    _simparamDto->step_interval = ConfigManager::getInstance().step_interval;
    _simparamDto->step_dt = ConfigManager::getInstance().step_dt;
    _simparamDto->step_freq = ConfigManager::getInstance().step_freq;
    _simparamDto->one_second_sim_step_second = ConfigManager::getInstance().one_second_sim_step_second;

    return _simparamDto;
}

oatpp::Object<simparamDto> AgentService::outputSimReviewParamCtrl()
{
    std::cout<<"*************recv query sim params"<<std::endl;
    auto _simparamDto = simparamDto::createShared();

    _simparamDto->step_interval = ConfigManager::getInstance().review_step_interval;
    _simparamDto->step_dt = ConfigManager::getInstance().review_step_dt;
    _simparamDto->step_freq = ConfigManager::getInstance().review_step_freq;
    _simparamDto->one_second_sim_step_second = ConfigManager::getInstance().review_one_second_sim_step_second;

    _simparamDto->review_speed = DataManager::getInstance().m_review_speed;
    _simparamDto->review_status = DataManager::getInstance().m_review_status.toStdString();
    return _simparamDto;
}

oatpp::Object<flightQueryReturnDto> AgentService::inputFlightQuery(const oatpp::Object<flightQueryRunningDto> &dto)
{
    std::cout<<"*************recv query flight data"<<std::endl;
    auto _FlightQueryReturnDto = flightQueryReturnDto::createShared();
    _FlightQueryReturnDto->flightInfos = oatpp::Vector<oatpp::Object<flightInfoDto>>::createShared();
    auto itor = dto->flightsId->begin();
    while(itor != dto->flightsId->end())
    {
        auto flightidstr_val = QString::fromStdString(*itor);

        auto flightid = flightidstr_val.toULongLong();
        auto _FlightInfoDto = flightInfoDto::createShared();
        _FlightInfoDto->flightid = flightidstr_val.toStdString();

        auto flightinfos = DataManager::getInstance().getflightInfo(flightid);
        tagPath_Plan* path_plan = flightinfos._tagPath_Plan;
        FlightPlanConf* pFlightPlanConf = flightinfos.pflightdata;

        if(pFlightPlanConf &&
            path_plan &&
            pFlightPlanConf->flightid == QString::number(flightid))
        {
            _FlightInfoDto->target_parkingpoint = flightinfos.pathplanvalidinfo.target_parkingpoint.toStdString();
            _FlightInfoDto->target_runway = flightinfos.pathplanvalidinfo.target_runway.toStdString();
            _FlightInfoDto->alloc_parkingpoint = flightinfos.pathplanvalidinfo.alloc_parkingpoint.toStdString();
            _FlightInfoDto->alloc_runway = flightinfos.pathplanvalidinfo.alloc_runway.toStdString();
//            _FlightInfoDto->date = pFlightPlanConf->m_Date.toStdString();
//            _FlightInfoDto->filghtNumber = pFlightPlanConf->m_FilghtNumber.toStdString();
//            _FlightInfoDto->depArrFlag = pFlightPlanConf->m_DepArrFlag.toStdString();
//            _FlightInfoDto->planeNum = pFlightPlanConf->m_PlaneNum.toStdString();
//            _FlightInfoDto->planeType = pFlightPlanConf->m_PlaneType.toStdString();
//            _FlightInfoDto->flightClass = pFlightPlanConf->m_FlightClass.toStdString();
//            _FlightInfoDto->flightLeg = pFlightPlanConf->m_FlightLeg.toStdString();
//            _FlightInfoDto->flightStartPlace = pFlightPlanConf->m_FlightStartPlace.toStdString();
//            _FlightInfoDto->flightEndPlace = pFlightPlanConf->m_FlightEndPlace.toStdString();
//            _FlightInfoDto->planDateTimeTakeOff = pFlightPlanConf->m_PlanDateTimeTakeOff.toStdString();
//            _FlightInfoDto->planDateTimeLanding = pFlightPlanConf->m_PlanDateTimeLanding.toStdString();
//            _FlightInfoDto->seat = pFlightPlanConf->m_Seat.toStdString();
//            _FlightInfoDto->terminal = pFlightPlanConf->m_Terminal.toStdString();
//            _FlightInfoDto->runway = pFlightPlanConf->m_Runway.toStdString();
        }
        _FlightQueryReturnDto->flightInfos->push_back(_FlightInfoDto);
        itor++;
    }
    return _FlightQueryReturnDto;
}

oatpp::Object<flightReviewQueryReturnDto> AgentService::inputFlightReviewQuery(const oatpp::Object<flightQueryRunningDto> &dto)
{

    std::cout<<"*************recv query flight review data"<<std::endl;
    auto _FlightQueryReturnDto = flightReviewQueryReturnDto::createShared();
    _FlightQueryReturnDto->flightInfos = oatpp::Vector<oatpp::Object<flightInfoReviewDto>>::createShared();
    auto itor = dto->flightsId->begin();
    while(itor != dto->flightsId->end())
    {
        auto flightidstr_val = QString::fromStdString(*itor);

        auto flightid = flightidstr_val.toULongLong();
        auto _FlightInfoDto = flightInfoReviewDto::createShared();
        _FlightInfoDto->flightid = flightidstr_val.toStdString();

        auto flightinfos = DataManager::getInstance().get_flightInfo_from_db(flightid);

        if(flightinfos.flightid == QString::number(flightid).toStdString())
        {
            _FlightInfoDto->target_parkingpoint = flightinfos.target_parkingpoint;
            _FlightInfoDto->target_runway = flightinfos.target_runway;
            _FlightInfoDto->alloc_parkingpoint = flightinfos.alloc_parkingpoint;
            _FlightInfoDto->alloc_runway = flightinfos.alloc_runway;
            _FlightInfoDto->date = flightinfos.m_Date;
            _FlightInfoDto->filghtNumber = flightinfos.m_FilghtNumber;
            _FlightInfoDto->depArrFlag = flightinfos.m_DepArrFlag;
            _FlightInfoDto->planeNum = flightinfos.m_PlaneNum;
            _FlightInfoDto->planeType = flightinfos.m_PlaneType;
            _FlightInfoDto->flightClass = flightinfos.m_FlightClass;
            _FlightInfoDto->flightLeg = flightinfos.m_FlightLeg;
            _FlightInfoDto->flightStartPlace = flightinfos.m_FlightStartPlace;
            _FlightInfoDto->flightEndPlace = flightinfos.m_FlightEndPlace;
            _FlightInfoDto->planDateTimeTakeOff = flightinfos.m_PlanDateTimeTakeOff;
            _FlightInfoDto->planDateTimeLanding = flightinfos.m_PlanDateTimeLanding;
            _FlightInfoDto->seat = flightinfos.m_Seat;
            _FlightInfoDto->terminal = flightinfos.m_Terminal;
            _FlightInfoDto->runway = flightinfos.m_Runway;
        }
        _FlightQueryReturnDto->flightInfos->push_back(_FlightInfoDto);
        itor++;
    }
    return _FlightQueryReturnDto;
}

oatpp::Object<simulationDataDto> AgentService::inputSimulationData(const oatpp::Object<simulationDataDto> &dto)
{
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject jsonobj  = FunctionAssistant::string_to_json_object(QString::fromStdString(result));
    qDebug()<<"*************set sim runtime data"<<jsonobj;
    m_httpreceive_callback(E_DATA_TYPE_SIM_DATA, jsonobj);
    return dto;
}

oatpp::Object<recordCtrlDto> AgentService::inputRecordCtrl(const oatpp::Object<recordCtrlDto> &dto)
{
    std::cout<<"*************set record status"<<std::endl;
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject jsonobj  = FunctionAssistant::string_to_json_object(QString::fromStdString(result));
    m_httpreceive_callback(E_DATA_TYPE_RECORD_CTRL, jsonobj);
    return dto;
}

oatpp::Object<simulationReviewReturnDto> AgentService::outputSimulationReview()
{
    std::cout<<"*************recv query sim review list"<<std::endl;

    auto _simulationReviewReturnDto = simulationReviewReturnDto::createShared();
    _simulationReviewReturnDto->reviewdatas = oatpp::Vector<oatpp::Object<simulationReviewItemDto>>::createShared();

    DataManager::getInstance().loadreviews();

    auto itor = DataManager::getInstance().m_reviewdata.begin();
    while (itor != DataManager::getInstance().m_reviewdata.end())
    {
        auto _simulationReviewCtrlDto = simulationReviewItemDto::createShared();

        _simulationReviewCtrlDto->reviewdata = itor->first.toStdString();

        _simulationReviewCtrlDto->size = itor->second.m_size;
        _simulationReviewCtrlDto->date = itor->second.m_date.toStdString();
        _simulationReviewCtrlDto->simid = itor->second.m_simid.toStdString();
        QString titlename_ = itor->second.m_titlename.mid(0, itor->second.m_titlename.lastIndexOf("_"));
        _simulationReviewCtrlDto->titlename = titlename_.toStdString();
        _simulationReviewCtrlDto->iTimeStampStart  = itor->second.m_iTimeStampStart;
        _simulationReviewCtrlDto->iTimeStampEnd  = itor->second.m_iTimeStampEnd;
        _simulationReviewCtrlDto->iValidDataTotalLen = itor->second.m_iValidDataTotalLen;
        _simulationReviewCtrlDto->iFrames = itor->second.m_iFrames;

        QDateTime startTime = QDateTime::fromMSecsSinceEpoch(itor->second.m_iTimeStampStart);
        QDateTime endTime = QDateTime::fromMSecsSinceEpoch(itor->second.m_iTimeStampEnd);
        qint64 msecs = startTime.msecsTo(endTime);

        startTime.setTimeSpec(Qt::UTC);
        endTime.setTimeSpec(Qt::UTC);
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
        QDateTime localstartTime = startTime;
        QDateTime localendTime = endTime;
#else /* UNIX */
        // 获取本地时区
//        //QTimeZone localTimeZone = QTimeZone::systemTimeZone();
//        // 设置时区偏移量为+8:00
//        QTimeZone localTimeZone = QTimeZone("Asia/Shanghai");

//        // 将UTC时间转换为本地时间
//        QDateTime localstartTime = startTime.toTimeZone(localTimeZone);
//        QDateTime localendTime = endTime.toTimeZone(localTimeZone);
        QDateTime localstartTime = startTime.toOffsetFromUtc(8 * 60 * 60);
        QDateTime localendTime = endTime.toOffsetFromUtc(8 * 60 * 60);
#endif
        _simulationReviewCtrlDto->iTimeStampStartStr = localstartTime.toString("yyyy-MM-dd hh:mm:ss").toStdString();
        _simulationReviewCtrlDto->iTimeStampEndStr = localendTime.toString("yyyy-MM-dd hh:mm:ss").toStdString();

        auto formatTimeStr=[](qint64 msecs)->std::string{
            qint64 dd =  msecs / (3600 * 1000 * 24);
            qint64 hh = (msecs - dd * (3600 * 1000 * 24)) / (3600 * 1000);
            qint64 mm = (msecs - hh * (3600 * 1000)) / (60 * 1000);
            qint64 ss = (msecs - hh * (3600 * 1000) - mm * (60 * 1000)) / 1000;
            qint64 ms = (msecs - hh * (3600 * 1000) - mm * (60 * 1000) - ss * 1000) % 1000;
            QString timestr = QString("%1 %2:%3:%4.%5").arg(QString::number(dd), 2, '0').arg(QString::number(hh), 2, '0').arg(QString::number(mm), 2, '0').arg(QString::number(ss), 2, '0').arg(QString::number(ms), 3, '0');
            return timestr.toStdString();
        };
        _simulationReviewCtrlDto->iTimeStampStr = formatTimeStr(msecs);

        _simulationReviewReturnDto->reviewdatas->push_back(_simulationReviewCtrlDto);
        itor++;
    }

    return _simulationReviewReturnDto;
}

oatpp::Object<simulationReviewDataDto> AgentService::inputSimulationReviewData(const oatpp::Object<simulationReviewDataDto> &dto)
{
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject jsonobj  = FunctionAssistant::string_to_json_object(QString::fromStdString(result));
    qDebug()<<"*************set sim review data"<<jsonobj;
    m_httpreceive_callback(E_DATA_TYPE_SIM_REVIEW_DATA, jsonobj);
    return dto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::inputSimulationCtrl(const oatpp::Object<simulationCtrlDto> &dto)
{
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject jsonobj  = FunctionAssistant::string_to_json_object(QString::fromStdString(result));
    qDebug()<<"*************set sim runtime ctrl"<<jsonobj;
    m_httpreceive_callback(E_DATA_TYPE_SIM_CTRL, jsonobj);
    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = dto->ctrltype;
    _simulationCtrlReturnDto->ctrlparam = dto->ctrlparam;

    if(jsonobj.contains("ctrltype") && jsonobj.contains("ctrlparam") && jsonobj.value("ctrltype").isString() && jsonobj.value("ctrlparam").isString())
    {
        auto ctrltype = jsonobj.value("ctrltype").toString();
        auto param = jsonobj.value("ctrlparam").toString();
        if(ctrltype == "prgctrl")
        {
            double speed = param.toDouble();
            if(speed >= MIN_SPEED_FREQ && speed <= MAX_SPEED_FREQ)
            {
                _simulationCtrlReturnDto->ctrlparam_status = "ok";
            }
            else
            {
                _simulationCtrlReturnDto->ctrlparam_status = QString("failed over range %1 - %2").arg(MIN_SPEED_FREQ).arg(MAX_SPEED_FREQ).toStdString();
            }
        }
    }
    return _simulationCtrlReturnDto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::outputSimulationCtrl()
{    
    std::cout<<"*************recv query sim runtime status ctrl"<<std::endl;
    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = "processctrl";
//    _simulationCtrlReturnDto->ctrltype = dto->ctrltype;
//    _simulationCtrlReturnDto->ctrlparam = dto->ctrlparam;

    EventDriver* pEventDriver = DataManager::getInstance().peventDriver();
    if(pEventDriver)
    {
        auto status = pEventDriver->running_status();
        switch (status) {
        case E_RUNNING_STATUS_TYPE_START:_simulationCtrlReturnDto->ctrlparam = "start";break;
        case E_RUNNING_STATUS_TYPE_PAUSE:_simulationCtrlReturnDto->ctrlparam = "pause";break;
        case E_RUNNING_STATUS_TYPE_RESUME:_simulationCtrlReturnDto->ctrlparam = "resume";break;
        case E_RUNNING_STATUS_TYPE_STOP:_simulationCtrlReturnDto->ctrlparam = "stop";break;
        default:
            break;
        }
    }
    else
    {
        _simulationCtrlReturnDto->ctrlparam = "";
    }
    return _simulationCtrlReturnDto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::outputSimulationProcessCtrl()
{
    std::cout<<"*************recv query sim runtime prg ctrl"<<std::endl;

    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = "prgctrl";
    EventDriver* pEventDriver = DataManager::getInstance().peventDriver();
    if(pEventDriver)
    {
        double speed = pEventDriver->speed_coeff();
        _simulationCtrlReturnDto->ctrlparam = QString::number(speed).toStdString();
    }
    else
    {
        _simulationCtrlReturnDto->ctrlparam = "";
    }

    return _simulationCtrlReturnDto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::inputSimulationReviewCtrl(const oatpp::Object<simulationCtrlDto> &dto)
{
    auto result = m_jsonmapper->writeToString(dto);
    QJsonObject jsonobj  = FunctionAssistant::string_to_json_object(QString::fromStdString(result));
    qDebug()<<"*************set sim review ctrl"<<jsonobj;
    m_httpreceive_callback(E_DATA_TYPE_SIM_REVIEW_CTRL, jsonobj);
    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = dto->ctrltype;
    _simulationCtrlReturnDto->ctrlparam = dto->ctrlparam;

    if(jsonobj.contains("ctrltype") && jsonobj.contains("ctrlparam") && jsonobj.value("ctrltype").isString() && jsonobj.value("ctrlparam").isString())
    {
        auto ctrltype = jsonobj.value("ctrltype").toString();
        auto param = jsonobj.value("ctrlparam").toString();
        if(ctrltype == "prgctrl")
        {
            double speed = param.toDouble();
            if(speed >= MIN_SPEED_FREQ && speed <= MAX_SPEED_FREQ)
            {
                _simulationCtrlReturnDto->ctrlparam_status = "ok";
            }
            else
            {
                _simulationCtrlReturnDto->ctrlparam_status = QString("failed over range %1 - %2").arg(MIN_SPEED_FREQ).arg(MAX_SPEED_FREQ).toStdString();
            }
        }
    }
    return _simulationCtrlReturnDto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::outputSimulationReviewCtrl()
{
    std::cout<<"*************recv query sim review status ctrl"<<std::endl;

    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = "processctrl";
    _simulationCtrlReturnDto->ctrlparam = DataManager::getInstance().m_review_status.toStdString();
    return _simulationCtrlReturnDto;
}

oatpp::Object<simulationCtrlReturnDto> AgentService::outputSimulationProcessReviewCtrl()
{
    std::cout<<"*************recv query sim review prg ctrl"<<std::endl;

    auto _simulationCtrlReturnDto= simulationCtrlReturnDto::createShared();
    _simulationCtrlReturnDto->ctrltype = "prgctrl";
    _simulationCtrlReturnDto->ctrlparam = QString::number(DataManager::getInstance().m_review_speed).toStdString();
    return _simulationCtrlReturnDto;
}



void AgentService::setDataCallback(http_receive_callback func)
{
    m_httpreceive_callback = std::move(func);
}


