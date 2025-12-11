#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "gaeactor_transmit_interface.h"
#include "src/OriginalDynamicBuffer.h"
#include <QGeoView/QGVGlobal.h>
#include "transformdata_define.h"
#include "snowflake.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
namespace iox {
namespace popo {
class UntypedPublisher;
class Listener;
class UntypedSubscriber;
}
namespace runtime {
class PoshRuntime;
}
}
namespace gaeactortransmit {
class GaeactorTransmit;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_pushButton_5_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void dealentityHexidex(const CHANNEL_INFO *channelinfo, TYPE_ULID ulid, const LAT_LNG& pos, INT32 alt, FLOAT32 roll, FLOAT32 pitch, FLOAT32 yaw, bool bSensor, bool bClear = false);
    void clearentityHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID ulid);
    void dealHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID ulid, const LAT_LNG& pos, const HEXIDX_ARRAY& hexidxslist, const std::vector<transdata_param_seq_polygon>& _polygon, int slient_time_gap,bool bClear = false);
    void clearHexidex(const CHANNEL_INFO *channelinfo,TYPE_ULID ulid);


    QGV::GeoPos randPos(const QGV::GeoRect& targetArea);

    QGV::GeoRect randRect(const QGV::GeoRect& targetArea, const QSizeF& size);

    QGV::GeoRect randRect(const QGV::GeoRect& targetArea, int baseSize);
    QGV::GeoRect targetArea() const;
    QGV::GeoRect targetArea2() const;
    QSizeF randSize(int baseSize);
    void on_pushButton_4_clicked();

    void on_pushButton_7_clicked();

private:
    void receive_callback(const E_CHANNEL_TRANSMITDATA_TYPE &channelTransmitDataType, const BYTE*pdata, const UINT32& ilen, const BYTE *pOrignaldata, const UINT32 &iOrignallen);

private slots:
    void timeout_slot();

    void on_pushButton_6_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_lineEdit_5_editingFinished();

private:
    Ui::MainWindow *ui;

    QList<std::tuple<std::string, CHANNEL_INFO*>>  m_eventstrlist;

    std::tuple<std::string, CHANNEL_INFO*> m_eventstr;
    iox::runtime::PoshRuntime* m_pRuntime;


    QVector<std::tuple<std::tuple<std::string, CHANNEL_INFO*>,TYPE_ULID, QGV::GeoPos, QGV::GeoPos, QVector<transdata_posatt_hexidx>>> m_entitylineposs;
    QVector<std::tuple<std::tuple<std::string, CHANNEL_INFO*>,TYPE_ULID, QGV::GeoPos, QVector<QGV::GeoPos>>> m_sensor;

    std::atomic_bool m_playdata{false};

    QTimer * m_pUpdateTimer;

    uint32_t ientitycount;
    TYPE_ULID m_indexid = 0;
    QHash<TYPE_ULID,bool> m_existid;

    Snowflake m_snowflake;

    gaeactortransmit::GaeactorTransmit *m_pGaeactorTransmit;
};
#endif // MAINWINDOW_H
