#ifndef QUATYPEMODEL_H
#define QUATYPEMODEL_H

#include <QUaNodeModel>
#include <QUaServer>

class QUaTypeModel : public QUaNodeModel
{
    Q_OBJECT

public:
    explicit QUaTypeModel(QObject *parent = nullptr);
    ~QUaTypeModel();

    template<typename T>
    void bindType(QUaServer* server);

    //template<typename T>
    //void unbindType();

    void unbindAll();

private:
    template<typename T>
    void bindInstance(T* instance);
    
    QHash<QString, QMetaObject::Connection> m_connections;
};

template<typename T>
inline void QUaTypeModel::bindType(QUaServer* server)
{
    auto metaObject = T::staticMetaObject;
    // check if OPC UA relevant
    if (!metaObject.inherits(&QUaNode::staticMetaObject))
    {
        Q_ASSERT_X(false, 
            "QUaTypeModel::bindType", 
            "Unsupported type. It must derive from QUaNode.");
        return;
    }
    // check not bound yet
    QString strTypeName = QString(metaObject.className());
    if (m_connections.contains(strTypeName))
    {
        Q_ASSERT_X(false, 
            "QUaTypeModel::bindType", 
            "Type already bound.");
        return;
    }
    // bind existing children
    auto instances = server->typeInstances<T>();
    for (auto instance : instances)
    {
        this->bindInstance<T>(instance);
    }
    // bind new children
    m_connections[strTypeName] =
    server->instanceCreated<T>([this](T * instance) {
        this->bindInstance<T>(instance);
    });
}

template<typename T>
inline void QUaTypeModel::bindInstance(T* instance)
{
    QModelIndex index = m_root->index();
    // get new node's row
    int row = m_root->children().count();
    // notify views that row will be added
    this->beginInsertRows(index, row, row);
    // create new wrapper
    auto* wrapper = new QUaNodeWrapper(instance, m_root, false);
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
    QObject::connect(instance, &QObject::destroyed, this,
    [this, wrapper]() {
        Q_CHECK_PTR(wrapper);
        Q_ASSERT(m_root);
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
    }, Qt::QueuedConnection);
}



#endif // QUATYPEMODEL_H

