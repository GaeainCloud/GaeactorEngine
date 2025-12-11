#include "instAgentsListWidget.h"

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

InstAgentsListWidget::InstAgentsListWidget(QWidget* parent)
	:QWidget(parent),
	m_qmlWidget(nullptr),
	m_pAgentEditWidget(nullptr)
{
	m_qmlWidget = new QQuickWidget();
	m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_qmlWidget->rootContext()->setContextProperty("parentWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);
	m_qmlWidget->setSource(QUrl("qrc:/qml/instangenteditpanelwidget.qml"));

	QObject* pRoot = (QObject*)m_qmlWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
	}
	m_qmlWidget->hide();
	m_pAgentEditWidget = new AgentEditWidget(this);
	m_pAgentEditWidget->setVisible(false);


	connect(m_pAgentEditWidget, &AgentEditWidget::qml_quit_agent_edit_panel_sig, this, &InstAgentsListWidget::closeModelWidget_slot);

	this->setStyleSheet("InstAgentsListWidget{background-color:#2e2f30;}");

	m_pQStackedWidget = new QStackedWidget(this);


	m_pQStackedWidget->addWidget(m_qmlWidget);
	m_pQStackedWidget->addWidget(m_pAgentEditWidget);

	m_pLayout = new QVBoxLayout(this);
	m_pLayout->addWidget(m_pQStackedWidget);
	m_pLayout->setSpacing(0);
	m_pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_pLayout);

	m_pQStackedWidget->setCurrentIndex(0);

	request_agentdata();
}

InstAgentsListWidget::~InstAgentsListWidget()
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

void InstAgentsListWidget::editAgent(const QVariant& agentdata)
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
    QString InsagentId = contextJsonobj.value("agentId").toString();

	QString agentKey = contextJsonobj.value("agentKey").toString();

    QJsonArray jsarr;
    QJsonObject jsobj;
    std::unordered_map<QString, AgentKeyItemInfo>& agentKeys = DataManager::getInstance().agentKeyMaps();

    std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

    AgentKeyItemInfo targetAgent;
    auto agentKeys_itor = std::find_if(agentKeys.begin(),
                                       agentKeys.end(), [&](const std::unordered_map<QString, AgentKeyItemInfo>::value_type& vt) {
                                           return vt.second.agentKey == agentKey;
                                       });
    if (agentKeys_itor != agentKeys.end())
    {
        targetAgent = agentKeys_itor->second;
    }

    AgentInstanceInfo agentinstanceinfo;
    agentinstanceinfo.m_agentinfo.agentKeyItem = targetAgent;
    agentinstanceinfo.m_agentinfo.agentKeyItem.agentKey = "AGENTKEY_10503387614339227422";
    agentinstanceinfo.m_agentinfo.agentKeyItem.agentId = InsagentId;
    agentinstanceinfo.m_agentinfo.agentInstId = contextJsonobj.value("agentInstId").toString();
    agentinstanceinfo.m_agentinfo.agentKeyItem.agentName = contextJsonobj.value("agentName").toString();
    agentinstanceinfo.m_agentinfo.agentKeyItem.agentNameI18n = contextJsonobj.value("agentNameI18n").toString();
    agentinstanceinfo.m_agentinfo.agentKeyItem.modelUrlFat = "qrc:/res/img/undefined.jpg";

    QJsonObject jsitem;
    agentinstanceinfo.toJson(jsitem);

    jsarr.push_back(jsitem);

    auto agentInstances_itor = std::find_if(agentInstances.begin(),
                                            agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
                                                return vt.first == InsagentId;
                                            });
    if (agentInstances_itor == agentInstances.end())
    {
        agentInstances.insert(std::make_pair(InsagentId, std::move(agentinstanceinfo)));
    }


	//	bool bNew = false;
	//	QString _instagent_type = contextJsonobj.value("agentType").toString();
	//	auto itor = std::find_if(m_agentdata.begin(),
	//		m_agentdata.end(), [&](const std::unordered_map<QString, QJsonObject>::value_type &vt) {
	//		return vt.first == agentKey;
	//	});
	//	if (itor == m_agentdata.end())
	//	{
	//		m_agentdata.insert(std::make_pair(std::move(agentKey), contextJsonobj));
	//		bNew = true;
	//	}
	//	else
	//	{
	//		contextJsonobj = itor->second;
	//		bNew = false;
	//	}


	//	if (_agent_type == "Instagent")
	//	{
	//		m_pAgentEditWidget->setAgentType(E_AGENT_TYPE_INSTAGENT);
	//	}
	//	else
	//	{
	//		m_pAgentEditWidget->setAgentType(E_AGENT_TYPE_SCENE);
	//	}

	//	m_pQStackedWidget->setCurrentIndex(0);

	//	m_pAgentEditWidget->loadAgent(contextJsonobj, bNew);
}

QVariant InstAgentsListWidget::getAgentKey()
{
	return QVariant::fromValue(QString::number(FunctionAssistant::generate_random_positive_uint64()));
}

Q_INVOKABLE void InstAgentsListWidget::refreshAgents()
{
//	QObject* pRoot = (QObject*)m_qmlWidget->rootObject();
//	QMetaObject::invokeMethod(pRoot, "resetData");

	request_agentdata();
}

QVariant InstAgentsListWidget::getAgentKeysArray()
{
	return QVariant::fromValue(m_agentkeysarr);
}

void InstAgentsListWidget::initData()
{
//	QObject* pRoot = (QObject*)m_qmlWidget->rootObject();
//	QMetaObject::invokeMethod(pRoot, "resetData");

	request_agentdata();
}

void InstAgentsListWidget::closeModelWidget_slot()
{
	m_pQStackedWidget->setCurrentIndex(0);
}

void InstAgentsListWidget::qml_quit_agent_edit_panel_slot()
{

}



void InstAgentsListWidget::deal_instagentData_slot(const QString& airport_code, const QStringList& allowRunway)
{
	refreshAgents();
	std::unordered_map<QString, AgentKeyItemInfo>& agentKeys = DataManager::getInstance().agentKeyMaps();

    std::unordered_map<QString, std::unordered_map<QString, QString> >& _InstagentInstance = DataManager::getInstance().m_InstagentInstance;

	std::unordered_map<QString, AgentInstanceInfo>& agentInstances = DataManager::getInstance().agentInstances();

	QJsonArray jsarr;
	QJsonObject jsobj;

	auto _InstagentInstance_itor = _InstagentInstance.begin();
	while (_InstagentInstance_itor != _InstagentInstance.end())
	{
		const std::unordered_map<QString, QString>& instance = _InstagentInstance_itor->second;
		auto _instance_itor = instance.begin();
		while (_instance_itor != instance.end())
		{
			AgentKeyItemInfo targetAgent;
			QString targetName = _instance_itor->first;
//            targetName = "a350-1000";
//            targetName = "A350-1000";
			targetName = "a";
			auto agentKeys_itor = std::find_if(agentKeys.begin(),
				agentKeys.end(), [&](const std::unordered_map<QString, AgentKeyItemInfo>::value_type& vt) {
					return vt.second.agentName == targetName;
				});
			if (agentKeys_itor != agentKeys.end())
			{
				targetAgent = agentKeys_itor->second;
			}
			if (!targetAgent.modelUrlSymbols.isEmpty())
			{
				QJsonObject modelUrlSymbols;

				modelUrlSymbols.insert("modelUrlSymbols", targetAgent.modelUrlSymbols);
				DataManager::getInstance().setEntityIcon(_instance_itor->second.toULongLong(), modelUrlSymbols);
			}

			AgentInstanceInfo agentinstanceinfo;
			agentinstanceinfo.m_agentinfo.agentKeyItem = targetAgent;
            agentinstanceinfo.m_agentinfo.agentKeyItem.agentKey = "AGENTKEY_10503387614339227422";
			agentinstanceinfo.m_agentinfo.agentKeyItem.agentId = _instance_itor->second;
			agentinstanceinfo.m_agentinfo.agentInstId = _instance_itor->first;
			agentinstanceinfo.m_agentinfo.agentKeyItem.agentName = tr("Aircraft Registration:") + _instance_itor->first + "\n" + tr("Aircraft Type:") + _InstagentInstance_itor->first;
			agentinstanceinfo.m_agentinfo.agentKeyItem.agentNameI18n = tr("Aircraft Registration:") + _instance_itor->first + "\n" + tr("Aircraft Type:") + _InstagentInstance_itor->first;
			agentinstanceinfo.m_agentinfo.agentKeyItem.modelUrlFat = "qrc:/res/img/undefined.jpg";

			QJsonObject jsitem;
			agentinstanceinfo.toJson(jsitem);

			jsarr.push_back(jsitem);

			auto agentInstances_itor = std::find_if(agentInstances.begin(),
				agentInstances.end(), [&](const std::unordered_map<QString, AgentInstanceInfo>::value_type& vt) {
					return vt.first == _instance_itor->second;
				});
			if (agentInstances_itor == agentInstances.end())
			{
				agentInstances.insert(std::make_pair(_instance_itor->second, std::move(agentinstanceinfo)));
			}
			_instance_itor++;
		}
		_InstagentInstance_itor++;
	}

//    QObject* pRoot = (QObject*)m_qmlWidget->rootObject();
//    QMetaObject::invokeMethod(pRoot, "addAgentItems",
//        Q_ARG(QVariant, QVariant::fromValue(jsarr)));
    emit deal_instagentData_sig(airport_code, allowRunway);
}

void InstAgentsListWidget::showEvent(QShowEvent* event)
{
	request_agentdata();
	m_pQStackedWidget->currentWidget()->show();
	QWidget::showEvent(event);
}

void InstAgentsListWidget::hideEvent(QHideEvent* event)
{
	m_pQStackedWidget->currentWidget()->hide();
	QWidget::hideEvent(event);
}

void InstAgentsListWidget::request_agentdata()
{
	std::unordered_map<QString, AgentKeyItemInfo>& agentKeys = DataManager::getInstance().agentKeyMaps();

	agentKeys.clear();
	QJsonObject jsobj;
	m_agentkeysarr = QJsonArray();

	if (DataManager::getInstance().pHttpClient() && DataManager::getInstance().pHttpClient()->requeset_agent_data(jsobj))
	{
		auto dataarray = jsobj.value("data").toArray();
		for (auto dataarrayitem : dataarray)
		{
			auto dataarrayitemobj = dataarrayitem.toObject();
			AgentKeyItemInfo agentkeyitem;
			agentkeyitem.fromJson(dataarrayitemobj);
			agentkeyitem.modelUrlFat = "qrc:/res/img/undefined.jpg";
			QJsonObject subjs;
			agentkeyitem.toJson(subjs);
			m_agentkeysarr.push_back(subjs);

			auto agentKeys_itor = std::find_if(agentKeys.begin(),
				agentKeys.end(), [&](const std::unordered_map<QString, AgentKeyItemInfo>::value_type& vt) {
					return vt.first == agentkeyitem.agentKey;
				});
			if (agentKeys_itor == agentKeys.end())
			{
				agentKeys.insert(std::make_pair(agentkeyitem.agentKey, std::move(agentkeyitem)));
			}
		}
	}
}
