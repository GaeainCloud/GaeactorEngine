#pragma execution_character_set("utf-8")
#include "pathpanel.h"
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
#include "pathitemwidget.h"
#include <QCoreApplication>

#include "PathItemWidget.h"
#include <QAbstractListModel>
#include <QListView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QEvent>

#define  ROWHEIGHT 48

#include <QStandardItemModel>
PathPanel::PathPanel(QWidget *parent)
	: QWidget(parent)
{
	initMember();
	initUI();
	initSignalSlot();
	//initData();
}

PathPanel::~PathPanel()
{
	m_pvisiable_update_callback = nullptr;
}


void PathPanel::setVisableCallback(visiable_update_callback _pvisiable_update_callback)
{
	m_pvisiable_update_callback = std::move(_pvisiable_update_callback);
}


void PathPanel::initMember()
{
	m_labIndex = new QLabel(this);
	m_labVisable = new QLabel(this);
	m_labContext = new QLabel(this);
	m_labSize = new QLabel(this);
	m_labOperate = new QLabel(this);
	m_labDate = new QLabel(this);

	m_labextendwpsnum = new QLabel(this);
	m_labextendwpsrunwaynum = new QLabel(this);

	m_labIndex->setObjectName("lab_index");
	m_labVisable->setObjectName("lab_visable");
	m_labContext->setObjectName("lab_context");

	m_labDate->setObjectName("lab_date");
	m_labSize->setObjectName("lab_size");
	m_labOperate->setObjectName("lab_operate");
	m_labextendwpsnum->setObjectName("lab_wpsnum");
	m_labextendwpsrunwaynum->setObjectName("lab_runwaywpsnum");





	m_listView = new MyListView(this);
	

	m_model = new DataSrcListViewModel(this);
	// 设置数据模型
	m_listView->setModel(m_model);


	// 设置委托来禁止双击编辑
	m_delegate = new DataSrcItemDelegate(m_model, m_listView, this);
	m_listView->setItemDelegate(m_delegate);


	connect(m_listView, &MyListView::refreshSig, [&]() {
		m_delegate->getPathItemWidgetEdit()->hide();
		m_listView->update();
		QModelIndex modelindex = m_model->index(m_delegate->currentEditRow());
		emit m_model->dataChanged(modelindex, modelindex);
	});

}

void PathPanel::initUI()
{
	setVisible(true);
	m_labIndex->setText(tr("Index"));
	m_labVisable->setText(tr("Visiable"));
	m_labContext->setText(tr("Parking Point"));
	m_labSize->setText(tr("Arr OR Dep"));
	m_labDate->setText(tr("RunWay"));
	m_labextendwpsnum->setText(tr("Extend Wps Num"));
	m_labextendwpsrunwaynum->setText(tr("Extend Runway Num"));
	

	m_labOperate->setText(tr("Taxing Points"));


	m_labIndex->setAlignment(Qt::AlignCenter);
	m_labVisable->setAlignment(Qt::AlignCenter);
	m_labContext->setAlignment(Qt::AlignCenter);
	m_labSize->setAlignment(Qt::AlignCenter);
	m_labDate->setAlignment(Qt::AlignCenter);
	m_labOperate->setAlignment(Qt::AlignCenter);
	m_labextendwpsnum->setAlignment(Qt::AlignCenter);
	m_labextendwpsrunwaynum->setAlignment(Qt::AlignCenter);



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
	m_labVisable->show();
	m_labContext->show();
	m_labSize->show();
	m_labDate->show();
	m_labOperate->show();
	m_labextendwpsnum->show();
	m_labextendwpsrunwaynum->show();
}

void PathPanel::initSignalSlot()
{
	QObject::connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected) {
		for (const auto &index : deselected.indexes()) {
			if (index.row() < m_model->m_dataSourceList.size())
			{
				tagPath_Plan* ptagPath_Plan = m_model->m_dataSourceList[index.row()].m_ptagPath_Plan;
				if (m_model->m_dataSourceList[index.row()].bUsaged)
				{

					QString arr_dep_runway_str;
					if (ptagPath_Plan->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
					{
						arr_dep_runway_str = "出港";
					}
					else if (ptagPath_Plan->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
					{
						arr_dep_runway_str = "进港";
					}
					m_pvisiable_update_callback(E_PATH_OPERATE_TYPE_UNSELECT, ptagPath_Plan->m_parkingpoint,
						arr_dep_runway_str,
						ptagPath_Plan->m_runway,
						ptagPath_Plan);
				}
			}
		}
		for (const auto &index : selected.indexes()) {

			if (index.row() < m_model->m_dataSourceList.size())
			{
				tagPath_Plan* ptagPath_Plan = m_model->m_dataSourceList[index.row()].m_ptagPath_Plan;

				if (m_model->m_dataSourceList[index.row()].bUsaged)
				{
					QString arr_dep_runway_str;
					if (ptagPath_Plan->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
					{
						arr_dep_runway_str = "出港";
					}
					else if (ptagPath_Plan->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
					{
						arr_dep_runway_str = "进港";
					}
					m_pvisiable_update_callback(E_PATH_OPERATE_TYPE_SELECT, ptagPath_Plan->m_parkingpoint,
						arr_dep_runway_str,
						ptagPath_Plan->m_runway,
						ptagPath_Plan);
				}
			}
		}

	});

	connect(m_delegate->getPathItemWidgetEdit(), &PathItemWidget::signalVisiable, this, &PathPanel::slotVisiable);
	connect(m_delegate->getPathItemWidgetEdit(), &PathItemWidget::currentPointChangedSig, this, &PathPanel::currentPointChangedSlot);
}


void PathPanel::slotVisiable(int index ,tagPath_Plan *pathindex,bool bVisiable)
{
	m_model->m_dataSourceList[index].bChecked = bVisiable;
	
	QString arr_dep_runway_str;
	if (pathindex->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
	{
		arr_dep_runway_str = "出港";
	}
	else if (pathindex->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
	{
		arr_dep_runway_str = "进港";
	}
	m_pvisiable_update_callback(bVisiable ? E_PATH_OPERATE_TYPE_VISIABLE : E_PATH_OPERATE_TYPE_UNVISIABLE, pathindex->m_parkingpoint,
		arr_dep_runway_str,
		pathindex->m_runway,
		pathindex);

	if (bVisiable)
	{
		m_pvisiable_update_callback(E_PATH_OPERATE_TYPE_SELECT, pathindex->m_parkingpoint,
			arr_dep_runway_str,
			pathindex->m_runway,
			pathindex);
	}
}



void PathPanel::currentPointChangedSlot(int _index, tagPath_Plan *pathindex, const QString& current, bool bSelect)
{

	QString arr_dep_runway_str;
	if (pathindex->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
	{
		arr_dep_runway_str = "出港";
	}
	else if (pathindex->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
	{
		arr_dep_runway_str = "进港";
	}

	m_pvisiable_update_callback(bSelect ? E_PATH_OPERATE_TYPE_SELECT_POINT : E_PATH_OPERATE_TYPE_UNSELECT_POINT, current,
		arr_dep_runway_str,
		pathindex->m_runway,
		pathindex);

	if (_index < m_model->m_dataSourceList.size() && m_model->m_dataSourceList[_index].bChecked)
	{
		m_pvisiable_update_callback(E_PATH_OPERATE_TYPE_SELECT, pathindex->m_parkingpoint,
			arr_dep_runway_str,
			pathindex->m_runway,
			pathindex);
	}
}

void PathPanel::initDataSlot(const QStringList& vallist1)
{
	if (m_delegate->getPathItemWidgetEdit()->isVisiable())
	{
		m_delegate->getPathItemWidgetEdit()->hide();
		slotVisiable(m_delegate->currentEditRow(), m_model->m_dataSourceList[m_delegate->currentEditRow()].m_ptagPath_Plan, false);
		m_listView->update();
		QModelIndex modelindex = m_model->index(m_delegate->currentEditRow());
		emit m_model->dataChanged(modelindex, modelindex);
	}
	m_model->clear();
	tagAirPortInfo * ptagAirPortInfo = DataManager::getInstance().getCurrentAirportInfo();
	if (ptagAirPortInfo)
	{
		std::map<QString, ARR_DEP_RUNWAY_PATH>& pathplans = ptagAirPortInfo->m_Path_Plans;

		auto pathplans_itor = pathplans.begin();
		while (pathplans_itor != pathplans.end())
		{
			const QString& parkingpoint = pathplans_itor->first;
			ARR_DEP_RUNWAY_PATH & arr_dep_runway = pathplans_itor->second;
			if ((vallist1.at(3) == "all") || (vallist1.at(3) == parkingpoint))
			{
				auto arr_dep_runway_itor = arr_dep_runway.begin();
				while (arr_dep_runway_itor != arr_dep_runway.end())
				{
					QString arr_dep_runway_str;
					if (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_DEP)
					{
						arr_dep_runway_str = "出港";
					}
					else if (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_ARR)
					{
						arr_dep_runway_str = "进港";
					}
					if (vallist1.at(1) == "all" || ((vallist1.at(1) == "arr" && (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_ARR)) ||
						(vallist1.at(1) == "dep" && (arr_dep_runway_itor->first == E_FLIGHT_DEP_ARR_TYPE_DEP))))
					{
						RUNWAY_PATH & runway_path = arr_dep_runway_itor->second;
						auto runway_path_itor = runway_path.begin();
						while (runway_path_itor != runway_path.end())
						{
							const QString& runway = runway_path_itor->first;
							if (vallist1.at(2) == "all" || vallist1.at(2) == runway)
							{
								tagPathPlanInfo & pathplaninfo = runway_path_itor->second;
								if (vallist1.at(0) == "all")
								{
									m_model->add(&pathplaninfo.pathindex);
								}
								else if (vallist1.at(0) == "invalid")
								{
									if (pathplaninfo.pathindex.m_extendwpslatlng.empty())
									{
										m_model->add(&pathplaninfo.pathindex);
									}
								}
								else
								{
									if (!pathplaninfo.pathindex.m_extendwpslatlng.empty())
									{
										m_model->add(&pathplaninfo.pathindex);
									}
								}
							}

							runway_path_itor++;
						}

					}
					arr_dep_runway_itor++;
				}
			}

			pathplans_itor++;
		}
	}
}


void PathPanel::showEvent(QShowEvent *event)
{
	m_listView->setVisible(true);
	QWidget::showEvent(event);
}

void PathPanel::hideEvent(QHideEvent *event)
{
	m_listView->setVisible(false);
	QWidget::hideEvent(event);
}


void PathPanel::resizeUI()
{
	int w = width() / 18;
	m_labIndex->setGeometry(0, 5, w, 32);
	m_labContext->setGeometry(m_labIndex->geometry().right(), 5, w, 32);
	m_labVisable->setGeometry(m_labContext->geometry().right(), 5, w, 32);
	m_labSize->setGeometry(m_labVisable->geometry().right(), 5, w, 32);
	m_labDate->setGeometry(m_labSize->geometry().right(), 5, w, 32);
	m_labextendwpsnum->setGeometry(m_labDate->geometry().right(), 5, w, 32);
	m_labextendwpsrunwaynum->setGeometry(m_labextendwpsnum->geometry().right(), 5, w, 32);	
	m_labOperate->setGeometry(m_labextendwpsrunwaynum->geometry().right(), 5, w * 11, 32);
	//	m_listWidget->setGeometry(0, m_labOperate->geometry().bottom() + 1, this->width(), height() - 32 - 6);
	m_listView->setGeometry(0, m_labOperate->geometry().bottom() + 1, this->width(), height() - 32 - 6);

}

void PathPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	resizeUI();
	m_delegate->getPathItemWidgetEdit()->hide();
	m_listView->update();
	QModelIndex modelindex = m_model->index(m_delegate->currentEditRow());
	emit m_model->dataChanged(modelindex, modelindex);
}

void PathPanel::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}


DataSrcListViewModel::DataSrcListViewModel(QObject *parent /*= nullptr*/)
	:QAbstractListModel(parent)
{
	qRegisterMetaType<tagPath_Plan*>("tagPath_Plan*");
	qRegisterMetaType<tagPath_Plan>("tagPath_Plan");
};
int DataSrcListViewModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return m_dataSourceList.size();
};
QVariant DataSrcListViewModel::data(const QModelIndex &index, int role) const
{
	switch (role)
	{
	case Qt::SizeHintRole:
		return ROWHEIGHT;
	default:
		break;
	}
	return QVariant();
};

void DataSrcListViewModel::clear()
{
	m_dataSourceList.clear();
	//m_dataSourceList.append(item_info);
	//QModelIndex modelindex = this->index(m_dataSourceList.size() - 1);
	//emit dataChanged(modelindex, modelindex);
}

void DataSrcListViewModel::add(tagPath_Plan* mapitem)
{
	tagPath_Plan_Item_Info item_info;
	item_info.m_ptagPath_Plan = mapitem;
	item_info.bChecked = false;

	if (mapitem->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_DEP)
	{
		item_info.m_bArrType = false;
	}
	else if (mapitem->m_flight_dep_arr_type == E_FLIGHT_DEP_ARR_TYPE_ARR)
	{
		item_info.m_bArrType = true;
	}

	if (!mapitem->m_trackinglatlng.empty() &&
		!mapitem->m_tracking_osm_path_info.empty() &&
		!mapitem->m_tracking_osm_path_info_calibrate.empty() &&
		!mapitem->m_extendwpslatlng.empty())
	{
		item_info.bUsaged = true;
	}
	else
	{
		item_info.bUsaged = false;
	}

	m_dataSourceList.append(item_info);
	QModelIndex modelindex = this->index(m_dataSourceList.size() - 1);
	emit dataChanged(modelindex, modelindex);
}


DataSrcItemDelegate::DataSrcItemDelegate(DataSrcListViewModel *model, QListView* view, QObject *parent/* = nullptr*/)
	:QStyledItemDelegate(parent)
	, m_model(model)
	, m_ListViewCtrl(view)
{
	m_pPathItemWidget = new PathItemWidget(m_ListViewCtrl);
	m_pPathItemWidgetEdit = new PathItemWidget(m_ListViewCtrl);
	m_pPathItemWidget->hide();
	m_pPathItemWidgetEdit->hide();

	QObject::connect(m_ListViewCtrl->selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected) {
		for (const auto &index : deselected.indexes()) {
			//        emit currentPointChangedSig(index.data().toString(), false);
		}
		for (const auto &index : selected.indexes()) {
			m_currenteditrow = index.row();
			//        emit currentPointChangedSig(index.data().toString(), true);
		}

	});
}
DataSrcItemDelegate::~DataSrcItemDelegate()
{
}


PathItemWidget* DataSrcItemDelegate::getPathItemWidgetEdit()
{
	return m_pPathItemWidgetEdit;
}

int DataSrcItemDelegate::currentEditRow()
{
	return  m_currenteditrow;
}

bool DataSrcItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if (event->type() == QEvent::MouseButtonPress) {
		auto item = m_model->m_dataSourceList[index.row()];
		auto rc = option.rect;
		
		m_pPathItemWidget->setArrType(item.m_bArrType);
		m_pPathItemWidget->setUsabled(item.bUsaged);
		m_pPathItemWidget->setSelect(true);

		m_pPathItemWidgetEdit->initadata(item.bChecked, item.m_ptagPath_Plan, index.row());
		m_pPathItemWidgetEdit->resize(option.rect.width(), option.rect.height());
		QRect rect = option.rect;
		int x = rect.x();
		int y = rect.y();

		// 设置控件的位置并显示
		m_pPathItemWidgetEdit->move(x, y);
		m_pPathItemWidgetEdit->show();
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}
void DataSrcItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (m_currenteditrow == index.row() && m_pPathItemWidget->isVisiable())
	{
		return;// QStyledItemDelegate::paint(painter, option, index);
	}
	painter->save();
	auto item = m_model->m_dataSourceList[index.row()];
	auto rc = option.rect;
	m_pPathItemWidget->setSelect(false);

	m_pPathItemWidget->setArrType(item.m_bArrType);
	m_pPathItemWidget->setUsabled(item.bUsaged);

	m_pPathItemWidget->initadata(item.bChecked, item.m_ptagPath_Plan, index.row());
	m_pPathItemWidget->resize(option.rect.width(), option.rect.height());
	painter->drawPixmap(rc, m_pPathItemWidget->grab());

	painter->restore();



	//QStyledItemDelegate::paint(painter, option, index);

}
QSize DataSrcItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(m_ListViewCtrl->width(), 48);
}