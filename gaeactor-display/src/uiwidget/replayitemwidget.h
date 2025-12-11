#ifndef DOWNLOADITEMWIDGET_H
#define DOWNLOADITEMWIDGET_H

#include <QWidget>
#define LOG_ITEM_HIGHT (48)
#define LOG_ITEM_WIDTH (540)
#include "playwidget.h"

class QLabel;
class QCheckBox;
class QLineEdit;
class QtMaterialProgress;
class QPushButton;
class ReplayItemWidget : public QWidget
{
	Q_OBJECT

public:
    ReplayItemWidget(QWidget *parent = Q_NULLPTR);
    ReplayItemWidget(tagReplayItemInfo* item, QWidget* parent = Q_NULLPTR);
	
    virtual ~ReplayItemWidget() override;
    tagReplayItemInfo *getItem();

    void hideCtrl(bool bHide);

private:
	void initMemeber();
    void initUI();
	void initSignalSlot();
    void resizeUI();
	QToolButton * creatToolButton(const QString& icon);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *target, QEvent *e) override;
signals:
    void signalplay();
    void signaldelete();
private slots:
    void btnPlaySlot();
    void btnDeleteSlot();
private:
	QLabel* labTitle;
    QLabel* labIndex;
    QLabel* labSize;
    QLabel* labDate;
    QLabel *_labDownloadProcessNum;
    QToolButton * m_btnPlay;
	QToolButton * m_btnDelete;
	tagReplayItemInfo* m_item;
};

#endif // BASEBUTTONBAR_H
