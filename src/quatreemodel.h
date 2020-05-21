#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaModel>

template <typename N, int I = 0>
class QUaTreeModel : public QUaModel<N, I>
{
public:
    explicit QUaTreeModel(QObject* parent = nullptr);
    ~QUaTreeModel();

    N    rootNode() const;
    void setRootNode(N rootNode = nullptr);

private:
    void bindRoot(typename QUaModel<N, I>::QUaNodeWrapper* root);
    void bindRecursivelly(typename QUaModel<N, I>::QUaNodeWrapper* wrapper);
};

template<class N, int I>
inline QUaTreeModel<N, I>::QUaTreeModel(QObject* parent)
    : QUaModel<N, I>(parent)
{
}

template<class N, int I>
inline QUaTreeModel<N, I>::~QUaTreeModel()
{
    if (QUaModel<N, I>::m_root)
    {
        delete QUaModel<N, I>::m_root;
        QUaModel<N, I>::m_root = nullptr;
    }
}

template<class N, int I>
inline N QUaTreeModel<N, I>::rootNode() const
{
    return QUaModel<N>::m_root ? QUaModel<N>::m_root->node() : nullptr;
}

template<class N, int I>
inline void QUaTreeModel<N, I>::setRootNode(N rootNode)
{
    this->bindRoot(new typename QUaModel<N, I>::QUaNodeWrapper(rootNode));
}

template<class N, int I>
inline void QUaTreeModel<N, I>::bindRoot(
    typename QUaModel<N, I>::QUaNodeWrapper* root
)
{
    if (QUaModel<N, I>::m_root == root)
    {
        return;
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // if old root node was valid, disconnect to recv signals recursivelly
    if (QUaModel<N, I>::m_root)
    {
        delete QUaModel<N, I>::m_root;
        QUaModel<N, I>::m_root = nullptr;
    }
    // copy
    QUaModel<N, I>::m_root = root;
    // subscribe to changes
    this->bindRecursivelly(QUaModel<N, I>::m_root);
    // notify views new data is available
    this->endResetModel();
}

template<class N, int I>
inline void QUaTreeModel<N, I>::bindRecursivelly(
    typename QUaModel<N, I>::QUaNodeWrapper* wrapper
)
{
    // subscribe to node removed
    auto conn = QUaModelItemTraits::DestroyCallback<N, I>(wrapper->node(),
        static_cast<std::function<void(void)>>([this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        auto root = QUaModel<N, I>::m_root;
        if (wrapper == root)
        {
            this->setRootNode(QUaModelItemTraits::GetInvalid<N, I>());
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) 
        //        already handled
        auto parent = wrapper->parent();
        Q_ASSERT(parent);
        // only use indexes created by model
        int row = wrapper->index().row();
        QModelIndex index = parent->index();
        Q_ASSERT(parent == root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
        // notify views that row will be removed
        this->beginRemoveRows(index, row, row);
        // remove from parent, destructor deletes wrapper sub-tree recursivelly
        Q_ASSERT(wrapper == parent->children().at(row));
        delete parent->children().takeAt(row);
        // notify views that row removal has finished
        this->endRemoveRows();
        // force index re-creation (indirectly)
        // so we can delete multiple rows in a loop inmediatly (without having to queue them)
        bool indexOk = this->checkIndexRecursive(
            index,
            QAbstractItemModel::CheckIndexOption::IndexIsValid,
            parent == root
        );
        Q_ASSERT(indexOk);
        Q_UNUSED(indexOk);
    }));
    // NOTE : QUaNodeWrapper destructor removes connections
    if (conn)
    {
        wrapper->connections() << conn;
    }
    // subscribe to new child node added
    conn = QUaModelItemTraits::NewChildCallback<N, I>(wrapper->node(),
        static_cast<std::function<void(N)>>([this, wrapper](N childNode) {
            // TODO : implement a mechanism similar to QUaModelBaseEventer::execLater
            //        to make all inserts requested in a single event loop call
            //        at the same time and not one by one because now takes up to 
            //        30% cpu time in a large > 10k deserialization test
        // get new node's row
        int row = wrapper->children().count();
        // only use indexes created by model
        QModelIndex index = wrapper->index();
        auto root = QUaModel<N, I>::m_root;
        Q_ASSERT(wrapper == root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
        Q_UNUSED(root);
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* childWrapper = new typename QUaModel<N, I>::QUaNodeWrapper(
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
        bool indexOk = this->checkIndexRecursive(
            this->index(row, 0, index),
            QAbstractItemModel::CheckIndexOption::IndexIsValid
        );
        Q_ASSERT(indexOk);
        Q_UNUSED(indexOk);
        // emit added signal
        this->handleNodeAddedRecursive(childWrapper);
    }));
    // NOTE : QUaNodeWrapper destructor removes connections
    if (conn)
    {
        wrapper->connections() << conn;
    }
    // bind callback for data change on each column
    this->bindChangeCallbackForAllColumns(wrapper);
    // recurse children
    for (auto child : wrapper->children())
    {
        this->bindRecursivelly(child);
    }
}

#endif // QUATREEMODEL_H

