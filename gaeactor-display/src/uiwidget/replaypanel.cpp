#pragma execution_character_set("utf-8")
#include "replaypanel.h"
#include <QListWidgetItem>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDir>
#include <QDateTime>
#include <QPainter>
#include <QDebug>
#include <QToolButton>
#include <QThread>
#include <QMessageBox>
#include "replayitemwidget.h"
#include <QCoreApplication>
#include <QTimeZone>
#include "src/storage/OriginalDataInputManager.h"
#include "runningmodeconfig.h"

ReplayPanel::ReplayPanel(QWidget *parent)
    : QWidget(parent)
{
	qRegisterMetaType<tagReplayItemInfo*>("tagReplayItemInfo*");
    initMember();
    initUI();
    initSignalSlot();
}

ReplayPanel::~ReplayPanel()
{
    clearWidget();

    if(m_pThread)
    {
        m_pThread->quit();
        m_pThread->wait();
        m_pThread->deleteLater();
    }
}

void ReplayPanel::initMember()
{
    m_labIndex = new QLabel(this);
    m_labContext = new QLabel(this);
    m_labSize = new QLabel(this);
    m_labOperate = new QLabel(this);
    m_labDate = new QLabel(this);

    m_labIndex->setObjectName("lab_index");
    m_labContext->setObjectName("lab_context");

    m_labDate->setObjectName("lab_date");
    m_labSize->setObjectName("lab_size");
    m_labOperate->setObjectName("lab_operate");


    m_listWidget = new QListWidget(this);
    m_btnRefresh = creatToolButton("refresh.svg");

    m_listWidget->setMouseTracking(true);

    m_pThread = new QThread(this);
}

void ReplayPanel::initUI()
{
    setVisible(true);
    m_labIndex->setText(tr("Index"));
    m_labContext->setText(tr("logFile Name"));
    m_labSize->setText(tr("filesize"));
    m_labDate->setText(tr("Modifed Date"));
    m_labOperate->setText(tr("operate"));
    //m_btnRefresh->setText(tr("Refresh List"));
    m_listWidget->setFrameShape(QListWidget::NoFrame);


    m_labIndex->setAlignment(Qt::AlignCenter);
    m_labContext->setAlignment(Qt::AlignCenter);
    m_labSize->setAlignment(Qt::AlignCenter);
    m_labDate->setAlignment(Qt::AlignCenter);
    m_labOperate->setAlignment(Qt::AlignCenter);


	QFile file(QCoreApplication::applicationDirPath() + "./res/qss/playwidget.qss");
    bool res = file.open(QIODevice::ReadOnly);
    if (!res)
    {
        return;
    }

    const QString style = file.readAll();
    this->setStyleSheet(style);

    file.close();

	m_labIndex->show();
    m_labContext->show();
    m_labSize->show();
    m_labDate->show();
    m_labOperate->show();
    m_btnRefresh->show();
    m_listWidget->show();
}

void ReplayPanel::initSignalSlot()
{
    connect(m_btnRefresh, &QPushButton::clicked, this, &ReplayPanel::btnClickSlot);

    connect(m_pThread, &QThread::started, this,&ReplayPanel::dealData,Qt::DirectConnection);

    connect(this,&ReplayPanel::appenditemSignal,this,&ReplayPanel::appenditemSlot);
}

void ReplayPanel::loadFiles()
{
    if(runningmode::RunningModeConfig::getInstance().get_MODE_USING_939_MODE())
    {

        QString savedir = QCoreApplication::applicationDirPath() + "/OriginalData";


        bool bsend = false;
        QDir timedir(savedir);
        timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
        QFileInfoList fileEntryListtimedate = timedir.entryInfoList();
        QStringList ss = timedir.entryList();
        int index = 0;
        for (auto fileinfotimedate : fileEntryListtimedate)
        {
            if (fileinfotimedate.isDir())
            {
                QString  reocrdsavedir = fileinfotimedate.absoluteFilePath();

                QDir rootdir(reocrdsavedir);
                rootdir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                QFileInfoList fileEntryList = rootdir.entryInfoList();
                int index = 0;
                for (auto fileinfo : fileEntryList)
                {
                    if (fileinfo.isDir())
                    {
                        QString titledir = fileinfo.absoluteFilePath();
                        QDir timedir(titledir);
                        timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                        QFileInfoList fileEntryListtime = timedir.entryInfoList();
                        QStringList ss = timedir.entryList();
                        for (auto fileinfotime : fileEntryListtime)
                        {
                            if (fileinfotime.isDir())
                            {
                                QString recordname = fileinfo.fileName() + "_" + fileinfotime.fileName();
                                QString dir_send_str = fileinfotime.absoluteFilePath();
                                QDir dir_send(dir_send_str);
                                if (!dir_send.exists())
                                {
                                    fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                                    continue;
                                }

                                if (dir_send.exists())
                                {
                                    bsend = getDirFilesInfo(++index,titledir,fileinfo.fileName(), dir_send_str, false);
                                }
                                if (!bsend)
                                {
                                    clearDir(fileinfotime.absoluteFilePath());
                                    fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                                }
                                else
                                {
                                    index++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        QString savedir = QCoreApplication::applicationDirPath() + "/OriginalData";
        bool bsend = false;
        QDir timedir(savedir);
        timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
        QFileInfoList fileEntryListtimedate = timedir.entryInfoList();
        QStringList ss = timedir.entryList();
        int index = 0;
        for (auto fileinfotimedate : fileEntryListtimedate)
        {
            if (fileinfotimedate.isDir())
            {
                //202411251341
                QString  reocrdsavedir = fileinfotimedate.absoluteFilePath();

                QDir rootdir(reocrdsavedir);
                rootdir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                QFileInfoList fileEntryList = rootdir.entryInfoList();
                int index = 0;
                for (auto fileinfo : fileEntryList)
                {
                    if (fileinfo.isDir())
                    {
                        //default
                        QString titledir = fileinfo.absoluteFilePath();
                        QDir timedir(titledir);
                        timedir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
                        QFileInfoList fileEntryListtime = timedir.entryInfoList();
                        QStringList ss = timedir.entryList();
                        for (auto fileinfotime : fileEntryListtime)
                        {
                            if (fileinfotime.isDir() && fileinfotime.fileName().contains("reviewdata"))
                            {
                                QString recordname = fileinfo.fileName() + "_" + fileinfotime.fileName();
                                QString dir_send_str = fileinfotime.absoluteFilePath()+"/gaeactor_out";
                                qDebug()<<dir_send_str;
                                QDir dir_send(dir_send_str);
                                if (!dir_send.exists())
                                {
                                    fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                                    continue;
                                }

                                if (dir_send.exists())
                                {
                                    bsend = getDirFilesInfo_ex(++index, titledir,fileinfotimedate.fileName(),fileinfo.fileName(), dir_send_str, false);
                                }
                                if (!bsend)
                                {
                                    clearDir(fileinfotime.absoluteFilePath());
                                    fileinfotime.dir().rmdir(fileinfotime.absoluteFilePath());
                                }
                                else
                                {
                                    index++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

bool ReplayPanel::clearDir(QString path)
{
	if (path.isEmpty())
	{
		return false;
	}

	QDir dir(path);
	if (!dir.exists())
	{
		return false;
	}
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
	QFileInfoList fileList = dir.entryInfoList();
	for (auto file : fileList)
	{
		if (file.isFile())
		{
			file.dir().remove(file.fileName());
		}
		else
		{
			clearDir(file.absoluteFilePath());
			file.dir().rmdir(file.absoluteFilePath());
		}
	}

	return true;
}

bool ReplayPanel::getDirFilesInfo_ex(int index,const QString& titledir,const QString &datedir,const QString &srcparetndir, const QString &srcdir, bool bRecv)
{
    QString prefixstr = (bRecv) ? "Recv" : "Send";

    //QString srcdir = QDir::currentPath() + srcobj.value("RecordFilePath" + prefixstr).toString();
    tagReplayItemInfo *item = new tagReplayItemInfo();
    item->m_index = index;
    QStringList running_info = datedir.split("-");
    QString running_date = running_info.at(0);
    QString running_id = running_info.at(1);
    QDateTime datetime = QDateTime::fromString(running_date, "yyyyMMddhhmm");

    datetime.setTimeSpec(Qt::UTC);
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
    QDateTime localTime = datetime;
#else /* UNIX */
    //    // 获取本地时区
    //    //QTimeZone localTimeZone = QTimeZone::systemTimeZone();
    //    // 设置时区偏移量为+8:00
    //    QTimeZone localTimeZone = QTimeZone("Asia/Shanghai");

    //    // 将UTC时间转换为本地时间
    //    QDateTime localTime = datetime.toTimeZone(localTimeZone);
    QDateTime localTime = datetime.toOffsetFromUtc(8 * 60 * 60);
#endif

    item->m_date = localTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    item->m_absoluteFilePath = srcdir;
    item->m_titlename = srcparetndir+"_"+running_date;
    item->m_titledir = titledir;
    item->m_simid = running_id;
    //item.m_startPos = srcobj.value("RecordCountDataSize" + prefixstr).toString().toULongLong();
    //item.m_startFrames = srcobj.value("RecordCountFrameCount" + prefixstr).toString().toULongLong();

    QDir recv_dir(srcdir);
    recv_dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
    QFileInfoList fileEntryListrecv = recv_dir.entryInfoList();
    bool bexist = false;
    for (auto fileinforecv : fileEntryListrecv)
    {
        if (fileinforecv.isFile() && fileinforecv.fileName().endsWith(FILE_SUFFIX) && fileinforecv.fileName().startsWith(prefixstr.toLower()))
        {
            item->m_size += fileinforecv.size();
            originaldatastoragestd::tagOriginalDataFileHeadRecordInfo recordInfo;
            if (originaldatastoragestd::OriginalDataInputManager::getFileTimeStamp(fileinforecv.absoluteFilePath().toStdString().c_str(), recordInfo))
            {
                item->m_iValidDataTotalLen += recordInfo.getCurrentFileValidDataMaxLen();
                item->m_iFrames += recordInfo.getCurrentFileFrames();
                if (0 == item->m_iTimeStampStart)
                {
                    item->m_iTimeStampStart = recordInfo.m_iallTimeStampStart;
                }
                item->m_iTimeStampEnd = recordInfo.m_iallTimeStampEnd;
                bexist = true;
            }
        }
    }
    if (bexist)
    {
        emit appenditemSignal(item);
    }
    else
    {
        delete  item;
    }
    return bexist;
}


bool ReplayPanel::getDirFilesInfo(int index,const QString& titledir,const QString &srcparetndir, const QString &srcdir, bool bRecv)
{
	QString prefixstr = (bRecv) ? "Recv" : "Send";

	//QString srcdir = QDir::currentPath() + srcobj.value("RecordFilePath" + prefixstr).toString();
	tagReplayItemInfo *item = new tagReplayItemInfo();
	item->m_index = index;
	QDateTime datetime = QDateTime::fromString(srcparetndir, "yyyyMMddhhmmsszzz");

    datetime.setTimeSpec(Qt::UTC);
#if (defined(_WIN32)||defined(_WIN64)) /* WINDOWS */
    QDateTime localTime = datetime;
#else /* UNIX */
    //    // 获取本地时区
    //    //QTimeZone localTimeZone = QTimeZone::systemTimeZone();
    //    // 设置时区偏移量为+8:00
    //    QTimeZone localTimeZone = QTimeZone("Asia/Shanghai");

    //    // 将UTC时间转换为本地时间
    //    QDateTime localTime = datetime.toTimeZone(localTimeZone);
    QDateTime localTime = datetime.toOffsetFromUtc(8 * 60 * 60);
#endif

	item->m_date = localTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    item->m_absoluteFilePath = srcdir;
    item->m_titlename = srcparetndir;
	item->m_titledir = titledir;
	//item.m_startPos = srcobj.value("RecordCountDataSize" + prefixstr).toString().toULongLong();
	//item.m_startFrames = srcobj.value("RecordCountFrameCount" + prefixstr).toString().toULongLong();

	QDir recv_dir(srcdir);
	recv_dir.setFilter(QDir::NoDotAndDotDot | QDir::AllEntries);
	QFileInfoList fileEntryListrecv = recv_dir.entryInfoList();
	bool bexist = false;
	for (auto fileinforecv : fileEntryListrecv)
	{
		if (fileinforecv.isFile() && fileinforecv.fileName().endsWith(FILE_SUFFIX) && fileinforecv.fileName().startsWith(prefixstr.toLower()))
		{
			item->m_size += fileinforecv.size();
			originaldatastoragestd::tagOriginalDataFileHeadRecordInfo recordInfo;
			if (originaldatastoragestd::OriginalDataInputManager::getFileTimeStamp(fileinforecv.absoluteFilePath().toStdString().c_str(), recordInfo))
			{
				item->m_iValidDataTotalLen += recordInfo.getCurrentFileValidDataMaxLen();
				item->m_iFrames += recordInfo.getCurrentFileFrames();
				if (0 == item->m_iTimeStampStart)
				{
					item->m_iTimeStampStart = recordInfo.m_iallTimeStampStart;
				}
				item->m_iTimeStampEnd = recordInfo.m_iallTimeStampEnd;
				bexist = true;
			}
		}
	}
	if (bexist)
	{
		emit appenditemSignal(item);
	}
	else
	{
		delete  item;
	}
	return bexist;
}

void ReplayPanel::requestDealData()
{
    if (m_pThread->isRunning() == true)
    {
        m_pThread->quit();
        m_pThread->wait();
        m_pThread->start();
    }
    else
    {
        m_pThread->start();
    }
}

void ReplayPanel::btnClickSlot(bool checked/* = false*/)
{
    clearWidget();
    requestDealData();
}


void ReplayPanel::dealData()
{
    loadFiles();
}


void ReplayPanel::appenditemSlot(tagReplayItemInfo *item)
{
    appendItem(item);
}


void ReplayPanel::slotplay()
{
	ReplayItemWidget* pReplayItemWidget = qobject_cast<ReplayItemWidget*>(sender());
	if (pReplayItemWidget)
	{
		tagReplayItemInfo* currentitem = pReplayItemWidget->getItem();

		if (currentitem)
		{
			emit initializeReadFileSig(currentitem);
			this->hide();
		}
	}
}

void ReplayPanel::slotdelete()
{
    ReplayItemWidget* pReplayItemWidget= qobject_cast<ReplayItemWidget*>(sender());
    if(pReplayItemWidget)
    {
        QMessageBox::StandardButton res = QMessageBox::question(nullptr, tr("tips"), tr("make sure delete item"));
        if (res == QMessageBox::No)
		{
			return;
		}
        const tagReplayItemInfo* currentitem  = pReplayItemWidget->getItem();

        if(currentitem)
        {
            QDir dir(currentitem->m_absoluteFilePath);
            clearDir(currentitem->m_absoluteFilePath);
            dir.rmdir(currentitem->m_absoluteFilePath);
        }
        clearWidget();
        requestDealData();
    }
}




void ReplayPanel::clearWidget()
{
    QList<std::tuple<ReplayItemWidget*, QListWidgetItem*, tagReplayItemInfo*>>::iterator itor = m_nodeWidgetMap.begin();
    while(itor != m_nodeWidgetMap.end())
    {
		std::tuple<ReplayItemWidget*, QListWidgetItem*, tagReplayItemInfo*> &tuple = *itor;
        ReplayItemWidget* pItemWidget = std::get<0>(tuple);
        if(pItemWidget)
        {
            disconnect(pItemWidget, &ReplayItemWidget::signalplay, this, &ReplayPanel::slotplay);
            disconnect(pItemWidget, &ReplayItemWidget::signaldelete, this, &ReplayPanel::slotdelete);

            pItemWidget->deleteLater();
        }
        QListWidgetItem* pQListWidgetItem = std::get<1>(tuple);
        if(pQListWidgetItem)
        {
            delete pQListWidgetItem;
        }

		tagReplayItemInfo* pInfoItem = std::get<2>(tuple);
        if(pInfoItem)
        {
            delete pInfoItem;
        }
        itor++;
    }
    m_nodeWidgetMap.clear();
    m_listWidget->clear();
}

QToolButton *ReplayPanel::creatToolButton(const QString &icon)
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


void ReplayPanel::appendItem(tagReplayItemInfo *item)
{
    QListWidgetItem* pQListWidgetItem = new QListWidgetItem(m_listWidget);

    ReplayItemWidget* pItemWidget = new ReplayItemWidget(item, this);

    connect(pItemWidget, &ReplayItemWidget::signalplay, this, &ReplayPanel::slotplay);
    connect(pItemWidget, &ReplayItemWidget::signaldelete, this, &ReplayPanel::slotdelete);
    m_listWidget->addItem(pQListWidgetItem);
    m_listWidget->setItemWidget(pQListWidgetItem,pItemWidget);
    m_nodeWidgetMap.push_back(std::make_tuple(pItemWidget,pQListWidgetItem,item));
}

void ReplayPanel::resizeUI()
{
    m_labIndex->setGeometry(0, 24+22, width()/6, 32);
    m_labContext->setGeometry(m_labIndex->geometry().right(), 24+22, width()/6, 32);
    m_labSize->setGeometry(m_labContext->geometry().right(), 24+22, width()/6, 32);
    m_labDate->setGeometry(m_labSize->geometry().right(), 24+22, width()*2/6, 32);
    m_labOperate->setGeometry(m_labDate->geometry().right(), 24+22, width()/6, 32);
    m_listWidget->setGeometry(0, m_labOperate->geometry().bottom()+1, this->width(), height()-90-(m_labOperate->geometry().bottom()+1));
	m_btnRefresh->setGeometry(width() / 2 - 36 / 2, m_listWidget->geometry().bottom()+5, 36, 36);
}

void ReplayPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeUI();
}

void ReplayPanel::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}




