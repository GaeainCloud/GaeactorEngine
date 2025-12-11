#pragma execution_character_set("utf-8")
#include "replayitemwidget.h"
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QDateTime>
#include "playwidget.h"
#include "./Component/qtmaterialprogress.h"
#include <QDir>
#include <QToolButton>
#include <QEvent>
#include <QCoreApplication>


ReplayItemWidget::ReplayItemWidget(QWidget *parent)
	: QWidget(parent)
{
	this->setAttribute(Qt::WA_TranslucentBackground, true);
 	initMemeber();
    initUI();
 	initSignalSlot();
}

ReplayItemWidget::ReplayItemWidget(tagReplayItemInfo* item, QWidget* parent /*= Q_NULLPTR*/)
    : QWidget(parent), m_item(item)
{
	initMemeber();
    initUI();
	initSignalSlot();
    this->labIndex->setText(QString::number(m_item->m_index));

    this->labTitle->setText(m_item->m_titlename);

	QString sizeTmp = (m_item->m_size < 1024) ? (QString::number(m_item->m_size) + "B") : \
		QString::number((float)m_item->m_size / 1024.0f) + "kB";
	this->labSize->setText(sizeTmp);
	this->labDate->setText(m_item->m_date);
	QString iTimeStampStartstr = QDateTime::fromMSecsSinceEpoch(m_item->m_iTimeStampStart).toString("yyyy-MM-dd hh:mm:ss.zzz");
	QString iTimeStampEndstr = QDateTime::fromMSecsSinceEpoch(m_item->m_iTimeStampEnd).toString("yyyy-MM-dd hh:mm:ss.zzz");

    m_btnPlay->setToolTip(tr("Play"));
    m_btnDelete->setToolTip(tr("Delete"));
}

ReplayItemWidget::~ReplayItemWidget()
{
}



tagReplayItemInfo* ReplayItemWidget::getItem()
{
    return m_item;
}

void ReplayItemWidget::hideCtrl(bool bHide)
{
    m_btnPlay->setVisible(bHide?false:true);
    m_btnDelete->setVisible(bHide?false:true);
}

void ReplayItemWidget::initMemeber()
{
    labIndex = new QLabel(this);
    labIndex->setObjectName("lab_index");
    labTitle = new QLabel(this);
    labTitle->setObjectName("lab_context");

    labSize = new QLabel(this);
    labSize->setObjectName("lab_labSize");

    labDate = new QLabel(this);
    labDate->setObjectName("lab_labDate");

    _labDownloadProcessNum = new QLabel(this);

    m_btnPlay = creatToolButton("view.svg");
    m_btnPlay->installEventFilter(this);

    m_btnDelete = creatToolButton("delete.svg");
    m_btnDelete->installEventFilter(this);

}

void ReplayItemWidget::initUI()
{
    resize(LOG_ITEM_WIDTH,LOG_ITEM_HIGHT);

    labIndex->setAlignment(Qt::AlignCenter);
    labTitle->setAlignment(Qt::AlignCenter);
    labSize->setAlignment(Qt::AlignCenter);
    labDate->setAlignment(Qt::AlignCenter);


    _labDownloadProcessNum->setStyleSheet("color: white; font-size: 12px");

    _labDownloadProcessNum->show();
    _labDownloadProcessNum->setAlignment(Qt::AlignCenter);
}

void ReplayItemWidget::initSignalSlot()
{
}

void ReplayItemWidget::resizeUI()
{
	int w = width() / 6;
	int ww = w / 2;
	labIndex->setGeometry(0, 0, w, height());
    labTitle->setGeometry(labIndex->geometry().right(),0,w,height());
    labSize->setGeometry(labTitle->geometry().right(),0,w,height());
    labDate->setGeometry(labSize->geometry().right(),0,w*2,height());
    m_btnPlay->setGeometry(labDate->geometry().right()+(ww /2 - 36/2),height()/2-36/2,36,36);
    m_btnDelete->setGeometry(labDate->geometry().right() + ww + (ww / 2 - 36 / 2), height()/2-36/2,36,36);
}




QToolButton * ReplayItemWidget::creatToolButton(const QString& icon)
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

void ReplayItemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeUI();
}



bool ReplayItemWidget::eventFilter(QObject *target, QEvent *e)
{
    if(target == m_btnPlay && ((e->type() == QEvent::MouseButtonRelease)))
    {
        btnPlaySlot();
    }
    if(target == m_btnDelete && ((e->type() == QEvent::MouseButtonRelease)))
    {
        btnDeleteSlot();
    }
    return QWidget::eventFilter(target,e);
}

void ReplayItemWidget::btnPlaySlot()
{
	QString prefixstr = m_item->m_titlename.endsWith("recv") ? "recv" : "send";
	QDir recv_dir(m_item->m_absoluteFilePath);
	recv_dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
	QFileInfoList fileEntryListrecv = recv_dir.entryInfoList();
	bool bexist = false;
	for (auto fileinforecv : fileEntryListrecv)
	{
		if (fileinforecv.isFile() && fileinforecv.fileName().endsWith(FILE_SUFFIX) && fileinforecv.fileName().startsWith(prefixstr))
		{
			bexist = true;
		}
	}
	if (bexist)
	{
		emit signalplay();
	}
}

void ReplayItemWidget::btnDeleteSlot()
{    

		QString prefixstr = m_item->m_titlename.endsWith("recv") ? "recv" : "send";
		QDir recv_dir(m_item->m_absoluteFilePath);
		recv_dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
		QFileInfoList fileEntryListrecv = recv_dir.entryInfoList();
		bool bexist = false;
		for (auto fileinforecv : fileEntryListrecv)
		{
			if (fileinforecv.isFile() && fileinforecv.fileName().endsWith(FILE_SUFFIX) && fileinforecv.fileName().startsWith(prefixstr))
			{
				bexist = true;
			}
		}
		if (bexist)
		{
			emit signaldelete();
		}
}
