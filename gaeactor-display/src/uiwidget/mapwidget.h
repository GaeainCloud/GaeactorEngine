#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#ifdef USING_QT_OPENGL_FUNCTION

#include "components/function.h"
#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QOpenGLFunctions>
#include <QOffscreenSurface>
#include <QOpenGLContext>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Compatibility>
#include <QOpenGLShaderProgram>
#include "widget2d/Shader.h"
#include "widget2d/maprender.h"
#include "head_define.h"
#include "snowflake.h"

#include "src/OriginalMutex.h"
#include "../datamanager/datamanager.hpp"
#include "./proto/protoc/AgentRelationInfo.pb.h"
#include "./proto/protoc/AgentPositionInfo.pb.h"
#include "./proto/protoc/AgentCommSnrInfo.pb.h"
#include "./proto/protoc/CommStackFrameResultElement.pb.h"
class QComboBox;
class QPushButton;
class QLabel;
class FunctionAssistant;
class GaeactorManager;
class GaeactorManagerHelper;
class EventDriver;
class MapWidget;
class MapRenderThread : public QThread, public QOpenGLFunctions
{
    Q_OBJECT

public:
    MapRenderThread(QSurface *surface, QOpenGLContext *mainContext, MapWidget *parent = nullptr);
    ~MapRenderThread();

    QMutex* getRendererMutex();

    void setNewSize(int width, int height);
    void stop();
    QOpenGLContext *renderContext() const;

signals:
    void imageReady();
protected:
    void run() override;

private:
    MapRenderThread(const MapRenderThread &) = delete;
    MapRenderThread &operator =(const MapRenderThread &) = delete;
    MapRenderThread(const MapRenderThread &&) = delete;
    MapRenderThread &operator =(const MapRenderThread &&) = delete;

private:
    bool m_running = true;

    int m_width = 100;
    int m_height = 100;
    QMutex _resizeMutex;

    QOpenGLContext *m_mainContext;
    QOpenGLContext *m_renderContext = nullptr;
    QSurface *m_surface;

    bool _setDefaultFboId;

    QMutex _renderOSGSceneMutex;

    bool _pause=false;
    QMutex mutex;
    QWaitCondition condition;

    MapWidget *m_pMapWidget;
};
namespace stdutils {
	class OriThread;
}
class QtOSGWidget;
//#define USING_PATH_GENERATE
class MapWidget : public QOpenGLWidget, public QOpenGLFunctions_3_3_Compatibility
{
    Q_OBJECT

public:
    enum E_MAP_MODE
    {
        E_MAP_MODE_DISPLAY,
		E_MAP_MODE_DISPLAY_REVIEW,
		E_MAP_MODE_SELECT,
		E_MAP_MODE_SELECT_PATH
	};
	enum E_DRAW_TYPE
	{
		E_DRAW_TYPE_POINTS,
		E_DRAW_TYPE_BOUNDARY_POLYGON,
		E_DRAW_TYPE_HEX_POLYGON
	};
    MapWidget(E_MAP_MODE eMapMode, QWidget *parent = nullptr);
    ~MapWidget() override;
    void render(QOpenGLContext *_renderContext);

	void clearGeoData();
	void drawGeoData(const GeoJsonInfos& geoinfos,bool bDrawExtendArea, bool bDetail, bool bDrawDynamic);

    void prepare();

    bool bTransfer() const;

	void updateText(const QString& context);

	void drawPoint(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, bool bClear = false);
	void drawText(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, bool bClear = false);
	void updateTextColor(const UINT64& item_id, const LAT_LNG& pos, const QString& context, float textsize, DrawElements::E_ELEMENT_TYPES eDrawType, const QColor& color);
	void drawTrackingLine(const quint64 &trackingid, const  std::vector<LAT_LNG> &waypts, const QColor& color, float linewidthScale = 1.0f);
	void drawRader(const UINT64& item_id,const LAT_LNG& center, const double& startangle, const double& spanangle, const double& radius, const double& rotationSpeed,QColor color= QColor(0, 255, 0, 255));
	void drawTrackingDashedLine(const quint64 &trackingid, const  std::vector<LAT_LNG> &waypts, const QColor& color, float linewidthScale = 1.0f);
	void drawTrackingPoints(const quint64 &trackingid, const  std::vector<LAT_LNG> &waypts, const QColor& color, float linewidthScale = 1.0f);


	void setQtOSGWidget(QtOSGWidget* pModelWidget2);
	void updateEntityItemSelect(UINT64 id, bool bSelect);
	void updateElementSelect(UINT64 id, bool bSelect, bool bClearOld = true);
	GaeactorManager* getGaeactorManager();

	void setEventDriver(EventDriver *peventDriver);

    void drawHex_ex(const TYPE_ULID &uildsrc, const HEXIDX_HGT_ARRAY& hexidxslist,const std::vector<QColor> &cls = std::vector<QColor>());
public slots:
	void sim_displayHexidxPosCallback_slot(const TYPE_ULID &uildsrc, const transdata_entityposinfo& eninfo);
signals:
    void btn_click_sig(int type);
	void select_airport_sig(const QString& airport_code);
private:
	void drawLatLngToHex(const std::vector<LAT_LNG> & originLatLngs, const QColor& color1, bool bDrawDynamic, int32_t res = -1);
	void data_deal_thread_func(void *pParam);
protected:
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void initializeGL() override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;


    virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
private:
    void displayHexidxCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
	void displayHexidxPosCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const transdata_entityposinfo& eninfo, E_DISPLAY_MODE eDdisplayMode);
	void displayIntersectionHexidxCallback(const TYPE_ULID &uildsrc, const TYPE_ULID &uilddst, const std::vector<std::tuple<TYPE_ULID, TYPE_ULID, H3INDEX>>& hexidxslistinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE &echowaveinfo, const HEXIDX_HGT_ARRAY &hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave);
    void dealeventlist_update_callback(const E_EVENT_MODE& eventmode,const std::vector<EVENT_INFO>& eventlist);
	void dealpath_update_callback(const TYPE_ULID & src, const TYPE_ULID & dst, const tagPathInfo& path);
    void dealsensor_update_callback(const TYPE_ULID & src, const E_EVENT_MODE & type);
    void dealAgentRelationInfo_update_callback(const ::msg::AgentRelationInfo::msg_AgentRelationInfo * agentrelationdata);
    void dealAgentCommSnrInfo_update_callback(const ::msg::AgentCommSnrInfo::msg_AgentCommSnrInfo * agentrelationdata);
    void dealAgentCommStackInfo_update_callback(const ::msg::CommStackFrameResultElement::msg_CommStackFrameResultElement * agentrelationdata);
    void dealSmdInfo_update_callback(const ::msg::AgentPositionInfo::msg_transdata_smd * smdinfodata);
    void dealPrejdugement_update_callback(const ::msg::AgentPositionInfo::msg_transprejusdgmentline * smdinfodata);

    void initRenderThread();

	QString  getEntityIcon(const TYPE_ULID &uildsrc);
private slots:
#ifdef USING_PATH_GENERATE
	void currentIndexChangedSlot(const QString& index);
#endif
	void requestEntityIconSlot(const TYPE_ULID &uildsrc);

private:
	void drawDataToUpdateColor(const E_EVENT_MODE& eventmode,const TYPE_ULID& uildsrc, const TYPE_SENSORINFO& sensorinfo, E_DRAW_TYPE drawType);
    void drawDataToHex(const TYPE_ULID& uildsrc, const TYPE_ULID& agentid, const TYPE_ULID& sensingmediaid, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DRAW_TYPE drawType, const std::unordered_map<H3INDEX,QColor>& HEXIDX_COLOR_LIST = std::unordered_map<H3INDEX, QColor>(), bool bFiill = true);
protected:


	glm::mat4 m_TransformProjectionMatrix;
	glm::mat4 m_TransformViewMatrix;
	glm::mat4 m_TransformModelMatrix;

    QTimer * m_pUpdateTimer;
    QTimer * m_pUpdateTimer2;

	int64_t m_ieventcount = 0;
	int64_t m_ieventcount_plus = 0;
	int64_t m_ieventcount_sub = 0;
    std::unordered_map<uint64_t, std::tuple<std::vector<LAT_LNG>, int>> m_trackingistitem;
#ifdef USING_PATH_GENERATE
	std::unordered_map < uint64_t, std::tuple<uint64_t, LAT_LNG> > m_node;
	QComboBox * m_srccombox;
	QComboBox * m_dstcombox;

	QLabel * m_textLabel;
	UInt64 m_routeid;
	UInt64 m_routesrcid;
	UInt64 m_routedstid;
#endif
	Snowflake m_snowflake;
	QComboBox * m_airportombox;

    std::unordered_map <EVENT_KEY_TYPE, std::tuple<int,uint64_t>> m_linepair;
    std::unordered_map <EVENT_KEY_TYPE, std::tuple<int,uint64_t>> m_linepair2;
private:
    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    std::unique_ptr<QOpenGLShaderProgram> m_program;
protected:
    MapRender *m_maprender;
	QOpenGLExtraFunctions  *m_pQOpenGLExtraFunctions;
    bool bPressed;

    QPoint m_lastpt;
    E_MAP_MODE m_eMapMode;

    Shader *mapshader = nullptr;
    Shader *m_lineelementshader = nullptr;
    Shader *m_trielementshader = nullptr;
    Shader *m_pTextshader = nullptr;
	GLuint  m_lineubouniform;
    GLuint  m_triubouniform;


    int m_w;
    int m_h;

    MapRenderThread *m_thread = nullptr;

	QVector<TYPE_ULID> m_spacesensorlist;
	std::unordered_map<QPair<TYPE_ULID, TYPE_ULID>, std::tuple<HEXIDX_ARRAY, POLYGON_LIST, TYPE_SENSORINFO, E_DISPLAY_MODE>> m_hexidxcallbackdata;
	std::unordered_map<uint64_t, DrawItem::ENUM_TYPE> m_geoDrawItems;
	std::vector<uint64_t> m_geoDrawEntityItems;

	GaeactorManagerHelper * m_pGaeactorManagerHelper;
	GaeactorManager* m_pGaeactorManager;
	QThread *m_GaeactorManager_thread;

	//std::unordered_map<QString, UINT64> m_poisinfo;


	stdutils::OriThread* m_hDataRequsetThread;
	stdutils::OriMutexLock m_dealmutex;
	stdutils::OriWaitCondition m_dealfullCond;

	QMutex m_requestListmutex;
	QList<TYPE_ULID> m_requestList;

	struct tagEntityRunningInfo 
	{
		TYPE_ULID speedline_id;
		LAT_LNG startpos;
		TIMESTAMP_TYPE starttimetamp;
		LAT_LNG lstpos;
		TIMESTAMP_TYPE lsttimetamp;
		TIMESTAMP_TYPE updateinterval;
		double totaldistance;
		double runningdistance;
		double runningspeed;
		double calcdistance;
		double calcspeed;
	};
	std::unordered_map<TYPE_ULID, tagEntityRunningInfo> m_entitySpeedLines;


	EventDriver *m_peventDriver;

	std::unordered_map<TYPE_ULID, QString> m_arr;

	std::unordered_map<TYPE_ULID, QString> m_dep;

	TYPE_ULID m_arr_text_id;
	TYPE_ULID m_dep_text_id;

    std::unordered_map<TYPE_ULID, LAT_LNG> m_entityPos;

	QtOSGWidget* m_pModelWidget2;

};
#else
#endif

#endif // MAINWINDOW_H
