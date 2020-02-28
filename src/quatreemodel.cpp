#include "quatreemodel.h"

#include <QUaNode>

QUaTreeModel::QUaTreeModel(QObject *parent)
    : QUaNodeModel(parent)
{
}

QUaTreeModel::~QUaTreeModel()
{
    if (m_root)
    {
        this->unbindNodeRecursivelly(m_root);
        delete m_root;
        m_root = nullptr;
    }
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


void QUaTreeModel::bindRecursivelly(QUaNodeWrapper* wrapper)
{
    // subscribe to node removed
    // unbind tree must run inmediatly
    QObject::connect(wrapper->node(), &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        // stop all events for sub-tree so children's QObject::destroyed is not handled
        this->unbindNodeRecursivelly(wrapper);
    }, Qt::DirectConnection);
    // remove rows better be queued in the event loop
    QObject::connect(wrapper->node(), &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        if (wrapper == m_root)
        {
            this->setRootNode(nullptr);
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) already handled
        Q_ASSERT(wrapper->parent());
        // only use indexes created by model
        int row = wrapper->index().row();
        QModelIndex index = wrapper->parent()->index();
        Q_ASSERT(wrapper->parent() == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be removed
        this->beginRemoveRows(index, row, row);
        // remove from parent, destructor deletes wrapper sub-tree recursivelly
        Q_ASSERT(wrapper == wrapper->parent()->children().at(row));
        delete wrapper->parent()->children().takeAt(row);
        // notify views that row removal has finished
        this->endRemoveRows();
    }, Qt::QueuedConnection);
    // subscribe to new child node added
    // insert rows better be queued in the event loop
    QObject::connect(wrapper->node(), &QUaNode::childAdded, this,
    [this, wrapper](QUaNode* childNode) {
        // get new node's row
        int row = wrapper->children().count();
        // only use indexes created by model
        QModelIndex index = wrapper->index();
        Q_ASSERT(wrapper == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* childWrapper = new QUaNodeWrapper(childNode, wrapper);
        // apprend to parent's children list
        wrapper->children() << childWrapper;
        // bind new instance for changes
        this->bindRecursivelly(childWrapper);
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
    this->bindChangeCallbackForAllColumns(wrapper);
    // recurse children
    for (auto child : wrapper->children())
    {
        this->bindRecursivelly(child);
    }
}

void QUaTreeModel::unbindNodeRecursivelly(QUaNodeWrapper* wrapper)
{
    Q_CHECK_PTR(wrapper);
    // disconnect from internal node
    if (wrapper->node())
    {
        this->disconnect(wrapper->node());
        wrapper->node()->disconnect(this);
    }
    // disconnect children
    for (auto child : wrapper->children())
    {
        this->unbindNodeRecursivelly(child);
    }
}

