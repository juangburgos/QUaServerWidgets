#ifndef QUATABLEMODEL_H
#define QUATABLEMODEL_H

#include <QUaModel>

template <typename N>
class QUaTableModel : public QUaModel<N>
{
public:
    explicit QUaTableModel(QObject *parent = nullptr);
    ~QUaTableModel();

    void addNode(N node);

    void addNodes(const QList<N> &nodes);

	template<typename X = N>
	typename std::enable_if<std::is_pointer<X>::value, bool>::type
    removeNode(N node);

	template<typename X = N>
	typename std::enable_if<!std::is_pointer<X>::value, bool>::type
	removeNode(N * node);

    void clear();

protected:
    void removeWrapper(typename QUaModel<N>::QUaNodeWrapper * wrapper);
};

template<typename N>
inline QUaTableModel<N>::QUaTableModel(QObject* parent) :
	QUaModel<N>(parent)
{
    QUaModel<N>::m_root = new typename QUaModel<N>::QUaNodeWrapper(
			QUaModelItemTraits::GetInvalid<N>()
		);
}

template<typename N>
inline QUaTableModel<N>::~QUaTableModel()
{
    if (QUaModel<N>::m_root)
	{
        delete QUaModel<N>::m_root;
        QUaModel<N>::m_root = nullptr;
	}
}

template<typename N>
inline void QUaTableModel<N>::addNode(N node)
{
    //Q_ASSERT(!QUaModel<N>::m_root->childByNode(node));
    QModelIndex index = QUaModel<N>::m_root->index();
	// get new node's row
    int row = QUaModel<N>::m_root->children().count();
	// notify views that row will be added
	this->beginInsertRows(index, row, row);
	// create new wrapper
    auto* wrapper = new typename QUaModel<N>::QUaNodeWrapper(node, QUaModel<N>::m_root, false);
	// apprend to parent's children list
    QUaModel<N>::m_root->children() << wrapper;
	// notify views that row addition has finished
	this->endInsertRows();
	// force index creation (indirectly)
	// because sometimes they are not created until a view requires them
	// and if a child is added and parent's index is not ready then crash
    bool indexOk = this->checkIndex(this->index(row, 0, index), QAbstractItemModel::CheckIndexOption::IndexIsValid);
	Q_ASSERT(indexOk);
	Q_UNUSED(indexOk);
	// bind callback for data change on each column
	this->bindChangeCallbackForAllColumns(wrapper, false);
	// subscribe to instance removed
	// remove rows better be queued in the event loop
	auto conn = QUaModelItemTraits::DestroyCallback<N>(wrapper->node(),
        static_cast<std::function<void(void)>>([this, wrapper]() {
			Q_CHECK_PTR(wrapper);
            Q_ASSERT(QUaModel<N>::m_root);
			// remove
			this->removeWrapper(wrapper);
        })
	);
	if (conn)
	{
		// NOTE : QUaNodeWrapper destructor removes connections
		wrapper->connections() << conn;
	}
}

template<typename N>
inline void QUaTableModel<N>::addNodes(const QList<N>& nodes)
{
	for (auto node : nodes)
	{
        this->addNode(node);
    }
}

template<typename N>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, bool>::type 
	QUaTableModel<N>::removeNode(N node)
{
	auto wrapper = QUaModel<N>::m_root->childByNode(node);
	if (!wrapper)
	{
		return false;
	}
	// NOTE : QUaNodeWrapper destructor removes connections
	this->removeWrapper(wrapper);
	return true;
}

template<typename N>
template<typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, bool>::type
QUaTableModel<N>::removeNode(N* node)
{
	auto wrapper = QUaModel<N>::m_root->childByNode(node);
	if (!wrapper)
	{
		return false;
	}
	// NOTE : QUaNodeWrapper destructor removes connections
	this->removeWrapper(wrapper);
	return true;
}

template<typename N>
inline void QUaTableModel<N>::clear()
{
	this->beginResetModel();
    while (QUaModel<N>::m_root->children().count() > 0)
	{
        auto wrapper = QUaModel<N>::m_root->children().takeFirst();
		// NOTE : QUaNodeWrapper destructor removes connections
		delete wrapper;
	}
	this->endResetModel();
}

template<typename N>
inline void QUaTableModel<N>::removeWrapper(typename QUaModel<N>::QUaNodeWrapper* wrapper)
{
	// only use indexes created by model
	int row = wrapper->index().row();
    QModelIndex index = QUaModel<N>::m_root->index();
	// when deleteing a node of type N that has children of type N, 
	// QObject::destroyed is triggered from top to bottom without 
	// giving a change for the model to update its indices and rows wont match
    if (row >= QUaModel<N>::m_root->children().count() ||
        wrapper != QUaModel<N>::m_root->children().at(row))
	{
		// force reindexing
        for (int r = 0; r < QUaModel<N>::m_root->children().count(); r++)
		{
			this->index(r, 0, index);
		}
		row = wrapper->index().row();
	}
    Q_ASSERT(this->checkIndex(this->index(row, 0, index), QAbstractItemModel::CheckIndexOption::IndexIsValid));
    Q_ASSERT(wrapper == QUaModel<N>::m_root->children().at(row));
	// notify views that row will be removed
	this->beginRemoveRows(index, row, row);
	// remove from parent
    delete QUaModel<N>::m_root->children().takeAt(row);
	// notify views that row removal has finished
	this->endRemoveRows();
}

#endif // QUATABLEMODEL_H
