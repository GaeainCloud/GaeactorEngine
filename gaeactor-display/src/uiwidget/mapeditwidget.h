#ifndef MAPEDITWIDGET_H
#define MAPEDITWIDGET_H

#include "mapwidget.h"
class PickingObject;
class PickingTexture;
class MapEditWidget : public MapWidget
{
    Q_OBJECT

public:
    MapEditWidget(E_MAP_MODE eMapMode, QWidget *parent = nullptr);
    ~MapEditWidget() override;
    void remove_entity_waypoint(const quint64 &entityid, const quint64 &waypointid,const std::list<waypointinfo> &waypts,const QColor& color);
    void clear_entity_waypoint_tracking(const quint64 &entityid);
    void add_entity_waypoint_tracking(const quint64 &entityid,const QColor& color);
    void add_entity_waypoint(const quint64 &entityid, const quint64 &waypointid,const LAT_LNG& currentpos,const std::list<waypointinfo> &waypts,const QColor& color, bool bNeedTrans84GC = true);
    void locate_entity_waypoint(const quint64 &entityid, const quint64 &waypointid);
    void locate_entity_tracking(const quint64 &entityid);

	GeoJsonInfos addJsonFile(const QString& jsfilename);

signals:
    void appendWaypoint_sig(quint64 id, double lng, double lat);
    void updateWaypoint_sig(quint64 id, double lng, double lat);
    void selectWaypoint_sig(quint64 id);
public:
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
protected:
    virtual void paintGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void initializeGL() override;

private:
    void PickingPhase(int x, int y);
private:
    PickingObject* m_pPickingObject;
    PickingTexture* m_pPickingTexture;
    Shader *m_pickshader;

    bool m_bPressed;
    bool m_bCtrl;
    int m_select_x;
    int m_select_y;

    uint64_t m_seletitemid;
};
#endif // MAINWINDOW_H
