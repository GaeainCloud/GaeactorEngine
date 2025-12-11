#pragma execution_character_set("utf-8")
#include "playwidget.h"
#include <QListWidgetItem>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QToolButton>
#include <QComboBox>

#include <QCoreApplication>
#include "../components/configmanager.h"

#include "./Component/qtmaterialcircularprogress.h"

#include "src/storage/OriginalDataInputManager.h"
#include "src/storage/OriginalDataOutputManager.h"
#include "./Component/qtmaterialprogress.h"
#include "./Component/qtmaterialslider.h"
#include "../components/gaeactormanager.h"


#include "head_define.h"
#include "base_define.h"

#include "gaeactor_transmit_define.h"
#include "../datamanager/datamanager.hpp"


#include <QJsonObject>
#include <QJsonArray>

#define SLIDER_MIN (0)
#define SLIDER_MAX (1000000.0f)
#define SLIDER_RANGE (SLIDER_MAX - SLIDER_MIN)

#define SPEED_STEP (0.02f)
//#define JUMP_BY_DATAPOS

PlayWidget::PlayWidget(E_PLAY_MODE ePlayMode, QWidget *parent)
	: QWidget(parent),
	m_slider(nullptr),
	m_speedup(nullptr),
	m_speeddown(nullptr),
	m_play(nullptr),
	m_pause(nullptr),
	m_cbx_speed(nullptr),
	m_list(nullptr),
	m_bPressedSlider(false),
	m_playstatus(false),
	m_pausestatus(false),
	m_ePlayMode(ePlayMode),
	m_pOriginalDataInputManager(nullptr),
	m_reviewcallbackfunc(nullptr),
	m_bShowList(false)
{
	qRegisterMetaType<E_PLAY_OPERATE_TYPE>("E_PLAY_OPERATE_TYPE");
	initMember();
	initUI();
	initSignalSlot();
	iPrePackDataSendTime = QDateTime::currentDateTime().toUTC().toMSecsSinceEpoch();
}

PlayWidget::~PlayWidget()
{
}

void PlayWidget::setDataCallback(review_callback func)
{
	m_reviewcallbackfunc = std::move(func);
}

void PlayWidget::initMember()
{
	m_play = creatToolButton("play.svg");
	m_pause = creatToolButton("pause.svg");

	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		m_list = creatToolButton("list.svg");
		m_list->setToolTip(tr("List"));

		m_speedup = creatToolButton("plus.svg");
		m_speeddown = creatToolButton("minus.svg");
		m_speedup->setToolTip(tr("Speedup"));
		m_speeddown->setToolTip(tr("Speeddown"));


		m_speedup->setObjectName("btn_speedup");
		m_speedup->raise();

		m_speeddown->setObjectName("btn_speeddown");
		m_speeddown->raise();

		m_labSpeed = new QLabel(this);

    }break;
	case E_PLAY_MODE_REALTIME:
	{
		m_cbx_speed = new QComboBox(this);

		QStringList speedlist;

		auto appendspeed = [&](const double& speedval)
		{ 
			m_speedindex.insert(std::make_pair(speedval, speedlist.size()));
			speedlist << QString::number(speedval);
		};
		//appendspeed(0.1);
		//appendspeed(0.2);
		//appendspeed(0.3);
		//appendspeed(0.4);
		//appendspeed(0.5);
		//appendspeed(0.6);
		//appendspeed(0.7);
		//appendspeed(0.8);
		//appendspeed(0.9);
		appendspeed(1.0);
		appendspeed(2.0);
		appendspeed(3.0);
		appendspeed(4.0);
		appendspeed(5.0);
		appendspeed(6.0);
		appendspeed(7.0);
		appendspeed(8.0);
		appendspeed(9.0);
		appendspeed(10.0);
		appendspeed(20.0);
		appendspeed(30.0);
		appendspeed(40.0);
		appendspeed(50.0);
		appendspeed(60.0);
		appendspeed(70.0);
		appendspeed(80.0);
		appendspeed(90.0);
		appendspeed(100.0);

		m_cbx_speed->addItems(speedlist);

	}
    default:break;
	}
	m_play->setToolTip(tr("Play"));
	m_pause->setToolTip(tr("Pause"));

	m_play->setObjectName("btn_play");
	m_play->raise();

	m_pause->setObjectName("btn_pause");
	m_pause->raise();





	m_slider = new QtMaterialSlider(this);
	_labDownloadProcessNum = new QLabel(this);
	m_labPlayTime = new QLabel(this);
	m_labResidueTime = new QLabel(this);
	m_labTotalTime = new QLabel(this);

}



void PlayWidget::initUI()
{
	resize(QSize(1200, 600));
	this->setWindowTitle(tr("Raw Data Player"));
	setVisible(true);

	QFile file(QCoreApplication::applicationDirPath() + "./res/qss/playwidget.qss");
	bool res = file.open(QIODevice::ReadOnly);
	if (!res)
	{
		return;
	}

	const QString style = file.readAll();
	this->setStyleSheet(style);

	file.close();

	m_slider->setRange(SLIDER_MIN, SLIDER_MAX);
	_labDownloadProcessNum->setText("");  // 设置在线更新进度条初始文字显示
	_labDownloadProcessNum->setStyleSheet("font-family: Microsoft YaHei;color: white; font-size: 16px");
	m_labPlayTime->setText("");  // 设置在线更新进度条初始文字显示
	m_labPlayTime->setStyleSheet("font-family: Microsoft YaHei;color: white; font-size: 16px");

	m_labTotalTime->setText("");  // 设置在线更新进度条初始文字显示
	m_labTotalTime->setStyleSheet("font-family: Microsoft YaHei;color: white; font-size: 16px");

	m_labResidueTime->setText("");  // 设置在线更新进度条初始文字显示
	m_labResidueTime->setStyleSheet("font-family: Microsoft YaHei;color: white; font-size: 16px");



	m_slider->setMinimum(SLIDER_MIN);        // 设置进度条最小值
	m_slider->setMaximum(SLIDER_MAX);      // 设置进度条最大值
	m_slider->setValue(SLIDER_MIN);          // 设置当前的运行值

	m_slider_min = SLIDER_MIN;
	m_slider_max = SLIDER_MAX;
	m_slider_cur = SLIDER_MIN;


	m_slider->show();
	_labDownloadProcessNum->show();
	m_labPlayTime->show();
	m_labResidueTime->show();
	m_labTotalTime->show();
	_labDownloadProcessNum->setAlignment(Qt::AlignCenter);
	m_labPlayTime->setAlignment(Qt::AlignCenter);
	m_labTotalTime->setAlignment(Qt::AlignLeft);
	m_labResidueTime->setAlignment(Qt::AlignCenter);



    switch (m_ePlayMode)
    {
    case PlayWidget::E_PLAY_MODE_REVIEW:
    {
		m_labSpeed->setAlignment(Qt::AlignCenter);

		m_labSpeed->setText("");  // 设置在线更新进度条初始文字显示
		m_labSpeed->setStyleSheet("font-family: Microsoft YaHei;color: white; font-size: 16px");

        m_slider->setEnabled(false);
        m_speedup->setEnabled(false);
        m_speeddown->setEnabled(false);
        m_play->setEnabled(false);
		m_pause->setEnabled(false);
    }break;
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		m_cbx_speed->setStyleSheet("font-family: Microsoft YaHei;color: black; font-size: 16px");
	}
	break;
    default:break;
    }
}

void PlayWidget::initSignalSlot()
{
	connect(m_play, &QToolButton::clicked, this, &PlayWidget::btnPlayClickSlot);
	connect(m_pause, &QToolButton::clicked, this, &PlayWidget::btnPauseClickSlot);

	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		connect(m_speedup, &QToolButton::clicked, this, &PlayWidget::btnSpeedClickSlot);
		connect(m_speeddown, &QToolButton::clicked, this, &PlayWidget::btnSpeedClickSlot);

		connect(m_list, &QToolButton::clicked, this, &PlayWidget::btnListClickSlot);
	}break;
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		connect(m_cbx_speed, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &PlayWidget::currentIndexChangedSlot, Qt::UniqueConnection);
	}
	break;
	}
	

	connect(m_slider, &QtMaterialSlider::sliderPressed, this, &PlayWidget::sliderPressedSlot);
	connect(m_slider, &QtMaterialSlider::sliderMoved, this, &PlayWidget::sliderMovedSlot);
	
	connect(m_slider, &QtMaterialSlider::sliderReleased, this, &PlayWidget::sliderReleasedSlot);

	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		connect(this, &PlayWidget::sendDataSignal, this, &PlayWidget::sendDataSlot);
	}
	break;
	default:
		break;
	}

}
void PlayWidget::setSliderRange(int min, int max)
{
	m_slider_min = min;
	m_slider_max = max;
	m_slider_cur = 0;
	m_slider_adjust = 0;

	m_slider->setRange(min, max);

	//	m_labResidueTime->setText((formatTimeStr(range)));
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(DataManager::getInstance().m_play_max*1000);
	QString dt_st = dt.toString("yyyy-MM-dd hh:mm:ss");

	m_labResidueTime->setText(dt_st);
//	m_labPlayTime->setText((formatTimeStr((0) * 1000)));
	//m_labResidueTime->setText((formatTimeStr((0) * 1000)));
	//m_labTotalTime->setText((formatTimeStr((0) * 1000)));
}

void PlayWidget::setSliderValue(uint64_t val)
{
	if (!m_bPressedSlider)
	{
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(val*1000);
		QString dt_st = dt.toString("yyyy-MM-dd hh:mm:ss");
//		m_labPlayTime->setText((formatTimeStr(val * 1000)));
		m_labPlayTime->setText(dt_st);
        m_slider_cur = val - DataManager::getInstance().m_play_min;
        m_slider->setValue(m_slider_cur);
	}
}

int PlayWidget::getSliderAdjust()
{
	return m_slider_adjust;
}

int PlayWidget::getSliderCur()
{
    return m_slider_cur;
}

void PlayWidget::setPlayClickSlot(bool bPlay)
{
    m_playstatus = bPlay;
    switch (m_ePlayMode)
    {
    case PlayWidget::E_PLAY_MODE_REALTIME:
    {
    }
    break;
    case PlayWidget::E_PLAY_MODE_REVIEW:
    {
        if (!m_pOriginalDataInputManager)
        {
            return;
        }
        if (m_playstatus)
        {
            if(!m_pOriginalDataInputManager->getBRunning())
            {
                m_pOriginalDataInputManager->start();
            }
            m_playstatus = true;
            DataManager::getInstance().m_review_status = "start";
            setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
        }
        else
        {
            if(m_pOriginalDataInputManager->getBRunning())
            {
                m_pOriginalDataInputManager->stop();
                setReadSpeedContext(0.0);
            }
            m_playstatus = false;
            DataManager::getInstance().m_review_status = "stop";
            setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
        }
    }break;
    }
    if (!m_playstatus)
    {
        m_play->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/play.svg"));
        m_play->setToolTip(tr("Play"));


        if(m_list)
        {
            m_list->setEnabled(true);
        }
    }
    else
    {
        m_play->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/termination.svg"));
        m_play->setToolTip(tr("Termination"));
        if(m_list)
        {
            m_list->setEnabled(false);
        }
    }
}

void PlayWidget::setPauseClickSlot(bool bPause)
{
    m_pausestatus = bPause;
    switch (m_ePlayMode)
    {
    case PlayWidget::E_PLAY_MODE_REALTIME:
    {

    }
    break;
    case PlayWidget::E_PLAY_MODE_REVIEW:
    {
        if (!m_pOriginalDataInputManager)
        {
            return;
        }
        if(m_pausestatus)
        {
            if (m_pOriginalDataInputManager->getBRunning())
            {
                m_pausespeed = m_pOriginalDataInputManager->getReadSpeed();
                DataManager::getInstance().m_review_speed = 0.0;
                m_pOriginalDataInputManager->setReadSpeed(0.0);
            }
            DataManager::getInstance().m_review_status = "pause";
        }
        else
        {
            if (m_pOriginalDataInputManager->getBRunning())
            {
                DataManager::getInstance().m_review_speed = m_pausespeed;
                m_pOriginalDataInputManager->setReadSpeed(m_pausespeed);
            }
            DataManager::getInstance().m_review_status = "resume";
        }
        setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
    }break;
    }
    if (!m_pausestatus)
    {
        m_pause->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/pause.svg"));
        m_pause->setToolTip(tr("Pause"));
    }
    else
    {
        m_pause->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/resume.svg"));
        m_pause->setToolTip(tr("Resume"));
    }
}

void PlayWidget::setPressedSlider(bool bPressed)
{
	m_bPressedSlider = bPressed;
}

void PlayWidget::setReadSpeedContext(float readSpeed)
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		if (m_speedindex.find(readSpeed) != m_speedindex.end())
		{
			m_cbx_speed->setCurrentIndex(m_speedindex.at(readSpeed));
		}
	}
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (m_pOriginalDataInputManager)
        {
            DataManager::getInstance().m_review_speed = m_pOriginalDataInputManager->getReadSpeed();
			m_labSpeed->setText(QString::number(m_pOriginalDataInputManager->getReadSpeed(), 'g', 3));
		}
	}
	break;
	default:
		break;
	}
}

double PlayWidget::getSpeedContext()
{
	double spd = 1.0;
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		spd = m_cbx_speed->currentText().toDouble();
	}
	break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (m_pOriginalDataInputManager)
		{
			spd = m_labSpeed->text().toDouble();
		}
	}
	break;
	default:
		break;
	}
	return spd;
}

void PlayWidget::sendDataSlot(qint64 iGlobeFileReadValidDataPos, qint64 iDataSendTimeStamp)
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
		{
		}
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (!m_bPressedSlider && m_pCurrentitem)
		{
#ifdef JUMP_BY_DATAPOS
			int processorval = (float)(iGlobeFileReadValidDataPos) / (float)pitem->m_iValidDataTotalLen*SLIDER_RANGE*1.0f;
			if (iGlobeFileReadValidDataPos == pitem->m_iValidDataTotalLen)
			{
				emit setEnableSignal(true);
			}
			m_slider->setValue(processorval);
#else
			float processdata = (float)(iDataSendTimeStamp - m_pCurrentitem->m_iTimeStampStart) / (float)m_pCurrentitem->getTotalMSecs();
			int processorval = processdata * SLIDER_RANGE*1.0f;
			if ((iDataSendTimeStamp - m_pCurrentitem->m_iTimeStampStart) >= m_pCurrentitem->getTotalMSecs())
			{
				emit setEnableSignal(true);
			}
			m_slider->setValue(processorval);
#endif

#if 0
			QString text = QString("%1 %2/%3 %4/%5").arg(m_currentitem->m_titlename)
				.arg(QString::number(iGlobeFileReadValidDataPos))
				.arg(QString::number(m_currentitem->m_iValidDataTotalLen))
				.arg(QString::number(m_iDataFrames))
				.arg(QString::number(m_currentitem->m_iFrames));
#else

			QString text = QString("%1 %2/%3 %4/%5").arg("")
				.arg(QString::number(iGlobeFileReadValidDataPos))
				.arg(QString::number(m_pCurrentitem->m_iValidDataTotalLen))
				.arg(QString::number(m_iDataFrames))
				.arg(QString::number(m_pCurrentitem->m_iFrames));
#endif
			m_iDataFrames++;
			_labDownloadProcessNum->setText(text);
			qint64 msecs = m_pCurrentitem->getTotalMSecs();
			qint64 costmsecs = iDataSendTimeStamp - m_pCurrentitem->m_iTimeStampStart + 1;
			qint64 residuemsecs = msecs - costmsecs;

			QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();
			m_labResidueTime->setText(formatTimeStr(residuemsecs));
			m_labPlayTime->setText(formatTimeStr(costmsecs));
			m_labTotalTime->setText(formatTimeStr(msecs) + "_" + formatTimeStr(m_payStart.msecsTo(currentDateTime)) + "_" + QDateTime::fromMSecsSinceEpoch(m_pCurrentitem->m_iTimeStampStart).time().toString("hh:mm:ss.zzzz") + "_" + QDateTime::fromMSecsSinceEpoch(iDataSendTimeStamp).time().toString("hh:mm:ss.zzzz") + "_" + QString::number(iDataSendTimeStamp - iPrePackDataSendTime));


            {
                DataManager::getInstance().getTimestampRange_reviewdata(iPrePackDataSendTime, iDataSendTimeStamp);
            }

            iPrePackDataSendTime = iDataSendTimeStamp;

            if(iPrePackDataSendTime == DataManager::getInstance().m_currentreview.m_iValidDataTotalLen)
            {
                emit trigger_review_event_end_sig();
            }
		}


	}
	break;
	default:
		break;
	}
}

void PlayWidget::currentIndexChangedSlot(const QString& index)
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		emit btn_click_sig(E_PLAY_OPERATE_TYPE_ADJUST_SPEED);
	}
	break;
	}
}

void PlayWidget::btnSpeedClickSlot()
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (!m_pOriginalDataInputManager)
		{
			return;
		}
		float speed = m_pOriginalDataInputManager->getReadSpeed();
		if (sender() == m_speedup)
		{
			speed += SPEED_STEP;
			speed = speed > 50.0f ? 50.0f : speed;
		}
		else if (sender() == m_speeddown)
		{
			speed -= SPEED_STEP;
			speed = speed < SPEED_STEP ? SPEED_STEP : speed;
		}

        DataManager::getInstance().m_review_speed = speed;
        m_pOriginalDataInputManager->setReadSpeed(speed);
        setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
    }break;
    default:
        break;
	}
}

void PlayWidget::btnPlayClickSlot()
{
    m_playstatus = !m_playstatus;
    switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		if (m_playstatus)
		{
			emit btn_click_sig(E_PLAY_OPERATE_TYPE_PLAY);
		}
		else
		{
			emit btn_click_sig(E_PLAY_OPERATE_TYPE_TERMINATION);
		}
	}
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
        if (!m_pOriginalDataInputManager)
        {
            return;
        }
        if (m_playstatus)
        {
            if(!m_pOriginalDataInputManager->getBRunning())
            {
                m_pOriginalDataInputManager->start();
            }
            m_playstatus = true;
            DataManager::getInstance().m_review_status = "start";
        }
        else
        {
            if(m_pOriginalDataInputManager->getBRunning())
            {
                m_pOriginalDataInputManager->stop();
                m_pOriginalDataInputManager->setReadSpeed(0.0);
            }
            m_playstatus = false;
            DataManager::getInstance().m_review_status = "stop";
        }
        DataManager::getInstance().m_review_speed = m_pOriginalDataInputManager->getReadSpeed();
        setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
    }break;
	}
	if (!m_playstatus)
	{
		m_play->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/play.svg"));
		m_play->setToolTip(tr("Play"));


        if(m_list)
        {
            m_list->setEnabled(true);
        }
	}
	else
	{
		m_play->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/termination.svg"));
		m_play->setToolTip(tr("Termination"));
        if(m_list)
        {
            m_list->setEnabled(false);
        }
	}
}

void PlayWidget::btnPauseClickSlot()
{
    m_pausestatus = !m_pausestatus;
    switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		if (m_pausestatus)
		{
            emit btn_click_sig(E_PLAY_OPERATE_TYPE_PASUE);
        }
		else
		{
            emit btn_click_sig(E_PLAY_OPERATE_TYPE_RESUME);
        }
	}
	break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
    {
		if (!m_pOriginalDataInputManager)
		{
			return;
		}
        if (!m_pOriginalDataInputManager)
        {
            return;
        }
        if(m_pausestatus)
        {
            if (m_pOriginalDataInputManager->getBRunning())
            {
                m_pausespeed = m_pOriginalDataInputManager->getReadSpeed();
                DataManager::getInstance().m_review_speed = 0.0;
                m_pOriginalDataInputManager->setReadSpeed(0.0);
            }
            DataManager::getInstance().m_review_status = "pause";
        }
        else
        {
            if (m_pOriginalDataInputManager->getBRunning())
            {
                DataManager::getInstance().m_review_speed = m_pausespeed;
                m_pOriginalDataInputManager->setReadSpeed(m_pausespeed);
            }
            DataManager::getInstance().m_review_status = "resume";
        }
        setReadSpeedContext(m_pOriginalDataInputManager->getReadSpeed());
	}break;
	}
	if (!m_pausestatus)
	{
		m_pause->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/pause.svg"));
		m_pause->setToolTip(tr("Pause"));
	}
	else
	{
		m_pause->setIcon(QIcon(QCoreApplication::applicationDirPath() + "/res/svg/resume.svg"));
		m_pause->setToolTip(tr("Resume"));
	}
}

void PlayWidget::btnListClickSlot()
{
	m_bShowList = !m_bShowList;
	emit btn_click_sig(m_bShowList? E_PLAY_OPERATE_TYPE_REPLAY_LIST_SHOW : E_PLAY_OPERATE_TYPE_REPLAY_LIST_HIDE);
}

void PlayWidget::sliderPressedSlot()
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		return;
		m_bPressedSlider = true;
	}
	break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (!m_pOriginalDataInputManager)
		{
			return;
		}
		m_bPressedSlider = true;
	}break;
	}
}

void PlayWidget::sliderMovedSlot(int position)
{
	switch (m_ePlayMode)
	{
		case PlayWidget::E_PLAY_MODE_REALTIME:
		{
			return;
			int value = position;
			m_slider_adjust = value;

            m_labPlayTime->setText((formatTimeStr(DataManager::getInstance().m_play_min+value * 1000)));
		}
		break;
		case PlayWidget::E_PLAY_MODE_REVIEW:
		{
			if (!m_pOriginalDataInputManager)
			{
				return;
			}
		}break;
	}
}

void PlayWidget::sliderReleasedSlot()
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
		{
			return;
			int value = m_slider->value();
			m_slider_adjust = value;

            m_labPlayTime->setText((formatTimeStr((DataManager::getInstance().m_play_min+value) * 1000)));
			emit btn_click_sig(E_PLAY_OPERATE_TYPE_ADJUST_PERCENT);
			m_bPressedSlider = false;
		}
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (!m_pOriginalDataInputManager)
		{
			return;
		}
		if (m_pOriginalDataInputManager)
		{
			int value = m_slider->value();
			m_slider_cur = value;
			double percent = (value / SLIDER_RANGE * 1.0f);
#ifdef JUMP_BY_DATAPOS
			m_iDataFrames = m_pPlayElement->jumpToDataPosPercent(percent)
#else
			m_iDataFrames = this->jumpToDataMillisecondPosOffsetPercent(percent);
#endif
			m_bPressedSlider = false;
		}
	}break;
	}
}

QToolButton *PlayWidget::speeddown() const
{
	return m_speeddown;
}

QToolButton *PlayWidget::playctrl() const
{
	return m_play;
}



QToolButton * PlayWidget::pausectrl() const
{
	return m_pause;
}

quint64 PlayWidget::jumpToDataPos(quint64 iJumpDataPos)
{
	if (m_pOriginalDataInputManager)
	{
		m_iDataFrames = m_pOriginalDataInputManager->jumpToDataPos(iJumpDataPos) - 1;
	}
	return m_iDataFrames;
}

quint64 PlayWidget::jumpToDataMillisecondPos(quint64 iJumpDataMillisecondPos)
{
	if (m_pOriginalDataInputManager && m_pCurrentitem)
	{
		m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPos - m_pCurrentitem->m_iTimeStampStart) - 1;
	}
	return m_iDataFrames;
}

quint64 PlayWidget::jumpToDataMillisecondPosOffset(quint64 iJumpDataMillisecondPosOffset)
{
	if (m_pOriginalDataInputManager)
	{
		m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPosOffset) - 1;
	}
	return m_iDataFrames;
}

quint64 PlayWidget::jumpToDataMillisecondPosOffsetPercent(double percent)
{
	if (m_pOriginalDataInputManager && m_pCurrentitem)
	{
		m_pOriginalDataInputManager->stop();
        quint64 totalMSecs = m_pCurrentitem->getTotalMSecs();
        quint64 iJumpDataMillisecondPos = totalMSecs *percent;
		m_iDataFrames = m_pOriginalDataInputManager->jumpToDataMillisecondPos(iJumpDataMillisecondPos) - 1;
		m_pOriginalDataInputManager->start();

        /////////////////////////////////////////////////////////////
        auto formatTimeStr=[](qint64 msecs)->std::string{
            qint64 dd =  msecs / (3600 * 1000 * 24);
            qint64 hh = (msecs - dd * (3600 * 1000 * 24)) / (3600 * 1000);
            qint64 mm = (msecs - hh * (3600 * 1000)) / (60 * 1000);
            qint64 ss = (msecs - hh * (3600 * 1000) - mm * (60 * 1000)) / 1000;
            qint64 ms = (msecs - hh * (3600 * 1000) - mm * (60 * 1000) - ss * 1000) % 1000;
            QString timestr = QString("%1 %2:%3:%4.%5").arg(QString::number(dd), 2, '0').arg(QString::number(hh), 2, '0').arg(QString::number(mm), 2, '0').arg(QString::number(ss), 2, '0').arg(QString::number(ms), 3, '0');
            return timestr.toStdString();
        };
        std::cout<<"trigger jump to "<<percent<<" "<<iJumpDataMillisecondPos<<"-->"<<formatTimeStr(iJumpDataMillisecondPos)<<" / "<<totalMSecs<<"-->"<<formatTimeStr(totalMSecs)<<"\n";


        /////////////////////////////////////////////////////////////
    }
	return m_iDataFrames;

}

quint64 PlayWidget::jumpToDataPosPercent(double percent)
{
	if (m_pOriginalDataInputManager && m_pCurrentitem)
	{
		m_pOriginalDataInputManager->stop();
		quint64 iJumpDataPos = 0;
		iJumpDataPos = m_pCurrentitem->m_iValidDataTotalLen * percent;
		m_iDataFrames = m_pOriginalDataInputManager->jumpToDataPos(iJumpDataPos) - 1;
		m_pOriginalDataInputManager->start();
	}
	return m_iDataFrames;
}


bool PlayWidget::initializeReadFileSlot(tagReplayItemInfo* _currentitem)
{
	if (nullptr == _currentitem)
	{
		return false;
	}
	m_pCurrentitem = _currentitem;
    DataManager::getInstance().setCurrentReviewItem(*_currentitem);
	//    m_bInitialize = false;

	if (m_pOriginalDataInputManager != nullptr)
	{
		m_pOriginalDataInputManager->clear();
		delete m_pOriginalDataInputManager;
		m_pOriginalDataInputManager = nullptr;
	}

	try
	{
		m_pOriginalDataInputManager = new originaldatastoragestd::OriginalDataInputManager();
	}
	catch (...)
	{
		m_pOriginalDataInputManager = nullptr;

		return false;
	}


	if (!m_pOriginalDataInputManager->initialize(m_pCurrentitem->m_absoluteFilePath.toStdString().c_str(), nullptr, std::bind(&PlayWidget::data_callback, this,
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3,
		std::placeholders::_4,
		std::placeholders::_5)))
	{
		return false;
	}
	//    m_bInitialize = true;
	m_slider->setEnabled(true);
	m_play->setEnabled(true);
	m_pause->setEnabled(true);
	m_bShowList = false;


	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{ 
	}
	break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		m_speedup->setEnabled(true);
		m_speeddown->setEnabled(true);
	}break;
	}
	return true;
}

bool PlayWidget::data_callback(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp)
{
	auto inputstorageType = m_pOriginalDataInputManager->getInputStorageType();
	//    inputstorageType = originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS;
	switch (inputstorageType)
	{
	case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND:
	case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV:
	{
//        QDateTime datetime = QDateTime::fromMSecsSinceEpoch(iTimeStamp);
//        QDateTime currentDateTime = QDateTime::currentDateTime().toUTC();;
//        qDebug() << formatTimeStr(iDataSendTimeStamp - m_pCurrentitem->m_iTimeStampStart) << "_" << formatTimeStr(m_payStart.msecsTo(currentDateTime)) << "_" << formatTimeStr(m_payStart.msecsTo(datetime));
//        qDebug() << datetime.toString("yyyy-MM-dd hh:mm:ss.zzz") << "len " << iDataLen /*<< ":" << array.toHex(' ')*/;
        emit this->sendDataSignal(iGlobeFileReadValidDataPos, iDataSendTimeStamp);

        if (m_reviewcallbackfunc)
        {
            const char *frame_head = (const char*)pData;
            uint32_t *crcval_recv = (uint32_t*)(pData + sizeof(uint32_t));
            const BYTE *dataptr = pData + sizeof(uint32_t) + sizeof(uint32_t);

//            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType = *((E_CHANNEL_TRANSMITDATA_TYPE*)(dataptr));
//            uint32_t irecvlen = *(uint32_t*)(dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
//            const char *protocoldata_ptr = dataptr + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(uint32_t);

//            uint32_t ipayloadlen = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + irecvlen;


            const BYTE* pSrcData = dataptr;
            E_CHANNEL_TRANSMITDATA_TYPE channelTransmitDataType;
            UINT32 iLen;
            const BYTE *pContextData = nullptr;

            memcpy(&channelTransmitDataType, pSrcData, sizeof(E_CHANNEL_TRANSMITDATA_TYPE));
            memcpy(&iLen, pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE), sizeof(UINT32));
            pContextData = pSrcData + sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32);
            uint32_t userPayloadSize = sizeof(E_CHANNEL_TRANSMITDATA_TYPE) + sizeof(UINT32) + iLen;
            if (userPayloadSize)
            {
                m_reviewcallbackfunc(channelTransmitDataType, pContextData, iLen, pSrcData, userPayloadSize);
            }
        }

        DataManager::getInstance().deal_review(pData, iDataLen, iTimeStamp, iGlobeFileReadValidDataPos, iDataSendTimeStamp);
	}
	break;
	case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_SEND_COMPRESS:
	case originaldatastoragestd::OriginalDataInputManager::ENUM_INPUTSTORAGE_TYPE_RECV_COMPRESS:
	{
		//        this->decodestateData(pData,iDataLen,iTimeStamp,iGlobeFileReadBeginValidDataPos,iDataSendTimeStamp, true);
	}
	break;
	default:
		break;
    }
    return true;
}

originaldatastoragestd::OriginalDataInputManager *PlayWidget::pOriginalDataInputManager() const
{
    return m_pOriginalDataInputManager;
}

float PlayWidget::getReadSpeed()
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (!m_pOriginalDataInputManager)
		{
			return 1.0f;
		}
		return m_pOriginalDataInputManager->getReadSpeed();
	}break;
	}
}

void PlayWidget::setReadSpeed(float readSpeed)
{
	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
		break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		if (m_pOriginalDataInputManager)
        {
            DataManager::getInstance().m_review_speed = readSpeed;
			m_pOriginalDataInputManager->setReadSpeed(readSpeed);
		}
	}break;
	}
}


QString PlayWidget::formatTimeStr(qint64 msecs)
{
	qint64 hh = msecs / (3600 * 1000);
	qint64 mm = (msecs - hh * (3600 * 1000)) / (60 * 1000);
	qint64 ss = (msecs - hh * (3600 * 1000) - mm * (60 * 1000)) / 1000;
	qint64 ms = (msecs - hh * (3600 * 1000) - mm * (60 * 1000) - ss * 1000) % 1000;
	QString timestr = QString("%1:%2:%3.%4").arg(QString::number(hh), 2, '0').arg(QString::number(mm), 2, '0').arg(QString::number(ss), 2, '0').arg(QString::number(ms), 3, '0');
	return timestr;
}


QIcon PlayWidget::qiconFromSvg(QString svg_path)
{
	QPixmap img(svg_path);
	QPainter qp(&img);
	qp.setCompositionMode(QPainter::CompositionMode_SourceIn);
	qp.fillRect(img.rect(), QColor(128, 128, 128));
	qp.end();
	return QIcon(img);
}

QToolButton *PlayWidget::creatToolButton(const QString &icon)
{
	QToolButton *btn = new QToolButton(this);
	btn->resize(QSize(36, 36));
	btn->setIconSize(QSize(36, 36));
	btn->setAutoRaise(true);
	btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	QIcon ic = QIcon(QCoreApplication::applicationDirPath() + "/res/svg/" + icon);
	btn->setIcon(ic);
	return btn;
}

void PlayWidget::resizeUI()
{
	m_play->setGeometry(32, 10, 36, 36);
	m_pause->setGeometry(m_play->geometry().right() + 10, 10, 36, 36);
	m_labPlayTime->setGeometry(m_pause->geometry().right() + 10, 10, 32 * 6, 36);
	m_slider->setGeometry(m_labPlayTime->geometry().right() + 10, 10, (width() - 64 - 50 - 32 - 36 - 32 * 10 - 36 - 36 - 36-32*4), 36);

	m_labResidueTime->setGeometry(m_slider->geometry().right() + 10, 10, 32 * 6, 36);

	switch (m_ePlayMode)
	{
	case PlayWidget::E_PLAY_MODE_REALTIME:
	{
		m_cbx_speed->setGeometry(m_labResidueTime->geometry().right() + 10, 10, 10 * 3 + 32 * 2 + 36 * 2, 36);

		if (m_list)
		{
			m_list->setGeometry(m_cbx_speed->geometry().right() + 10, 10, 36, 36);
		}

	}
	break;
	case PlayWidget::E_PLAY_MODE_REVIEW:
	{
		m_speeddown->setGeometry(m_labResidueTime->geometry().right() + 10, 10, 36, 36);
		m_labSpeed->setGeometry(m_speeddown->geometry().right() + 10, 10, 32 * 2, 36);
		m_speedup->setGeometry(m_labSpeed->geometry().right() + 10, 10, 36, 36);

		if (m_list)
		{
			m_list->setGeometry(m_speedup->geometry().right() + 10, 10, 36, 36);
		}
	}
	break;
	default:
		break;
	}






	m_labTotalTime->setGeometry(32, m_labResidueTime->geometry().bottom() + 1, 32 * 12, 36);

	_labDownloadProcessNum->setGeometry(m_labTotalTime->geometry().right() + 10, m_labResidueTime->geometry().bottom() + 1, (width() - 64), 36);

	m_slider->update();
}

void PlayWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	resizeUI();
}

void PlayWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}




