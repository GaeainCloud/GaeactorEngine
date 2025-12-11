#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QtOSGWidget;
class QTimer;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QtOSGWidget* m_pModelWidget;
    QTimer *m_ptimer;


    uint64_t agentid = 0;
    double lon = 0;
    double lat = 0;
    double hgt = 0;
    double roll = 0;
    double pitch = 0;
    double yaw = 0;


    struct tagTrajectoryItem
    {
        QString strtimestamp;
        uint64_t itimestamp;
        double lon;
        double lat;
        double hgt;
        double speed;
        double roll;
        double pitch;
        double yaw;
    };


    struct tagFightTrajectorys
    {
        QString filghtcode;
        std::list<std::vector<tagTrajectoryItem>> trajectorys;
    };
    std::unordered_map<QString, tagFightTrajectorys> m_FightTrajectorys;

    struct tagFlightVenet
    {
        uint64_t agentid = 0;
        QString filghtcode;
        const tagTrajectoryItem*ptagTrajectoryItem;
    };
    std::unordered_map<uint64_t, std::list<tagFlightVenet>> m_flights;


    std::unordered_map<uint64_t, std::list<uint64_t>> m_flights_trigger;

    uint64_t min_timestamp = 0xffffffffffffffff;
    uint64_t max_timestamp = 0;
    uint64_t start_itimeindex = 0;
};
#endif // MAINWINDOW_H
