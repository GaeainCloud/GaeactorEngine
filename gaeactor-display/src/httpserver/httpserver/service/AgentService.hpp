
#ifndef AGENT_SERVICE_HPP
#define AGENT_SERVICE_HPP

#include "../dto/AgentDto.hpp"

#include "oatpp/web/protocol/http/Http.hpp"
#include "oatpp/core/macro/component.hpp"
#include "LocationHelper.h"
#include "head_define.h"

namespace oatpp {
namespace parser {
namespace json {
namespace mapping {

class ObjectMapper;
}
}
}
}
class AgentService
{
public:
    AgentService();
    ~AgentService();
private:
    typedef oatpp::web::protocol::http::Status Status;
public:
    oatpp::Object<simulationRangeDto> outputSimulationRange();
    oatpp::Object<simulationReviewRangeDto> outputSimulationReviewRange();
    oatpp::Object<recordCtrlDto> outputRecordCtrl();
    oatpp::Object<simparamDto> outputSimParamCtrl();
    oatpp::Object<simparamDto> outputSimReviewParamCtrl();
    oatpp::Object<flightQueryReturnDto> inputFlightQuery(const oatpp::Object<flightQueryRunningDto>& dto);
    oatpp::Object<flightReviewQueryReturnDto> inputFlightReviewQuery(const oatpp::Object<flightQueryRunningDto> &dto);

    oatpp::Object<simulationDataDto> inputSimulationData(const oatpp::Object<simulationDataDto>& dto);
    oatpp::Object<recordCtrlDto> inputRecordCtrl(const oatpp::Object<recordCtrlDto>& dto);

    oatpp::Object<simulationReviewReturnDto> outputSimulationReview();
    oatpp::Object<simulationReviewDataDto> inputSimulationReviewData(const oatpp::Object<simulationReviewDataDto>& dto);


    oatpp::Object<simulationCtrlReturnDto> inputSimulationCtrl(const oatpp::Object<simulationCtrlDto>& dto);
    oatpp::Object<simulationCtrlReturnDto> outputSimulationCtrl();
    oatpp::Object<simulationCtrlReturnDto> outputSimulationProcessCtrl();


    oatpp::Object<simulationCtrlReturnDto> inputSimulationReviewCtrl(const oatpp::Object<simulationCtrlDto>& dto);
    oatpp::Object<simulationCtrlReturnDto> outputSimulationReviewCtrl();
    oatpp::Object<simulationCtrlReturnDto> outputSimulationProcessReviewCtrl();

public:
    void setDataCallback(http_receive_callback func);
private:
    http_receive_callback m_httpreceive_callback;
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> m_jsonmapper;
};


#endif //AGENT_SERVICE_HPP
