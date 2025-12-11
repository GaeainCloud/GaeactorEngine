#pragma execution_character_set("utf-8")
#include "treectrl.h"

#include <QHBoxLayout>
#include <QQuickWidget>
#include <QQmlContext>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

#include "widget3d/OSGManager.h"
#include "widget3d/ModelSceneData.h"
#include "components/function.h"
#include "../components/global_variables.h"

///////////////////////////////////////////////////////////////////////////////////////
TreeNode::TreeNode(E_NODE_TYPE eNodeType, const QList<QVariant> &data, TreeNode *parent, uint64_t id)
	:pParentNode(parent),
	mNodeData(data),
	m_eNodeType(eNodeType),
	m_id(id)
{
	qRegisterMetaType<TreeNode::E_NODE_TYPE>("TreeNode::E_NODE_TYPE");
	qRegisterMetaType<uint64_t>("uint64_t");

}

TreeNode::~TreeNode()
{
	for (auto item : mChildNodes)
	{
		item->setParent(nullptr);
	}
	pParentNode = nullptr;
}

void TreeNode::appendChild(TreeNode *child)
{
	child->setParent(this);
	mChildNodes.append(child);
}

void TreeNode::deleteAllChild()
{
	for (int index = 0; index < mChildNodes.size(); index++)
	{
		mChildNodes[index]->setParent(nullptr);
		mChildNodes[index]->deleteAllChild();
	}
	qDeleteAll(mChildNodes);
	mChildNodes.clear();
}

TreeNode *TreeNode::child(int row)
{
	return mChildNodes.value(row);
}

int TreeNode::childCount() const
{
	return mChildNodes.count();
}

int TreeNode::columnCount() const
{
	return mNodeData.count();
}

QVariant TreeNode::data(int column) const
{
	return mNodeData.value(column);
}

int TreeNode::row() const
{
	return pParentNode == nullptr ? 0 : pParentNode->mChildNodes.indexOf(const_cast<TreeNode*>(this));
}

TreeNode *TreeNode::parent()
{
	return pParentNode;
}

void TreeNode::setParent(TreeNode *parent)
{
	pParentNode = parent;
}

void TreeNode::setNodeData(QVariant data, int index)
{
	mNodeData[index] = data;
}

TreeNode::E_NODE_TYPE TreeNode::eNodeType() const
{
	return m_eNodeType;
}

QList<QVariant>& TreeNode::getChidData()
{
	return mNodeData;
}

uint64_t TreeNode::id() const
{
	return m_id;
}
///////////////////////////////////////////////////////////////////////////////////////
TreeModel::TreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	qRegisterMetaType<TreeNode::E_NODE_TYPE>("TreeNode::E_NODE_TYPE");
	QList<QVariant> list;
	list.append("RootNode");
	pRootNode = new TreeNode(TreeNode::E_NODE_TYPE_NULL, list, NULL);
}

TreeModel::~TreeModel()
{
	pRootNode->deleteAllChild();
	delete pRootNode;
}

void TreeModel::init(TreeNode* ptreenode)
{
	if (pRootNode->childCount()/* && count == 0*/) {	//刷新时先删除所有子节点
		beginRemoveRows(QModelIndex(), 0, pRootNode->childCount() - 1);
		pRootNode->deleteAllChild();
		endRemoveRows();
	}

	ptreenode->setParent(pRootNode);
	beginInsertRows(QModelIndex(), pRootNode->childCount(), pRootNode->childCount());
	pRootNode->appendChild(ptreenode);
	endInsertRows();
}

void TreeModel::clearData()
{
	auto clearNode = [&](TreeNode::E_NODE_TYPE type){
		QModelIndex agentmodelindex;
		if (getSubModelIndex(type, agentmodelindex,0))
		{
			TreeNode* clickNode = static_cast<TreeNode*>(agentmodelindex.internalPointer());
			if (clickNode)
			{
				clickNode->deleteAllChild();
			}
		}
	};
	clearNode(TreeNode::E_NODE_TYPE_CONFIGS);
	clearNode(TreeNode::E_NODE_TYPE_SUBMODELS);
	clearNode(TreeNode::E_NODE_TYPE_ACTIONS);
	clearNode(TreeNode::E_NODE_TYPE_FIELDMEDIAS);
	clearNode(TreeNode::E_NODE_TYPE_WAYPOINTS);
	clearNode(TreeNode::E_NODE_TYPE_OODAS);
	clearNode(TreeNode::E_NODE_TYPE_POIS);
	clearNode(TreeNode::E_NODE_TYPE_VAEDEFS);
	clearNode(TreeNode::E_NODE_TYPE_FENCES);
}

QModelIndex TreeModel::appendSubChild(const QModelIndex &sindex, uint64_t& id)
{
	QModelIndex modelindex;
	TreeNode* clickNode = static_cast<TreeNode*>(sindex.internalPointer());

	if (clickNode)
	{
		if (clickNode->parent() == pRootNode->child(0))
		{
			QList<QVariant> list_sub;
			QString nameval;

			TreeNode::E_NODE_TYPE eNodeType = clickNode->eNodeType();
			id = FunctionAssistant::generate_random_positive_uint64();
			switch (clickNode->eNodeType())
			{
				//			case TreeNode::E_NODE_TYPE_CONFIGS:
				//			{
				//				nameval = QString("%1 %2").arg(tr("General Configs")).arg(QString::number(clickNode->childCount() + 1));
				//			}
				//			break;
			case TreeNode::E_NODE_TYPE_ACTIONS:
			{
				nameval = QString("%1 %2").arg(tr("Action")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_WAYPOINTS:
			{
				nameval = QString("%1 %2").arg(tr("Waypoint")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_OODAS:
			{
				nameval = QString("%1 %2").arg(tr("OODA")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_POIS:
			{
				nameval = QString("%1 %2").arg(tr("POI")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_FENCES:
			{
				nameval = QString("%1 %2").arg(tr("Fence")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_VAEDEFS:
			{
				nameval = QString("%1 %2").arg(tr("Vardef")).arg(QString::number(clickNode->childCount() + 1));
			}
			break;
			case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
			{
				nameval = QString("%1 %2").arg(tr("Field Medias")).arg(QString::number(clickNode->childCount() + 1));
				eNodeType = TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS;
			}
			break;
			default:break;
			}

			if (!nameval.isEmpty())
			{
				list_sub.append(nameval);
				list_sub.append(eNodeType);
				list_sub.append(id);

				TreeNode* pNode = new TreeNode(eNodeType, list_sub, clickNode, id);
				beginInsertRows(sindex, clickNode->childCount(), clickNode->childCount());
				clickNode->appendChild(pNode);
				endInsertRows();
				modelindex = index(clickNode->childCount() - 1, 0, sindex);

				emit add_type_sig(pNode->eNodeType(), id);
			}
		}
		else
		{
			if (clickNode->eNodeType() == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS)
			{
				QList<QVariant> list_sub;
				QString nameval = QString("%1 %2").arg(tr("sensing Medias")).arg(QString::number(clickNode->childCount() + 1));
				TreeNode::E_NODE_TYPE eNodeType = TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE;

				id = FunctionAssistant::generate_random_positive_uint64();
				if (!nameval.isEmpty())
				{
					list_sub.append(nameval);
					list_sub.append(eNodeType);
					list_sub.append(id);

					TreeNode* pNode = new TreeNode(eNodeType, list_sub, clickNode, id);
					beginInsertRows(sindex, clickNode->childCount(), clickNode->childCount());
					clickNode->appendChild(pNode);
					endInsertRows();
					modelindex = index(clickNode->childCount() - 1, 0, sindex);

					emit add_type_sig(pNode->eNodeType(), id);
				}
			}
		}
	}

	return modelindex;
}

void TreeModel::selectSubChild(const QModelIndex &index, bool expand)
{
	TreeNode* clickNode = static_cast<TreeNode*>(index.internalPointer());

	//if (clickNode && ((clickNode->parent() == pRootNode->child(0)) ||
	//    (clickNode->parent()->eNodeType() == TreeNode::E_NODE_TYPE_SUBMODELS) ||
	//    (clickNode->parent()->eNodeType() == TreeNode::E_NODE_TYPE_FIELDMEDIAS) ||
	//    (clickNode->parent()->eNodeType() == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS)))
	if (clickNode)
	{
		QString nameval;
		switch (clickNode->eNodeType())
		{
		case TreeNode::E_NODE_TYPE_SUBMODELS:
		case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
		case TreeNode::E_NODE_TYPE_CONFIGS:
		case TreeNode::E_NODE_TYPE_ACTIONS:
		case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
		case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
		case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
		case TreeNode::E_NODE_TYPE_WAYPOINTS:
		case TreeNode::E_NODE_TYPE_OODAS:
		case TreeNode::E_NODE_TYPE_POIS:
		case TreeNode::E_NODE_TYPE_VAEDEFS:
		case TreeNode::E_NODE_TYPE_FENCES:
		{
			bool bSubRootNode = (clickNode->parent() == pRootNode->child(0)) ? true : false;
			emit edit_type_sig(clickNode->eNodeType(), clickNode->id(), expand, bSubRootNode);
		}
		break;
		default:break;
		}
	}
}

QModelIndex TreeModel::appendSubAgentChild(const QModelIndex &sindex, QPair<uint64_t, QString> agentInfo, bool bEmit /*= true*/)
{
	QModelIndex modelindex;
	TreeNode* clickNode = static_cast<TreeNode*>(sindex.internalPointer());

	if (clickNode && clickNode->parent() == pRootNode->child(0))
	{
		if (clickNode->eNodeType() == TreeNode::E_NODE_TYPE_SUBMODELS)
		{
			QList<QVariant> list_sub;
			//emit edit_type_sig(clickNode->eNodeType());
			TreeNode::E_NODE_TYPE eNodeType = TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE;
			list_sub.append(agentInfo.second);
			list_sub.append(eNodeType);
			list_sub.append(agentInfo.first);

			TreeNode* pNode = new TreeNode(eNodeType, list_sub, clickNode, agentInfo.first);
			beginInsertRows(sindex, clickNode->childCount(), clickNode->childCount());
			clickNode->appendChild(pNode);
			endInsertRows();

			modelindex = index(clickNode->childCount() - 1, 0, sindex);

			if (bEmit)
			{
				emit edit_type_sig(clickNode->eNodeType(), clickNode->id(), false, true);
			}
		}
	}
	return modelindex;
}

QModelIndex TreeModel::getRootModelInx()
{
	QModelIndex rootmodelindex = index(0, 0, QModelIndex());
	return rootmodelindex;
}

bool TreeModel::getSubModelIndex(TreeNode::E_NODE_TYPE type, QModelIndex &agentmodelindex, uint64_t fieldid)
{
	bool bExist = false;
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_CONFIGS:
	case TreeNode::E_NODE_TYPE_SUBMODELS:
	case TreeNode::E_NODE_TYPE_ACTIONS:
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	case TreeNode::E_NODE_TYPE_OODAS:
	case TreeNode::E_NODE_TYPE_POIS:
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(type, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			agentmodelindex = subagnetmodelindex;
			bExist = true;
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		int row = TreeNode::E_NODE_TYPE_FIELDMEDIAS;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				if (pNodetmp && pNodetmp->id() == fieldid)
				{
					agentmodelindex = tmpmodelindex;
					bExist = true;
					break;
				}
			}
		}
	}break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
	}
	break;

	default:
		break;
	}

	return bExist;
}

bool TreeModel::getTargetModelIndex(uint64_t id, QModelIndex &agentmodelindex)
{
	return getTargetIndex(TreeNode::E_NODE_TYPE_SUBMODELS, id, agentmodelindex);
}

bool TreeModel::getTargetIndex(TreeNode::E_NODE_TYPE type, uint64_t id, QModelIndex & agentmodelindex)
{
	bool bExist = false;
	switch (type)
	{
	case TreeNode::E_NODE_TYPE_NULL:
	case TreeNode::E_NODE_TYPE_CONFIGS:
	case TreeNode::E_NODE_TYPE_SUBMODELS:
	case TreeNode::E_NODE_TYPE_ACTIONS:
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	case TreeNode::E_NODE_TYPE_OODAS:
	case TreeNode::E_NODE_TYPE_POIS:
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	case TreeNode::E_NODE_TYPE_FENCES:
	{
		int row = type;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				if (pNodetmp && pNodetmp->id() == id)
				{
					agentmodelindex = tmpmodelindex;
					bExist = true;
					break;
				}
			}
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
	{
		int row = TreeNode::E_NODE_TYPE_SUBMODELS;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				if (pNodetmp && pNodetmp->id() == id)
				{
					agentmodelindex = tmpmodelindex;
					bExist = true;
					break;
				}
			}
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	{
		int row = TreeNode::E_NODE_TYPE_FIELDMEDIAS;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				if (pNodetmp && pNodetmp->id() == id)
				{
					agentmodelindex = tmpmodelindex;
					bExist = true;
					break;
				}
			}
		}
	}
	break;
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		int row = TreeNode::E_NODE_TYPE_FIELDMEDIAS;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				for (int j = 0; j < pNodetmp->childCount(); j++)
				{
					QModelIndex subtmpmodelindex = index(j, 0, tmpmodelindex);
					TreeNode* subpNodetmp = static_cast<TreeNode*>(subtmpmodelindex.internalPointer());
					if (subpNodetmp && subpNodetmp->id() == id)
					{
						agentmodelindex = subtmpmodelindex;
						bExist = true;
						break;
					}
				}
				if (bExist)
				{
					break;
				}
			}
		}
	}
	break;
	default:
		break;
	}

	return bExist;
}

bool TreeModel::getTargetParentId(TreeNode::E_NODE_TYPE type, uint64_t sensingid, uint64_t& fieldid)
{
	bool bExist = false;
	if (type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE)
	{
		int row = TreeNode::E_NODE_TYPE_FIELDMEDIAS;
		QModelIndex rootmodelindex = getRootModelInx();
		QModelIndex subagnetmodelindex = index(row, 0, rootmodelindex);

		TreeNode* pNode = static_cast<TreeNode*>(subagnetmodelindex.internalPointer());
		if (pNode)
		{
			for (int i = 0; i < pNode->childCount(); i++)
			{
				QModelIndex tmpmodelindex = index(i, 0, subagnetmodelindex);
				TreeNode* pNodetmp = static_cast<TreeNode*>(tmpmodelindex.internalPointer());
				if (pNodetmp)
				{
					for (int j = 0; j < pNodetmp->childCount(); j++)
					{
						QModelIndex subtmpmodelindex = index(j, 0, tmpmodelindex);
						TreeNode* subpNodetmp = static_cast<TreeNode*>(subtmpmodelindex.internalPointer());
						if (subpNodetmp && subpNodetmp->id() == sensingid)
						{
							fieldid = pNodetmp->id();
							bExist = true;
							break;
						}
					}
				}
				if (bExist)
				{
					break;
				}
			}
		}
	}
	return bExist;
}

void TreeModel::appendChild(const QModelIndex& index, QList<QVariant> dataList, int count)
{
	TreeNode* clickNode = static_cast<TreeNode*>(index.internalPointer());

	if (clickNode)
	{
		if (clickNode->childCount() && count == 0) {	//刷新时先删除所有子节点
			beginRemoveRows(index, 0, clickNode->childCount() - 1);
			clickNode->deleteAllChild();
			endRemoveRows();
		}

		TreeNode* pNode = new TreeNode(clickNode->eNodeType(), dataList, clickNode);
		beginInsertRows(index, clickNode->childCount(), clickNode->childCount());
		clickNode->appendChild(pNode);
		endInsertRows();
	}
}

QModelIndex TreeModel::appendNodeChild(TreeNode::E_NODE_TYPE type, const QString& nodeName, uint64_t id, uint64_t fieldid)
{
	QModelIndex modelindex;
	QModelIndex subrootindex;
	if (getSubModelIndex(type, subrootindex, fieldid))
	{
		TreeNode* clickNode = static_cast<TreeNode*>(subrootindex.internalPointer());
		if (clickNode)
		{
			TreeNode::E_NODE_TYPE eNodeType = clickNode->eNodeType();
			if(type == TreeNode::E_NODE_TYPE_FIELDMEDIAS)
			{
				eNodeType = TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS;
			}
			else if (type == TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS)
			{
				eNodeType = TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE;
			}
			
			QList<QVariant> list_sub;
			list_sub.append(nodeName);
			list_sub.append(eNodeType);
			list_sub.append(id);

			TreeNode* pNode = new TreeNode(eNodeType, list_sub, clickNode, id);
			beginInsertRows(subrootindex, clickNode->childCount(), clickNode->childCount());
			clickNode->appendChild(pNode);
			endInsertRows();
			modelindex = index(clickNode->childCount() - 1, 0, subrootindex);
		}
	}

	return modelindex;
}

void TreeModel::setNodeName(const QString& appendName, QModelIndex index)
{
	TreeNode *pTreeNode = static_cast<TreeNode*>(index.internalPointer());
	if (pTreeNode)
	{
		pTreeNode->setNodeData(appendName, 0);

		emit dataChanged(index, index);	//更新树节点数据
	}
}

void TreeModel::setRootNodeName(QString appendName)
{
	if (pRootNode->childCount())
	{
		pRootNode->child(0)->setNodeData(appendName, 0);
		emit layoutChanged();
	}
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	return 1;

	if (parent.isValid())
	{
		return static_cast<TreeNode*>(parent.internalPointer())->columnCount();
	}
	else
	{
		return pRootNode->columnCount();
	}
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeNode *parentNode;
	if (parent.column() > 0)
	{
		return 0;
	}

	if (!parent.isValid())
	{
		parentNode = pRootNode;
	}
	else
	{
		parentNode = static_cast<TreeNode*>(parent.internalPointer());
	}

	return parentNode->childCount();
}

QHash<int, QByteArray> TreeModel::roleNames() const
{
	QHash<int, QByteArray> names(QAbstractItemModel::roleNames());
	names[NAME] = "name";
	//    names[TYPE] = "type";


	return names;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}

	switch (role)
	{
	case NAME:
	{
		return static_cast<TreeNode*>(index.internalPointer())->data(0);
	}
	case TYPE:
	{
		return static_cast<TreeNode*>(index.internalPointer())->data(1);
	}
	}
	return QVariant();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		return 0;
	}

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	TreeNode *parentNode;
	if (!parent.isValid())
	{
		parentNode = pRootNode;
	}
	else
	{
		parentNode = static_cast<TreeNode*>(parent.internalPointer());
	}

	TreeNode *childNode = parentNode->child(row);
	if (childNode)
	{
		return createIndex(row, column, childNode);
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	TreeNode *childNode = static_cast<TreeNode*>(index.internalPointer());
	TreeNode *parentNode = childNode->parent();

	if (parentNode == nullptr || parentNode == pRootNode)
	{
		return QModelIndex();
	}
	
	switch (childNode->eNodeType())
	{
	case TreeNode::E_NODE_TYPE_CONFIGS:
	case TreeNode::E_NODE_TYPE_SUBMODELS:
	case TreeNode::E_NODE_TYPE_SUBAGENTS:
	case TreeNode::E_NODE_TYPE_VAEDEFS:
	case TreeNode::E_NODE_TYPE_ACTIONS:
	case TreeNode::E_NODE_TYPE_FIELDMEDIAS:
	case TreeNode::E_NODE_TYPE_WAYPOINTS:
	case TreeNode::E_NODE_TYPE_OODAS:
	case TreeNode::E_NODE_TYPE_POIS:
	case TreeNode::E_NODE_TYPE_FENCES:
	case TreeNode::E_NODE_TYPE_SUBMODELS_INSTANCE:
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS:
	case TreeNode::E_NODE_TYPE_SUB_FIELDMEDIAS_INSTANCE:
	{
		return createIndex(parentNode->row(), 0, parentNode);
	}
	break;
	default:return QModelIndex(); break;
	}
}

TreeNode *TreeModel::getPRootNode() const
{
	return pRootNode;
}
///////////////////////////////////////////////////////////////////////////////////////

TreeViewController::TreeViewController(QObject *parent)
	: QObject(parent)
{
	m_TreeModel = new TreeModel();

	connect(m_TreeModel, &TreeModel::edit_type_sig, this, &TreeViewController::edit_type_sig);
	connect(m_TreeModel, &TreeModel::add_type_sig, this, &TreeViewController::add_type_sig);
}

TreeViewController::~TreeViewController()
{
	delete m_TreeModel;
}

void TreeViewController::updateNodeContext(TreeNode::E_NODE_TYPE type, uint64_t id, const QString &context)
{
	QModelIndex agentmodelindex;
	if (m_TreeModel->getTargetIndex(type, id, agentmodelindex))
	{
		m_TreeModel->setNodeName(context, agentmodelindex);
	}
}

QAbstractItemModel *TreeViewController::getTreeModel()
{
	return m_TreeModel;
}

void TreeViewController::updateNodeName(const QModelIndex& index)
{
	m_TreeModel->setNodeName("1", index);
}

void TreeViewController::addSubNode(const QModelIndex& index)
{
	uint64_t id;
	m_TreeModel->appendSubChild(index, id);
}

void TreeViewController::updateSelectNode(const QModelIndex &index, const QVariant& expand)
{
	m_TreeModel->selectSubChild(index, expand.toBool());
}

QVariant TreeViewController::getModelIndexType(const QModelIndex &index)
{
	QVariant val = m_TreeModel->data(index, TreeModel::TYPE);
	return val;
}


