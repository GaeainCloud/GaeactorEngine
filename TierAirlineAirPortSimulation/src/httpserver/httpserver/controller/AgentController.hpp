
#ifndef AGENT_CONTROLLER_H
#define AGENT_CONTROLLER_H

#include "../service/AgentService.hpp"
#include "../dto/StatusDto.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "settingsconfig.h"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen



//#define CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_ORIGIN "*"
//#define CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_METHODS "GET, POST, PUT, OPTIONS, DELETE"
//#define CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_HEADERS "DNT, User-Agent, X-Requested-With, If-Modified-Since, Cache-Control, Content-Type, Range, Authorization"
//#define CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_MAX_AGE "1728000"

#define CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY(RESPONSE) \
    RESPONSE->putOrReplaceHeader(oatpp::web::protocol::http::Header::CORS_ORIGIN, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_ORIGIN); \
    RESPONSE->putOrReplaceHeader(oatpp::web::protocol::http::Header::CORS_METHODS, "GET, POST, PUT, OPTIONS, DELETE"); \
    RESPONSE->putOrReplaceHeader(oatpp::web::protocol::http::Header::CORS_HEADERS, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_HEADERS); \
    RESPONSE->putOrReplaceHeader(oatpp::web::protocol::http::Header::CORS_MAX_AGE, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_MAX_AGE);


#define  STATEMENT_POST(API,FUNCNAME,INPUT_STRUCT,RETURN_STRUCT, DESC ,SUMARY) \
ADD_CORS(FUNCNAME, SettingsConfig::getInstance().lavicdesktop_connect_url(), "GET, POST, OPTIONS")\
    ENDPOINT_INFO(FUNCNAME) {\
        info->summary = SUMARY;\
        info->description = DESC;\
        info->addConsumes<Object<INPUT_STRUCT>>("application/json");\
        info->addResponse<Object<RETURN_STRUCT>>(Status::CODE_200, "application/json");\
        info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");\
        info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");\
}\
    ENDPOINT_ASYNC("POST", API, FUNCNAME) {\
        ENDPOINT_ASYNC_INIT(FUNCNAME)\
        Action act() override {\
        auto headers = request->getHeaders();\
            headers.put(oatpp::web::protocol::http::Header::CORS_ORIGIN, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_ORIGIN);\
            headers.put(oatpp::web::protocol::http::Header::CORS_HEADERS, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_HEADERS);\
            headers.put(oatpp::web::protocol::http::Header::CORS_METHODS, "GET, POST, PUT, DELETE, OPTIONS");\
            headers.put(oatpp::web::protocol::http::Header::CORS_MAX_AGE, OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY_DEFAULT_MAX_AGE);\
                              return request->readBodyToDtoAsync<oatpp::Object<INPUT_STRUCT>>(controller->getDefaultObjectMapper()).callbackTo(&FUNCNAME::returnResponse);\
}\
    Action returnResponse(const oatpp::Object<INPUT_STRUCT>& userDto) {\
        auto response = controller->createDtoResponse(Status::CODE_200, controller->m_agentService.FUNCNAME(userDto));\
        CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY(response) \
        return _return(response);\
}\
};


#define  STATEMENT_GET(API,FUNCNAME,RETURN_STRUCT, DESC ,SUMARY) \
ADD_CORS(FUNCNAME, SettingsConfig::getInstance().lavicdesktop_connect_url(), "GET, POST, OPTIONS")\
    ENDPOINT_INFO(FUNCNAME) {\
        info->summary = SUMARY;\
        info->description = DESC;\
        info->addResponse<Object<RETURN_STRUCT>>(Status::CODE_200, "application/json");\
        info->addResponse<Object<StatusDto>>(Status::CODE_404, "application/json");\
        info->addResponse<Object<StatusDto>>(Status::CODE_500, "application/json");\
}\
    ENDPOINT_ASYNC("GET", API, FUNCNAME) {\
        ENDPOINT_ASYNC_INIT(FUNCNAME)\
        Action act() override {\
                              auto response = controller->createDtoResponse(Status::CODE_200, controller->m_agentService.FUNCNAME());\
    CUSTOM_OATPP_MACRO_API_CONTROLLER_ADD_CORS_BODY(response) \
    return _return(response);\
}\
};


/**
 * User REST controller.
 */
class AgentController : public oatpp::web::server::api::ApiController {
private:
    typedef AgentController __ControllerType;
private:
    AgentService m_agentService;
public:
  AgentController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper)
  {}

public:

  static std::shared_ptr<AgentController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<AgentController>(objectMapper);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //post
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_POST( "FlightQuery", inputFlightQuery, flightQueryRunningDto, flightQueryReturnDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_POST( "progressCtrl", inputSimulationCtrl, simulationCtrlDto, simulationCtrlReturnDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_POST( "simulationData", inputSimulationData, simulationDataDto, simulationDataDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_POST( "FlightReviewQuery", inputFlightReviewQuery, flightQueryRunningDto, flightReviewQueryReturnDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_POST( "progressReviewCtrl", inputSimulationReviewCtrl, simulationCtrlDto, simulationCtrlReturnDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_POST( "simulationReview", inputSimulationReviewData, simulationReviewDataDto, simulationReviewDataDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_POST( "recordCtrl", inputRecordCtrl, recordCtrlDto, recordCtrlDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //get
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_GET( "/getSimulationRange", outputSimulationRange, simulationRangeDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getProcessCtrl", outputSimulationCtrl, simulationCtrlReturnDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getPrgCtrl", outputSimulationProcessCtrl, simulationCtrlReturnDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getSimParamCtrl", outputSimParamCtrl, simparamDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_GET( "/getSimulationReviewRange", outputSimulationReviewRange, simulationReviewRangeDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getReviewPrgCtrl", outputSimulationProcessReviewCtrl, simulationCtrlReturnDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getReviewProcessCtrl", outputSimulationReviewCtrl, simulationCtrlReturnDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getSimReviewParamCtrl", outputSimReviewParamCtrl, simparamDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  STATEMENT_GET( "/getRecordCtrl", outputRecordCtrl, recordCtrlDto,"Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  STATEMENT_GET( "/getsimreview", outputSimulationReview, simulationReviewReturnDto, "Input Agent Query Running", "<h1>Input</h1><p>agentsId: the list of the Agents's id to query running status</p><h1>Return</h1><p>the context of SensingInfos</p>");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void setDataCallback(http_receive_callback func)
  {
    m_agentService.setDataCallback(func);
  }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif /* AGENT_CONTROLLER_H */
