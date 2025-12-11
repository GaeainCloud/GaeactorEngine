#ifndef TREECTRL_H
#define TREECTRL_H

#include <QWidget>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

#include <QObject>
#include <QList>
#include <QVariant>
#include <QStringList>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <osg/Vec3f>
///////////////////////////////////////////////////////////////////////////////////////

class TreeNode
{
public:
    enum E_NODE_TYPE:int32_t
    {
        E_NODE_TYPE_NULL = -1,
        E_NODE_TYPE_CONFIGS = 0x00,
        E_NODE_TYPE_SUBMODELS = 0x01,
        E_NODE_TYPE_SUBAGENTS = 0x02,
        E_NODE_TYPE_VAEDEFS = 0x03,
        E_NODE_TYPE_ACTIONS = 0x04,
        E_NODE_TYPE_FIELDMEDIAS = 0x05,
        E_NODE_TYPE_WAYPOINTS = 0x06,
        E_NODE_TYPE_OODAS = 0x07,
        E_NODE_TYPE_POIS = 0x08,
        E_NODE_TYPE_FENCES = 0x09,

		///////////////////////////////////////////////////////////////////////////////////////
        E_NODE_TYPE_SUBMODELS_INSTANCE = 0x0108,
        E_NODE_TYPE_SUB_FIELDMEDIAS = 0x0109,
        E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE = 0x010a,
	};
    TreeNode(E_NODE_TYPE eNodeType, const QList<QVariant> &data, TreeNode *parent,uint64_t id  = 0);
    ~TreeNode();

    void appendChild(TreeNode *child);
    void deleteAllChild();

    TreeNode *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TreeNode *parent();
    void setParent(TreeNode *parent);

    void setNodeData(QVariant data, int index);

    E_NODE_TYPE eNodeType() const;
    QList<QVariant> &getChidData();
    uint64_t id() const;

private:
    TreeNode *pParentNode;
    QList<TreeNode*> mChildNodes;
    QList<QVariant> mNodeData;
    E_NODE_TYPE m_eNodeType;
	uint64_t m_id;
};
///////////////////////////////////////////////////////////////////////////////////////
class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum nodeRoles {
        NAME = Qt::UserRole + 1,
        TYPE = Qt::UserRole + 2
    };

    TreeModel(QObject *parent = nullptr);
    ~TreeModel();

    void init(TreeNode *ptreenode);
	void clearData();
    QModelIndex appendSubChild(const QModelIndex& index, uint64_t& id);
	QModelIndex appendNodeChild(TreeNode::E_NODE_TYPE type, const QString& nodeName, uint64_t id, uint64_t fieldid);
	void selectSubChild(const QModelIndex& index, bool expand);
    QModelIndex appendSubAgentChild(const QModelIndex& index,QPair<uint64_t, QString> agentInfo,bool bEmit = true);

	QModelIndex getRootModelInx();
    bool getSubModelIndex(TreeNode::E_NODE_TYPE type,QModelIndex & agentmodelindex, uint64_t fieldid);

    bool getTargetModelIndex(uint64_t id, QModelIndex & agentmodelindex);
    bool getTargetIndex(TreeNode::E_NODE_TYPE type, uint64_t id, QModelIndex & agentmodelindex);


	bool getTargetParentId(TreeNode::E_NODE_TYPE type, uint64_t sensingid, uint64_t& fieldid);

    void appendChild(const QModelIndex& index, QList<QVariant> dataList, int count);
    void setNodeName(const QString& appendName, QModelIndex index);
    void setRootNodeName(QString appendName);

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QHash<int, QByteArray> roleNames() const;
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    TreeNode *getPRootNode() const;
signals:
    void edit_type_sig(TreeNode::E_NODE_TYPE type,uint64_t id, bool expand, bool bSubRootNode);
	void add_type_sig(TreeNode::E_NODE_TYPE type, uint64_t id);

private:
    TreeNode *pRootNode;
};
///////////////////////////////////////////////////////////////////////////////////////

class TreeViewController : public QObject
{
    Q_OBJECT

public:
    TreeViewController(QObject *parent = nullptr);
    ~TreeViewController();
    void updateNodeContext(TreeNode::E_NODE_TYPE type,uint64_t id, const QString& context);
signals:
    void edit_type_sig(TreeNode::E_NODE_TYPE type,uint64_t id, bool expand, bool bSubRootNode);
	void add_type_sig(TreeNode::E_NODE_TYPE type, uint64_t id);
public slots://被qml调用
    Q_INVOKABLE QAbstractItemModel *getTreeModel();
    Q_INVOKABLE void updateNodeName(const QModelIndex& index);
    Q_INVOKABLE void addSubNode(const QModelIndex& index);
    Q_INVOKABLE void updateSelectNode(const QModelIndex& index,const QVariant& expand);
    Q_INVOKABLE QVariant getModelIndexType(const QModelIndex& index);

private:
    TreeModel* m_TreeModel;
};
///////////////////////////////////////////////////////////////////////////////////////

#endif // AGENTEDITWIDGET_H
