#include "httpserver.h"

#include "./httpserver/HttpServerComponent.hpp"

#include "./httpserver/controller/StaticController.hpp"
#include "./httpserver/controller/AgentController.hpp"
#include "base_define.h"
#include "oatpp/network/Server.hpp"
#include "oatpp-swagger/AsyncController.hpp"
#include <iostream>
#include "loghelper.h"
HttpServer::HttpServer(QObject *parent)
    :m_pAppComponent(nullptr)
    ,m_server(nullptr)
    ,m_httpreceive_callback(nullptr)
{
    //initEnvironment();
    //init();
}

HttpServer::~HttpServer()
{
    if(m_pAppComponent)
    {
        delete m_pAppComponent;
    }

    if(m_server)
    {
        delete m_server;
    }

    //destroyEnvironment();
}

void HttpServer::initEnvironment()
{
    // 初始化 oatpp 环境
    oatpp::base::Environment::init();
}

void HttpServer::destroyEnvironment()
{
    // 销毁 oatpp 环境
    oatpp::base::Environment::destroy();
}

void HttpServer::init()
{
    m_pAppComponent = new HttpServerComponent();

    oatpp::web::server::api::Endpoints docEndpoints;

    m_agentcontroller = AgentController::createShared();
    m_agentcontroller->setDataCallback(std::bind(&HttpServer::httpdatareceive_callback,this, std::placeholders::_1, std::placeholders::_2));
    docEndpoints.append(m_pAppComponent->httpRouter.getObject()->addController(m_agentcontroller)->getEndpoints());

    m_pAppComponent->httpRouter.getObject()->addController(oatpp::swagger::AsyncController::createShared(docEndpoints));
    m_pAppComponent->httpRouter.getObject()->addController(StaticController::createShared());

    /* create server */
   m_server = new oatpp::network::Server(m_pAppComponent->serverConnectionProvider.getObject(),
                                          m_pAppComponent->serverConnectionHandler.getObject());
}

void HttpServer::run()
{
   initEnvironment();
   init();

   QString tracestr = QString("http Server Running on port %1:%2").arg(QString::fromStdString(m_pAppComponent->serverConnectionProvider.getObject()->getProperty("host").toString()->c_str()))
                          .arg(QString::fromStdString(m_pAppComponent->serverConnectionProvider.getObject()->getProperty("port").toString()->c_str()));
   LOG_PRINT_STR_EX(tracestr);

   m_server->run();

   destroyEnvironment();
}

void HttpServer::stop()
{
    m_server->stop();
}

void HttpServer::transmitData(const BYTE *pData, UINT32 iLen)
{
    if(m_pAppComponent)
    {
        m_pAppComponent->transmitData(pData, iLen);
    }
}

void HttpServer::setDataCallback(http_receive_callback func)
{
    m_httpreceive_callback = std::move(func);
}


bool HttpServer::httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject &val)
{
    return m_httpreceive_callback(eDataType,val);
}

