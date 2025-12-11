#ifndef INSTAGENTSLISTWIDGET_H
#define INSTAGENTSLISTWIDGET_H

#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
class QQuickWidget;
class QVBoxLayout;
class RuntimeEditWidget;
class HttpClient;
class AgentEditWidget;
class QStackedWidget;
class InstAgentsListWidget : public QWidget
{
    Q_OBJECT

public:
    InstAgentsListWidget(QWidget *parent = nullptr);
    ~InstAgentsListWidget() override;
    Q_INVOKABLE void editAgent(const QVariant &agentdata);
    Q_INVOKABLE QVariant getAgentKey();
    Q_INVOKABLE void refreshAgents();
    Q_INVOKABLE QVariant getAgentKeysArray();
    void initData();
signals:
    void deal_instagentData_sig(const QString& airport_code, const QStringList& allowRunway);
public slots:
    void deal_instagentData_slot(const QString& airport_code, const QStringList& allowRunway);
private slots:
    void closeModelWidget_slot();
    void qml_quit_agent_edit_panel_slot();

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private:
	void request_agentdata();
private:
    QQuickWidget *m_qmlWidget;
    QVBoxLayout *m_pLayout;    
    QStackedWidget *m_pQStackedWidget;
    AgentEditWidget* m_pAgentEditWidget;

    QJsonArray m_agentkeysarr;

};

#endif // MAINWIDGET_H
