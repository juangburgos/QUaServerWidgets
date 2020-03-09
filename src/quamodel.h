#ifndef QUANODEMODEL_H
#define QUANODEMODEL_H

#include <QAbstractItemModel>
#include <QUaModelItemTraits>

template <class T>
class QUaModel : public QAbstractItemModel
{

public:
    explicit QUaModel(QObject *parent = nullptr);
    ~QUaModel();

    T nodeFromIndex(const QModelIndex& index) const;

    void setColumnDataSource(
        const int& column, 
        const QString &strHeader, 
        std::function<QVariant(T)> dataCallback,
        std::function<QMetaObject::Connection(T, std::function<void()>)> changeCallback = nullptr,
        std::function<bool(T)> editableCallback = nullptr
    );
    void removeColumnDataSource(const int& column);

    // Qt required API:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

protected:

    class QUaNodeWrapper
    {
    public:
        explicit QUaNodeWrapper(
            T node, 
            QUaModel<T>::QUaNodeWrapper* parent = nullptr,
            const bool &recursive = true);
        ~QUaNodeWrapper();

        T node() const;

        QModelIndex index() const;
        void setIndex(const QModelIndex &index);

        QUaModel<T>::QUaNodeWrapper* parent() const;
        QUaModel<T>::QUaNodeWrapper* childByNode(T node) const;

        // NOTE : return by reference
        QList<QUaModel<T>::QUaNodeWrapper*> & children();
        QList<QMetaObject::Connection> & connections();

        std::function<void()> getChangeCallbackForColumn(const int& column, QAbstractItemModel* model);

    private:
        // internal data
        T m_node;
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
    int m_columnCount;

    void bindChangeCallbackForColumn(
        const int& column,
        QUaNodeWrapper* wrapper,
        const bool& recursive = true);

    void bindChangeCallbackForAllColumns(
        QUaNodeWrapper* wrapper,
        const bool& recursive = true);

    struct ColumnDataSource
    {
        QString m_strHeader;
        std::function<QVariant(T)> m_dataCallback;
        std::function<QMetaObject::Connection(T, std::function<void()>)> m_changeCallback;
        std::function<bool(T)> m_editableCallback;
    };
    QMap<int, ColumnDataSource> m_mapDataSourceFuncs;
};

template<class T>
inline QUaModel<T>::QUaModel(QObject* parent) :
	QAbstractItemModel(parent)
{
	m_root = nullptr;
	m_columnCount = 1;
}

template<class T>
inline QUaModel<T>::~QUaModel()
{
	if (m_root)
	{
		delete m_root;
		m_root = nullptr;
	}
}

template<class T>
inline T QUaModel<T>::nodeFromIndex(const QModelIndex& index) const
{
	if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
	{
		return m_root->node();
	}
	auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
	Q_CHECK_PTR(wrapper);
	return wrapper->node();
}

template<class T>
inline void QUaModel<T>::setColumnDataSource(
    const int& column, 
    const QString& strHeader, 
    std::function<QVariant(T)> dataCallback, 
    std::function<QMetaObject::Connection(T, std::function<void()>)> changeCallback, 
    std::function<bool(T)> editableCallback)
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

template<class T>
inline void QUaModel<T>::removeColumnDataSource(const int& column)
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

template<class T>
inline QVariant QUaModel<T>::headerData(int section, Qt::Orientation orientation, int role) const
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

template<class T>
inline QModelIndex QUaModel<T>::index(int row, int column, const QModelIndex& parent) const
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

template<class T>
inline QModelIndex QUaModel<T>::parent(const QModelIndex& index) const
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

template<class T>
inline int QUaModel<T>::rowCount(const QModelIndex& parent) const
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

template<class T>
inline int QUaModel<T>::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	if (!m_root)
	{
		return 0;
	}
	// minimum 1 column
	return m_columnCount;
}

template<class T>
inline QVariant QUaModel<T>::data(const QModelIndex& index, int role) const
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
	auto wrapper = static_cast<QUaModel<T>::QUaNodeWrapper*>(index.internalPointer());
	// check internal wrapper data is valid, because wrapper->node() is always deleted before wrapper
	if(!QUaModelItemTraits::IsValid<T>(wrapper->node()))
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

template<class T>
inline Qt::ItemFlags QUaModel<T>::flags(const QModelIndex& index) const
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
	if(!QUaModelItemTraits::IsValid<T>(wrapper->node()))
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

template<class T>
inline void QUaModel<T>::bindChangeCallbackForColumn(
	const int& column, 
	QUaNodeWrapper* wrapper, 
	const bool& recursive
)
{
	Q_CHECK_PTR(wrapper);
	if (QUaModelItemTraits::IsValid<T>(wrapper->node()) &&
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

template<class T>
inline void QUaModel<T>::bindChangeCallbackForAllColumns(
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

template<class T>
inline QUaModel<T>::QUaNodeWrapper::QUaNodeWrapper(
	T node, 
	QUaModel<T>::QUaNodeWrapper* parent/* = nullptr*/,
	const bool& recursive/* = true*/) :
	m_node(node),
	m_parent(parent)
{
	// m_node = null only supported if this is root (i.e. m_parent = null)
	Q_ASSERT_X(QUaModelItemTraits::IsValid<T>(m_node) ? 
		true : 
		!m_parent, 
		"QUaNodeWrapper", "Invalid node argument"
	);
	// nothing else to do if root
	if (!QUaModelItemTraits::IsValid<T>(m_node))
	{
		return;
	}
	// subscribe to node destruction, store connection to disconnect on destructor
	QMetaObject::Connection conn = QUaModelItemTraits::DestroyCallback<T>(
			m_node, 
			(std::function<void(void)>)[this]() {
				this->m_node = 
					QUaModelItemTraits::GetInvalid<T>();
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
	QList<T> children = QUaModelItemTraits::GetChildren<T>(m_node);
	for (auto child : children)
	{
		m_children << new QUaModel<T>::QUaNodeWrapper(child, this);
	}
}

template<class T>
inline QUaModel<T>::QUaNodeWrapper::~QUaNodeWrapper()
{
	while (m_connections.count() > 0)
	{
		QObject::disconnect(m_connections.takeFirst());
	}
	qDeleteAll(m_children);
}

template<class T>
inline T QUaModel<T>::QUaNodeWrapper::node() const
{
	return m_node;
}

template<class T>
inline QModelIndex QUaModel<T>::QUaNodeWrapper::index() const
{
	return m_index;
}

template<class T>
inline void QUaModel<T>::QUaNodeWrapper::setIndex(const QModelIndex& index)
{
	m_index = index;
}

template<class T>
inline typename QUaModel<T>::QUaNodeWrapper* 
	QUaModel<T>::QUaNodeWrapper::parent() const
{
	return m_parent;
}

template<class T>
inline typename QUaModel<T>::QUaNodeWrapper * 
	QUaModel<T>::QUaNodeWrapper::childByNode(T node) const
{
	auto res = std::find_if(m_children.begin(), m_children.end(),
	[node](QUaModel<T>::QUaNodeWrapper* wrapper) {
		return QUaModelItemTraits::IsEqual<T>(wrapper->m_node, node);
	});
	return res == m_children.end() ? nullptr : *res;
}

template<class T>
inline QList<typename QUaModel<T>::QUaNodeWrapper*>& 
	QUaModel<T>::QUaNodeWrapper::children()
{
	return m_children;
}

template<class T>
inline QList<QMetaObject::Connection>& 
	QUaModel<T>::QUaNodeWrapper::connections()
{
	return m_connections;
}

template<class T>
inline std::function<void()> 
	QUaModel<T>::QUaNodeWrapper::getChangeCallbackForColumn(
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
