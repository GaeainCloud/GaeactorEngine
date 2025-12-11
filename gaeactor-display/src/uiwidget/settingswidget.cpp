#include "settingswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QCoreApplication>
#include <QCheckBox>
#include <QFileInfo>
#include <QProcess>
#include "components/function.h"
#include "../components/configmanager.h"
#include <QFile>
#include "runningmodeconfig.h"
#include "settingsconfig.h"

#include "../datamanager/datamanager.hpp"
SettingsWidget::SettingsWidget(QWidget *parent)
    :QWidget(parent)
{
	QFile file(QCoreApplication::applicationDirPath() + "./res/qss/playwidget.qss");
	bool res = file.open(QIODevice::ReadOnly);
	if (!res)
	{
		return;
	}

	QString style = file.readAll();
	style += "SettingsWidget{background-color:#2e2f30;}";
	this->setStyleSheet(style);

	file.close();

	m_pLayout = new QVBoxLayout(this);

	QHBoxLayout * pHLayout = new QHBoxLayout(this);

	QLabel* labTitle = new QLabel(this);
	labTitle->setText(tr("Recording:"));
	labTitle->setFixedWidth(120);
	m_record_chkbtn = new QCheckBox(this);
	m_record_chkbtn->setObjectName("Recordobj");
    m_record_chkbtn->setText(tr("Record Disable"));
	pHLayout->addWidget(labTitle);
	pHLayout->addWidget(m_record_chkbtn);

	connect(m_record_chkbtn, &QCheckBox::clicked, this, &SettingsWidget::clickedslot);
	pHLayout->setAlignment(Qt::AlignLeft);
	m_pLayout->addLayout(pHLayout);
	
	//m_pLayout->addWidget(m_qmlWidget);
	m_pLayout->setStretch(0, 1);
	m_pLayout->setStretch(1, 15);
	m_pLayout->setSpacing(10);
	m_pLayout->setContentsMargins(200, 200, 0, 0);
	setLayout(m_pLayout);

	updateSettings();
}


SettingsWidget::~SettingsWidget()
{
    if(m_pLayout)
    {
        m_pLayout->deleteLater();
    }

    runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
}

void SettingsWidget::updateRecordstatus(bool bRedord)
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if (bRedord)
        {
            m_record_chkbtn->setText(tr("Record Enable"));
            runningmode::RunningModeConfig::getInstance().start_process_detached("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
        }
        else
        {
            m_record_chkbtn->setText(tr("Record Disable"));
            runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
        }
    }
    SettingsConfig::getInstance().updateRecordstatus(bRedord);
}

QToolButton * SettingsWidget::creatToolButton(const QString& icon,const QString& context)
{
	QToolButton *btn = new QToolButton(this);
	btn->resize(QSize(36, 36));
	btn->setIconSize(QSize(36, 36));
	btn->setAutoRaise(true);
	if (context.isEmpty())
	{
		btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
		btn->setText(context);
	}
	else
	{
		btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		btn->setText(context);
	}
	QIcon ic = QIcon(QCoreApplication::applicationDirPath() + "/res/svg/" + icon);
	btn->setIcon(ic);
	return btn;
}

void SettingsWidget::clickedslot(bool checked /*= false*/)
{
	if (m_record_chkbtn == sender())
    {
        updateRecordstatus((m_record_chkbtn->checkState() == Qt::Checked)?true:false);
	}
}


void SettingsWidget::updateSettings()
{
    m_record_chkbtn->setCheckState((SettingsConfig::getInstance().lavic_desktop_cfg().m_recording) ? Qt::Checked : Qt::Unchecked);
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {
        if (SettingsConfig::getInstance().lavic_desktop_cfg().m_recording)
        {
            m_record_chkbtn->setText(tr("Record Enable"));
            runningmode::RunningModeConfig::getInstance().start_process_detached("gaeactor-record",QStringList()<<DataManager::getInstance().m_simname);
        }
        else
        {
            m_record_chkbtn->setText(tr("Record Disable"));
            runningmode::RunningModeConfig::getInstance().kill_process("gaeactor-record");
        }
    }
}



void SettingsWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void SettingsWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void SettingsWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
