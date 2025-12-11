#include "agenteditwidget.h"

#include <QHBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QJsonObject>
#include <QJsonArray>
#include "agenteditpanel.h"
#include "LocationHelper.h"
#include <QStackedWidget>
#include "widget3d/OSGManager.h"
#include "widget3d/ModelSceneData.h"

#include "../components/global_variables.h"
AgentEditWidget::AgentEditWidget(QWidget *parent)
    :QWidget(parent),
    m_pModelWidget(nullptr),
    m_pQStackedWidget(nullptr)
{
    this->setStyleSheet("AgentEditWidget{background-color:#2e2f30;}");

    m_pModelWidget = new QtModelWidget(QtOSGWidget::E_OSG_SHOW_TYPE_MODEL, this);
    m_pModelWidget->setVisible(false);
    connect(m_pModelWidget, &QtModelWidget::qml_quit_agent_edit_panel_sig, this, &AgentEditWidget::qml_quit_agent_edit_panel_sig);

    connect(m_pModelWidget, &QtModelWidget::edit_type_sig,this,&AgentEditWidget::edit_type_slot);
	connect(m_pModelWidget, &QtModelWidget::add_type_sig, this, &AgentEditWidget::add_type_slot);
	

    m_pQStackedWidget = new QStackedWidget(this);
    initQmlWidgets();
    /////////////////////////////////////////////////////////////////////////

    m_pLayout = new QHBoxLayout(this);
    m_pLayout->addWidget(m_pModelWidget);
    m_pLayout->addWidget(m_pQStackedWidget);
    m_pLayout->setSpacing(10);
    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setStretch(0, 2);
    m_pLayout->setStretch(1, 1);
    setLayout(m_pLayout);

}

AgentEditWidget::~AgentEditWidget()
{
    if(m_pQStackedWidget)
    {
        m_pQStackedWidget->deleteLater();
    }


    if(m_pModelWidget)
    {
        m_pModelWidget->deleteLater();
    }
    if(m_pLayout)
    {
        m_pLayout->deleteLater();
    }
}

void AgentEditWidget::setAgentType(E_AGENT_TYPE agentType)
{
	if (m_pModelWidget)
	{
		m_pModelWidget->setAgentType(agentType);
	}
}

void AgentEditWidget::loadAgent(const QJsonObject &contextJsonobj,bool bNew)
{
    m_currentAgentJson = contextJsonobj;
    if(m_pModelWidget)
    {
		if (bNew)
		{
			m_pModelWidget->loadNewAgent(contextJsonobj);
		}
		else
		{
			m_pModelWidget->loadOldAgent(contextJsonobj);
		}
    }
}

void AgentEditWidget::qml_add_entity_slot()
{

}

void AgentEditWidget::edit_type_slot(TreeNode::E_NODE_TYPE type,uint64_t id, bool expand, bool bSubRootNode)
{
	if (bSubRootNode)
	{
		m_pQStackedWidget->hide();
	}
	else
	{
		m_pQStackedWidget->show();
    }
    if((TreeNode::E_NODE_TYPE_CONFIGS == type) ||
        (TreeNode::E_NODE_TYPE_SUBMODELS == type))
    {
        m_pQStackedWidget->show();
    }

    add_type_slot(type,id);
}

void AgentEditWidget::add_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id)
{
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_CONFIGS:
	{
        m_pQStackedWidget->setCurrentIndex(7);
	}
	break;
    case TreeNode::E_NODE_TYPE_SUBMODELS:
    case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
	{
		m_pQStackedWidget->setCurrentIndex(6);
	}
	break;
	case TreeNode::E_NODE_TYPE_ACTIONS:
	{
		m_pQStackedWidget->setCurrentIndex(0);
	}
	break;
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	{
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		m_pQStackedWidget->setCurrentIndex(1);
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		m_pQStackedWidget->setCurrentIndex(4);
	}
	break;
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	{
		m_pQStackedWidget->setCurrentIndex(5);
	}
	break;
	case TreeNode::E_NODE_TYPE_OODAS:
	{
		m_pQStackedWidget->setCurrentIndex(2);
	}
	break;
	case TreeNode::E_NODE_TYPE_POIS:
	{
        m_pQStackedWidget->setCurrentIndex(8);
	}
	break;
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	{
		m_pQStackedWidget->setCurrentIndex(3);
	}
    break;
    case TreeNode::E_NODE_TYPE_FENCES:
    {
        m_pQStackedWidget->setCurrentIndex(9);
    }
    break;

	default:break;
	}
}

void AgentEditWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}


void AgentEditWidget::showEvent(QShowEvent *event)
{
    m_pModelWidget->setVisible(true);
    m_pQStackedWidget->currentWidget()->setVisible(true);
    QWidget::showEvent(event);
}

void AgentEditWidget::hideEvent(QHideEvent *event)
{
    m_pModelWidget->setVisible(false);
    m_pQStackedWidget->currentWidget()->setVisible(false);
    QWidget::hideEvent(event);
}

void AgentEditWidget::initQmlWidgets()
{
    auto addQmlWidget=[&](const QUrl &url){
        /////////////////////////////////////////////////////////////////////////
        auto pqmlwidget = new QQuickWidget(m_pQStackedWidget);
        pqmlwidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

        pqmlwidget->rootContext()->setContextProperty("parentWidget",this);
        QMLGlobalVariableHelper::setWidgetGlobalVariable(pqmlwidget);
        pqmlwidget->rootContext()->setContextProperty("modelWidget",m_pModelWidget);
        pqmlwidget->setSource(url);

        pqmlwidget->setAttribute(Qt::WA_AlwaysStackOnTop);
        pqmlwidget->setAttribute(Qt::WA_TranslucentBackground);
        pqmlwidget->setClearColor(Qt::transparent);  // 设置 QML 视图的背景为透明

        QObject *pattributeWidgetRoot = (QObject*)pqmlwidget->rootObject();
        if (pattributeWidgetRoot != NULL) {
//            connect(pattributeWidgetRoot, SIGNAL(qml_add_entity_signal()), this, SLOT(qml_add_entity_slot()));
//            connect(this, SIGNAL(sendmsg_sig()), pattributeWidgetRoot, SIGNAL(cSignal()));
        }
        pqmlwidget->hide();
        m_pQStackedWidget->addWidget(pqmlwidget);
    };
    addQmlWidget(QUrl("qrc:/qml/agentpage/actionswidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/sensingwidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/oodawidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/paramswidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/sensingmediawidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/waypointswidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/agenteditmodelwidget.qml"));

    addQmlWidget(QUrl("qrc:/qml/agentpage/generalwidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/poiwidget.qml"));
    addQmlWidget(QUrl("qrc:/qml/agentpage/fencewidget.qml"));

	QVector<QWidget*> listwidgets;
    listwidgets.reserve(m_pQStackedWidget->count());
	for (int idx = 0; idx < m_pQStackedWidget->count(); idx++)
	{
		listwidgets.push_back(m_pQStackedWidget->widget(idx));
	}
    m_pModelWidget->setModelListWidget(listwidgets);
    m_pQStackedWidget->setCurrentIndex(0);
	m_pQStackedWidget->hide();
}



