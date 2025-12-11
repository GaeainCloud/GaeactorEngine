#ifndef AGENTEDITPANEL_H
#define AGENTEDITPANEL_H

#include <QWidget>
#include <QDebug>
#include <QJsonArray>
#include "widget3d/QtOsgWidget.h"
#include "qjsonobject.h"

#include <QObject>
#include <QList>
#include <QVariant>
#include <QStringList>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <osg/Vec3f>
#include "treectrl.h"
#include "components/function.h"
#include "widget3d/ModelSceneData.h"
///////////////////////////////////////////////////////////////////////////////////////
enum E_AGENT_TYPE:int32_t
{
    E_AGENT_TYPE_INSTAGENT = 0x00,
    E_AGENT_TYPE_SCENE = 0x01,
};
class QQuickWidget;
class QHBoxLayout;
class HttpClient;
class MapEditWidget;
class QStackedWidget;
class QtModelWidget:public QtModelWidgetBase
{
    Q_OBJECT
public:
    QtModelWidget(QtOSGWidget::E_OSG_SHOW_TYPE show_type, QWidget *parent = nullptr);
    virtual ~QtModelWidget();

    Q_INVOKABLE void saveAgent();
    Q_INVOKABLE void deleteAgent();
    Q_INVOKABLE void addSubNode(const QModelIndex& index);
    Q_INVOKABLE void addModelNode(const QModelIndex& index, const QVariant& modelpath);
    Q_INVOKABLE void switchHideModel(const QVariant& datatye,const QVariant& instanceid,const QVariant& modelid);
    Q_INVOKABLE void selectModel(const QVariant& datatye,const QVariant& instanceid, const QVariant& modelid);
    Q_INVOKABLE void updateContext(const QVariant& dataType_Id,const QVariant& edit_id,const QVariant& context);
    Q_INVOKABLE void addPoiGeoJson(const QVariant& dataType_Id,const QVariant& edit_id, const QVariant& modelpath);
    Q_INVOKABLE void selectPoiPoint(const QVariant& dataType_Id,const QVariant& edit_id,const QVariant& waypoint_id);

#if 0
    Q_INVOKABLE void fenceAppendPoint(const QVariant& pointid, const QVariant& lngval, const QVariant& latval, const QVariant& data);
	Q_INVOKABLE void fenceUpdatePoint(const QVariant& pointid, const QVariant& lngval, const QVariant& latval, const QVariant& data);
	Q_INVOKABLE void fenceSelectPoint(const QVariant& pointid, const QVariant& data);
#endif

    Q_INVOKABLE void updateFieldName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context);
    Q_INVOKABLE void updateActionName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context);
    Q_INVOKABLE void updatePoiName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context);
    Q_INVOKABLE void updateOODAName(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context);
    Q_INVOKABLE void updateSensingRadius(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& context);
    Q_INVOKABLE void updateFieldPos(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& offsetx, const QVariant& offsety, const QVariant& offsetz);
    Q_INVOKABLE void updateSensingAngle(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& yawAngle, const QVariant& yawFlareAngle, const QVariant& pitchAngle, const QVariant& pitchFlareAngle);
	Q_INVOKABLE void updatePoiPos(const QVariant& dataType_Id, const QVariant& edit_id, const QVariant& offsetx, const QVariant& offsety, const QVariant& offsetz);

    Q_INVOKABLE QVariant getAgentType();

    void setModelListWidget(QVector<QWidget*>& listwidgets);
	void setAgentType(E_AGENT_TYPE agentType);
    void loadNewAgent(const QJsonObject &contextJsonobj);
	void loadOldAgent(const QJsonObject &contextJsonobj);
	void loadAgent();
	void updateOpreateNodeInfo(uint64_t instanceid, uint64_t id, int level, int mode, osg::Vec3f trans, osg::Vec3f rotate, osg::Vec3f scale);
    void updateOpreateNodeSelect(uint64_t instanceid, uint64_t id);

	void updateFieldInfo(uint64_t iFieldId, osg::Vec3f trans);
	void updateSensingInfo(uint64_t iFieldId, uint64_t iSensingId, float yawAngle, float yawFlareAngle, float pitchAngle, float pitchFlareAngle);

	void updateFieldSelect(uint64_t iFieldId);
	void updateSensingSelect(uint64_t iFieldId, uint64_t iSensingId);

	void clearData();

	void addModelFinished(QPair<uint64_t, QString> &modelinfo, const  QModelIndex& index, bool bLoad);
signals:
    void edit_type_sig(TreeNode::E_NODE_TYPE type,uint64_t id, bool expand, bool bSubRootNode);
	void add_type_sig(TreeNode::E_NODE_TYPE type, uint64_t id);
    void qml_quit_agent_edit_panel_sig();
private slots:
    void edit_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id, bool expand, bool bSubRootNode);
	void add_type_slot(TreeNode::E_NODE_TYPE type, uint64_t id);
	void qml_quit_agent_edit_panel_slot();

	void appendWaypoint_slot(quint64 waypointid, double lng, double lat);
	void updateWaypoint_slot(quint64 waypointid, double lng, double lat);
	void selectWaypoint_slot(quint64 waypointid);
private:
    void initTreemodel();
    void updateAgentDesc(const QString& desc);
    void showSubPage(QQuickWidget *qmlWidget, uint64_t id, QJsonObject& datacontext);
    void updateFieldNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context);
    void updatePoiNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context);
    void updateActionNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context);
    void updateOODANodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString& context);
	void appendWaypoint(TreeNode::E_NODE_TYPE _currenttype, quint64 editid, quint64 waypointid, double lng, double lat);
protected:

	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;
    virtual void hideEvent(QHideEvent *event) override;
private:
    TreeViewController* m_treeViewController;
    QQuickWidget *m_qmlpanelWidget;    
    QJsonObject m_currentAgentJson;
	QVector<QWidget*> m_listwidgets;

	struct geometry_points_info {
		QColor cl;
		QString m_name;
		uint64_t m_geometry_id;
		std::list<waypointinfo> m_pts;
		std::vector<std::tuple<std::vector<LAT_LNG>, QColor>> coordinatesExtend;
		geometry_points_info(uint64_t geometry_id, const QColor& color)
		{
			m_geometry_id = geometry_id;
			cl = color;
		}
	};
	typedef struct tagAgentData
	{
		QJsonObject m_general;
		std::unordered_map<uint64_t, QJsonObject> m_actions;
		std::unordered_map<uint64_t, QJsonObject> m_waypoints;
		std::unordered_map<uint64_t, QJsonObject> m_oodas;
		std::unordered_map<uint64_t, QJsonObject> m_params;
        std::unordered_map<uint64_t, QJsonObject> m_pois;
        std::unordered_map<uint64_t, QJsonObject> m_fences;
        std::unordered_map<uint64_t, QJsonObject> m_field;
		std::unordered_map<uint64_t, QJsonObject> m_sensing;
		std::unordered_map<uint64_t, QJsonObject> m_agentinstance;
		std::unordered_map<uint64_t, std::list<uint64_t>> m_fieldsensing;
		std::unordered_map<uint64_t, QJsonObject> m_modelAbstracted;

		std::unordered_map<quint64, geometry_points_info> m_poi_geometrypointmap;
		std::unordered_map<quint64, geometry_points_info> m_fence_geometrypointmap;

	}AGENTDATA;

    AGENTDATA m_agentData;
    QString m_agent_type;
    uint64_t m_currentEditId;
	TreeNode::E_NODE_TYPE m_currenttype;

	E_AGENT_TYPE m_eAgentType;

    bool m_bNewAgent;
    QString m_currentagentKey;

	QStackedWidget *m_pQStackedWidget;

	QtOSGWidget *m_pModelWidget;
	MapEditWidget* m_pMapEditWidget;

	std::unordered_map<uint64_t, uint64_t> m_add_waypoints;

};
#endif // AGENTEDITWIDGET_H
