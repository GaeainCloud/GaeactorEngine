#include "httpclient.hpp"

#include <iostream>
#include <QCoreApplication>
#include "../function.h"
#include "settingsconfig.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>

//#define DISABLE_SAVE

HttpClient::HttpClient(QObject *parent)
	:QObject(parent)
{
	qRegisterMetaType<GeoTilePos>("GeoTilePos");

	initEnvironment();

    get_token();
}

HttpClient::~HttpClient()
{
	destroyEnvironment();
}

void HttpClient::setTileRequestCallback(void * pObj, img_request_callback pCallback)
{
	if (m_pCallbacks.contains(pObj))
	{
		m_pCallbacks[pObj] = std::move(pCallback);
	}
	else
	{
		m_pCallbacks.insert(pObj, std::move(pCallback));
	}
	if (m_mRequest.contains(pObj))
	{
		QMap<GeoTilePos, QNetworkReply*> mRequest = m_mRequest[pObj];

		QMap<GeoTilePos, QNetworkReply*>::iterator itor = mRequest.begin();
		while (itor != mRequest.end())
		{
			QNetworkReply* reply = itor.value();
			if (reply == nullptr) {
				return;
			}
			reply->abort();
			reply->close();
			reply->deleteLater();
			itor++;
		}
		mRequest.clear();
	}
	else
	{
		m_mRequest.insert(pObj, QMap<GeoTilePos, QNetworkReply*>());
	}
}

void HttpClient::setNetworkManager(QNetworkAccessManager* manager)
{
	networkManager = manager;

	connect(this, &HttpClient::requestQNetworkSyncTileSig, this, &HttpClient::requestQNetworkSyncTileSlot);
	connect(networkManager, &QNetworkAccessManager::finished, this, &HttpClient::onReplyFinished);
}

QNetworkAccessManager* HttpClient::getNetworkManager()
{
	return networkManager;
}

void HttpClient::initEnvironment()
{
	// 初始化 cURL
	curl_global_init(CURL_GLOBAL_ALL);
	//    // 创建 cURL 句柄
    //    m_pCurl = curl_easy_init();

}

void HttpClient::destroyEnvironment()
{
	//    // 清除 cURL 句柄
	//    curl_easy_cleanup(m_pCurl);
		// 清除全局 cURL 资源
	curl_global_cleanup();
}



bool HttpClient::append_agentruntime_data(const QJsonObject &jsobj)
{
	std::string response;
    auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_addagentpatternurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_agentruntime_data(const QJsonObject &jsobj)
{
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_updateagentpatternurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_agentruntime_data(const QJsonObject &jsobj)
{
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString deleteidurl = SettingsConfig::getInstance().lavic_desktop_cfg().m_deleteagentpatternurlstr + "?id=" + jsobj.value("id").toString();
	CURLcode res = httpPost(deleteidurl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::execute_agentruntime_data(const QJsonObject &jsobj)
{
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_executeagentpatternurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::execute_agent_data(const QJsonObject &jsobj)
{
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_executeagentdataurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::execute_running_speed(const QJsonObject &tmp)
{
	QJsonObject jsobj = tmp;
	//jsobj.insert("speed", 1.0);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_executerunningspeedurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::execute_poicmd(const QJsonObject &jsobj)
{
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_executepoicmdurlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::append_agent_data(const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_agentdata_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_agent_data(const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_Agent_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_agent_data(const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString deleteidurl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_delete_agentdata_urlstr).arg(jsobj.value("agentKey").toString());
	CURLcode res = httpPost(deleteidurl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}



bool HttpClient::append_action_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_Action_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_action_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_Action_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_action_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString deleteidurl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_delete_action_urlstr).arg(agentKey).arg(jsobj.value("scriptId").toString());
	CURLcode res = httpPost(deleteidurl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}


bool HttpClient::append_ooda_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_OODAData_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_ooda_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_OODAData_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_ooda_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString deleteidurl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_delete_OODAData_urlstr).arg(agentKey).arg(jsobj.value("scriptId").toString());
	CURLcode res = httpPost(deleteidurl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}


bool HttpClient::append_variable_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_Variable_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_variable_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_Variable_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_variable_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString deleteidurl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_delete_Variable_urlstr).arg(agentKey).arg(jsobj.value("varKeyword").toString()).arg(jsobj.value("varSig").toString());
	CURLcode res = httpPost(deleteidurl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}


bool HttpClient::append_field_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_FldmdForAgent_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_field_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_FldmdforAgent_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_field_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
    QString url = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_delete_FldmdforAgent_urlstr).arg(agentKey).arg(jsobj.value("fldmdKey").toString());
	CURLcode res = httpPost(url.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::select_FldmdForAgent(const QString &agentKey)
{
	std::string response;
	std::string jsonstr;
    QString selecturl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_select_FldmdForAgent_urlstr).arg(agentKey);
	CURLcode res = httpPost(selecturl.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}



bool HttpClient::append_sensing_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_save_SmdforFldmd_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::update_sensing_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	QJsonObject sendobj = jsobj;
	sendobj.insert("agentKey", agentKey);
	std::string response;
	auto jsonstr = FunctionAssistant::json_object_to_string(sendobj).toStdString();
    CURLcode res = httpPost(SettingsConfig::getInstance().lavic_desktop_cfg().m_update_Smd_urlstr.toStdString().c_str(), jsonstr, response, 3);
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::delete_sensing_data(const QString &agentKey, const QJsonObject &jsobj)
{
#ifdef DISABLE_SAVE
    return false;
#endif

	//    std::string response;
	//    auto jsonstr = FunctionAssistant::json_object_to_string(jsobj).toStdString();
	//    QString deleteidurl = QString(m_delete_FldmdforAgent_urlstr).arg(agentKey).arg(jsobj.value("fldmdKey").toString());
	//    CURLcode res = httpPost(deleteidurl.toStdString().c_str(),jsonstr, response, 3);
	//    return (res == CURLE_OK) ? true : false;
	return false;
}

bool HttpClient::requeset_agentByid_data(const QString agentKey, QJsonObject &jsobj)
{
	std::string response;
    QString requesturl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_get_AgentByIdurlstr).arg(agentKey);
	CURLcode res = httpGet(requesturl.toStdString().c_str(), response, 3);
	if (res == CURLE_OK)
	{
		auto jstmp = QString::fromStdString(response);
		jsobj = FunctionAssistant::string_to_json_object(jstmp);
    }
    return (res == CURLE_OK) ? true : false;
}

bool HttpClient::requeset_sim_data(const std::string &url, const std::string &filname)
{
    CURLcode res = CURLE_OK;
    try {
        res = httpGetFile(url, filname, 3);
    }
    catch (...)
    {
        return false;
    }
    if (res == CURLE_OK)
    {
    }
    return (res == CURLE_OK) ? true : false;
}


bool HttpClient::request_agentid_agentdata(const UINT64 &agentid, QJsonObject &jsobj)
{
	std::string response;
    QString requesturl = QString(SettingsConfig::getInstance().lavic_desktop_cfg().m_get_AgentByAgentIdurlstr).arg(agentid);
	CURLcode res = httpGet(requesturl.toStdString().c_str(), response, 3);
	if (res == CURLE_OK)
	{
		auto jstmp = QString::fromStdString(response);
		QJsonObject jsobjtmp = FunctionAssistant::string_to_json_object(jstmp);
		if (jsobjtmp.contains("data") && jsobjtmp.value("data").isObject())
		{
			jsobj = jsobjtmp.value("data").toObject();
		}
		//qDebug() << "request" << requesturl << "response ---------------" << jsobjtmp<<"---->"<< jsobj;
	}
	return (res == CURLE_OK) ? true : false;
}

bool HttpClient::requeset_agent_data(QJsonObject& jsobj)
{
    std::string response;
    CURLcode res = httpGet(SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentdataurlstr.toStdString().c_str(), response, 3);
    if (res == CURLE_OK)
    {
        qDebug() << " requeset_agent_data " << SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentdataurlstr << " succeed ";
        auto jstmp = QString::fromStdString(response);
        jsobj = FunctionAssistant::string_to_json_object(jstmp);
    }
    else
    {
        qDebug() << " requeset_agent_data " << SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentdataurlstr << " failed load local agentdata";
        QString _local_db_dir = QCoreApplication::applicationDirPath()+"/data/instancedata";
        QDir dir_(_local_db_dir);
        if(dir_.exists(_local_db_dir))
        {
            QString db_path = QString("%1/agentdata.json").arg(_local_db_dir);
            jsobj = FunctionAssistant::read_json_file_object(db_path);
        }
    }
    return true;
}

bool HttpClient::requeset_agentruntime_data(QJsonObject &jsobj)
{
    std::string response;
    CURLcode res = httpGet(SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentpatternurlstr.toStdString().c_str(), response, 3);
    if (res == CURLE_OK)
    {
        qDebug() << " requeset_agentruntime_data " << SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentpatternurlstr << " succeed ";
        jsobj = FunctionAssistant::string_to_json_object(QString::fromStdString(response));
    }
    else
    {
        qDebug() << " requeset_agentruntime_data " << SettingsConfig::getInstance().lavic_desktop_cfg().m_getagentpatternurlstr << " failed load local agentruntime_data";
        QString _local_db_dir = QCoreApplication::applicationDirPath()+"/data/instancedata";
        QDir dir_(_local_db_dir);
        if(dir_.exists(_local_db_dir))
        {
            QString db_path = QString("%1/patterndata.json").arg(_local_db_dir);
            jsobj = FunctionAssistant::read_json_file_object(db_path);
        }
    }
    return true;
}

void HttpClient::removeReply(void * pObj, const GeoTilePos& tilePos)
{
	if (m_mRequest.contains(pObj))
	{
		QMap<GeoTilePos, QNetworkReply*> mRequest = m_mRequest[pObj];
		QNetworkReply* reply = mRequest.value(tilePos, nullptr);
		if (reply == nullptr) {
			return;
		}
		mRequest.remove(tilePos);
		reply->abort();
		reply->close();
		reply->deleteLater();
	}
}

size_t HttpClient::receive_data(void *ptr, size_t size, size_t nmemb, void *stream)
{

	std::string* str = dynamic_cast<std::string*>((std::string *)stream);
	if (NULL == str || NULL == ptr)
	{
		return -1;
	}
	char* pData = (char*)ptr;
    str->append(pData, size * nmemb);
    return size * nmemb;
}
#include <stdio.h>

size_t HttpClient::write_file_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
    return written;
}

CURLcode HttpClient::httpGet(const std::string &url, std::string &response, int timeout)
{
	// 创建 cURL 句柄
	m_pCurl = curl_easy_init();
	//设置请求的URL地址
	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

    // 设置请求头，包括 Authorization 头部
    struct curl_slist *headers = NULL;

    if(!m_token.empty())
    {
        headers = curl_slist_append(headers, m_token.c_str());
        curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);
    }


	//设置ssl验证
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, false);

	//CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
	curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 0);

	curl_easy_setopt(m_pCurl, CURLOPT_FAILONERROR, 1L);

	curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, NULL);


	// 设置接收响应数据的回调函数
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, HttpClient::receive_data);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);

	curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1);

	//设置超时时间
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, timeout); // set transport and time out time
	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, timeout);


	// 发送请求并接收响应
	CURLcode res = curl_easy_perform(m_pCurl);

	if (res != CURLE_OK) {
		std::cout << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
	}
	else {
		// 处理响应数据
		//        std::cout << "response: " << response << std::endl;
	}
	// 清除 cURL 句柄
	curl_easy_cleanup(m_pCurl);
	return res;
}


void HttpClient::requestTile(const GeoTilePos& tile)
{
    QString url = FunctionAssistant::getMapUrl(tile, SettingsConfig::getInstance().lavic_desktop_cfg().m_map_web_url);
	TileRequestTaskManager::getInstance().appendProcessor(nullptr, tile, url.toStdString());
}



bool HttpClient::cancelQNetworkSyncTile(void * pObj)
{
	if (m_mRequest.contains(pObj))
	{
		QMap<GeoTilePos, QNetworkReply*> mRequest = m_mRequest[pObj];

		QMap<GeoTilePos, QNetworkReply*>::iterator itor = mRequest.begin();
		while (itor != mRequest.end())
		{
			QNetworkReply* reply = itor.value();
			if (reply != nullptr) {
				reply->abort();
				reply->close();
				reply->deleteLater();
			}
			itor++;
		}
		mRequest.clear();
	}
	return true;
}

bool HttpClient::requestQNetworkSyncTile(void * pObj, const GeoTilePos &tile)
{
	requestQNetworkSyncTileSlot(pObj, tile);
	return true;
}


// 回调函数用于处理返回的数据
size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	// 将返回的数据保存到内存中
	std::vector<char>* mem = static_cast<std::vector<char>*>(userdata);
	size_t realsize = size * nmemb;
	mem->insert(mem->end(), ptr, ptr + realsize);
	return realsize;
}


size_t headerCallback(void* contents, size_t size, size_t nmemb, std::string* header) {
	size_t totalSize = size * nmemb;
	header->append((char*)contents, totalSize);
	return totalSize;
}

bool HttpClient::requestAsyncTile(const GeoTilePos &tile, std::vector<char> &buffer, int timeout)
{
    QString url = FunctionAssistant::getMapUrl(tile, SettingsConfig::getInstance().lavic_desktop_cfg().m_map_web_url);

	CURL* easy_handle;
	CURLM* multi_handle;

	// 创建 easy handle
	easy_handle = curl_easy_init();

	// 设置请求的 URL
	curl_easy_setopt(easy_handle, CURLOPT_URL, url.toStdString().c_str());

	// 设置User-Agent头部字段
	//std::string header = "Mozilla/5.0 (Windows; U; MSIE 6.0; Windows NT 5.1; SV1; .NET  CLR 2.0.50727)";
	curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; MSIE 6.0; Windows NT 5.1; SV1; .NET  CLR 2.0.50727)");

	//curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, headerCallback);
	//curl_easy_setopt(easy_handle, CURLOPT_HEADERDATA, &header);

	// 设置请求完成后的回调函数
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &buffer);

	//curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 1L);


	curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYPEER, false);

	curl_easy_setopt(easy_handle, CURLOPT_SSL_VERIFYHOST, false);

	//设置超时时间
	curl_easy_setopt(easy_handle, CURLOPT_CONNECTTIMEOUT, timeout); // set transport and time out time
	curl_easy_setopt(easy_handle, CURLOPT_TIMEOUT, timeout);

	// 创建 multi handle
	multi_handle = curl_multi_init();

	curl_easy_setopt(multi_handle, CURLOPT_FAILONERROR, 1L);
	// 添加 easy handle 到 multi handle
	curl_multi_add_handle(multi_handle, easy_handle);

	int running_handles;
	// 执行请求
	CURLMcode retcode;
	do {
		retcode = curl_multi_perform(multi_handle, &running_handles);
	} while (retcode == CURLM_CALL_MULTI_PERFORM);

	bool bRet = true;
	// 循环直到所有请求完成
	while (running_handles) {
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

		// 等待事件发生
		int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, NULL);

		if (rc == -1) {
			// 错误处理
			bRet = false;
			break;
		}

		// 处理 libcurl 的事件
		do {
			retcode = curl_multi_perform(multi_handle, &running_handles);
		} while (retcode == CURLM_CALL_MULTI_PERFORM);
	}

	if (retcode != CURLM_OK)
	{
		bRet = false;
	}

	//CURLcode res = curl_easy_perform(easy_handle);
 //   if(res == CURLE_HTTP_RETURNED_ERROR) {
	//	bRet = false;
 //       /* an HTTP response error problem */
 //   }

 //   //检查是否404
 //   auto httpCode = curl_easy_getinfo(easy_handle,CURLINFO_HTTP_CODE);
 //   if(httpCode == 404)
 //   {
 //       bRet = false;
 //   }

	// 清理
	curl_multi_remove_handle(multi_handle, easy_handle);
	curl_easy_cleanup(easy_handle);
	curl_multi_cleanup(multi_handle);
	return bRet;
}

bool HttpClient::requestTile(const GeoTilePos &tile, std::vector<char> &buffer, int timeout)
{
    QString url = FunctionAssistant::getMapUrl(tile, SettingsConfig::getInstance().lavic_desktop_cfg().m_map_web_url);

	// 创建 cURL 句柄
	m_pCurl = curl_easy_init();
	//设置请求的URL地址
	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.toStdString().c_str());

	//设置ssl验证
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, false);

	//CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
	curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 0);

	curl_easy_setopt(m_pCurl, CURLOPT_FAILONERROR, 1L);

	curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, NULL);


	// 设置接收响应数据的回调函数
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &buffer);

	curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1);

	//设置超时时间
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, timeout); // set transport and time out time
	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, timeout);


	// 发送请求并接收响应
	CURLcode res = curl_easy_perform(m_pCurl);
	bool bret = false;
	if (res != CURLE_OK) {
		bret = false;
	}
	else {
		// 处理响应数据
		//        std::cout << "response: " << response << std::endl;
		bret = true;
	}

	//检查是否404
//    auto httpCode = curl_easy_getinfo(m_pCurl,CURLINFO_HTTP_CODE);
//    if(httpCode == CURLE_UNKNOWN_OPTION)
//    {
//        bret = false;
//    }
	// 清除 cURL 句柄
	curl_easy_cleanup(m_pCurl);
	return bret;
}

void HttpClient::httpAsyncGet(const std::string &url)
{
	CURL* easy_handle;
	CURLM* multi_handle;


	// 创建 easy handle
	easy_handle = curl_easy_init();

	// 设置请求的 URL
	curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());

	std::vector<char> buffer;

	// 设置请求完成后的回调函数
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &buffer);

	// 创建 multi handle
	multi_handle = curl_multi_init();

	// 添加 easy handle 到 multi handle
	curl_multi_add_handle(multi_handle, easy_handle);

	int running_handles;
	// 执行请求
	while (curl_multi_perform(multi_handle, &running_handles) == CURLM_CALL_MULTI_PERFORM);

	// 循环直到所有请求完成
	while (running_handles) {
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

		// 等待事件发生
		int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, NULL);

		if (rc == -1) {
			// 错误处理
			break;
		}

		// 处理 libcurl 的事件
		while (curl_multi_perform(multi_handle, &running_handles) == CURLM_CALL_MULTI_PERFORM);
	}

	// 清理
	curl_multi_remove_handle(multi_handle, easy_handle);
	curl_easy_cleanup(easy_handle);
	curl_multi_cleanup(multi_handle);

}

CURLcode HttpClient::httpPost(const std::string &url, const std::string postdata, std::string &response, int timeout)
{
	// 创建 cURL 句柄
	m_pCurl = curl_easy_init();
	//设置请求的URL地址
	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

	// 设置请求头
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json;charset=UTF-8");
	curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);

	// 设置 POST 请求体
	curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, postdata.c_str());

	//CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
	curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 0);

	curl_easy_setopt(m_pCurl, CURLOPT_FAILONERROR, 1L);


	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, HttpClient::receive_data);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &response);

	//设置超时时间
	curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, timeout); // set transport and time out time
	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, timeout);


	// 发送请求并接收响应
	CURLcode res = curl_easy_perform(m_pCurl);

	if (res != CURLE_OK) {
		std::cout << "Failed to send OTAPP request: " << curl_easy_strerror(res) << std::endl;
	}
	else {
		// 处理响应数据
		//std::cout << "OTAPP response: " << response << std::endl;
	}

	// 释放请求头资源
	curl_slist_free_all(headers);

	// 清除 cURL 句柄
    curl_easy_cleanup(m_pCurl);
    return res;
}

void HttpClient::get_token()
{
//    const char* token_val="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhdWQiOiIyMjUzODY4Njc1MTg0MDY2NTYiLCJwYXNzd29yZCI6ImNhODNhOTZjNjI0NGQxNTdjNTQ2M2U5MTg0OTY2ZTM5IiwiZXhwIjoxNzIzMjMwNDc2LCJpYXQiOjE3MjMxODcyNzYsImFjY291bnQiOiJzdXBlckFkbWluOTM5In0.AxsZuIMowDIvPJKGRZiVWIinLv782ruB-48Rnf0IPAE";
//    m_token= "Authorization: Bearer " + std::string(token_val);
    m_token="";
    if(!SettingsConfig::getInstance().lavic_desktop_cfg().m_tokenstr.isEmpty())
    {
//        m_token= "Authorization: Bearer " + SettingsConfig::getInstance().lavic_desktop_cfg().m_tokenstr.toStdString();
        m_token= SettingsConfig::getInstance().lavic_desktop_cfg().m_tokenstr.toStdString();
    }
}

CURLcode HttpClient::httpGetFile(const std::string &url, const std::string &filname, int timeout)
{
    std::cout<<"get file "<<url<<" filepath "<<filname<<"\n";
#if 1
	CURLcode res = CURLE_OK;
	//// 创建 cURL 句柄
	//m_pCurl = curl_easy_init();

 //   /* set URL to get */
 //   curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

 //   /* no progress meter please */
 //   curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 0L);

 //   /* send all data to this function  */
 //   curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, HttpClient::write_file_data);

 //   /* open the files */
	//FILE* bodyfile = NULL;
	//errno_t rt = fopen_s(&bodyfile, filname.c_str(), "wb");
	//if (rt != 0 || bodyfile == NULL)
	//{
	//	curl_easy_cleanup(m_pCurl);
	//	return res;
	//}

 //   /* we want the body be written to this file handle instead of stdout */
 //   curl_easy_setopt(m_pCurl,   CURLOPT_WRITEDATA, bodyfile);


	//// 发送请求并接收响应
	//res = curl_easy_perform(m_pCurl);

	//if (res != CURLE_OK) {
	//	std::cout << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
	//}
	//else {
	//	// 处理响应数据
	//	//        std::cout << "response: " << response << std::endl;
	//}
 //   /* close the body file */
 //   fclose(bodyfile);

 //   return res;


	//初始化curl，这个是必须的  
	m_pCurl = curl_easy_init();
	curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

//    if(!m_token.empty())
//    {
//        headers = curl_slist_append(headers, m_token.c_str());
//        curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, headers);
//    }

	//设置接收数据的回调  
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, HttpClient::write_file_data);
	//curl_easy_setopt(curl, CURLOPT_INFILESIZE, lFileSize);  
	//curl_easy_setopt(curl, CURLOPT_HEADER, 1);  
	//curl_easy_setopt(curl, CURLOPT_NOBODY, 1);  
	//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);  
	// 设置重定向的最大次数  
	//curl_easy_setopt(m_pCurl, CURLOPT_MAXREDIRS, 5);
	//// 设置301、302跳转跟随location  
	//curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1);
	//curl_easy_setopt(m_pCurl, CURLOPT_NOPROGRESS, 0);
	//设置进度回调函数  
	//curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	//curl_easy_getinfo(curl,  CURLINFO_CONTENT_LENGTH_DOWNLOAD, &lFileSize);  
	//curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, g_hDlgWnd);  
	// 
	//     /* open the files */
	FILE* bodyfile = NULL;
#ifdef _MSC_VER
	errno_t rt = fopen_s(&bodyfile, filname.c_str(), "wb");
	if (rt != 0 || bodyfile == NULL)
	{
		curl_easy_cleanup(m_pCurl);
		return res;
	}
#else
    bodyfile = fopen(filname.c_str(), "wb");
    if (bodyfile == NULL)
    {
        curl_easy_cleanup(m_pCurl);
        return res;
    }
#endif

	/* we want the body be written to this file handle instead of stdout */
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, bodyfile);

	//开始执行请求  
	res = curl_easy_perform(m_pCurl);
	//查看是否有出错信息  
	const char* pError = curl_easy_strerror(res);




//	// 发送请求并接收响应
//	res = curl_easy_perform(m_pCurl);
//
	if (res != CURLE_OK) {
		std::cout << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
	}
	else {
		// 处理响应数据
		//        std::cout << "response: " << response << std::endl;
}
	//清理curl，和前面的初始化匹配  
	curl_easy_cleanup(m_pCurl);
    /* close the body file */
    fclose(bodyfile);
    return res;
#else
    CURLcode res = CURLE_OK;
    // 创建 cURL 句柄
    m_pCurl = curl_easy_init();
    //设置请求的URL地址
    curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

    //设置ssl验证
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYPEER, false);
    curl_easy_setopt(m_pCurl, CURLOPT_SSL_VERIFYHOST, false);

    //CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
    curl_easy_setopt(m_pCurl, CURLOPT_VERBOSE, 0);

    curl_easy_setopt(m_pCurl, CURLOPT_FAILONERROR, 1L);

    curl_easy_setopt(m_pCurl, CURLOPT_READFUNCTION, NULL);

    /* open the files */
    FILE* bodyfile = NULL;
    errno_t rt = fopen_s(&bodyfile,filname.c_str(), "wb");
    if (rt != 0 || bodyfile == NULL)
    {
        curl_easy_cleanup(m_pCurl);
        return res;
    }
    // 设置接收响应数据的回调函数
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, HttpClient::write_file_data);
    curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, bodyfile);

    curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1);

    //设置超时时间
    curl_easy_setopt(m_pCurl, CURLOPT_CONNECTTIMEOUT, timeout); // set transport and time out time
    curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT, timeout);


    // 发送请求并接收响应
    res = curl_easy_perform(m_pCurl);

    if (res != CURLE_OK) {
        std::cout << "Failed to send request: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        // 处理响应数据
        //        std::cout << "response: " << response << std::endl;
    }
    // 清除 cURL 句柄
    curl_easy_cleanup(m_pCurl);
	/* close the body file */
	fclose(bodyfile);
    return res;
#endif
}


void HttpClient::requestQNetworkSyncTileSlot(void * pObj, const GeoTilePos &tile)
{
    QString urlstr = FunctionAssistant::getMapUrl(tile, SettingsConfig::getInstance().lavic_desktop_cfg().m_map_web_url);
	const QUrl url(urlstr);
	QNetworkRequest request(url);
	request.setRawHeader("User-Agent",
		"Mozilla/5.0 (Windows; U; MSIE "
		"6.0; Windows NT 5.1; SV1; .NET "
		"CLR 2.0.50727)");
	request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
	request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
	QNetworkReply* reply = getNetworkManager()->get(request);
	reply->setProperty("TILE_OWNER", QVariant::fromValue(this));
	reply->setProperty("TILE_REQUEST", true);
	reply->setProperty("TILE_POS", QVariant::fromValue(tile));
	reply->setProperty("TILE_MAPRENDER_OWNER", QVariant::fromValue(pObj));
	reply->setProperty("TILE_URL", QVariant::fromValue(urlstr));
	if (m_mRequest.contains(pObj))
	{
		QMap<GeoTilePos, QNetworkReply*> mRequest = m_mRequest[pObj];
		mRequest[tile] = reply;
	}
	//std::cout << "++++++++++++++++++++++++++request net data ..... " << urlstr.toStdString()<< std::endl;
}

void HttpClient::onReplyFinished(QNetworkReply* reply)
{
	const auto tileRequest = reply->property("TILE_REQUEST").toBool();
	if (!tileRequest) {
		return;
	}
	const auto tileOwner = reply->property("TILE_OWNER").value<HttpClient*>();
	if (tileOwner != this) {
		return;
	}
	const auto tileMaprenderOwner = reply->property("TILE_MAPRENDER_OWNER").value<void*>();
	if (tileMaprenderOwner == nullptr) {
		return;
	}
	if (m_pCallbacks.contains(tileMaprenderOwner) && m_pCallbacks[tileMaprenderOwner])
	{
		const auto tilePos = reply->property("TILE_POS").value<GeoTilePos>();

		if (reply->error() != QNetworkReply::NoError) {
			if (reply->error() != QNetworkReply::OperationCanceledError) {
				qDebug() << "ERROR" << reply->errorString();
			}
			 removeReply(tileMaprenderOwner,tilePos);
			return;
		}
		const auto rawImage = reply->readAll();

		m_pCallbacks[tileMaprenderOwner](tilePos, rawImage);
		removeReply(tileMaprenderOwner, tilePos);
		//const auto urlstr = reply->property("TILE_URL").value<QString>();
		//std::cout << "------------------------response net data ..... " << urlstr.toStdString() << std::endl;
	}
}

TileRequestTaskManager& TileRequestTaskManager::getInstance()
{
	static TileRequestTaskManager threadPoolTaskManager;
	return threadPoolTaskManager;
}

TileRequestTaskManager::TileRequestTaskManager(QObject* parent)
	:QObject(parent)
{
	m_threadPool = new QThreadPool(this);
#if 0
	m_threadPool->setMaxThreadCount((std::thread::hardware_concurrency() > 1) ? std::thread::hardware_concurrency() : 1);
#else
	//线程池中最大工作线程数量设置为4
	m_threadPool->setMaxThreadCount(4);
#endif
}

TileRequestTaskManager::~TileRequestTaskManager()
{
	if (m_threadPool)
	{
		m_threadPool->clear();
		m_threadPool->waitForDone();
		delete m_threadPool;
		m_threadPool = nullptr;
	}
}

void TileRequestTaskManager::appendProcessor(deal_tile_data_func_callback _pCallbackfunc, const GeoTilePos &tile, const std::string &url)
{
	TileRequestProcessor *pThreadTaskProcessor = new TileRequestProcessor(_pCallbackfunc, tile, url);
	pThreadTaskProcessor->setAutoDelete(true);
    m_threadPool->start(pThreadTaskProcessor, QThread::HighestPriority);
}


int TileRequestTaskManager::getActiveThreadCount() const
{
	return m_threadPool->activeThreadCount();
}

TileRequestProcessor::TileRequestProcessor(deal_tile_data_func_callback _pCallbackfunc, const GeoTilePos &tile, const std::string &url)
	:m_pCallbackfunc(std::move(_pCallbackfunc))
	, m_url(url)
	, m_tile(tile)
{
}

TileRequestProcessor::~TileRequestProcessor()
{

}


void TileRequestProcessor::run()
{
	CURL* easy_handle;
	CURLM* multi_handle;

	// 创建 easy handle
	easy_handle = curl_easy_init();

	// 设置请求的 URL
	curl_easy_setopt(easy_handle, CURLOPT_URL, m_url.c_str());


	curl_easy_setopt(easy_handle, CURLOPT_FAILONERROR, 1L);
	// 设置请求完成后的回调函数
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &m_data);

	// 创建 multi handle
	multi_handle = curl_multi_init();

	// 添加 easy handle 到 multi handle
	curl_multi_add_handle(multi_handle, easy_handle);

	int running_handles;
	// 执行请求
	while (curl_multi_perform(multi_handle, &running_handles) == CURLM_CALL_MULTI_PERFORM);

	// 循环直到所有请求完成
	while (running_handles) {
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

		// 等待事件发生
		int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, NULL);

		if (rc == -1) {
			// 错误处理
			break;
		}

		// 处理 libcurl 的事件
		while (curl_multi_perform(multi_handle, &running_handles) == CURLM_CALL_MULTI_PERFORM);
	}

	// 清理
	curl_multi_remove_handle(multi_handle, easy_handle);
	curl_easy_cleanup(easy_handle);
	curl_multi_cleanup(multi_handle);

	if (m_pCallbackfunc)
	{
		m_pCallbackfunc(m_tile, m_data);
	}


}
