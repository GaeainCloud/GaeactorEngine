#include "sostepslistwidget.h"

#include <QVBoxLayout>
#include <QQuickWidget>
#include <QJsonArray>
#include <QDateTime>
#include <QQmlContext>
#include <QStackedWidget>
#include "../qgantt/ganttwidget.h"
#include "components/function.h"
#include "../components/global_variables.h"
#include "../datamanager/datamanager.hpp"
#include "runningmodeconfig.h"

SoStepListWidget::SoStepListWidget(QWidget *parent)
	:QWidget(parent),
	m_qmlWidget(nullptr),
    m_pGanttWidget(nullptr)
{
	m_qmlWidget = new QQuickWidget();
	m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_qmlWidget->rootContext()->setContextProperty("parentWidget", this);
	QMLGlobalVariableHelper::setWidgetGlobalVariable(m_qmlWidget);
    m_qmlWidget->setSource(QUrl("qrc:/qml/sosteppanelwidget.qml"));

	QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
	//QObject *pRoot = (QObject*)pWidget->rootObject();
	if (pRoot != NULL) {
	}
	m_qmlWidget->hide();
    m_pGanttWidget = new GanttWidget(this);
    m_pGanttWidget->setVisible(false);
    
    connect(m_pGanttWidget, &GanttWidget::deal_instagentData_sig, this, &SoStepListWidget::deal_instagentData_sig);

    connect(m_pGanttWidget, &GanttWidget::qml_quit_agent_edit_panel_sig, this, &SoStepListWidget::closeModelWidget_slot);

    this->setStyleSheet("SoStepListWidget{background-color:#2e2f30;}");

	m_pQStackedWidget = new QStackedWidget(this);


	m_pQStackedWidget->addWidget(m_qmlWidget);
    m_pQStackedWidget->addWidget(m_pGanttWidget);

	m_pLayout = new QVBoxLayout(this);
	m_pLayout->addWidget(m_pQStackedWidget);
	m_pLayout->setSpacing(0);
	m_pLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(m_pLayout);

	m_pQStackedWidget->setCurrentIndex(0);
}



SoStepListWidget::~SoStepListWidget()
{
	if (m_qmlWidget)
	{
		m_qmlWidget->deleteLater();
	}
	if (m_pLayout)
	{
		m_pLayout->deleteLater();
	}
    if (m_pGanttWidget)
	{
        m_pGanttWidget->deleteLater();
	}
}

void SoStepListWidget::editAgent(const QVariant &agentdata)
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

    QString agentPath = contextJsonobj.value("agentPath").toString();

    m_pGanttWidget->importexcel(agentPath);
    m_pQStackedWidget->setCurrentIndex(1);
    m_pQStackedWidget->currentWidget()->show();
}

QVariant SoStepListWidget::getAgentKey()
{
    return QVariant::fromValue("AGENTKEY_"+QString::number(FunctionAssistant::generate_random_positive_uint64()));
}

Q_INVOKABLE void SoStepListWidget::refreshAgents()
{
//	QObject *pRoot = (QObject*)m_qmlWidget->rootObject();
//	QMetaObject::invokeMethod(pRoot, "resetData");

    request_agentdata();
}

void SoStepListWidget::initData()
{
    QJsonArray jsarray;
    QJsonObject jsobj;

    jsobj.insert("agentKey", "AGENTKEY_"+QString::number(FunctionAssistant::generate_random_positive_uint64()));
    jsobj.insert("agentType", "Instagent");
    QString path = QCoreApplication::applicationDirPath();


    jsobj.insert("agentKeyword", path+"/data/plan/20190102.xlsx");
    jsobj.insert("agentName", path + "/data/plan/20190102.xlsx");
    jsobj.insert("agentNameI18n", path + "/data/plan/20190102.xlsx");
    jsobj.insert("agentPath", path + "/data/plan/20190102.xlsx");
    jsobj.insert("agentDesc", path + "/data/plan/20190102.xlsx");

    jsobj.insert("image_src", "qrc:/res/img/undefined.jpg" );
    jsarray.push_back(jsobj);

//    QObject *pRoot = (QObject*)m_qmlWidget->rootObject();

//    QMetaObject::invokeMethod(pRoot, "addAgentItems",
//                              Q_ARG(QVariant, QVariant::fromValue(jsarray)));
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        m_pGanttWidget->importexcel(path+"/data/plan/20190102.xlsx");
//        m_pGanttWidget->importexcel(path+"/data/plan/201901021.xlsx");
    }
    //m_pGanttWidget->importexcel(path + "/data/plan/20190102-04.xlsx");
	m_pQStackedWidget->setCurrentIndex(1);
//        m_pQStackedWidget->currentWidget()->show();
}

void SoStepListWidget::closeModelWidget_slot()
{
	m_pQStackedWidget->setCurrentIndex(0);
}

void SoStepListWidget::qml_quit_agent_edit_panel_slot()
{

}

void SoStepListWidget::importexcel(const QString &fileName)
{
    m_pGanttWidget->importexcel(fileName);
}

void SoStepListWidget::setAirportInfos(const QString &airport_code, const QStringList &allowRunway)
{
    m_pGanttWidget->setAirportInfos(airport_code, allowRunway);
}


void SoStepListWidget::showEvent(QShowEvent *event)
{
	m_pQStackedWidget->currentWidget()->show();
	QWidget::showEvent(event);
}

void SoStepListWidget::hideEvent(QHideEvent *event)
{
	m_pQStackedWidget->currentWidget()->hide();
	QWidget::hideEvent(event);
}

void SoStepListWidget::request_agentdata()
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
