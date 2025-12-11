#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QMap>
#include <QWidget>
#include <QDateTime>
#include "base_define.h"

#include "gaeactor_transmit_define.h"

typedef std::function<void(const E_CHANNEL_TRANSMITDATA_TYPE &, const BYTE*, const UINT32&, const BYTE*, const UINT32&)> review_callback;

class QPushButton;
class QLabel;
class QListWidget;
class QListWidgetItem;
class LoginInfoPanel;
class QtMaterialProgress;
class QtMaterialSlider;
class QToolButton;
class QComboBox;
namespace aosStream {
class BaseDevice;
}


struct tagReplayItemInfo
{
	int m_index;
	bool m_bSelected;
	quint64 m_size;
	QString m_date;
	QString m_filename;
	QString m_absoluteFilePath;
	QString m_titlename;
	QString m_titledir;
    QString m_simid;
	qint64 m_iTimeStampStart;
	qint64 m_iTimeStampEnd;
	quint64 m_iValidDataTotalLen;
	quint64 m_iFrames;

	quint64 m_startPos;
	quint64 m_startFrames;
	qint64 m_startPosMillseconds;
	tagReplayItemInfo()
	{
		m_size = 0;
		m_bSelected = false;
		m_date = "";
		m_filename = "";
		m_absoluteFilePath = "";
		m_titledir="";
		m_iTimeStampStart = 0;
		m_iTimeStampEnd = 0;
		m_iValidDataTotalLen = 0;
		m_iFrames = 0;
		m_startPos = 0;
		m_startFrames = 0;
		m_startPosMillseconds = 0;
	}

	qint64 getTotalMSecs() const
	{
		QDateTime startTime = QDateTime::fromMSecsSinceEpoch(m_iTimeStampStart);
		QDateTime endTime = QDateTime::fromMSecsSinceEpoch(m_iTimeStampEnd);
		qint64 msecs = startTime.msecsTo(endTime);
		return msecs;
	}
};

namespace originaldatastoragestd {
class OriginalDataInputManager;
}

enum E_PLAY_OPERATE_TYPE
{
	E_PLAY_OPERATE_TYPE_PLAY,
	E_PLAY_OPERATE_TYPE_TERMINATION,
	E_PLAY_OPERATE_TYPE_PASUE,
	E_PLAY_OPERATE_TYPE_RESUME,
	E_PLAY_OPERATE_TYPE_ADJUST_SPEED,
	E_PLAY_OPERATE_TYPE_ADJUST_PERCENT,
	E_PLAY_OPERATE_TYPE_REPLAY_LIST_SHOW,
	E_PLAY_OPERATE_TYPE_REPLAY_LIST_HIDE
};

class PlayWidget : public QWidget
{
    Q_OBJECT
public:
	enum E_PLAY_MODE
	{
		E_PLAY_MODE_REALTIME,
		E_PLAY_MODE_REVIEW,
	};
    PlayWidget(E_PLAY_MODE ePlayMode, QWidget *parent = nullptr);
    virtual ~PlayWidget() override;

	void setDataCallback(review_callback func);

	void setReadSpeedContext(float readSpeed);
	double getSpeedContext();
	void setSliderRange(int min, int max);
	void setSliderValue(uint64_t val);
	int getSliderAdjust();
	int getSliderCur();

    void setPlayClickSlot(bool bPlay);
    void setPauseClickSlot(bool bPause);
    originaldatastoragestd::OriginalDataInputManager *pOriginalDataInputManager() const;

    quint64 jumpToDataMillisecondPosOffsetPercent(double percent);
signals:
	void btn_click_sig(E_PLAY_OPERATE_TYPE type);
    void trigger_review_event_end_sig();
private:
    void setPressedSlider(bool bPressed);

    static QString formatTimeStr(qint64 msecs);

    QToolButton *speeddown() const;
    QToolButton *playctrl() const;
	QToolButton *pausectrl() const;

	void setReadSpeed(float readSpeed);
	float getReadSpeed();

	quint64 jumpToDataPos(quint64 iJumpDataPos);
	quint64 jumpToDataMillisecondPos(quint64 iJumpDataMillisecondPos);
    quint64 jumpToDataMillisecondPosOffset(quint64 iJumpDataMillisecondPosOffset);
	quint64 jumpToDataPosPercent(double percent);


public slots:
    void sendDataSlot(qint64 iGlobeFileReadValidDataPos,qint64 iDataSendTimeStamp);
	bool initializeReadFileSlot(tagReplayItemInfo* _currentitem);
protected:
    void initMember();
    void initUI();
    void initSignalSlot();
    void resizeUI();
	QIcon qiconFromSvg(QString svg_path);
    QToolButton * creatToolButton(const QString& icon);
protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
signals:
    void setEnableSignal(bool bEnable);

	void sendDataSignal(qint64 iGlobeFileReadBeginValidDataPos, qint64 iDataSendTimeStamp);
protected slots:
	void currentIndexChangedSlot(const QString& index);
    void btnSpeedClickSlot();
    void btnPlayClickSlot();
	void btnPauseClickSlot();
	void btnListClickSlot();

    void sliderPressedSlot();
	void sliderMovedSlot(int position);
	void sliderReleasedSlot();
private:


	bool data_callback(BYTE *pData, UINT32 iDataLen, TIMESTAMP_TYPE iTimeStamp, INT64 iGlobeFileReadValidDataPos, TIMESTAMP_TYPE iDataSendTimeStamp);
public:
    QLabel *m_labPlayTime;
    QLabel *m_labResidueTime;
    QLabel *m_labTotalTime;
    QtMaterialSlider       *m_slider;
    QLabel *_labDownloadProcessNum;
    quint64 m_iDataFrames;
    QDateTime m_payStart;
    QToolButton *m_speedup;
    QToolButton *m_speeddown;
    QToolButton *m_play;
	QToolButton *m_pause;
	QComboBox * m_cbx_speed;
	QToolButton *m_list;
	QLabel *m_labSpeed;
    bool m_bPressedSlider;
    qint64 iPrePackDataSendTime;

	bool m_playstatus;
	bool m_pausestatus;

	E_PLAY_MODE m_ePlayMode;

	tagReplayItemInfo* m_pCurrentitem;
	originaldatastoragestd::OriginalDataInputManager *m_pOriginalDataInputManager;
	review_callback m_reviewcallbackfunc;

	bool m_bShowList;

	int m_slider_min;
	int m_slider_max;
	int m_slider_cur;
	int m_slider_adjust;


    float m_pausespeed;

	std::unordered_map<double, int> m_speedindex;
};



#endif // BASEBUTTONBAR_H
