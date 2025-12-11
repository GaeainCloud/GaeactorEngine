#ifndef RUNTIMELISTWIDGET_H
#define RUNTIMELISTWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include "./src/../head_define.h"
#include "../datamanager/datamanager.hpp"

#include "../components/eventdriver/eventdriver.h"
class QQuickWidget;
class QVBoxLayout;
class QStackedWidget;
class RuntimeEditWidget;
class HttpClient;
class EventDriver;

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

class RuntimeListWidget : public QWidget
{
    Q_OBJECT

public:
    RuntimeListWidget(QWidget *parent = nullptr);
    ~RuntimeListWidget() override;
    Q_INVOKABLE void runRuntimeStyle(const QVariant &runtiemstyleid);
    Q_INVOKABLE void editRuntimeStyle(const QVariant &runtiemstyleid);
    Q_INVOKABLE void deleteRuntimeStyle(const QVariant &runtiemstyleid);
	void trigger_flight_to_run_slot(uint64_t triggertimestamp, const QList<tagTriggerFlightInfo>& triggerflights);
	void setEventDriver(EventDriver *_peventDriver);

	void eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype);
public slots:
    void qml_send_agent_data_slot();
signals:
	void updatepoicolor_sig(const QString& poiname, const QColor &cl, float textsize);
public slots:
    void deal_instagentData_slot(const QString& airport_code, const QStringList& allowRunway);
private slots:
    void closeModelWidget_slot();
protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private slots:
    void qml_add_runtime_slot();
    void add_runtime_style_slot(const QJsonObject & runtimestyle);

private:
    void request_agentdata();
    void request_agentruntimedata();

    void append_agentruntime_data(const QJsonObject &jsobj);
    void update_agentruntime_data(const QJsonObject &jsobj);
    void delete_agentruntime_data(const QJsonObject &jsobj);
    void run_agentruntime_data(const QJsonObject &jsobj);
    void run_agent_data(const QJsonObject &jsobj);

    quint64 getPath(FlightPlanConf *pflighltData, quint64 flightid, QJsonArray &wps, const QString &runtimeid);
private:
    QQuickWidget *m_qmlWidget;
    QVBoxLayout *m_pLayout;
    QStackedWidget *m_pQStackedWidget;
    RuntimeEditWidget* m_pRuntimeEditWidget;

    std::unordered_map<QString,std::tuple<QJsonObject,bool>> m_runtimedatas;

    std::unordered_map<uint64_t, std::unordered_map<QString, tagTriggerFlightInfo>> m_runfilghtId;
	QString m_senceruntimeid;
	
	EventDriver *m_peventDriver;


	 QString m_airport_code;
	QStringList m_allowRunway;

};

#endif // MAINWIDGET_H
