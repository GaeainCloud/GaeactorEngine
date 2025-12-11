#ifndef AGENTEDITWIDGET_H
#define AGENTEDITWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QJsonArray>
#include "agenteditpanel.h"
#include "qjsonobject.h"
class QQuickWidget;
class QHBoxLayout;
class QtModelWidget;
class QStackedWidget;
class AgentEditWidget : public QWidget
{
    Q_OBJECT

public:
    AgentEditWidget(QWidget *parent = nullptr);
    ~AgentEditWidget() override;
	void setAgentType(E_AGENT_TYPE agentType);
    void loadAgent(const QJsonObject& contextJsonobj, bool bNew);
signals:
    void qml_quit_agent_edit_panel_sig();
    void sendmsg_sig();
private slots:
    void qml_add_entity_slot();
    void edit_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id, bool expand, bool bSubRootNode);
	void add_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private:
    void initQmlWidgets();
private:
    QtModelWidget* m_pModelWidget;
    QHBoxLayout *m_pLayout;
    QJsonObject m_currentAgentJson;
    QStackedWidget *m_pQStackedWidget;
};

#endif // AGENTEDITWIDGET_H
