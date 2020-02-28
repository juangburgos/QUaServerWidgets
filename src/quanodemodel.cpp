#include "quanodemodel.h"
#include <QUaNode>

QUaNodeModel::QUaNodeWrapper::QUaNodeWrapper(
    QUaNode* node, 
    QUaNodeModel::QUaNodeWrapper* parent/* = nullptr*/,
    const bool& recursive/* = true*/) :
        m_node(node), 
        m_parent(parent)
{
    // m_node = null only supported if this is root (i.e. m_parent = null)
    Q_ASSERT_X(m_node ? true : !m_parent, "QUaNodeWrapper", "Invalid node argument");
    // nothing else to do if root
    if (!m_node)
    {
        return;
    }
    // subscribe to node destruction, store connection to disconnect on destructor
    m_connections <<
    QObject::connect(m_node, &QObject::destroyed,
    [this]() {
        this->m_node = nullptr;
    });
    // check if need to add children
    if (!recursive)
    {
        return;
    }
    // build children tree
    for (auto child : m_node->browseChildren())
    {
        m_children << new QUaNodeModel::QUaNodeWrapper(child, this);
    }
}

QUaNodeModel::QUaNodeWrapper::QUaNodeWrapper::~QUaNodeWrapper()
{
    while (m_connections.count() > 0)
    {
        QObject::disconnect(m_connections.takeFirst());
    }
    qDeleteAll(m_children);
}

QUaNode* QUaNodeModel::QUaNodeWrapper::node() const
{
    return m_node;
}

QModelIndex QUaNodeModel::QUaNodeWrapper::index() const
{
    return m_index;
}

void QUaNodeModel::QUaNodeWrapper::setIndex(const QModelIndex& index)
{
    m_index = index;
}

QUaNodeModel::QUaNodeWrapper* QUaNodeModel::QUaNodeWrapper::parent() const
{
    return m_parent;
}

QUaNodeModel::QUaNodeWrapper* QUaNodeModel::QUaNodeWrapper::childByNode(QUaNode* node) const
{
    auto res = std::find_if(m_children.begin(), m_children.end(), 
    [node](QUaNodeModel::QUaNodeWrapper* wrapper) {
        return wrapper->m_node == node;
    });
    return res == m_children.end() ? nullptr : *res;
}

QList<QUaNodeModel::QUaNodeWrapper*>& QUaNodeModel::QUaNodeWrapper::children()
{
    return m_children;
}

QList<QMetaObject::Connection>& QUaNodeModel::QUaNodeWrapper::connections()
{
    return m_connections;
}

std::function<void()> QUaNodeModel::QUaNodeWrapper::getChangeCallbackForColumn(const int& column, QAbstractItemModel *model)
{
    return [this, column, model]()
    {
        QModelIndex index = column == m_index.column() ? 
            m_index :
            m_index.siblingAtColumn(column);
        Q_ASSERT(index.isValid());
        emit model->dataChanged(index, index, QVector<int>() << Qt::DisplayRole);
    };
}

QUaNodeModel::QUaNodeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_root = nullptr;
    m_columnCount = 1;
}

QUaNodeModel::~QUaNodeModel()
{
    if (m_root)
    {
        delete m_root;
        m_root = nullptr;
    }
}

QUaNode* QUaNodeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
    {
        return m_root->node();
    }
    auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
    Q_CHECK_PTR(wrapper);
    return wrapper->node();
}

void QUaNodeModel::setColumnDataSource(
    const int& column, 
    const QString& strHeader, 
    std::function<QVariant(QUaNode*)> dataCallback, 
    std::function<QMetaObject::Connection(QUaNode*, std::function<void()>)> changeCallback/* = nullptr*/,
    std::function<bool(QUaNode*)> editableCallback/* = nullptr*/
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

void QUaNodeModel::removeColumnDataSource(const int& column)
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

QVariant QUaNodeModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        return tr("Display Name");
    }
    // empty if no ColumnDataSource defined for this column
    if (!m_mapDataSourceFuncs.contains(section))
    {
        return QVariant();
    }
    // use user-defined ColumnDataSource
    return m_mapDataSourceFuncs[section].m_strHeader;
}

QModelIndex QUaNodeModel::index(int row, int column, const QModelIndex &parent) const
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

QModelIndex QUaNodeModel::parent(const QModelIndex &index) const
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

int QUaNodeModel::rowCount(const QModelIndex &parent) const
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

int QUaNodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_root)
    {
        return 0;
    }
    // minimum 1 column
    return m_columnCount;
}

QVariant QUaNodeModel::data(const QModelIndex& index, int role) const
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
    if (!wrapper->node())
    {
        return QVariant();
    }
    // default implementation if no ColumnDataSource has been defined
    if (m_mapDataSourceFuncs.isEmpty())
    {
        Q_ASSERT(m_columnCount == 1);
        return wrapper->node()->displayName();
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

Qt::ItemFlags QUaNodeModel::flags(const QModelIndex &index) const
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
    if (!wrapper->node())
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

void QUaNodeModel::bindChangeCallbackForColumn(
    const int& column, 
    QUaNodeWrapper* wrapper,
    const bool& recursive/* = true*/)
{
    Q_CHECK_PTR(wrapper);
    if (wrapper->node() && m_mapDataSourceFuncs[column].m_changeCallback)
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

void QUaNodeModel::bindChangeCallbackForAllColumns(
    QUaNodeWrapper* wrapper,
    const bool& recursive)
{
    if (!m_mapDataSourceFuncs.isEmpty())
    {
        for (auto column : m_mapDataSourceFuncs.keys())
        {
            this->bindChangeCallbackForColumn(column, wrapper, recursive);
        }
    }
}
