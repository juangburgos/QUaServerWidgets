#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaModel>

template <class T>
class QUaTreeModel : public QUaModel<T>
{
public:
    explicit QUaTreeModel(QObject* parent = nullptr);
    ~QUaTreeModel();

    T    rootNode() const;
    void setRootNode(T rootNode = nullptr);

private:
    void bindRoot(typename QUaModel<T>::QUaNodeWrapper* root);
    void bindRecursivelly(typename QUaModel<T>::QUaNodeWrapper* wrapper);
    void unbindNodeRecursivelly(typename QUaModel<T>::QUaNodeWrapper* wrapper);
};

template<class T>
inline QUaTreeModel<T>::QUaTreeModel(QObject* parent)
    : QUaModel<T>(parent)
{
}

template<class T>
inline QUaTreeModel<T>::~QUaTreeModel()
{
    if (QUaModel<T>::m_root)
    {
        this->unbindNodeRecursivelly(QUaModel<T>::m_root);
        delete QUaModel<T>::m_root;
        QUaModel<T>::m_root = nullptr;
    }
}

template<class T>
inline T QUaTreeModel<T>::rootNode() const
{
    return QUaModel<T>::m_root ? QUaModel<T>::m_root->node() : nullptr;
}

template<class T>
inline void QUaTreeModel<T>::setRootNode(T rootNode)
{
    this->bindRoot(
        rootNode ? 
        new typename QUaModel<T>::QUaNodeWrapper(rootNode) :
        nullptr
    );
}

template<class T>
inline void QUaTreeModel<T>::bindRoot(
    typename QUaModel<T>::QUaNodeWrapper* root
)
{
    if (QUaModel<T>::m_root == root)
    {
        return;
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // if old root node was valid, disconnect to recv signals recursivelly
    if (QUaModel<T>::m_root)
    {
        this->unbindNodeRecursivelly(QUaModel<T>::m_root);
        delete QUaModel<T>::m_root;
        QUaModel<T>::m_root = nullptr;
    }
    // copy
    QUaModel<T>::m_root = root;
    // subscribe to changes
    if (QUaModel<T>::m_root)
    {
        this->bindRecursivelly(QUaModel<T>::m_root);
    }
    // notify views new data is available
    this->endResetModel();
}

template<class T>
inline void QUaTreeModel<T>::bindRecursivelly(
    typename QUaModel<T>::QUaNodeWrapper* wrapper
)
{
     // subscribe to node removed
    // unbind tree must run inmediatly
    QObject::connect(wrapper->node(), &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        // stop all events for sub-tree so children's QObject::destroyed 
        // is not handled
        this->unbindNodeRecursivelly(wrapper);
    }, Qt::DirectConnection);
    // remove rows better be queued in the event loop
    QObject::connect(wrapper->node(), &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        if (wrapper == QUaModel<T>::m_root)
        {
            this->setRootNode(nullptr);
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) 
        //        already handled
        Q_ASSERT(wrapper->parent());
        // only use indexes created by model
        int row = wrapper->index().row();
        QModelIndex index = wrapper->parent()->index();
        Q_ASSERT(wrapper->parent() == QUaModel<T>::m_root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
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
    QObject::connect(wrapper->node(), &std::remove_pointer<T>::type::childAdded, this,
    [this, wrapper](T childNode) {
        // get new node's row
        int row = wrapper->children().count();
        // only use indexes created by model
        QModelIndex index = wrapper->index();
        Q_ASSERT(wrapper == QUaModel<T>::m_root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* childWrapper = new typename QUaModel<T>::QUaNodeWrapper(
            childNode, wrapper
        );
        // apprend to parent's children list
        wrapper->children() << childWrapper;
        // bind new instance for changes
        this->bindRecursivelly(childWrapper);
        // notify views that row addition has finished
        this->endInsertRows();
        // force index creation (indirectly)
        // because sometimes they are not created until a view requires them
        // and if a child is added and parent's index is not ready then crash
        bool indexOk = this->checkIndex(
            this->index(row, 0, index), 
            QAbstractItemModel::CheckIndexOption::IndexIsValid
        );
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

template<class T>
inline void QUaTreeModel<T>::unbindNodeRecursivelly(
    typename QUaModel<T>::QUaNodeWrapper* wrapper
)
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

#endif // QUATREEMODEL_H

