#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <memory>
#include "./httpserver/dto/AgentDto.hpp"
#include "base_define.h"
namespace oatpp {
namespace network {
class Server;
class ServerConnectionProvider;
class ConnectionHandler;
}
}
class AgentController;
class HttpServerComponent;
class HttpServer : public QObject
{
public:
    HttpServer(QObject *parent=nullptr);
    ~HttpServer();

    static void initEnvironment();
    static void destroyEnvironment();
    void init();
    void run();
    void stop();
    void transmitData(const BYTE *pData, UINT32 iLen);
    void setDataCallback(http_receive_callback func);
private:
    bool httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject & val);
private:
    HttpServerComponent * m_pAppComponent;

    oatpp::network::Server* m_server;

    std::shared_ptr<AgentController> m_agentcontroller;


    http_receive_callback m_httpreceive_callback;

};
#endif // HttpServer
