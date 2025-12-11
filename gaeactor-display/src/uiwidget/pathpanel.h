#ifndef PATHPANEL_H
#define PATHPANEL_H

#include <QMap>
#include <QWidget>
#include <QAbstractListModel>
#include <QStyledItemDelegate>
#include <QStandardItemModel>

#include <QListView>
#include "playwidget.h"
class QPushButton;
class QListWidget;
class QListWidgetItem;
class QtMaterialProgress;
class QtMaterialSlider;
class QToolButton;

#include "../datamanager/datamanager.hpp"

class PathItemWidget;
class QListView;
struct tagPath_Plan_Item_Info
{
	bool bChecked;
	bool bUsaged;
	bool m_bArrType;
	tagPath_Plan* m_ptagPath_Plan;
};

class MyListView : public QListView
{
	Q_OBJECT
public:
	MyListView(QWidget *parent = nullptr) : QListView(parent) {}

signals:
	void refreshSig();
protected:
	virtual void scrollContentsBy(int dx, int dy) override
	{
		QListView::scrollContentsBy(dx, dy);
	}

	virtual void verticalScrollbarValueChanged(int value) override
	{
		emit refreshSig();
		QListView::verticalScrollbarValueChanged(value);
	}

	virtual void horizontalScrollbarValueChanged(int value) override
	{
		emit refreshSig();
		QListView::horizontalScrollbarValueChanged(value);
	}
};

class DataSrcListViewModel : public QAbstractListModel
{
	Q_OBJECT
public:

	explicit DataSrcListViewModel(QObject *parent = nullptr);
	~DataSrcListViewModel() {};
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual QVariant data(const QModelIndex &index, int role) const override;

	void clear();
	void add(tagPath_Plan* mapitem);
public:

	QList<tagPath_Plan_Item_Info> m_dataSourceList;
};



class DataSrcItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit DataSrcItemDelegate(DataSrcListViewModel *model, QListView* view, QObject *parent = nullptr);
	~DataSrcItemDelegate() override;

	PathItemWidget* getPathItemWidgetEdit();
	int currentEditRow();
protected:
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
	DataSrcListViewModel* m_model;
	QListView *m_ListViewCtrl;
	PathItemWidget* m_pPathItemWidget;
	PathItemWidget* m_pPathItemWidgetEdit;
	int m_currenteditrow;
};


class PathPanel : public QWidget
{
    Q_OBJECT
public:
	enum E_PATH_OPERATE_TYPE
	{
		E_PATH_OPERATE_TYPE_SELECT,
		E_PATH_OPERATE_TYPE_UNSELECT,
		E_PATH_OPERATE_TYPE_VISIABLE,
		E_PATH_OPERATE_TYPE_UNVISIABLE,
		E_PATH_OPERATE_TYPE_SELECT_POINT,
		E_PATH_OPERATE_TYPE_UNSELECT_POINT,
	};

	typedef std::function<void(E_PATH_OPERATE_TYPE bVisable,
		const QString&,
		const QString&,
		const QString&,
		const tagPath_Plan*)> visiable_update_callback;
	PathPanel(QWidget *parent = nullptr);
    virtual ~PathPanel() override;

	void initDataSlot(const QStringList& vallist1);

	void setVisableCallback(visiable_update_callback _pvisiable_update_callback);

protected:
    void initMember();
    void initUI();
    void initSignalSlot();
    void resizeUI();
	
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
protected slots:
    void slotVisiable(int index, tagPath_Plan *pathindex,bool bVisiable);

	void currentPointChangedSlot(int _index, tagPath_Plan *pathindex,const QString& current, bool bSelect);

protected:

	DataSrcListViewModel *m_model;
	DataSrcItemDelegate *m_delegate;

	MyListView * m_listView;
    QLabel *m_labIndex;
	QLabel *m_labVisable;
	QLabel *m_labContext;
    QLabel *m_labSize;
    QLabel *m_labDate;
	QLabel *m_labextendwpsnum;
	QLabel *m_labextendwpsrunwaynum;
	QLabel *m_labOperate;

	visiable_update_callback m_pvisiable_update_callback;

};



#endif // BASEBUTTONBAR_H
