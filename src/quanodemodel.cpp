#include "quanodemodel.h"
#include <QUaNode>

class QUaNodeModel::QUaNodeWrapper
{
public:
    explicit QUaNodeWrapper(QUaNode* node, QUaNodeWrapper* parent = nullptr) : 
        m_node(node), 
        m_parent(parent)
    {
        Q_ASSERT_X(m_node, "QUaNodeWrapper", "Invalid node argument");
        // subscribe to node destruction, store connection to disconnect on destructor
        m_connections <<
        QObject::connect(m_node, &QObject::destroyed,
        [this]() {
            this->m_node = nullptr;
        });
        // build children tree
        for (auto child : m_node->browseChildren())
        {
            m_children << new QUaNodeWrapper(child, this);
        }
    };

    QUaNodeWrapper::~QUaNodeWrapper()
    {
        while (m_connections.count() > 0)
        {
            QObject::disconnect(m_connections.takeFirst());
        }
        qDeleteAll(m_children);
    }

    std::function<void()> getChangeCallbackForColumn(const int& column, QAbstractItemModel *model)
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

    // internal data
    QUaNode* m_node;
    // NOTE : need index to support model manipulation 
    //        cannot use createIndex outside Qt's API
    //        (forbiden even inside a QAbstractItemModel member)
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

QUaNodeModel::QUaNodeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_root = nullptr;
    m_columnCount = 1;
}

QUaNode* QUaNodeModel::rootNode() const
{
    return m_root ? m_root->m_node : nullptr;
}

void QUaNodeModel::setRootNode(QUaNode* rootNode/* = nullptr*/)
{
    this->bindRoot(rootNode ? new QUaNodeWrapper(rootNode) : nullptr);
}

QUaNode* QUaNodeModel::nodeFromIndex(const QModelIndex& index) const
{
    if (!this->checkIndex(index, CheckIndexOption::IndexIsValid))
    {
        return m_root->m_node;
    }
    auto wrapper = static_cast<QUaNodeWrapper*>(index.internalPointer());
    Q_CHECK_PTR(wrapper);
    return wrapper->m_node;
}

void QUaNodeModel::bindRoot(QUaNodeWrapper* root)
{
    if (m_root == root)
    {
        return;
    }
    // notify views all old data is invalid
    this->beginResetModel();
    // if old root node was valid, disconnect to recv signals recursivelly
    if (m_root)
    {
        this->unbindNodeRecursivelly(m_root);
        delete m_root;
        m_root = nullptr;
    }
    // copy
    m_root = root;
    // subscribe to changes
    if (m_root)
    {
        this->bindRecursivelly(m_root);
    }
    // notify views new data is available
    this->endResetModel();
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
        this->bindChangeCallbackForColumnRecursivelly(column, m_root);
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
    if (!m_root || !m_root->m_node)
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
    if (!m_root || !m_root->m_node || !this->hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    QUaNodeWrapper* parentNode;
    // invalid parent index is root, else get internal reference
    if (!parent.isValid())
    {
		parentNode = m_root;
    }
    else
    {
        parentNode = static_cast<QUaNodeWrapper*>(parent.internalPointer());
    }
    Q_CHECK_PTR(parentNode);
    // browse n-th child wrapper node
    QUaNodeWrapper* childItem = parentNode->m_children.count() > row ?
        parentNode->m_children.at(row) : nullptr;
    if (!childItem)
    {
        return QModelIndex();
	}
	// create index
	QModelIndex index = this->createIndex(row, column, childItem);
    // store index to support model manipulation
    if (column == 0)
    {
	    childItem->m_index = index;
    }
	return index;
}

QModelIndex QUaNodeModel::parent(const QModelIndex &index) const
{
    if (!m_root || !m_root->m_node || !index.isValid())
    {
        return QModelIndex();
    }
    auto intId = index.internalId();
    Q_UNUSED(intId);
    // get child and parent node references
    QUaNodeWrapper* childNode  = static_cast<QUaNodeWrapper*>(index.internalPointer());
    Q_CHECK_PTR(childNode);
    QUaNodeWrapper* parentNode = static_cast<QUaNodeWrapper*>(childNode->m_parent);
    Q_CHECK_PTR(parentNode);
    if (parentNode == m_root)
    {
        // store index to support model manipulation
        QModelIndex pIndex = QModelIndex();
        parentNode->m_index = pIndex;
        return pIndex;
    }
    // get row of parent if grandparent is valid
    int row = 0;
    QUaNodeWrapper* grandpaNode = static_cast<QUaNodeWrapper*>(parentNode->m_parent);
    if (grandpaNode)
    {
        row = grandpaNode->m_children.indexOf(parentNode);
    }
    // store index to support model manipulation
    QModelIndex pIndex = this->createIndex(row, 0, parentNode);
    parentNode->m_index = pIndex;
    return pIndex;
}

int QUaNodeModel::rowCount(const QModelIndex &parent) const
{
    if (!m_root || !m_root->m_node || parent.column() > 0)
    {
        return 0;
    }
    QUaNodeWrapper* parentNode;
    // get internal wrapper reference
    if (!parent.isValid())
    {
        parentNode = m_root;
    }
    else
    {
        parentNode = static_cast<QUaNodeWrapper*>(parent.internalPointer());
    }
    // return number of children
    Q_CHECK_PTR(parentNode);
    int childCount = parentNode->m_children.count();
    return childCount;
}

int QUaNodeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (!m_root || !m_root->m_node)
    {
        return 0;
    }
    // minimum 1 column
    return m_columnCount;
}

QVariant QUaNodeModel::data(const QModelIndex& index, int role) const
{
    // early exit for inhandled cases
    if (!m_root || !m_root->m_node || !index.isValid())
    {
        return QVariant();
    }
    // only display data (text)
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    // get internal reference
    QUaNodeWrapper* node = static_cast<QUaNodeWrapper*>(index.internalPointer());
    // check internal wrapper data is valid, because node->m_node is always deleted before node
    if (!node->m_node)
    {
        return QVariant();
    }
    // default implementation if no ColumnDataSource has been defined
    if (m_mapDataSourceFuncs.isEmpty())
    {
        Q_ASSERT(m_columnCount == 1);
        return node->m_node->displayName();
    }
    // empty if no ColumnDataSource defined for this column
    if (!m_mapDataSourceFuncs.contains(index.column()) || 
        !m_mapDataSourceFuncs[index.column()].m_dataCallback)
    {
        return QVariant();
    }
    // use user-defined ColumnDataSource
    return m_mapDataSourceFuncs[index.column()].m_dataCallback(node->m_node);
}

Qt::ItemFlags QUaNodeModel::flags(const QModelIndex &index) const
{
    if (!m_root || !m_root->m_node || !index.isValid())
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
    QUaNodeWrapper* node = static_cast<QUaNodeWrapper*>(index.internalPointer());
    if (!node->m_node)
    {
        return flags;
    }
    // test callback returns true
    if (!m_mapDataSourceFuncs[index.column()].m_editableCallback(node->m_node))
    {
        return flags;
    }
    // finally, after all this, item is editable
    return flags |= Qt::ItemIsEditable;
}

void QUaNodeModel::bindRecursivelly(QUaNodeWrapper* node)
{
    // subscribe to node removed
    // unbind tree must run inmediatly
    QObject::connect(node->m_node, &QObject::destroyed, this,
    [this, node]() {
        Q_CHECK_PTR(node);
        // stop all events for sub-tree so children's QObject::destroyed is not handled
        this->unbindNodeRecursivelly(node);
    }, Qt::DirectConnection);
    // remove rows better be queued in the event loop
    QObject::connect(node->m_node, &QObject::destroyed, this,
    [this, node]() {
        Q_CHECK_PTR(node);
        if (node == m_root)
        {
            this->setRootNode(nullptr);
            return;
        }
        // NOTE : node->m_parent must be valid because (node == m_root) already handled
        Q_ASSERT(node->m_parent);
        // only use indexes created by model
        int row = node->m_index.row();
        QModelIndex index = node->m_parent->m_index;
        Q_ASSERT(node->m_parent == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be removed
        this->beginRemoveRows(index, row, row);
        // remove from parent, destructor deletes wrapper sub-tree recursivelly
        Q_ASSERT(node == node->m_parent->m_children.at(row));
        delete node->m_parent->m_children.takeAt(row);
        // notify views that row removal has finished
        this->endRemoveRows();
    }, Qt::QueuedConnection);
    // subscribe to new child node added
    // insert rows better be queued in the event loop
    QObject::connect(node->m_node, &QUaNode::childAdded, this,
    [this, node](QUaNode* childNode) {
        // get new node's row
        int row = node->m_children.count();
        // only use indexes created by model
        QModelIndex index = node->m_index;
        Q_ASSERT(node == m_root || this->checkIndex(index, CheckIndexOption::IndexIsValid));
        // notify views that row will be added
        this->beginInsertRows(index, row, row);
        // create new wrapper
        auto* wrapper = new QUaNodeWrapper(childNode, node);
        // apprend to parent's children list
        node->m_children << wrapper;
        // bind new instance for changes
        this->bindRecursivelly(wrapper);
        // notify views that row addition has finished
        this->endInsertRows();
        // force index creation (indirectly)
        // because sometimes they are not created until a view requires them
        // and if a child is added and parent's index is not ready then crash
        bool indexOk = this->checkIndex(this->index(row, 0, index), CheckIndexOption::IndexIsValid);
        Q_ASSERT(indexOk);
        Q_UNUSED(indexOk);
    }, Qt::QueuedConnection);
    // bind callback for data change on each column
    if (!m_mapDataSourceFuncs.isEmpty())
    {
        for (auto column : m_mapDataSourceFuncs.keys())
        {
            this->bindChangeCallbackForColumnRecursivelly(column, node);
        }
    }
    // recurse children
    for (auto child : node->m_children)
    {
        this->bindRecursivelly(child);
    }
}

void QUaNodeModel::unbindNodeRecursivelly(QUaNodeWrapper* node)
{
    Q_CHECK_PTR(node);
    // disconnect from internal node
    if (node->m_node)
	{
		this->disconnect(node->m_node);
		node->m_node->disconnect(this);
    }
    // disconnect children
    for (auto child : node->m_children)
    {
        this->unbindNodeRecursivelly(child);
    }
}

void QUaNodeModel::bindChangeCallbackForColumnRecursivelly(const int& column, QUaNodeWrapper* node)
{
    Q_CHECK_PTR(node);
    if (node->m_node && m_mapDataSourceFuncs[column].m_changeCallback)
    {
        // pass in callback that user needs to call when a value is udpated
        // store connection in wrapper so can be disconnected when wrapper deleted
        node->m_connections <<
        m_mapDataSourceFuncs[column].m_changeCallback(
            node->m_node, 
            node->getChangeCallbackForColumn(column, this)
        );
    }
    // recurse children
    for (auto child : node->m_children)
    {
        this->bindChangeCallbackForColumnRecursivelly(column, child);
    }
}
