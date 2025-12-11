#ifndef REPLAYPANEL_H
#define REPLAYPANEL_H

#include <QMap>
#include <QWidget>

#include "playwidget.h"
class QPushButton;
class QLabel;
class QListWidget;
class QListWidgetItem;
class LoginInfoPanel;
class QtMaterialProgress;
class QtMaterialSlider;
class QToolButton;
class QThread;
class ReplayItemWidget;

class ReplayPanel : public QWidget
{
    Q_OBJECT
public:
    ReplayPanel(QWidget *parent = nullptr);
    virtual ~ReplayPanel() override;
signals:
	bool initializeReadFileSig(tagReplayItemInfo* _currentitem);
protected:
    void initMember();
    void initUI();
    void initSignalSlot();
    void resizeUI();

    void appendItem(tagReplayItemInfo* item);


    void clearWidget();

    QToolButton * creatToolButton(const QString& icon);

    bool clearDir(QString path);
    void requestDealData();

    void loadFiles();
    bool getDirFilesInfo_ex(int index,const QString& titledir,const QString &datedir,const QString &srcparetndir, const QString &srcdir, bool bRecv);
    bool getDirFilesInfo(int index, const QString &titledir, const QString &srcparetndir, const QString &srcdir, bool bRecv);
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
signals:
	void appenditemSignal(tagReplayItemInfo *item);
protected slots:
    void btnClickSlot(bool checked = false);
    void slotplay();
    void slotdelete();

    void dealData();

    void appenditemSlot( tagReplayItemInfo *item);
protected:
    QListWidget *m_listWidget;
	QToolButton * m_btnRefresh;
    QLabel *m_labIndex;
    QLabel *m_labContext;
    QLabel *m_labSize;
    QLabel *m_labDate;
    QLabel *m_labOperate;
	QList<std::tuple<ReplayItemWidget*, QListWidgetItem*, tagReplayItemInfo*>> m_nodeWidgetMap;
    QThread *m_pThread; // 线程处理数据
};



#endif // BASEBUTTONBAR_H
