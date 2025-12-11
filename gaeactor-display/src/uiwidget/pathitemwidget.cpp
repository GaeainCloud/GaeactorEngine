#pragma execution_character_set("utf-8")
#include "pathitemwidget.h"
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
#include <QListView>
#include <QPainter>

#include <QStringListModel>
#include <QStandardItemModel>

PathItemWidget::PathItemWidget( QWidget *parent)
	: QWidget(parent),
	m_bArrType(false),
	m_bselect(false)
{
	this->setAttribute(Qt::WA_TranslucentBackground, true);
	initMemeber();
	initUI();
	initSignalSlot();


	model = new QStandardItemModel(this);
	// ��������ģ��
	m_listView->setModel(model);

	// ����ί������ֹ˫���༭
	NoEditDelegate *delegate = new NoEditDelegate(this);
	m_listView->setItemDelegate(delegate);

	QObject::connect(m_listView->selectionModel(), &QItemSelectionModel::selectionChanged, [&](const QItemSelection &selected, const QItemSelection &deselected) {
		for (const auto &index : deselected.indexes()) {
			emit currentPointChangedSig(m_index-1, m_pathindex,index.data().toString(), false);
		}
		for (const auto &index : selected.indexes()) {
			emit currentPointChangedSig(m_index-1, m_pathindex,index.data().toString(), true);
		}

	});
}


void PathItemWidget::initadata(bool bChecked, tagPath_Plan * pathplaninfo, int index)
{
	m_index = index+1;
	m_pathindex = pathplaninfo;
	this->m_btnPlay->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
	this->labIndex->setText(QString::number(m_index));

	this->labTitle->setText(pathplaninfo->m_parkingpoint);


    this->labSize->setText(pathplaninfo->m_flight_dep_arr_type ==  E_FLIGHT_DEP_ARR_TYPE_DEP ? "出发":"到达");
	this->labDate->setText(pathplaninfo->m_runway);

	this->m_labextendwpsnum->setText(QString::number(pathplaninfo->m_extendwpslatlng.size()));

	this->m_labextendrunwaywpsnum->setText(QString::number(pathplaninfo->m_runway_total.size()));
	


	for (int i = 0; i < model->rowCount(); i++)
	{
		QStandardItem * item = model->takeItem(i);
		if (item)
		{
			delete item;
		}
	}
	model->clear();

	for (int i = 0; i < pathplaninfo->m_pathPoints.size(); i++)
	{
		const QString &itemdata = std::get<0>(pathplaninfo->m_pathPoints.at(i));
		const bool& bValid = std::get<1>(pathplaninfo->m_pathPoints.at(i));
		QStandardItem* item = new QStandardItem(itemdata);
		item->setData(bValid);
		model->appendRow(item);
	}

	m_btnPlay->setToolTip(tr("Play"));
}

PathItemWidget::~PathItemWidget()
{
}

bool PathItemWidget::isVisiable()
{
	Qt::CheckState state = m_btnPlay->checkState();
	return state == Qt::Checked ? true : false;
}

void PathItemWidget::initMemeber()
{
	labIndex = new QLabel(this);
	labIndex->setObjectName("lab_index");
	labTitle = new QLabel(this);
	labTitle->setObjectName("lab_context");

	labSize = new QLabel(this);
	labSize->setObjectName("lab_labSize");

	labDate = new QLabel(this);
	labDate->setObjectName("lab_labDate");

	m_labextendwpsnum = new QLabel(this);
	m_labextendwpsnum->setObjectName("lab_extendwpsnum");

	m_labextendrunwaywpsnum = new QLabel(this);
	m_labextendrunwaywpsnum->setObjectName("lab_extendrunwaywpsnum");



	m_btnPlay = new QCheckBox(this);
	//m_btnPlay->installEventFilter(this);
	connect(m_btnPlay, &QCheckBox::clicked, this, &PathItemWidget::clickedSlot);
	m_listView = new QListView(this);
	m_listView->setLayoutMode(QListView::Batched);

	// ������ʽ����Ϊ����
	m_listView->setFlow(QListView::LeftToRight);
	m_listView->setIconSize(QSize(120, LOG_ITEM_HIGHT));


	QFile file(QCoreApplication::applicationDirPath() + "./res/qss/playwidget.qss");
	bool res = file.open(QIODevice::ReadOnly);
	if (!res)
	{
		return;
	}

	QString style = file.readAll();

	this->setStyleSheet(style);

	file.close();
}

void PathItemWidget::initUI()
{
	resize(LOG_ITEM_WIDTH, LOG_ITEM_HIGHT);

	labIndex->setAlignment(Qt::AlignCenter);
	labTitle->setAlignment(Qt::AlignCenter);
	labSize->setAlignment(Qt::AlignCenter);
	labDate->setAlignment(Qt::AlignCenter); 
	m_labextendwpsnum->setAlignment(Qt::AlignCenter); 
	m_labextendrunwaywpsnum->setAlignment(Qt::AlignCenter);
}

void PathItemWidget::initSignalSlot()
{
}

void PathItemWidget::resizeUI()
{
	int w = width() / 18;
	labIndex->setGeometry(0, 0, w, height());
	labTitle->setGeometry(labIndex->geometry().right(), 0, w, height());
	m_btnPlay->setGeometry(labTitle->geometry().right() + (w / 2 - 40 / 2), height() / 2 - 40 / 2, 40, 40);
	labSize->setGeometry(labTitle->geometry().right() + w, 0, w, height());
	labDate->setGeometry(labSize->geometry().right(), 0, w, height());
	m_labextendwpsnum->setGeometry(labDate->geometry().right(), 0, w, height());
	m_labextendrunwaywpsnum->setGeometry(m_labextendwpsnum->geometry().right(), 0, w, height());
	m_listView->setGeometry(m_labextendrunwaywpsnum->geometry().right(), 0, w * 11, height());
}


void PathItemWidget::setArrType(bool flag)
{
	m_bArrType = flag;
	this->style()->unpolish(this);
	this->style()->polish(this);
}

bool PathItemWidget::getArrType() const
{
	return m_bArrType;
}

void PathItemWidget::setUsabled(bool flag)
{
	m_btnPlay->setEnabled(flag);
	this->style()->unpolish(this);
	this->style()->polish(this);
}

bool PathItemWidget::getUsabled() const
{
	return m_btnPlay->isEnabled();
}

void PathItemWidget::setSelect(bool flag)
{
	m_bselect = flag;
	this->style()->unpolish(this);
	this->style()->polish(this);
}

bool PathItemWidget::getSelect() const
{
	return m_bselect;
}

void PathItemWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	resizeUI();
}



void PathItemWidget::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);

	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

bool PathItemWidget::eventFilter(QObject *target, QEvent *e)
{
	if (target == m_btnPlay && ((e->type() == QEvent::MouseButtonRelease)))
	{
		btnVisiableSlot();
	}
	return QWidget::eventFilter(target, e);
}

void PathItemWidget::btnVisiableSlot()
{
	Qt::CheckState state = m_btnPlay->checkState();
	emit signalVisiable(m_index-1, m_pathindex, state == Qt::Checked ? true : false);
}

void PathItemWidget::clickedSlot(bool checked /*= false*/)
{
	btnVisiableSlot();
}

void NoEditDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	// ��ȡ�������
	QVariant value = index.data(Qt::DisplayRole);

	bool bValid = index.data(Qt::UserRole + 1).toBool();

	if (bValid) {
		// �ж����Ƿ�ѡ��
		bool selected = option.state & QStyle::State_Selected;

		painter->setPen(QColor(255, 255, 255, 255));
		// ������ı�����ɫ
		if (selected) {
			painter->fillRect(option.rect, QColor(255, 0, 0, 128)); // ����ѡ����ı���Ϊ��ɫ
		}
		else {
			painter->fillRect(option.rect, QColor(0, 255, 0, 76)); // ���÷�ѡ����ı���Ϊ��ɫ
		}
	}
	else 
	{
		painter->setPen(QColor(0, 0, 0, 255));
		painter->fillRect(option.rect, QColor(128, 128, 128, 255 * 0.3)); // ����ѡ����ı���Ϊ��ɫ
	}


	// ��������ı�
	painter->drawText(option.rect, Qt::AlignCenter, value.toString());
}
