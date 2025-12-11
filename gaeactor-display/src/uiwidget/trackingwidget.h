#ifndef TRACKINGWIDGET_H
#define TRACKINGWIDGET_H

#include <QWidget>
#include <QDebug>
#include <vector>
#include "base_define.h"
#include "pathpanel.h"

class MapWidget;
class QComboBox;
class QLabel;

class TrackingWidget : public QWidget
{
    Q_OBJECT

public:
	TrackingWidget(QWidget *parent = nullptr);
    ~TrackingWidget() override;
private:
	QToolButton * creatToolButton(const QString& icon);

	void refreshTracking();
private slots:
	void visiableSlot(PathPanel::E_PATH_OPERATE_TYPE bVisable,
		const QString& parkingpoint,
		const QString& arr_dep_runway_str,
		const QString& runway,
		const tagPath_Plan* pathplaninfo);

	void select_airport_slot(const QString& airport_code);

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
private:
	MapWidget *m_mapWidget;

	std::unordered_map<QString, UINT64> m_poisinfo;

	struct tagTrackingInfo
	{
		UINT64 m_oriTrackingId;
		UINT64 m_extendTrackingId;
		UINT64 m_tracking_osm_path_infoId;
		UINT64 m_tracking_osm_path_infoId_calibrate;
		UINT64 m_tracking_runway_extendId;

		const tagPath_Plan* pathplaninfo;
		//std::vector<LAT_LNG> m_oriTrackingData;
		//std::vector<LAT_LNG> m_extendTrackingData;
		std::vector<LAT_LNG> m_tracking_osm_path_infoData;
		std::vector<LAT_LNG>	m_tracking_osm_path_infoId_calibrateData;
		QColor cl;
	};
	std::unordered_map<QString, tagTrackingInfo> m_trackingsinfo;
	bool m_bInit;

	QComboBox * m_srccombox;
	QComboBox * m_arrtype;
	QComboBox * m_runways;
	QComboBox * m_parkingpoint;
	QToolButton * m_btnRefresh;
	PathPanel* m_pPathPanel;
};

#endif // MAINWIDGET_H
