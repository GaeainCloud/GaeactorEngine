#include "agentslistwidget.h"

#include <QVBoxLayout>
#include <QQuickWidget>
#include <QJsonArray>
#include <QDateTime>
#include <QQmlContext>
#include <QStackedWidget>
#include "agenteditwidget.h"
#include "components/function.h"
#include "../components/global_variables.h"
#include "../datamanager/datamanager.hpp"

AgentsListWidget::AgentsListWidget(QWidget *parent)
	:QWidget(parent),
	m_qmlWidget(nullptr),
    m_pAgentEditWidget(nullptr)
{
	m_qmlWidget = new QQuickWidget();
	m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_qmlWidget->rootContext()->setContextProperty("parentWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);
	m_qmlWidget->setSource(QUrl("qrc:/qml/agentslistwidget.qml"));

	QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
	}
	m_qmlWidget->hide();
	m_pAgentEditWidget = new AgentEditWidget(this);
	m_pAgentEditWidget->setVisible(false);


	connect(m_pAgentEditWidget, &AgentEditWidget::qml_quit_agent_edit_panel_sig, this, &AgentsListWidget::closeModelWidget_slot);

	this->setStyleSheet("AgentsListWidget{background-color:#2e2f30;}");

	m_pQStackedWidget = new QStackedWidget(this);


	m_pQStackedWidget->addWidget(m_qmlWidget);
	m_pQStackedWidget->addWidget(m_pAgentEditWidget);

	m_pLayout = new QVBoxLayout(this);
	m_pLayout->addWidget(m_pQStackedWidget);
	m_pLayout->setSpacing(0);
	m_pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_pLayout);

	m_pQStackedWidget->setCurrentIndex(0);

}

AgentsListWidget::~AgentsListWidget()
{
	if (m_qmlWidget)
	{
		m_qmlWidget->deleteLater();
	}
	if (m_pLayout)
	{
		m_pLayout->deleteLater();
	}
	if (m_pAgentEditWidget)
	{
		m_pAgentEditWidget->deleteLater();
	}
}

void AgentsListWidget::editAgent(const QVariant &agentdata)
{
	QMap<QString, QVariant>  data = agentdata.toMap();
	QJsonObject contextJsonobj;
	for (auto key : data.keys())
	{
		QVariant dt = data.value(key);
		switch (dt.type())
		{
		case QVariant::String:contextJsonobj.insert(key, dt.toString()); break;
		case QVariant::Bool:contextJsonobj.insert(key, dt.toBool()); break;
		case QVariant::Int:contextJsonobj.insert(key, dt.toInt()); break;
		case QVariant::LongLong:contextJsonobj.insert(key, dt.toLongLong()); break;
		case QVariant::Double:contextJsonobj.insert(key, dt.toDouble()); break;
		case QVariant::List:
		{
			QJsonArray jsarry;

			QList<QVariant>  sublst = dt.toList();
			for (auto subitem : sublst)
			{
				switch (subitem.type())
				{
				case QVariant::String:
				{
					if (!subitem.toString().isEmpty())
					{
						jsarry.push_back(subitem.toString());
					}
				}break;
				case QVariant::Bool:jsarry.push_back(subitem.toBool()); break;
				case QVariant::Int:jsarry.push_back(subitem.toInt()); break;
				case QVariant::LongLong:jsarry.push_back(subitem.toLongLong()); break;
				case QVariant::Double:jsarry.push_back(subitem.toDouble()); break;
				default:break;
				}
			}
			contextJsonobj.insert(key, jsarry);
		}; break;
		default:break;
		}
	}
	
	QString agentKey = contextJsonobj.value("agentKey").toString();
	bool bNew = false;
	QString _agent_type = contextJsonobj.value("agentType").toString();
	auto itor = std::find_if(m_agentdata.begin(),
		m_agentdata.end(), [&](const std::unordered_map<QString, QJsonObject>::value_type &vt) {
		return vt.first == agentKey;
	});
	if (itor == m_agentdata.end())
	{
		m_agentdata.insert(std::make_pair(std::move(agentKey), contextJsonobj));
		bNew = true;
	}
	else
	{
		contextJsonobj = itor->second;
		bNew = false;
	}
	_agent_type = contextJsonobj.value("agentType").toString();


	if (_agent_type == "Instagent")
	{
		m_pAgentEditWidget->setAgentType(E_AGENT_TYPE_INSTAGENT);
	}
	else
	{
		m_pAgentEditWidget->setAgentType(E_AGENT_TYPE_SCENE);
	}

	m_pQStackedWidget->setCurrentIndex(1);

	m_pAgentEditWidget->loadAgent(contextJsonobj, bNew);
}

QVariant AgentsListWidget::getAgentKey()
{
    return QVariant::fromValue("AGENTKEY_"+QString::number(FunctionAssistant::generate_random_positive_uint64()));
}

Q_INVOKABLE void AgentsListWidget::refreshAgents()
{
//	QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//	QMetaObject::invokeMethod(pRoot, "resetData");

	request_agentdata();
}

void AgentsListWidget::closeModelWidget_slot()
{
	m_pQStackedWidget->setCurrentIndex(0);
}

void AgentsListWidget::qml_quit_agent_edit_panel_slot()
{

}



void AgentsListWidget::showEvent(QShowEvent *event)
{
	m_pQStackedWidget->currentWidget()->show();
	QWidget::showEvent(event);
}

void AgentsListWidget::hideEvent(QHideEvent *event)
{
	m_pQStackedWidget->currentWidget()->hide();
	QWidget::hideEvent(event);
}

void AgentsListWidget::request_agentdata()
{
	QJsonArray jsarr;
    QJsonObject jsobj;
    if (DataManager::getInstance().pHttpClient() && DataManager::getInstance().pHttpClient()->requeset_agent_data(jsobj))
	{
		m_agentdata.clear();
		auto dataarray = jsobj.value("data").toArray();
		for (auto dataarrayitem : dataarray)
		{
			auto dataarrayitemobj = dataarrayitem.toObject();
			auto agentKey = dataarrayitemobj.value("agentKey").toString();
			m_agentdata.insert(std::make_pair(std::move(agentKey), dataarrayitemobj));
			QJsonObject jsitem;
			jsitem.insert("agentKey", dataarrayitemobj.value("agentKey"));
			jsitem.insert("agentKeyword", dataarrayitemobj.value("agentKeyword"));
			jsitem.insert("agentName", dataarrayitemobj.value("agentName"));
			jsitem.insert("agentNameI18n", dataarrayitemobj.value("agentNameI18n"));
			jsitem.insert("agentType", dataarrayitemobj.value("agentType"));

			jsitem.insert("agentPath", "");
			jsitem.insert("agentDesc", "");
			if (dataarrayitemobj.contains("image_src"))
			{
				jsitem.insert("image_src", dataarrayitemobj.value("image_src"));
			}
			else
			{
				jsitem.insert("image_src", "qrc:/res/img/undefined.jpg");
			}
			jsarr.push_back(jsitem);
		}
//		QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//		QMetaObject::invokeMethod(pRoot, "addAgentItems",
//			Q_ARG(QVariant, QVariant::fromValue(jsarr)));
	}
}
