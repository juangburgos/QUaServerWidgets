#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaModel>

template <typename N>
class QUaTreeModel : public QUaModel<N>
{
public:
    explicit QUaTreeModel(QObject* parent = nullptr);
    ~QUaTreeModel();

    N    rootNode() const;
    void setRootNode(N rootNode = nullptr);

private:
    void bindRoot(typename QUaModel<N>::QUaNodeWrapper* root);
    void bindRecursivelly(typename QUaModel<N>::QUaNodeWrapper* wrapper);
};

template<class N>
inline QUaTreeModel<N>::QUaTreeModel(QObject* parent)
    : QUaModel<N>(parent)
{
}

template<class N>
inline QUaTreeModel<N>::~QUaTreeModel()
{
    if (QUaModel<N>::m_root)
    {
        delete QUaModel<N>::m_root;
        QUaModel<N>::m_root = nullptr;
    }
}

template<class N>
inline N QUaTreeModel<N>::rootNode() const
{
    return QUaModel<N>::m_root ? QUaModel<N>::m_root->node() : nullptr;
}

template<class N>
inline void QUaTreeModel<N>::setRootNode(N rootNode)
{
    this->bindRoot(new typename QUaModel<N>::QUaNodeWrapper(rootNode));
}

template<class N>
inline void QUaTreeModel<N>::bindRoot(
    typename QUaModel<N>::QUaNodeWrapper* root
)
{
    if (QUaModel<N>::m_root == root)
    {
        return;
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // if old root node was valid, disconnect to recv signals recursivelly
    if (QUaModel<N>::m_root)
    {
        delete QUaModel<N>::m_root;
        QUaModel<N>::m_root = nullptr;
    }
    // copy
    QUaModel<N>::m_root = root;
    // subscribe to changes
    this->bindRecursivelly(QUaModel<N>::m_root);
    // notify views new data is available
    this->endResetModel();
}

template<class N>
inline void QUaTreeModel<N>::bindRecursivelly(
    typename QUaModel<N>::QUaNodeWrapper* wrapper
)
{
    // subscribe to node removed
    auto conn = QUaModelItemTraits::DestroyCallback<N>(wrapper->node(),
        static_cast<std::function<void(void)>>([this, wrapper]() {
// NOTE : cannot queue or fuck up node delete with children on tree
// NOTE : cannot process events after of fuck up ua server
// NOTE : cannot call multiple QUaModelItemTraits::DestroyCallback in a loop of fuck up indexes
        Q_CHECK_PTR(wrapper);
        if (wrapper == QUaModel<N>::m_root)
        {
            this->setRootNode(QUaModelItemTraits::GetInvalid<N>());
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) 
        //        already handled
        Q_ASSERT(wrapper->parent());
        // only use indexes created by model
        int row = wrapper->index().row();
        QModelIndex index = wrapper->parent()->index();
        Q_ASSERT(wrapper->parent() == QUaModel<N>::m_root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
        // notify views that row will be removed
        this->beginRemoveRows(index, row, row);
        // remove from parent, destructor deletes wrapper sub-tree recursivelly
        Q_ASSERT(wrapper == wrapper->parent()->children().at(row));
        delete wrapper->parent()->children().takeAt(row);
        // notify views that row removal has finished
        this->endRemoveRows();
    }));
    // NOTE : QUaNodeWrapper destructor removes connections
    if (conn)
    {
        wrapper->connections() << conn;
    }
    // subscribe to new child node added
    conn = QUaModelItemTraits::NewChildCallback<N>(wrapper->node(),
        static_cast<std::function<void(N)>>([this, wrapper](N childNode) {
            // TODO : implement a mechanism similar to QUaModelBaseEventer::execLater
            //        to make all inserts requested in a single event loop call
            //        at the same time and not one by one because now takes up to 
            //        30% cpu time in a large > 10k deserialization test
        // get new node's row
        int row = wrapper->children().count();
        // only use indexes created by model
        QModelIndex index = wrapper->index();
        Q_ASSERT(wrapper == QUaModel<N>::m_root ||
            this->checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* childWrapper = new typename QUaModel<N>::QUaNodeWrapper(
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

