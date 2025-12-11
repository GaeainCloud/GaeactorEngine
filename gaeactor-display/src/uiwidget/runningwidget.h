#ifndef RUNNINGWIDGET_H
#define RUNNINGWIDGET_H

#include <QWidget>
#include <QDebug>
#include "base_define.h"
#include "playwidget.h"

#include "transformdata_define.h"

struct tagPoiShowinfo
{
	UINT64 m_id;
	LAT_LNG m_pos;
	QString m_context;
	tagPoiShowinfo(const UINT64& _id, const LAT_LNG& _pos, const QString& _context)
		:m_id(_id),
		m_pos(_pos),
		m_context(_context)
	{
	}
};
class MapWidget;
class EventDriver;
class QComboBox;
class QToolButton;
class Dijkstra;
class RuntimeListWidget;
class QtOSGWidget;
class RunningWidget : public QWidget
{
    Q_OBJECT

public:
	RunningWidget(QWidget *parent = nullptr);
    ~RunningWidget() override;
	void updateText(const QString& context);
	void updateSpeed(const float fspeed);
	void setEventDriver(EventDriver *_peventDriver);
	QToolButton * creatToolButton(const QString& icon);
	void setSliderRange(int min, int max);
	void setSliderValue(uint64_t val);
	void addSliderValue(uint64_t val);
	void driverSimTracking();
	void addSliderValue_dt(double val);

    void setPlayClickSlot(bool bPlay);
    void setPauseClickSlot(bool bPause);

	void setRuntimeListWidget(RuntimeListWidget* pRuntimeListWidget);
	void setQtOSGWidget(QtOSGWidget* pModelWidget2);
	

public slots:
	void updatepoitextcolor_slot(const QString& poiname, const QColor& color, float textsize);
signals:
	void setSliderValue_sig(uint64_t val);
	void sim_displayHexidxPosCallback_sig(const TYPE_ULID &uildsrc, const transdata_entityposinfo& eninfo);
private slots:
	void setSliderValue_slot(uint64_t val);
	void btn_click_slot(E_PLAY_OPERATE_TYPE type);
	void select_airport_slot(const QString& airport_code);
protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
	void setPoiPoint(bool bVisable);

	void register_sim_tracking();
private:
	PlayWidget *m_qmlWidget;
	MapWidget *m_mapWidget;
	EventDriver *m_peventDriver;
	
#if 0
	QComboBox * m_srccombox;
	QToolButton * m_btnRefresh;
#endif
	bool m_bInit;
		
	std::unordered_map<QString, std::unordered_map<QString, tagPoiShowinfo> > m_poisinfo;

	//uint64_t m_play_min;
	//uint64_t m_play_max;
	//uint64_t m_play_range;
	//uint64_t m_play_cur;
	//uint64_t m_play_pause;

	//double m_play_cur_d;
	std::unordered_map<UINT64, std::tuple<int,int, std::vector<LAT_LNG>>> m_wps;

	RuntimeListWidget* m_pRuntimeListWidget;
	
};

#endif // MAINWIDGET_H
