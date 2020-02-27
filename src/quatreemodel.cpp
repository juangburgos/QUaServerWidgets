#include "quatreemodel.h"

#include <QUaNode>

QUaTreeModel::QUaTreeModel(QObject *parent)
    : QUaNodeModel(parent)
{
}

QUaNode* QUaTreeModel::rootNode() const
{
    return m_root ? m_root->node() : nullptr;
}

void QUaTreeModel::setRootNode(QUaNode* rootNode/* = nullptr*/)
{
    this->bindRoot(rootNode ? new QUaNodeWrapper(rootNode) : nullptr);
}

void QUaTreeModel::bindRoot(QUaNodeWrapper* root)
{
    if (m_root == root)
    {
        return;
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // if old root node was valid, disconnect to recv signals recursivelly
    if (m_root)
    {
        this->unbindNodeRecursivelly(m_root);
        delete m_root;
        m_root = nullptr;
    }
    // copy
    m_root = root;
    // subscribe to changes
    if (m_root)
    {
        this->bindRecursivelly(m_root);
    }
    // notify views new data is available
    this->endResetModel();
}


void QUaTreeModel::bindRecursivelly(QUaNodeWrapper* node)
{
    // subscribe to node removed
    // unbind tree must run inmediatly
    QObject::connect(node->node(), &QObject::destroyed, this,
    [this, node]() {
        Q_CHECK_PTR(node);
        // stop all events for sub-tree so children's QObject::destroyed is not handled
        this->unbindNodeRecursivelly(node);
    }, Qt::DirectConnection);
    // remove rows better be queued in the event loop
    QObject::connect(node->node(), &QObject::destroyed, this,
    [this, node]() {
        Q_CHECK_PTR(node);
        if (node == m_root)
        {
            this->setRootNode(nullptr);
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) already handled
        Q_ASSERT(node->parent());
        // only use indexes created by model
        int row = node->index().row();
        QModelIndex index = node->parent()->index();
        Q_ASSERT(node->parent() == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be removed
        this->beginRemoveRows(index, row, row);
        // remove from parent, destructor deletes wrapper sub-tree recursivelly
        Q_ASSERT(node == node->parent()->children().at(row));
        delete node->parent()->children().takeAt(row);
        // notify views that row removal has finished
        this->endRemoveRows();
    }, Qt::QueuedConnection);
    // subscribe to new child node added
    // insert rows better be queued in the event loop
    QObject::connect(node->node(), &QUaNode::childAdded, this,
    [this, node](QUaNode* childNode) {
        // get new node's row
        int row = node->children().count();
        // only use indexes created by model
        QModelIndex index = node->index();
        Q_ASSERT(node == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* wrapper = new QUaNodeWrapper(childNode, node);
        // apprend to parent's children list
        node->children() << wrapper;
        // bind new instance for changes
        this->bindRecursivelly(wrapper);
        // notify views that row addition has finished
        this->endInsertRows();
        // force index creation (indirectly)
        // because sometimes they are not created until a view requires them
        // and if a child is added and parent's index is not ready then crash
        bool indexOk = this->checkIndex(this->index(row, 0, index), CheckIndexOption::IndexIsValid);
        Q_ASSERT(indexOk);
        Q_UNUSED(indexOk);
    }, Qt::QueuedConnection);
    // bind callback for data change on each column
    if (!m_mapDataSourceFuncs.isEmpty())
    {
        for (auto column : m_mapDataSourceFuncs.keys())
        {
            this->bindChangeCallbackForColumnRecursivelly(column, node);
        }
    }
    // recurse children
    for (auto child : node->children())
    {
        this->bindRecursivelly(child);
    }
}

void QUaTreeModel::unbindNodeRecursivelly(QUaNodeWrapper* node)
{
    Q_CHECK_PTR(node);
    // disconnect from internal node
    if (node->node())
    {
        this->disconnect(node->node());
        node->node()->disconnect(this);
    }
    // disconnect children
    for (auto child : node->children())
    {
        this->unbindNodeRecursivelly(child);
    }
}

void QUaTreeModel::bindChangeCallbackForColumn(const int& column, QUaNodeWrapper* node)
{
    this->bindChangeCallbackForColumnRecursivelly(column, node);
}

void QUaTreeModel::bindChangeCallbackForColumnRecursivelly(const int& column, QUaNodeWrapper* node)
{
    Q_CHECK_PTR(node);
    if (node->node() && m_mapDataSourceFuncs[column].m_changeCallback)
    {
        // pass in callback that user needs to call when a value is udpated
        // store connection in wrapper so can be disconnected when wrapper deleted
        node->connections() <<
        m_mapDataSourceFuncs[column].m_changeCallback(
            node->node(),
            node->getChangeCallbackForColumn(column, this)
        );
    }
    // recurse children
    for (auto child : node->children())
    {
        this->bindChangeCallbackForColumnRecursivelly(column, child);
    }
}
