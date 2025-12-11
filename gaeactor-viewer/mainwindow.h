#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QGeoView/QGVGlobal.h>
#include "LocationHelper.h"
#include "head_define.h"


#include "snowflake.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QGVMap;
class QGVWidgetText;
class QGVLayer;
class QGVWidget;
class QGVWidgetTools;
class EllipseItem;
class PieItem;
class RectangleItem;
class PolygonItem;
class LineItem;
class BaseItem;
class ImageItem;
class DataSrcListViewModel;
class DataSrcItemDelegate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum E_ENTITY_DISAPPEAR_TYPE
    {
        E_ENTITY_DISAPPEAR_TYPE_LOOP,
        E_ENTITY_DISAPPEAR_TYPE_END_AUTO_REMOVE,
        E_ENTITY_DISAPPEAR_TYPE_FOUND_REMOVE,
        E_ENTITY_DISAPPEAR_TYPE_FOUND_GUIDE,
        E_ENTITY_DISAPPEAR_TYPE_ENTIY_GUIDE,
    };

    enum E_SCRIPT_TYPE
    {
        E_SCRIPT_TYPE_NULL,
        E_SCRIPT_TYPE_STEP1,
        E_SCRIPT_TYPE_STEP2,
        E_SCRIPT_TYPE_STEP3,
        E_SCRIPT_TYPE_STEP4,
        E_SCRIPT_TYPE_STEP5,
    };
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private Q_SLOTS:
    void init();
    void currentIndexChangedSlot(const QString &);
    void onMouseMove(QPointF projPos);
    void onMousePress(QPointF projPos);
    void onMouseRelease(QPointF projPos);
    void onMouseDoubleClick(QPointF projPos);
    void onWheelEvent(QWheelEvent *event);

    void onKeyPressEvent(QKeyEvent *event);
    void onKeyReleaseEvent(QKeyEvent *event);
private:
    virtual void paintEvent(QPaintEvent *event) override;
private slots:
    void timeout_slot();
    void valueChangedSlot(int value);

    void on_pushButton_3_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_clicked();
    void on_pushButton_5_clicked();

    void updateDownloadProgressslot(int total,int reply,int zoom, int zoommax);

private:
    void initLayers();
    void initWidgets();
    void initgeomap();
    void startTracking(bool start);
    void enableAction(QGV::MouseAction action, bool enable);

    QGVMap* geoMap() const;
    QGV::GeoRect targetArea() const;

    QGV::GeoPos randPos(const QGV::GeoRect& targetArea);

    QGV::GeoRect randRect(const QGV::GeoRect& targetArea, const QSizeF& size);

    QGV::GeoRect randRect(const QGV::GeoRect& targetArea, int baseSize);
    QSizeF randSize(int baseSize);
    void generateHex(TYPE_ULID ulid, const QGV::GeoPos &geopos, const QVector<QGV::GeoPos> & pts, BaseItem* pMoveTrackingItem);
    void generateHexTmp(const QVector<QGV::GeoPos> & pts);


    void displayHexidxPosCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const transdata_entityposinfo& eninfo, E_DISPLAY_MODE eDdisplayMode);
    void displayHexidxCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const HEXIDX_HGT_ARRAY& hexidxslist, const POLYGON_LIST& polygonlist, const TYPE_SENSORINFO& sensorinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayIntersectionHexidxCallback(const TYPE_ULID &uildsrc,const TYPE_ULID &uilddst, const std::vector<std::tuple<TYPE_ULID,TYPE_ULID,H3INDEX>>& hexidxslistinfo, E_DISPLAY_MODE eDdisplayMode);
    void displayEchoWaveHexidxCallback(const TYPE_ULID &uildval, const EVENT_TUPLE &echowaveinfo, const HEXIDX_HGT_ARRAY &hexidxslist, const QVector<LAT_LNG> &geolatlnglist, bool bEchoWave);
    void dealeventlist_update_callback(const E_EVENT_MODE& eventmode,const std::vector<EVENT_INFO>& eventlist);
    void updateTrackingItems();
    void updateTrackingLineItems();

    void testCode();

    void step1();
    void step1_1();
    void step1_2();
    void step2();
    void step2_1();
    void step2_2();
    void step2_3();

    void dealFoundEvent(const EVENT_INFO &mapitem);


    void generateSensorFunc(const LAT_LNG &pos, const LAT_LNG *trackingpts, int ptcount, const QString &iconname, const QString &name, const QColor &color, double fspeed,double radius,E_ENTITY_DISAPPEAR_TYPE &disppeartype);
    void generateTrackingLineFunc(const LAT_LNG &pos, const LAT_LNG* trackingpts, int ptcount, const QString &iconname, const QString &name, const QColor &color, double fspeed,E_ENTITY_DISAPPEAR_TYPE &disppeartype);
    void generateentityFunc(const LAT_LNG &pos, const QString &iconname, const QString &name, const QColor &color, double fspeed,E_ENTITY_DISAPPEAR_TYPE &disppeartype);
    void updateLineLayerShow();
    void updateHexLayerShow();
    void updateSpeedCoeff();

    void updateDownloadLevel(QGVLayer* layer);
    void updateZoomLevel(QGVLayer* layer);

    void redis_callback(const char *channel, const char *data);
    void dealredisdata(std::string &data);

    void updateEntityImage(ImageItem *pImage,const TYPE_ULID &uildsrc);

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager* mManager;
    QNetworkAccessManager* mManager2;
    QNetworkDiskCache* mCache;
    QGVMap *m_geoMap;
    QGVWidgetText* mFooter;

    QList<QPair<QString, QGVLayer*>> m_layers;
    QList<QPair<QString, QGVWidget*>> m_widgets;

    QGVWidgetText* mText;
    QGVWidgetText* mText2;
    QGVWidgetText* mText3;
    QGVWidgetText* mText4;
    QGVWidgetText* mTextzoom;

    QGVLayer *m_pItemLayer;
    QGVLayer *m_pItemHexLayer;
    QGVLayer *m_pItemHexEchoLayer;
    QGVLayer *m_pItemIntersectionLayer;
    QGVLayer *m_pItemTmpLayer;
    QGVLayer *m_pLineLayer;
    QGVLayer *m_pEntityImageLayer;

    QGVWidgetTools * m_pQGVWidgetTools;

    EllipseItem*m_pCurrentEllipse;
    PieItem* m_pCurrentPie;
    LineItem* m_pCurrentLine;
    RectangleItem*m_pCurrentRectangle;
    PolygonItem*m_pCurrentPolygon;
    BaseItem* m_pMoveTrackingItem;

    QList<BaseItem*> m_TrackingItems;
    QList<LineItem*> m_TrackingLineItems;

    bool m_bSelectEnable;
    QPointF m_projPos_start;
    QPointF m_projPos_end;
    double m_distance;
    bool m_bdoubleclicked;
    int m_cellsNum;

    QTimer * m_pTimer;

    std::unordered_map<TYPE_ULID, QList<PolygonItem *>> m_polygonsIntersection;
    std::unordered_map<QPair<TYPE_ULID,TYPE_ULID>, QList<PolygonItem *>> m_polygons;
    std::unordered_map<TYPE_ULID, QList<std::tuple<ImageItem *, LineItem*>>> m_imageEntity;
    std::unordered_map<EVENT_KEY_TYPE, QList<PolygonItem *>> m_echowavepolygons;
    std::unordered_map<EVENT_KEY_TYPE, QList<PolygonItem *>> m_echowavegeopolygons;

    DataSrcListViewModel *m_pDataSrcListViewModel;
    DataSrcItemDelegate *m_pDataSrcItemDelegate;


    std::unordered_map<TYPE_ULID,std::tuple<QString,QString,E_ENTITY_DISAPPEAR_TYPE>> m_ulidicons;

    bool m_showTrackingLine;
    bool m_showHexLayer;

    std::unordered_map<TYPE_ULID,std::tuple<QGV::GeoPos,QList<LineItem*>>> m_targetfllowguide;

    QList<LineItem*> m_followItem;

    double m_speedcoeff;

    E_SCRIPT_TYPE  m_scriptstep = E_SCRIPT_TYPE_NULL;
    int m_interval = 3;
    bool m_bTransfer = true;

    std::unordered_map<uint64_t,std::tuple<QString,QString,QString,QString,QString>> m_actors;


    Snowflake m_snowflake;

    int64_t m_ieventcount = 0;
    int64_t m_ieventcount_plus = 0;
    int64_t m_ieventcount_sub = 0;
};
#endif // MAINWINDOW_H
