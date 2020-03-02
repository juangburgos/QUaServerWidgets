#include "quatablemodel.h"

#include <QUaNode>

QUaTableModel::QUaTableModel(QObject *parent)
    : QUaNodeModel(parent)
{
    m_root = new QUaNodeModel<QUaNode>::QUaNodeWrapper(nullptr);
}

QUaTableModel::~QUaTableModel()
{
    if (m_root)
    {
        delete m_root;
        m_root = nullptr;
    }
}

void QUaTableModel::addNode(QUaNode* node)
{
    Q_ASSERT(!m_root->childByNode(node));
    QModelIndex index = m_root->index();
    // get new node's row
    int row = m_root->children().count();
    // notify views that row will be added
    this->beginInsertRows(index, row, row);
    // create new wrapper
    auto* wrapper = new QUaNodeModel<QUaNode>::QUaNodeWrapper(node, m_root, false);
    // apprend to parent's children list
    m_root->children() << wrapper;
    // notify views that row addition has finished
    this->endInsertRows();
    // force index creation (indirectly)
    // because sometimes they are not created until a view requires them
    // and if a child is added and parent's index is not ready then crash
    bool indexOk = this->checkIndex(this->index(row, 0, index), CheckIndexOption::IndexIsValid);
    Q_ASSERT(indexOk);
    Q_UNUSED(indexOk);
    // bind callback for data change on each column
    this->bindChangeCallbackForAllColumns(wrapper, false);
    // subscribe to instance removed
    // remove rows better be queued in the event loop
    QObject::connect(node, &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        Q_ASSERT(m_root);
        // remove
        this->removeWrapper(wrapper);
    }, Qt::QueuedConnection);
}

void QUaTableModel::addNodes(const QList<QUaNode*>& nodes)
{
    for (auto node : nodes)
    {
        this->addNode(node);
    }
}

bool QUaTableModel::removeNode(QUaNode* node)
{
    auto wrapper = m_root->childByNode(node);
    if (!wrapper)
    {
        return false;
    }
    this->disconnect(node);
    node->disconnect(this);
    this->removeWrapper(wrapper);
    return true;
}

void QUaTableModel::clear()
{
    this->beginResetModel();
    while (m_root->children().count() > 0)
    {
        auto wrapper = m_root->children().takeFirst();
        this->disconnect(wrapper->node());
        wrapper->node()->disconnect(this);
        delete wrapper;
    }
    this->endResetModel();
}

void QUaTableModel::removeWrapper(QUaNodeModel<QUaNode>::QUaNodeWrapper* wrapper)
{
    // only use indexes created by model
    int row = wrapper->index().row();
    QModelIndex index = m_root->index();
    // when deleteing a node of type T that has children of type T, 
    // QObject::destroyed is triggered from top to bottom without 
    // giving a change for the model to update its indices and rows wont match
    if (row >= m_root->children().count() ||
        wrapper != m_root->children().at(row))
    {
        // force reindexing
        for (int r = 0; r < m_root->children().count(); r++)
        {
            this->index(r, 0, index);
        }
        row = wrapper->index().row();
    }
    Q_ASSERT(this->checkIndex(this->index(row, 0, index), CheckIndexOption::IndexIsValid));
    Q_ASSERT(wrapper == m_root->children().at(row));
    // notify views that row will be removed
    this->beginRemoveRows(index, row, row);
    // remove from parent
    delete m_root->children().takeAt(row);
    // notify views that row removal has finished
    this->endRemoveRows();
}
