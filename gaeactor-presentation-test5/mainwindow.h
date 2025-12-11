#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "components/function.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class Map2dWidget;
class Map2dEditWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void appendWaypoint_slot(quint64 waypointid, double lng, double lat);
    void updateWaypoint_slot(quint64 waypointid, double lng, double lat);
    void selectWaypoint_slot(quint64 waypointid);
private:

    Map2dWidget* m_pModelWidget;

    Map2dEditWidget* m_pModelWidget2;

    struct entity_waypoints_info{
        QColor cl;
        QString m_AgentKey;
        double m_azimuth;
        double m_speed0;
        int m_altitudeType;
        QString m_agentId;
        QString m_agentInstId;
        QString m_agentEntityId;
        QString m_agentLabel;
        QString m_agentNote;
        QString m_agentIcon;
        std::list<waypointinfo> m_waypts;
    };
    std::unordered_map<quint64, entity_waypoints_info> m_entitywaypointmap;
    quint64 m_currententityid;
};
#endif // MAINWINDOW_H
