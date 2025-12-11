
#ifndef HTTPSERVERCOMPONENT_H
#define HTTPSERVERCOMPONENT_H

#include "SwaggerComponent.hpp"

#include "ErrorHandler.hpp"

#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

//#include "oatpp/network/client/SimpleTCPConnectionProvider.hpp"
//#include "oatpp/network/server/SimpleTCPConnectionProvider.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"
#include "settingsconfig.h"
#include <iostream>

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class HttpServerComponent {
public:
  
  /**
   *  Swagger component
   */
  SwaggerComponent swaggerComponent;

//  /**
//   * Create Async Executor
//   */
//  OATPP_CREATE_COMPONENT(std::shared_ptr<WSInstanceListener>, wsInstanceListener)([] {
//      return std::make_shared<WSInstanceListener>();
//  }());

  /**
   * Create Async Executor
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)([] {
      return std::make_shared<oatpp::async::Executor>(
          4 /* Data-Processing threads */,
          1 /* I/O threads */,
          1 /* Timer threads */
          );
  }());


  /**
   * Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)([] {
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    //objectMapper->getSerializer()->getConfig()->includeNullFields = true;
    objectMapper->getDeserializer()->getConfig()->allowUnknownFields = true;
    return objectMapper;

//    auto serializerConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
//    serializerConfig->includeNullFields = true;

//    auto deserializerConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();
//    deserializerConfig->allowUnknownFields = true;

//    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared(serializerConfig, deserializerConfig);
//    return objectMapper;
  }());
  
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)("httpconnect",[] {
      QString localip = SettingsConfig::getInstance().lavicdesktop_localip();
      quint16 _http_port = SettingsConfig::getInstance().lavicdesktop_http_port();
      return oatpp::network::tcp::server::ConnectionProvider::createShared({localip.toStdString(), _http_port, oatpp::network::Address::IP_4});
  }());
  
  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)([] {
    return oatpp::web::server::HttpRouter::createShared();
  }());

  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)("http", [] {

      OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router); // get Router component
      OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper); // get ObjectMapper component

      OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor); // get Async executor component

      auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(router, executor);
      connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
      return connectionHandler;

  }());

//  /**
//   *  Create websocket connection handler
//   */
//  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)("websocket", [] {
//      OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
//      OATPP_COMPONENT(std::shared_ptr<WSInstanceListener>, wsInstanceListener);

//      auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
//      connectionHandler->setSocketInstanceListener(wsInstanceListener);
//      return connectionHandler;
//  }());

  void transmitData(const unsigned char *pData, uint32_t iLen)
  {
//      OATPP_COMPONENT(std::shared_ptr<WSInstanceListener>, wsInstanceListener);
//      wsInstanceListener->transmitData(pData, iLen);
  }

};

#endif /* AppComponent_hpp */
