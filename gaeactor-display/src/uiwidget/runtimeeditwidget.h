#ifndef RUNTIMEEDITWIDGET_H
#define RUNTIMEEDITWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QJsonArray>
#include "components/function.h"
#include "./src/../head_define.h"

#define RUN_SPEED (0.0)

class QQuickWidget;
class QHBoxLayout;
class MapEditWidget;
class RuntimeEditWidget : public QWidget
{
    Q_OBJECT

public:
    RuntimeEditWidget(QWidget *parent = nullptr);
    ~RuntimeEditWidget() override;
    void setShow(bool bVisiable);
    void resetData();
    Q_INVOKABLE void processArray(const QVariantList &arr);
    Q_INVOKABLE void setCurrentEntity(const QVariant &entityid);
    Q_INVOKABLE void deleteEntity(const QVariant &entityid);
    Q_INVOKABLE void locateWaypoint(const QVariant &entityid,const QVariant & waypointid);
    Q_INVOKABLE void deleteWaypoint(const QVariant &entityid,const QVariant & waypointid);
    Q_INVOKABLE QVariant getAgentKeysArray();
    Q_INVOKABLE void saveRuntimeData(const QVariant &runtimedata);
    void decodeRuntimeStyle(const QJsonObject & runtimestyle);
    quint64 runtiemstyle_id() const;
    void setRuntiemstyle_id(quint64 newRuntiemstyle_id);

    void updateAgentKeys(const std::unordered_map<QString, std::tuple<QString, QString>>& _agentkeysdata);

protected:
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
signals:
    void sendmsg_sig();
    void add_runtime_style_sig(const QJsonObject & runtimestyle);
    void qml_quit_agent_edit_panel_sig();
private slots:
    void qml_add_entity_slot();
    void appendWaypoint_slot(quint64 waypointid, double lng, double lat);
    void updateWaypoint_slot(quint64 waypointid, double lng, double lat);
    void selectWaypoint_slot(quint64 waypointid);
private:
    QQuickWidget *m_qmlWidget;
    MapEditWidget *m_mapWidget;
    QHBoxLayout *m_pLayout;
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
    quint64 m_runtiemstyle_id;

    QJsonArray m_agentkeysarr;

    QJsonObject m_runtimestyle_data;
};

#endif // MAINWIDGET_H
