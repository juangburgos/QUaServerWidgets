#include "quanodemodel.h"
#include <QUaNode>

QUaNodeModel::QUaNodeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootNode = nullptr;
}

void QUaNodeModel::bindRootNode(QUaNode* rootNode/* = nullptr*/)
{
    if (m_rootNode == rootNode)
    {
        return;
    }
    // if old root node was valid, disconnect to recv signals recursivelly
    if (m_rootNode)
    {
        this->disconnectNodeRecursivelly(m_rootNode);
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // copy
    m_rootNode = rootNode;
    // notify views new data is available
    this->endResetModel();
    // subscribe to root node deleted
    if (m_rootNode)
    {
        QObject::connect(m_rootNode, &QObject::destroyed, this,
        [this]() {
            this->bindRootNode(nullptr);
        });
    }
}

QVariant QUaNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // no header data if invalid root
    if (!m_rootNode)
    {
        return QVariant();
    }
    // TODO: Implement me!
    if (orientation == Qt::Horizontal && 
        role == Qt::DisplayRole &&
        section <= 1)
    {
        return tr("BrowseName");
    }
    return QVariant();
}

/*
bool QUaNodeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}
*/

QModelIndex QUaNodeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_rootNode || !this->hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    QUaNode* parentNode;
    // invalid parent index is root, else get internal reference
    if (!parent.isValid())
    {
		parentNode = m_rootNode;
    }
    else
    {
        parentNode = static_cast<QUaNode*>(parent.internalPointer());
    }
    Q_CHECK_PTR(parentNode);
    // browse n-th child node using QUaNode API
    auto childrenList = parentNode->browseChildren();
    QUaNode* childItem = childrenList.count() > row ? parentNode->browseChildren().at(row) : nullptr;
    if (childItem)
    {
        return this->createIndex(row, column, childItem);
    }
    return QModelIndex();
}

QModelIndex QUaNodeModel::parent(const QModelIndex &index) const
{
    if (!m_rootNode || !index.isValid())
    {
        return QModelIndex();
    }
    // get child and parent node references
    QUaNode* childNode  = static_cast<QUaNode*>(index.internalPointer());
    QUaNode* parentNode = static_cast<QUaNode*>(childNode->parent());
    Q_CHECK_PTR(childNode);
    Q_CHECK_PTR(parentNode);
    if (parentNode == m_rootNode)
    {
        return QModelIndex();
    }
    // get row of parent if grandparent is valid
    int row = 0;
    int col = 0;
    QUaNode* grandpaNode = static_cast<QUaNode*>(parentNode->parent());
    if (grandpaNode)
    {
        row = grandpaNode->browseChildren().indexOf(parentNode);
    }
    return createIndex(row, col, parentNode);
}

int QUaNodeModel::rowCount(const QModelIndex &parent) const
{
    QUaNode* parentNode;
    if (!m_rootNode || parent.column() > 0)
    {
        return 0;
    }
    // get internal QUaNode reference
    if (!parent.isValid())
    {
        parentNode = m_rootNode;
    }
    else
    {
        parentNode = static_cast<QUaNode*>(parent.internalPointer());
        // subscribe to node removed
        QObject::disconnect(parentNode, &QObject::destroyed, this, 0);
        QObject::connect(parentNode, &QObject::destroyed, this,
        [this, parent]() {
            // below can happen when ui gets destroyed (closed)
            if (!parent.isValid())
            {
                return;
            }
            // NOTE : there is an issue when removing last child because beginRemoveRows acually calls
            //        rowCount on the parent and checks with an internal assert that the last argment
            //        is less than the parent's row count, which is not true because the child has already
            //        been removed by the time this callback is executed. to fix it below, the rowCount is
            //        artifitially increased by 1 to avoid hitting the assert when removing the last child
            QUaNode* grandpaNode = parent.parent().isValid() ? 
                static_cast<QUaNode*>(parent.parent().internalPointer()) : m_rootNode;
            Q_CHECK_PTR(grandpaNode);
            Q_ASSERT(grandpaNode->property("rlastchild").toBool() == false);
            if (grandpaNode->browseChildren().count() == parent.row())
            {
                grandpaNode->setProperty("rlastchild", true);
            }
            // TODO : still happening below when deleting parent with a lot of children
            const_cast<QUaNodeModel*>(this)->beginRemoveRows(parent.parent(), parent.row(), parent.row());
            const_cast<QUaNodeModel*>(this)->endRemoveRows();
        });
    }
    // subscribe to new child node added
    QObject::disconnect(parentNode, &QUaNode::childAdded, this, 0);
    QObject::connect(parentNode, &QUaNode::childAdded, this,
    [this, parent, parentNode](QUaNode* childNode) {
        // notify views that row has been added
        int rows = parentNode->browseChildren().count();
        const_cast<QUaNodeModel*>(this)->beginInsertRows(parent, rows - 1, rows - 1);
        const_cast<QUaNodeModel*>(this)->endInsertRows();
    });
    // return number of children
    Q_CHECK_PTR(parentNode);
    int childCount = parentNode->browseChildren().count();
    // fix removing last child issue
    if (parentNode->property("rlastchild").toBool())
    {
        childCount++;
        parentNode->setProperty("rlastchild", false);
    }
    return childCount;
}

int QUaNodeModel::columnCount(const QModelIndex &parent) const
{
    if (!m_rootNode)
    {
        return 0;
    }
    // TODO: Implement me! Test with BrowseName
    return 1;
}

QVariant QUaNodeModel::data(const QModelIndex& index, int role) const
{
    // early exit for inhandled cases
    if (!m_rootNode || !index.isValid())
    {
        return QVariant();
    }
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    // TODO: Implement me!
    QUaNode* node = static_cast<QUaNode*>(index.internalPointer());
    Q_CHECK_PTR(node);
    return node->browseName();
}

/*
bool QUaNodeModel::hasChildren(const QModelIndex &parent) const
{
    // FIXME: Implement me!
    return true;
}

bool QUaNodeModel::canFetchMore(const QModelIndex &parent) const
{
    // FIXME: Implement me!
    return false;
}

void QUaNodeModel::fetchMore(const QModelIndex &parent)
{
    // FIXME: Implement me!
}
*/

Qt::ItemFlags QUaNodeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    // TODO: Implement me!
    //return Qt::ItemIsEditable;
    return QAbstractItemModel::flags(index);
}

void QUaNodeModel::disconnectNodeRecursivelly(QUaNode* rootNode)
{
    this->disconnect(rootNode);
    rootNode->disconnect(this);
    // disconnect children
    for (auto child : rootNode->browseChildren())
    {
        this->disconnectNodeRecursivelly(child);
    }
}

/*
bool QUaNodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        // FIXME: Implement me!
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}
*/

