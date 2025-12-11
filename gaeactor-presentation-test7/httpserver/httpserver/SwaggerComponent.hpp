
#ifndef SwaggerComponent_hpp
#define SwaggerComponent_hpp

#include "oatpp-swagger/Model.hpp"
#include "oatpp-swagger/Resources.hpp"
#include "oatpp/core/macro/component.hpp"
#include "settingsconfig.h"

#define OATPP_SWAGGER_RES_PATH "./res/oatpp-swagger/res"

/**
 *  Swagger ui is served at
 *  http://host:port/swagger/ui
 */
class SwaggerComponent {
public:
  
  /**
   *  General API docs info
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::DocumentInfo>, swaggerDocumentInfo)([] {
    
    oatpp::swagger::DocumentInfo::Builder builder;

    builder
#ifdef USING_GUI_SHOW
    .setTitle("Deconflictor_gui service")
    .setDescription("Deconflictor_gui with swagger docs")
#else
    .setTitle("Deconflictor_gui service")
    .setDescription("Deconflictor with swagger docs")
#endif
    .setVersion("1.0")
    .setContactName("Ivan Ovsyanochka")
    .setContactUrl("https://oatpp.io/")
    
    .setLicenseName("Apache License, Version 2.0")
    .setLicenseUrl("http://www.apache.org/licenses/LICENSE-2.0")
#ifdef USING_GUI_SHOW
        .addServer(SettingsConfig::getInstance().deconflictor_gui_http_url().toStdString(), "server on localhost");
#else
        .addServer(SettingsConfig::getInstance().deconflictor_http_url().toStdString(), "server on localhost");
#endif
    
    return builder.build();
    
  }());
  
  
  /**
   *  Swagger-Ui Resources (<oatpp-examples>/lib/oatpp-swagger/res)
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::swagger::Resources>, swaggerResources)([] {
    // Make sure to specify correct full path to oatpp-swagger/res folder !!!
    return oatpp::swagger::Resources::loadResources(OATPP_SWAGGER_RES_PATH);
  }());
  
};

#endif /* SwaggerComponent_hpp */
