#ifndef PATHITEMWIDGET_H
#define PATHITEMWIDGET_H

#include <QWidget>
#include <QPainter>
#define LOG_ITEM_HIGHT (48)
#define LOG_ITEM_WIDTH (540)
#include "playwidget.h"

#include "../datamanager/datamanager.hpp"


#include <QStyledItemDelegate>

class QStandardItemModel;
class NoEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit NoEditDelegate(QObject *parent = nullptr)
		:QStyledItemDelegate(parent)
	{
	};
	~NoEditDelegate() override {};
protected:
	QWidget *createEditor(QWidget *parent,
		const QStyleOptionViewItem &option,
		const QModelIndex &index) const override
	{
		Q_UNUSED(parent)
			Q_UNUSED(option)
			Q_UNUSED(index)
			return nullptr;
	};
	// 重写绘制项的方法
	void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

class QLabel;
class QCheckBox;
class QLineEdit;
class QtMaterialProgress;
class QPushButton;
class QListView;
class PathItemWidget : public QWidget
{
	Q_OBJECT
	Q_PROPERTY(bool usabled READ getUsabled WRITE setUsabled)
	Q_PROPERTY(bool arrType READ getArrType WRITE setArrType)
	Q_PROPERTY(bool selectType READ getSelect WRITE setSelect)
		
public:
	PathItemWidget(QWidget *parent = Q_NULLPTR);
	

	void initadata(bool bChecked, tagPath_Plan * pathplaninfo, int index);
    virtual ~PathItemWidget() override;
	bool isVisiable();


	void setArrType(bool flag);
	bool getArrType() const;

	void setUsabled(bool flag);
	bool getUsabled() const;

	void setSelect(bool flag);
	bool getSelect() const;
private:
	void initMemeber();
    void initUI();
	void initSignalSlot();
    void resizeUI();

protected:
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void paintEvent(QPaintEvent *event) override;
    virtual bool eventFilter(QObject *target, QEvent *e) override;
signals:
	void signalVisiable(int _index, tagPath_Plan *pathindex, bool bVisiable);

	void currentPointChangedSig(int _index, tagPath_Plan *pathindex,const QString& current, bool bSelect);
private slots:
    void btnVisiableSlot();

	void clickedSlot(bool checked = false);
private:
	QLabel* labTitle;
    QLabel* labIndex;
    QLabel* labSize;
    QLabel* labDate;
	QLabel* m_labextendwpsnum;
	QLabel *m_labextendrunwaywpsnum;
    QCheckBox * m_btnPlay;
	QStandardItemModel *model;
	QListView  *m_listView;

	tagPath_Plan *m_pathindex;
	bool m_bArrType;
	int m_index;
	bool m_bselect;
};

#endif // BASEBUTTONBAR_H
