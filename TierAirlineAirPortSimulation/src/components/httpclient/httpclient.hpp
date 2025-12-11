#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <memory>
#include <curl/curl.h>

#include <QHash>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>
#include "../function.h"
class QNetworkAccessManager;
class QNetworkReply;
class HttpClient : public QObject
{
    Q_OBJECT
public:
	typedef std::function<void(const GeoTilePos& srckey, const QByteArray& imgarray)> img_request_callback;
    HttpClient(QObject *parent=nullptr);
    ~HttpClient();

	void setTileRequestCallback(void * pObj, img_request_callback pCallback);
	void setNetworkManager(QNetworkAccessManager* manager);

	QNetworkAccessManager* getNetworkManager();

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
    void requestTile(const GeoTilePos &tile);
    bool requestAsyncTile(const GeoTilePos &tile, std::vector<char>&buffer, int timeout = 3);
    bool requestTile(const GeoTilePos &tile, std::vector<char>&buffer, int timeout = 3);

	bool cancelQNetworkSyncTile(void * pObj);
	bool requestQNetworkSyncTile(void * pObj, const GeoTilePos &tile);
private:
	void removeReply(void * pObj, const GeoTilePos& tilePos);
private:
    static size_t receive_data(void *contents, size_t size, size_t nmemb, void *stream);

    static size_t write_file_data(void *ptr, size_t size, size_t nmemb, void *stream);
    CURLcode httpGet(const std::string & url,std::string & response, int timeout);    
    void httpAsyncGet(const std::string & url);
    CURLcode httpPost(const std::string & url,const std::string postdata,std::string & response, int timeout);

    void get_token();

    CURLcode httpGetFile(const std::string & url, const std::string &filname, int timeout);

signals:
	void requestQNetworkSyncTileSig(void * pObj, const GeoTilePos &tile);
private slots:
	void requestQNetworkSyncTileSlot(void * pObj, const GeoTilePos &tile);
	void onReplyFinished(QNetworkReply* reply);
private:
    CURL * m_pCurl;

	QNetworkAccessManager* networkManager;

	QMap<void *, QMap<GeoTilePos, QNetworkReply*>> m_mRequest;
	QMap<void *, img_request_callback> m_pCallbacks;

    std::string m_token;
};


//线程任务回调（考虑参数可能会过多，使用时可添加结构体声明）
typedef std::function<void (const GeoTilePos &tile, const std::vector<char>&)> deal_tile_data_func_callback;

//任务模板
class TileRequestProcessor: public QRunnable
{
public:
    TileRequestProcessor(deal_tile_data_func_callback _pCallbackfunc,const GeoTilePos &tile,  const std::string& url);
    virtual ~TileRequestProcessor();
    virtual void run() override;
private:
    deal_tile_data_func_callback m_pCallbackfunc;
    std::string m_url;
    std::vector<char> m_data;
    GeoTilePos m_tile;
};


class TileRequestTaskManager:public QObject
{
    Q_OBJECT
public:
    static TileRequestTaskManager& getInstance();
    virtual ~TileRequestTaskManager();

    //添加线程任务
    void appendProcessor(deal_tile_data_func_callback _pCallbackfunc, const GeoTilePos &tile, const std::string& url);

    //获取线程池中活跃线程数
    int getActiveThreadCount() const;

private:
    TileRequestTaskManager(QObject* parent = nullptr);

private:
    QThreadPool* m_threadPool;
};


#endif // HttpServer
