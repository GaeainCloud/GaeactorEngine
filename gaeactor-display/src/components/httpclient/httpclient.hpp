#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <memory>
#include <curl/curl.h>

#include <QHash>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include "components/function.h"
class HttpClient : public QObject
{
    Q_OBJECT
public:
    HttpClient(QObject *parent=nullptr);
    ~HttpClient();
    void initEnvironment();
    void destroyEnvironment();
    bool append_agentruntime_data(const QJsonObject &jsobj);
    bool update_agentruntime_data(const QJsonObject &jsobj);
    bool delete_agentruntime_data(const QJsonObject &jsobj);
    bool execute_agentruntime_data(const QJsonObject &jsobj);
    bool execute_agent_data(const QJsonObject &jsobj);
	bool execute_running_speed(const QJsonObject &jsobj);
	bool execute_poicmd(const QJsonObject &jsobj);

	bool append_agent_data(const QJsonObject &jsobj);
    bool update_agent_data(const QJsonObject &jsobj);
    bool delete_agent_data(const QJsonObject &jsobj);


    bool append_action_data(const QString &agentKey,const QJsonObject &jsobj);
    bool update_action_data(const QString &agentKey,const QJsonObject &jsobj);
    bool delete_action_data(const QString &agentKey,const QJsonObject &jsobj);


    bool append_ooda_data(const QString &agentKey,const QJsonObject &jsobj);
    bool update_ooda_data(const QString &agentKey,const QJsonObject &jsobj);
    bool delete_ooda_data(const QString &agentKey,const QJsonObject &jsobj);


    bool append_variable_data(const QString &agentKey,const QJsonObject &jsobj);
    bool update_variable_data(const QString &agentKey,const QJsonObject &jsobj);
    bool delete_variable_data(const QString &agentKey,const QJsonObject &jsobj);


    bool append_field_data(const QString &agentKey,const QJsonObject &jsobj);
    bool update_field_data(const QString &agentKey,const QJsonObject &jsobj);
    bool delete_field_data(const QString &agentKey,const QJsonObject &jsobj);
    bool select_FldmdForAgent(const QString &agentKey);

    bool append_sensing_data(const QString &agentKey,const QJsonObject &jsobj);
    bool update_sensing_data(const QString &agentKey,const QJsonObject &jsobj);
    bool delete_sensing_data(const QString &agentKey,const QJsonObject &jsobj);


    bool requeset_agentByid_data(const QString agentKey,QJsonObject &jsobj);

    bool requeset_sim_data(const std::string & url, const std::string &filname);

	bool request_agentid_agentdata(const UINT64 &agentid, QJsonObject &jsobj);

    bool requeset_agent_data(QJsonObject &jsobj);
    bool requeset_agentruntime_data(QJsonObject &jsobj);

private:
    static size_t receive_data(void *contents, size_t size, size_t nmemb, void *stream);

    static size_t write_file_data(void *ptr, size_t size, size_t nmemb, void *stream);
    CURLcode httpGet(const std::string & url,std::string & response, int timeout);    
    void httpAsyncGet(const std::string & url);
    CURLcode httpPost(const std::string & url,const std::string postdata,std::string & response, int timeout);

    void get_token();

    CURLcode httpGetFile(const std::string & url, const std::string &filname, int timeout);


private:
    CURL * m_pCurl;

    std::string m_token;
};



#endif // HttpServer
