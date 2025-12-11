#include "replaywidget.h"

#include <QVBoxLayout>

#include "mapwidget.h"
#include "playwidget.h"
#include "replaypanel.h"
#include "../Components/gaeactormanager.h"

ReplayWidget::ReplayWidget(QWidget *parent)
    :QWidget(parent),
    m_qmlWidget(nullptr),
    m_mapWidget(nullptr)
{
    this->setStyleSheet("ReplayWidget{background-color:#2e2f30;}");
    m_mapWidget = new MapWidget(MapWidget::E_MAP_MODE_DISPLAY_REVIEW,this);
    m_qmlWidget = new PlayWidget(PlayWidget::E_PLAY_MODE_REVIEW, m_mapWidget);
	m_pReplayPanel = new ReplayPanel(m_mapWidget);
	m_pReplayPanel->hide();
    m_qmlWidget->setDataCallback(std::bind(&GaeactorManager::receive_callback, m_mapWidget->getGaeactorManager(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
	
	connect(m_qmlWidget, &PlayWidget::btn_click_sig, this, &ReplayWidget::btn_click_slot);
	connect(m_pReplayPanel, &ReplayPanel::initializeReadFileSig, m_qmlWidget, &PlayWidget::initializeReadFileSlot);
}

ReplayWidget::~ReplayWidget()
{
    if(m_qmlWidget)
    {
        m_qmlWidget->deleteLater();
    }
    if(m_mapWidget)
    {
        m_mapWidget->deleteLater();
    }
	if (m_pReplayPanel)
	{
        m_pReplayPanel->deleteLater();
    }
}

PlayWidget *ReplayWidget::getPlayWidget()
{
    if(m_qmlWidget)
    {
        return m_qmlWidget;
    }
    return nullptr;
}

void ReplayWidget::btn_click_slot(E_PLAY_OPERATE_TYPE type)
{
	switch (type)
	{
	case E_PLAY_OPERATE_TYPE_REPLAY_LIST_SHOW:
	{
		m_pReplayPanel->setVisible(true);
	}
	break;
	case E_PLAY_OPERATE_TYPE_REPLAY_LIST_HIDE:
	{
		m_pReplayPanel->setVisible(false);
	}
	break;
	default:
		break;
	}
}

void ReplayWidget::showEvent(QShowEvent *event)
{
	m_qmlWidget->show();
    m_mapWidget->setVisible(true);
    QWidget::showEvent(event);
}

void ReplayWidget::hideEvent(QHideEvent *event)
{
    m_qmlWidget->hide();
    m_mapWidget->setVisible(false);
    QWidget::hideEvent(event);
}

void ReplayWidget::resizeEvent(QResizeEvent *event)
{
    m_mapWidget->setGeometry(0, 0, this->width(), this->height());

    m_qmlWidget->setGeometry(0, this->height() - this->height()/15, this->width(), this->height());

	m_pReplayPanel->setGeometry(this->width() * 2 / 3, 0, this->width() / 3, this->height() - this->height() / 15);
	
    QWidget::resizeEvent(event);
}
