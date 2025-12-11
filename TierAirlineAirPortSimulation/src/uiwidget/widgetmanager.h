#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H
#include "../components/eventdriver/eventdriver.h"
#include <QJsonObject>

#include "../httpserver/httpserver/dto/AgentDto.hpp"

namespace stdutils
{
	class OriThread;
};
class HttpServer;


struct tagTriggerFlightInfo {
    QString m_FilghtNumber;
    E_FLIGHT_DEP_ARR_TYPE m_flight_dep_arr_type;
    QString m_Runway;
    FlightPlanConf *pflighltData;
    bool operator==(const tagTriggerFlightInfo &other)
    {
        return ((m_FilghtNumber == other.m_FilghtNumber) &&
                (m_flight_dep_arr_type == other.m_flight_dep_arr_type) &&
                (m_Runway == other.m_Runway));
    }
};


namespace originaldatastoragestd {
class OriginalDataInputManager;
}



class WidgetManager : public QObject
{
    Q_OBJECT

public:
    WidgetManager(QObject *parent = nullptr);
    ~WidgetManager() override;

	void start_HttpServer();
	void stop_HttpServer();
	void run();
    void stop();

	void eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype);

    void environment_init_succeed();
private:

    void importexcel(const QString &fileName);
    void dealTotalFlightData_ex(std::map<std::tuple<uint64_t, E_FLIGHT_DEP_ARR_TYPE, QString, QString>, FlightPlanConf *>& total_flightdata);
    void setAirportInfos(const QString& airport_code, const QStringList& allowRunway);

    void refreshAgents();
    void deal_instagentData_slot(const QString& airport_code, const QStringList& allowRunway);
    void deal_instagentData_slot_runtime(const QString& airport_code, const QStringList& allowRunway);
signals:
    void deal_sim_ctrl_sig(const QJsonObject &obj);
    void deal_sim_review_ctrl_sig(const QJsonObject &obj);
    void deal_sim_data_sig(const QJsonObject &obj);
    void deal_sim_review_sig(const QJsonObject &obj);
    void deal_record_ctrl_sig(const QJsonObject &obj);

    void environment_init_succeed_sig();
    void trigger_event_end_sig();
    void sendDataSignal(qint64 iGlobeFileReadBeginValidDataPos, qint64 iDataSendTimeStamp);
private slots:
    void deal_sim_ctrl_slot(const QJsonObject &obj);
    void deal_sim_review_ctrl_slot(const QJsonObject &obj);
    void deal_sim_data_slot(const QJsonObject &obj);
    void deal_sim_review_slot(const QJsonObject &obj);
    void deal_record_ctrl_slot(const QJsonObject &obj);
    void environment_init_succeed_slot();

    void trigger_runtime_event_end_slot();
    void trigger_review_event_end_slot();
    void trigger_event_to_deal_slot(uint64_t triggertimestamp, uint64_t trigger_event_id);
	void thread_httpserver_callback_Loop(void* param);
    bool httpdatareceive_callback(E_DATA_TYPE eDataType, const QJsonObject & obj);

    void sendDataSlot(qint64 iGlobeFileReadValidDataPos, qint64 iDataSendTimeStamp);
private:
    void deal_runtime(const QString& ctrltype,const QString& ctrlparam);
    void deal_review(const QString& ctrltype,const QString& ctrlparam);
    void run_agent_data(const QJsonObject &jsobj);
    void request_agentruntimedata();
    void qml_send_agent_data_slot();
    void runRuntimeStyle(const QVariant &runtiemstyleid);
    void run_agentruntime_data(const QJsonObject &jsobj);
    void append_agentruntime_data(const QJsonObject &jsobj);
    void add_runtime_style_slot(const QJsonObject &runtimestyle);
    void update_agentruntime_data(const QJsonObject &jsobj);
    void trigger_flight_to_run_slot(uint64_t triggertimestamp, const QList<tagTriggerFlightInfo>& triggerflights);
    quint64 getPath(FlightPlanConf *pflighltData, quint64 flightid, QJsonArray &wps, const QString &runtimeid);

    void deal_eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype);
    bool data_callback(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp);



    quint64 jumpToDataMillisecondPosOffsetPercent(double percent);
    void updateRecordstatus(bool bRedord);
    bool initializeReadFileSlot(tagReplayItemInfo* _currentitem);
private:
    uint32_t m_icurrentShowItem;

	EventDriver *m_peventDriver;

	std::map<int, int> m_btn_widget_id;

private:
	HttpServer* m_phttpserver;
	stdutils::OriThread *m_pHttpServerRunningThread;

    QString m_airport_code;
    QStringList m_allowRunway;


    bool m_runtime_playstatus;
    bool m_runtime_pausestatus;

    bool m_review_playstatus;
    bool m_review_pausestatus;
    double m_pausespeed;

    QString m_senceruntimeid;
    std::unordered_map<QString,std::tuple<QJsonObject,bool>> m_runtimedatas;

    std::unordered_map<uint64_t, std::unordered_map<QString, tagTriggerFlightInfo>> m_runfilghtId;

    originaldatastoragestd::OriginalDataInputManager *m_pOriginalDataInputManager;

    qint64 iPrePackDataSendTime;

    qint64 m_iDataFrames;

};

#endif // MAINWIDGET_H
