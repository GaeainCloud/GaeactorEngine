#ifndef SOSTEPLISTWIDGET_H
#define SOSTEPLISTWIDGET_H

#include <QWidget>
#include <QJsonObject>
class QQuickWidget;
class QVBoxLayout;
class RuntimeEditWidget;
class HttpClient;
class GanttWidget;
class QStackedWidget;
class SoStepListWidget : public QWidget
{
    Q_OBJECT

public:
    SoStepListWidget(QWidget *parent = nullptr);
    ~SoStepListWidget() override;
    Q_INVOKABLE void editAgent(const QVariant &agentdata);
    Q_INVOKABLE QVariant getAgentKey();
	Q_INVOKABLE void refreshAgents();
    void initData();

    void importexcel(const QString &fileName);
    void setAirportInfos(const QString& airport_code, const QStringList& allowRunway);
signals:
    void deal_instagentData_sig(const QString& airport_code, const QStringList& allowRunway);
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
    GanttWidget* m_pGanttWidget;

	std::unordered_map<QString, QJsonObject> m_agentdata;
};

#endif // MAINWIDGET_H
