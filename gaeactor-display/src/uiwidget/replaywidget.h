#ifndef REPLAYWIDGET_H
#define REPLAYWIDGET_H

#include <QWidget>
#include <QDebug>

#include "playwidget.h"
class QVBoxLayout;
class MapWidget;
class ReplayPanel;
class ReplayWidget : public QWidget
{
    Q_OBJECT

public:
	ReplayWidget(QWidget *parent = nullptr);
    ~ReplayWidget() override;

    PlayWidget* getPlayWidget();
private slots:
	void btn_click_slot(E_PLAY_OPERATE_TYPE type);
protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
private:
	PlayWidget *m_qmlWidget;
	MapWidget *m_mapWidget;
	ReplayPanel* m_pReplayPanel;
};

#endif // MAINWIDGET_H
