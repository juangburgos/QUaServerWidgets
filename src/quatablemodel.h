#ifndef QUATABLEMODEL_H
#define QUATABLEMODEL_H

#include <QUaModel>

template <typename N, int I = 0>
class QUaTableModel : public QUaModel<N, I>
{
public:
    explicit QUaTableModel(QObject *parent = nullptr);
    ~QUaTableModel();

	// NOTE : not copyable because might own the data, pass pointers intead
	QUaTableModel(const QUaTableModel&) = delete;

    void addNode(N node);

    void addNodes(const QList<N> &nodes);

	template<typename X = N>
	typename std::enable_if<std::is_pointer<X>::value, bool>::type
    removeNode(N node);

	template<typename X = N>
	typename std::enable_if<!std::is_pointer<X>::value, bool>::type
	removeNode(N * node);

	int count();

protected:
    
};

template<typename N, int I>
inline QUaTableModel<N, I>::QUaTableModel(QObject* parent) :
	QUaModel<N, I>(parent)
{
    QUaModel<N, I>::m_root = new typename QUaModel<N, I>::QUaNodeWrapper(
		QUaModelItemTraits::GetInvalid<N, I>()
	);
}

template<typename N, int I>
inline QUaTableModel<N, I>::~QUaTableModel()
{
    if (QUaModel<N, I>::m_root)
	{
        delete QUaModel<N, I>::m_root;
        QUaModel<N, I>::m_root = nullptr;
	}
}

template<typename N, int I>
inline void QUaTableModel<N, I>::addNode(N node)
{
    //Q_ASSERT(!QUaModel<N, I>::m_root->childByNode(node));
    QModelIndex index = QUaModel<N, I>::m_root->index();
	// get new node's row
    int row = QUaModel<N, I>::m_root->children().count();
	// notify views that row will be added
	this->beginInsertRows(index, row, row);
	// create new wrapper
    auto* wrapper = new typename QUaModel<N, I>::QUaNodeWrapper(node, QUaModel<N, I>::m_root, false);
	// apprend to parent's children list
    QUaModel<N, I>::m_root->children() << wrapper;
	// bind callback for data change on each column
	this->bindChangeCallbackForAllColumns(wrapper, false);
	// subscribe to instance removed
	auto conn = QUaModelItemTraits::DestroyCallback<N, I>(wrapper->node(),
        [this, wrapper]() {
			Q_CHECK_PTR(wrapper);
            auto root =
        #ifdef Q_OS_LINUX
                    QUaModel<N, I>::
        #endif
                    m_root;
            Q_ASSERT(root);
			Q_UNUSED(root);
			// remove
			this->removeWrapper(wrapper);
        }
	);
	if (conn)
	{
		// NOTE : QUaNodeWrapper destructor removes connections
		wrapper->connections() << conn;
	}
	// notify views that row addition has finished
	this->endInsertRows();
	// force index creation (indirectly)
	// because sometimes they are not created until a view requires them
	// and if a child is added and parent's index is not ready then crash
    bool indexOk = this->checkIndexRecursive(
		this->index(row, 0, index)
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
		, QAbstractItemModel::CheckIndexOption::IndexIsValid
#endif
	);
	Q_ASSERT(indexOk);
	Q_UNUSED(indexOk);
	// emit added signal
	this->handleNodeAddedRecursive(wrapper);
}

template<typename N, int I>
inline void QUaTableModel<N, I>::addNodes(const QList<N>& nodes)
{
	for (auto node : nodes)
	{
        this->addNode(node);
    }
}

template<typename N, int I>
inline int QUaTableModel<N, I>::count()
{
	return QUaModel<N, I>::m_root->children().count();
}

template<typename N, int I>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, bool>::type 
	QUaTableModel<N, I>::removeNode(N node)
{
	auto wrapper = QUaModel<N, I>::m_root->childByNode(node);
	if (!wrapper)
	{
		return false;
	}
	// NOTE : QUaNodeWrapper destructor removes connections
	this->removeWrapper(wrapper);
	return true;
}

template<typename N, int I>
template<typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, bool>::type
QUaTableModel<N, I>::removeNode(N* node)
{
	auto wrapper = QUaModel<N, I>::m_root->childByNode(node);
	if (!wrapper)
	{
		return false;
	}
	// NOTE : QUaNodeWrapper destructor removes connections
	this->removeWrapper(wrapper);
	return true;
}

#endif // QUATABLEMODEL_H
