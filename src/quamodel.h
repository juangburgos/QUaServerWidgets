#ifndef QUANODEMODEL_H
#define QUANODEMODEL_H

#include <QQueue>
#include <QAbstractItemModel>
#include <QUaModelItemTraits>

class QUaModelBaseEventer : public QObject
{
	Q_OBJECT
public:
	inline explicit QUaModelBaseEventer(QObject* parent = nullptr)
		: QObject(parent)
	{
		processing = false;
		QObject::connect(
			this,
			&QUaModelBaseEventer::sendEvent,
			this,
			&QUaModelBaseEventer::on_sendEvent,
			Qt::QueuedConnection
		);
	};
	template <typename M1 = const std::function<void(void)>&>
	inline void execLater(M1 &&func)
	{
		m_funcs.enqueue(func);
		if (processing)
		{
			return;
		}
		processing = true;
		emit this->sendEvent(QPrivateSignal());
	};
signals:
	void sendEvent(QPrivateSignal);
private slots:
	inline void on_sendEvent() 
	{
		Q_ASSERT(processing);
		if (m_funcs.isEmpty())
		{
			processing = false;
			return;
		}
		m_funcs.dequeue()();
		emit this->sendEvent(QPrivateSignal());
	};
private:
	bool processing;
	QQueue<std::function<void(void)>> m_funcs;
};

// SFINAE on members
// https://stackoverflow.com/questions/25492589/can-i-use-sfinae-to-selectively-define-a-member-variable-in-a-template-class
template <typename N, typename Enable = void>
class QUaModelBase 
{};

// pointer type specialization
template<typename N>
class QUaModelBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
{
protected:
	QUaModelBaseEventer m_eventer;
	int m_columnCount;
	// NOTE : no const on pointer otherwise qobject_cast fails
	struct ColumnDataSource
	{
		QString m_strHeader;
		std::function<QVariant(N)> m_dataCallback;
		std::function<QMetaObject::Connection(N, std::function<void()>)> m_changeCallback;
		std::function<bool(N)> m_editableCallback;
	};
	QMap<int, ColumnDataSource> m_mapDataSourceFuncs;
};

// instance specialization
template<typename N>
class QUaModelBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
{
protected:
	QUaModelBaseEventer m_eventer;
	int m_columnCount;
	struct ColumnDataSource
	{
		QString m_strHeader;
		std::function<QVariant(N*)> m_dataCallback;
		std::function<QMetaObject::Connection(N*, std::function<void()>)> m_changeCallback;
		std::function<bool(N*)> m_editableCallback;
	};
	QMap<int, ColumnDataSource> m_mapDataSourceFuncs;
};

template <typename N>
class QUaTableModel;

template <typename N>
class QUaTreeModel;

template <typename N>
class QUaModel : public QAbstractItemModel, public QUaModelBase<N>
{
    friend class QUaTableModel<N>;
    friend class QUaTreeModel<N>;
public:
    explicit QUaModel(QObject *parent = nullptr);
    ~QUaModel();

	template<typename X = N>
	typename std::enable_if<std::is_pointer<X>::value, X>::type
	nodeFromIndex(const QModelIndex& index) const;

	template<typename X = N>
	typename std::enable_if<!std::is_pointer<X>::value, X*>::type
	nodeFromIndex(const QModelIndex& index) const;

	template<
		typename M1 = const std::function<QVariant(N)>&,
		typename M2 = const std::function<QMetaObject::Connection(N, std::function<void(void)>)>&,
		typename M3 = const std::function<bool(N)>&,
		typename X  = N
	>
	typename std::enable_if<std::is_pointer<X>::value, void>::type
	setColumnDataSource(
		const int& column,
		const QString& strHeader,
		M1 &&dataCallback,              
		M2 &&changeCallback   = nullptr,
		M3 &&editableCallback = nullptr 
	);

	template<
		typename M1 = const std::function<QVariant(N*)>&,
		typename M2 = const std::function<QMetaObject::Connection(N*, std::function<void(void)>)>&,
		typename M3 = const std::function<bool(N*)>&,
		typename X  = N
	>
	typename std::enable_if<!std::is_pointer<X>::value, void>::type
	setColumnDataSource(
		const int& column,
		const QString& strHeader,
		M1 &&dataCallback,
		M2 &&changeCallback   = nullptr,
		M3 &&editableCallback = nullptr
	);

    void removeColumnDataSource(const int& column);

	template<typename M1 = const std::function<void(void)>&>
	inline void execLater(M1 &&func)
	{
		this->m_eventer.execLater(func);
	};

    // Qt required API:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:

    class QUaNodeWrapper
    {
    public:
        explicit QUaNodeWrapper(
            N node, 
            QUaModel<N>::QUaNodeWrapper* parent = nullptr,
            const bool &recursive = true);

        ~QUaNodeWrapper();

		template<typename X = N>
		typename std::enable_if<std::is_pointer<X>::value, X>::type
        node() const;

		template<typename X = N>
		typename std::enable_if<!std::is_pointer<X>::value, X*>::type
		node();

		void * userData() const;
		void   setUserData(const void* data);
		QUaModel<N>::QUaNodeWrapper* findChildByData(const void* childData) const;

        QModelIndex index() const;
        void setIndex(const QModelIndex &index);

        QUaModel<N>::QUaNodeWrapper* parent() const;

		template<typename X = N>
		typename std::enable_if<std::is_pointer<X>::value, QUaModel<N>::QUaNodeWrapper*>::type
        childByNode(N node) const;

		template<typename X = N>
		typename std::enable_if<!std::is_pointer<X>::value, QUaModel<N>::QUaNodeWrapper*>::type
		childByNode(N* node) const;

        // NOTE : return by reference
        QList<QUaModel<N>::QUaNodeWrapper*> & children();
        QList<QMetaObject::Connection> & connections();

        std::function<void()> getChangeCallbackForColumn(const int& column, QAbstractItemModel* model);

    private:
        // internal data
        N m_node;
		void * m_userData;
        // NOTE : need index to support model manipulation 
        //        cannot use createIndex outside Qt's API
        //        (doesnt work even inside a QAbstractItemModel member)
        //        and for beginRemoveRows and beginInsertRows
        //        we can only use the indexes provided by the model
        //        else random crashes occur when manipulating model
        //        btw do not use QPersistentModelIndex, they get corrupted
        QModelIndex m_index;
        // members for tree structure
        QUaNodeWrapper* m_parent;
        QList<QUaNodeWrapper*> m_children;
        QList<QMetaObject::Connection> m_connections;
    };

    QUaNodeWrapper* m_root;

    void bindChangeCallbackForColumn(
        const int& column,
        QUaNodeWrapper* wrapper,
        const bool& recursive = true);

    void bindChangeCallbackForAllColumns(
        QUaNodeWrapper* wrapper,
        const bool& recursive = true);

	void removeWrapper(typename QUaModel<N>::QUaNodeWrapper* wrapper);
};

template<class N>
inline QUaModel<N>::QUaModel(QObject* parent) :
	QAbstractItemModel(parent)
{
	m_root = nullptr;
	m_columnCount = 1;
}

template<class N>
inline QUaModel<N>::~QUaModel()
{
	if (m_root)
	{
		delete m_root;
		m_root = nullptr;
	}
}

template<class N>
inline void QUaModel<N>::removeColumnDataSource(const int& column)
{
	Q_ASSERT(column >= 0);
	if (column < 0 || column >= m_columnCount || !m_mapDataSourceFuncs.contains(column))
	{
		return;
	}
	m_mapDataSourceFuncs.remove(column);
	while (!m_mapDataSourceFuncs.contains(m_columnCount - 1) && m_columnCount > 1)
	{
		m_columnCount--;
	}
}

template<class N>
inline QVariant QUaModel<N>::headerData(int section, Qt::Orientation orientation, int role) const
{
	// no header data if invalid root
	if (!m_root)
	{
		return QVariant();
	}
	// handle only horizontal header text
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
	{
		return QVariant();
	}
	// default implementation if no ColumnDataSource has been defined
	if (m_mapDataSourceFuncs.isEmpty())
	{
		Q_ASSERT(m_columnCount == 1);
		return tr("");
	}
	// empty if no ColumnDataSource defined for this column
	if (!m_mapDataSourceFuncs.contains(section))
	{
		return QVariant();
	}
	// use user-defined ColumnDataSource
	return m_mapDataSourceFuncs[section].m_strHeader;
}

template<class N>
inline QModelIndex QUaModel<N>::index(int row, int column, const QModelIndex& parent) const
{
	if (!m_root || !this->hasIndex(row, column, parent))
	{
		return QModelIndex();
	}
	QUaNodeWrapper* parentWrapper;
	// invalid parent index is root, else get internal reference
	if (!parent.isValid())
	{
		parentWrapper = m_root;
	}
	else
	{
		parentWrapper = static_cast<QUaNodeWrapper*>(parent.internalPointer());
	}
	Q_CHECK_PTR(parentWrapper);
	// browse n-th child wrapper node
	auto childWrapper = parentWrapper->children().count() > row ?
		parentWrapper->children().at(row) : nullptr;
	if (!childWrapper)
	{
		return QModelIndex();
	}
	// create index
	QModelIndex index = this->createIndex(row, column, childWrapper);
	// store index to support model manipulation
	if (column == 0)
	{
		childWrapper->setIndex(index);
	}
	return index;
}

template<class N>
inline QModelIndex QUaModel<N>::parent(const QModelIndex& index) const
{
	if (!m_root || !index.isValid())
    {
        return QModelIndex();
    }
    auto intId = index.internalId();
    Q_UNUSED(intId);
    // get child and parent node references
    auto childWrapper  = static_cast<QUaNodeWrapper*>(index.internalPointer());
    Q_CHECK_PTR(childWrapper);
    auto parentWrapper = static_cast<QUaNodeWrapper*>(childWrapper->parent());
    Q_CHECK_PTR(parentWrapper);
    if (parentWrapper == m_root)
    {
        // store index to support model manipulation
        QModelIndex pIndex = QModelIndex();
        parentWrapper->setIndex(pIndex);
        return pIndex;
    }
    // get row of parent if grandparent is valid
    int row = 0;
    auto grandpaWrapper = static_cast<QUaNodeWrapper*>(parentWrapper->parent());
    if (grandpaWrapper)
    {
        row = grandpaWrapper->children().indexOf(parentWrapper);
    }
    // store index to support model manipulation
    QModelIndex pIndex = this->createIndex(row, 0, parentWrapper);
	parentWrapper->setIndex(pIndex);
    return pIndex;
}

template<class N>
inline int QUaModel<N>::rowCount(const QModelIndex& parent) const
{
	if (!m_root || parent.column() > 0)
	{
		return 0;
	}
	QUaNodeWrapper* parentWrapper;
	// get internal wrapper reference
	if (!parent.isValid())
	{
		parentWrapper = m_root;
	}
	else
	{
		parentWrapper = static_cast<QUaNodeWrapper*>(parent.internalPointer());
	}
	// return number of children
	Q_CHECK_PTR(parentWrapper);
	int childCount = parentWrapper->children().count();
	return childCount;
}

template<class N>
inline int QUaModel<N>::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	if (!m_root)
	{
		return 0;
	}
	// minimum 1 column
	return m_columnCount;
}

template<class N>
inline QVariant QUaModel<N>::data(const QModelIndex& index, int role) const
{
	// early exit for inhandled cases
	if (!m_root || !index.isValid())
	{
		return QVariant();
	}
	// only display data (text)
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}
	// get internal reference
	auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
	// check internal wrapper data is valid, because wrapper->node() is always deleted before wrapper
	if(!QUaModelItemTraits::IsValid<N>(wrapper->node()))
	{
		return QVariant();
	}
	// default implementation if no ColumnDataSource has been defined
	if (m_mapDataSourceFuncs.isEmpty())
	{
		Q_ASSERT(m_columnCount == 1);
		return tr("");
	}
	// empty if no ColumnDataSource defined for this column
	if (!m_mapDataSourceFuncs.contains(index.column()) ||
		!m_mapDataSourceFuncs[index.column()].m_dataCallback)
	{
		return QVariant();
	}
	// use user-defined ColumnDataSource
	return m_mapDataSourceFuncs[index.column()].m_dataCallback(wrapper->node());
}

template<class N>
inline bool QUaModel<N>::setData(const QModelIndex& index, const QVariant& value, int role)
{
	bool ok = QUaModelItemTraits::SetData<N>(this->nodeFromIndex(index), index.column(), value);
	if (ok)
	{
		emit this->dataChanged(index, index, QVector<int>() << role);
	}
	return ok;
}

template<class N>
inline Qt::ItemFlags QUaModel<N>::flags(const QModelIndex& index) const
{
	if (!m_root || !index.isValid())
	{
		return Qt::NoItemFlags;
	}
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	// test column defined and editable callback defined
	if (!m_mapDataSourceFuncs.contains(index.column()) ||
		!m_mapDataSourceFuncs[index.column()].m_editableCallback)
	{
		return flags;
	}
	// test node valid
	auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
	if(!QUaModelItemTraits::IsValid<N>(wrapper->node()))
	{
		return flags;
	}
	// test callback returns true
	if (!m_mapDataSourceFuncs[index.column()].m_editableCallback(wrapper->node()))
	{
		return flags;
	}
	// finally, after all this, item is editable
	return flags |= Qt::ItemIsEditable;
}

template<class N>
inline void QUaModel<N>::bindChangeCallbackForColumn(
	const int& column, 
	QUaNodeWrapper* wrapper, 
	const bool& recursive
)
{
	Q_CHECK_PTR(wrapper);
	if (QUaModelItemTraits::IsValid<N>(wrapper->node()) &&
		m_mapDataSourceFuncs[column].m_changeCallback)
	{
		// pass in callback that user needs to call when a value is udpated
		// store connection in wrapper so can be disconnected when wrapper deleted
		wrapper->connections() <<
			m_mapDataSourceFuncs[column].m_changeCallback(
				wrapper->node(),
				wrapper->getChangeCallbackForColumn(column, this)
			);
	}
	// check if recursive
	if (!recursive)
	{
		return;
	}
	// recurse children
	for (auto child : wrapper->children())
	{
		this->bindChangeCallbackForColumn(column, child);
	}
}

template<class N>
inline void QUaModel<N>::bindChangeCallbackForAllColumns(
	QUaNodeWrapper* wrapper, 
	const bool& recursive
)
{
	if (!m_mapDataSourceFuncs.isEmpty())
	{
		for (auto column : m_mapDataSourceFuncs.keys())
		{
			this->bindChangeCallbackForColumn(column, wrapper, recursive);
		}
	}
}


template<typename N>
inline void QUaModel<N>::removeWrapper(typename QUaModel<N>::QUaNodeWrapper* wrapper)
{
	auto parent = wrapper->parent();
	Q_CHECK_PTR(parent);
	// only use indexes created by model
	int row = wrapper->index().row();
	QModelIndex index = parent->index();
	// when deleteing a node of type N that has children of type N, 
	// QObject::destroyed is triggered from top to bottom without 
	// giving a change for the model to update its indices and rows wont match
	if (row >= parent->children().count() ||
		wrapper != parent->children().at(row))
	{
		// force reindexing
		for (int r = 0; r < parent->children().count(); r++)
		{
			this->index(r, 0, index);
		}
		row = wrapper->index().row();
	}
	Q_ASSERT(this->checkIndex(this->index(row, 0, index), QAbstractItemModel::CheckIndexOption::IndexIsValid));
	Q_ASSERT(wrapper == parent->children().at(row));
	// notify views that row will be removed
	this->beginRemoveRows(index, row, row);
	// remove from parent
	delete parent->children().takeAt(row);
	// notify views that row removal has finished
	this->endRemoveRows();
}

template<class N>
inline QUaModel<N>::QUaNodeWrapper::QUaNodeWrapper(
	N node, 
	QUaModel<N>::QUaNodeWrapper* parent/* = nullptr*/,
	const bool& recursive/* = true*/) :
	m_node(node),
	m_parent(parent),
	m_userData(nullptr)
{
	// m_node = nullptr must be supported for type model and category model
	// NOTE : QUaModelItemTraits methods must handle nullptr (or invalid) m_node
	// subscribe to node destruction, store connection to disconnect on destructor
	QMetaObject::Connection conn = QUaModelItemTraits::DestroyCallback<N>(
		this->node(),
        [this]() {
			this->m_node = QUaModelItemTraits::GetInvalid<N>();
        }
	);
	if (conn)
	{
		m_connections << conn;
	}
	// check if need to add children
	if (!recursive)
	{
		return;
	}
	// build children tree
	auto children = QUaModelItemTraits::GetChildren<N>(this->node());
	for (auto child : children)
	{
		m_children << new QUaModel<N>::QUaNodeWrapper(child, this);
	}
}

template<class N>
inline QUaModel<N>::QUaNodeWrapper::~QUaNodeWrapper()
{
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	qDeleteAll(m_children);
}

template<typename N>
template<typename X>
inline
typename std::enable_if<std::is_pointer<X>::value, X>::type
QUaModel<N>::nodeFromIndex(const QModelIndex& index) const
{
	if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
	{
		return m_root->node();
	}
	auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
	Q_CHECK_PTR(wrapper);
	return wrapper->node();
}

template<typename N>
template<typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, X*>::type
QUaModel<N>::nodeFromIndex(const QModelIndex& index) const
{
	if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
	{
		return m_root->node();
	}
	auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
	Q_CHECK_PTR(wrapper);
	return wrapper->node();
}

template<typename N>
template<typename M1, typename M2, typename M3, typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, void>::type 
QUaModel<N>::setColumnDataSource(
	const int& column, 
	const QString& strHeader, 
	M1 &&dataCallback,    // std::function<QVariant(X)>
	M2 &&changeCallback,  // std::function<QMetaObject::Connection(X, std::function<void(void)>)>
	M3 &&editableCallback // std::function<bool(X)>
)
{
	Q_ASSERT(column >= 0);
	if (column < 0)
	{
		return;
	}
	m_mapDataSourceFuncs.insert(
		column,
		{
			strHeader,
			dataCallback, 
			changeCallback, 
			editableCallback
		}
	);
	// call bind function recusivelly for each existing instance
	if (m_mapDataSourceFuncs[column].m_changeCallback)
	{
		this->bindChangeCallbackForColumn(column, m_root);
	}
	// keep always max num of columns
	m_columnCount = (std::max)(m_columnCount, column + 1);
}

template<typename N>
template<typename M1, typename M2, typename M3, typename X>
inline
typename std::enable_if<!std::is_pointer<X>::value, void>::type
QUaModel<N>::setColumnDataSource(
	const int& column,
	const QString& strHeader,
	M1 &&dataCallback,    // std::function<QVariant(X*)>
	M2 &&changeCallback,  // std::function<QMetaObject::Connection(X*, std::function<void(void)>)>
	M3 &&editableCallback // std::function<bool(X*)>
)
{
	Q_ASSERT(column >= 0);
	if (column < 0)
	{
		return;
	}
	m_mapDataSourceFuncs.insert(
		column,
		{
			strHeader,
			dataCallback,
			changeCallback,
			editableCallback
		}
	);
	// call bind function recusivelly for each existing instance
	if (m_mapDataSourceFuncs[column].m_changeCallback)
	{
		this->bindChangeCallbackForColumn(column, m_root);
	}
	// keep always max num of columns
	m_columnCount = (std::max)(m_columnCount, column + 1);
}

template<typename N>
template<typename X>
inline 
typename std::enable_if<std::is_pointer<X>::value, X>::type 
QUaModel<N>::QUaNodeWrapper::node() const
{
	return m_node;
}

template<typename N>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, X*>::type 
QUaModel<N>::QUaNodeWrapper::node()
{
	return &m_node;
}

template<typename N>
inline void* QUaModel<N>::QUaNodeWrapper::userData() const
{
	return m_userData;
}

template<typename N>
inline void QUaModel<N>::QUaNodeWrapper::setUserData(const void* data)
{
	m_userData = data;
}

template<typename N>
inline typename QUaModel<N>::QUaNodeWrapper*
QUaModel<N>::QUaNodeWrapper::findChildByData(const void* childData) const
{
	QUaModel<N>::QUaNodeWrapper* child = nullptr;
	auto res = std::find_if(m_children.begin(), m_children.end(),
	[childData](QUaModel<N>::QUaNodeWrapper* child) {
			return child->userData() == childData;
	});
	return res == m_children.end() ? nullptr : *res;
}

template<class N>
inline QModelIndex QUaModel<N>::QUaNodeWrapper::index() const
{
	return m_index;
}

template<class N>
inline void QUaModel<N>::QUaNodeWrapper::setIndex(const QModelIndex& index)
{
	m_index = index;
}

template<class N>
inline typename QUaModel<N>::QUaNodeWrapper* 
	QUaModel<N>::QUaNodeWrapper::parent() const
{
	return m_parent;
}

template<typename N>
template<typename X>
inline
typename std::enable_if<std::is_pointer<X>::value, typename QUaModel<N>::QUaNodeWrapper*>::type
QUaModel<N>::QUaNodeWrapper::childByNode(N node) const
{
	auto res = std::find_if(m_children.begin(), m_children.end(),
		[node](QUaModel<N>::QUaNodeWrapper* wrapper) {
			return QUaModelItemTraits::IsEqual<N>(wrapper->node(), node);
		});
	return res == m_children.end() ? nullptr : *res;
}

template<typename N>
template<typename X>
inline 
typename std::enable_if<!std::is_pointer<X>::value, typename QUaModel<N>::QUaNodeWrapper*>::type
	QUaModel<N>::QUaNodeWrapper::childByNode(N* node) const
{
	auto res = std::find_if(m_children.begin(), m_children.end(),
		[node](QUaModel<N>::QUaNodeWrapper* wrapper) {
			return QUaModelItemTraits::IsEqual<N>(wrapper->node(), node);
		});
	return res == m_children.end() ? nullptr : *res;
}

template<class N>
inline QList<typename QUaModel<N>::QUaNodeWrapper*>& 
	QUaModel<N>::QUaNodeWrapper::children()
{
	return m_children;
}

template<class N>
inline QList<QMetaObject::Connection>& 
	QUaModel<N>::QUaNodeWrapper::connections()
{
	return m_connections;
}

template<class N>
inline std::function<void()> 
	QUaModel<N>::QUaNodeWrapper::getChangeCallbackForColumn(
		const int& column, 
		QAbstractItemModel* model
	)
{
	return [this, column, model]()
	{
		QModelIndex index = (column == m_index.column()) ?
			m_index :
			m_index.siblingAtColumn(column);
		Q_ASSERT(index.isValid());
		emit model->dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
	};
}

#endif // QUANODEMODEL_H


