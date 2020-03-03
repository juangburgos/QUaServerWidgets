#ifndef QUATABLEMODEL_H
#define QUATABLEMODEL_H

#include <QUaModel>

template <class T>
class QUaTableModel : public QUaModel<T>
{
public:
    explicit QUaTableModel(QObject *parent = nullptr);
    ~QUaTableModel();

    void addNode(T node);

    void addNodes(const QList<T> &nodes);

    bool removeNode(T node);

    void clear();

protected:
    void removeWrapper(QUaModel<T>::QUaNodeWrapper * wrapper);
};

template<class T>
inline QUaTableModel<T>::QUaTableModel(QObject* parent) :
	QUaModel<T>(parent)
{
    m_root = new QUaModel<T>::QUaNodeWrapper(nullptr);
}

template<class T>
inline QUaTableModel<T>::~QUaTableModel()
{
	if (m_root)
	{
		delete m_root;
		m_root = nullptr;
	}
}

template<class T>
inline void QUaTableModel<T>::addNode(T node)
{
	Q_ASSERT(!m_root->childByNode(node));
	QModelIndex index = m_root->index();
	// get new node's row
	int row = m_root->children().count();
	// notify views that row will be added
	this->beginInsertRows(index, row, row);
	// create new wrapper
	auto* wrapper = new QUaModel<T>::QUaNodeWrapper(node, m_root, false);
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
	QMetaObject::Connection conn = /*QUaModelItemTraits<QUaHasModelItemTraits<T>::value>::template*/
		QUaModelItemTraits<QUaHasModelItemTraits<T>::value>::DestroyCallback<T>(node, (std::function<void(void)>)
			[this, wrapper]() {
				Q_CHECK_PTR(wrapper);
				Q_ASSERT(m_root);
				// remove
				this->removeWrapper(wrapper);
			}
	);
	if (conn)
	{
		// NOTE : QUaNodeWrapper destructor removes connections
		wrapper->connections() << conn;
	}
}

template<class T>
inline void QUaTableModel<T>::addNodes(const QList<T>& nodes)
{
	for (auto node : nodes)
	{
		this->addNode(node);
	}
}

template<class T>
inline bool QUaTableModel<T>::removeNode(T node)
{
	auto wrapper = m_root->childByNode(node);
	if (!wrapper)
	{
		return false;
	}
	// NOTE : QUaNodeWrapper destructor removes connections
	this->removeWrapper(wrapper);
	return true;
}

template<class T>
inline void QUaTableModel<T>::clear()
{
	this->beginResetModel();
	while (m_root->children().count() > 0)
	{
		auto wrapper = m_root->children().takeFirst();
		// NOTE : QUaNodeWrapper destructor removes connections
		delete wrapper;
	}
	this->endResetModel();
}

template<class T>
inline void QUaTableModel<T>::removeWrapper(QUaModel<T>::QUaNodeWrapper* wrapper)
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

#endif // QUATABLEMODEL_H
