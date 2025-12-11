#ifndef WIDGETMANAGER_H
#define WIDGETMANAGER_H
#include "../components/eventdriver/eventdriver.h"
#include <QWidget>
#include <QJsonObject>

#include "../httpserver/httpserver/dto/AgentDto.hpp"

namespace stdutils
{
	class OriThread;
};
class HttpServer;

class QQuickWidget;
class QHBoxLayout;
class QStackedWidget;
class WidgetManager : public QWidget
{
    Q_OBJECT

public:
    WidgetManager(QWidget *parent = nullptr);
    ~WidgetManager() override;

	void start_HttpServer();
	void stop_HttpServer();
	void run();
	void stop();

    Q_INVOKABLE void updateWidget(const QVariant &runtiemstyleid);
	void eventdriver_callback(const UINT64 &event_id, const E_EVENT_TYPE_ID& eventtype);

    void environment_init_succeed();
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private:
    void initWidgets();
signals:
    void deal_sim_ctrl_sig(const QJsonObject &obj);
    void deal_sim_review_ctrl_sig(const QJsonObject &obj);
    void deal_sim_data_sig(const QJsonObject &obj);
    void deal_sim_review_sig(const QJsonObject &obj);
    void deal_record_ctrl_sig(const QJsonObject &obj);

    void environment_init_succeed_sig();
    void trigger_event_end_sig();
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
private:
    void deal_runtime(const QString& ctrltype,const QString& ctrlparam);
    void deal_review(const QString& ctrltype,const QString& ctrlparam);
private:
    QQuickWidget *m_qmlWidget;
    QHBoxLayout *m_pLayout;
    uint32_t m_icurrentShowItem;
    QStackedWidget *m_pQStackedWidget;

	EventDriver *m_peventDriver;

	std::map<int, int> m_btn_widget_id;

private:
	HttpServer* m_phttpserver;
	stdutils::OriThread *m_pHttpServerRunningThread;

};

#endif // MAINWIDGET_H
