#ifndef AGENTSLISTWIDGET_H
#define AGENTSLISTWIDGET_H

#include <QWidget>
#include <QJsonObject>
class QQuickWidget;
class QVBoxLayout;
class RuntimeEditWidget;
class HttpClient;
class AgentEditWidget;
class QStackedWidget;
class AgentsListWidget : public QWidget
{
    Q_OBJECT

public:
    AgentsListWidget(QWidget *parent = nullptr);
    ~AgentsListWidget() override;
    Q_INVOKABLE void editAgent(const QVariant &agentdata);
    Q_INVOKABLE QVariant getAgentKey();
	Q_INVOKABLE void refreshAgents();
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

	std::unordered_map<QString, QJsonObject> m_agentdata;
};

#endif // MAINWIDGET_H
