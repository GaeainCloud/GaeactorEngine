
#ifndef CRUD_STATICCONTROLLER_HPP
#define CRUD_STATICCONTROLLER_HPP

//#include "oatpp-websocket/Handshaker.hpp"
#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class StaticController : public oatpp::web::server::api::ApiController {
private:
    typedef StaticController __ControllerType;
//private:
//    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler, "websocket");
public:
  StaticController(const std::shared_ptr<ObjectMapper>& objectMapper)
    : oatpp::web::server::api::ApiController(objectMapper)
  {}
public:

  static std::shared_ptr<StaticController> createShared(
    OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
  ){
    return std::make_shared<StaticController>(objectMapper);
  }
public:
  ENDPOINT_ASYNC("GET", "/", root) {

    ENDPOINT_ASYNC_INIT(root)

    const char* pageTemplate =
      "<html lang='en'>"
      "  <head>"
      "    <meta charset=utf-8/>"
      "  </head>"
      "  <body>"
      "    <p>Hello GaeactorHub!</p>"
      "    <a href='swagger/ui'>Checkout Swagger-UI page</a>"
      "  </body>"
      "</html>";
//    auto response = createResponse(Status::CODE_200, html);
//    response->putHeader(Header::CONTENT_TYPE, "text/html");
//    return response;

    Action act() override {
        return _return(controller->createResponse(Status::CODE_200, pageTemplate));
    }
  };

//  ENDPOINT_ASYNC("GET", "ws", WS) {

//    ENDPOINT_ASYNC_INIT(WS)

//    Action act() override {
//        auto response = oatpp::websocket::Handshaker::serversideHandshake(request->getHeaders(), controller->websocketConnectionHandler);
//        return _return(response);
//        }
//    };

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif //CRUD_STATICCONTROLLER_HPP
