#ifndef QUATYPEMODEL_H
#define QUATYPEMODEL_H

#include <QUaNodeModelItemTraits>
#include <QUaTableModel>
#include <QUaServer>

class QUaNodeTypeModel : public QUaTableModel<QUaNode*, 0>
{
    Q_OBJECT

public:
    explicit QUaNodeTypeModel(QObject *parent = nullptr);
    ~QUaNodeTypeModel();

    template<typename T>
    void bindType(QUaServer* server);

    template<typename T>
    void unbindType();

    void unbindAll();

private:
    
    QHash<QString, QMetaObject::Connection> m_connections;
};

inline QUaNodeTypeModel::QUaNodeTypeModel(QObject* parent)
    : QUaTableModel<QUaNode*, 0>(parent)
{

}

inline QUaNodeTypeModel::~QUaNodeTypeModel()
{
    if (m_root)
    {
        this->unbindAll();
        delete m_root;
        m_root = nullptr;
    }
}

template<typename T>
inline void QUaNodeTypeModel::bindType(QUaServer* server)
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
        this->addNode(instance);
    }
    // bind new children
    m_connections[strTypeName] =
    server->instanceCreated<T>([this](T * instance) {
        this->addNode(instance);
    });
}

template<typename T>
inline void QUaNodeTypeModel::unbindType()
{
    auto metaObject = T::staticMetaObject;
    // check if OPC UA relevant
    if (!metaObject.inherits(&QUaNode::staticMetaObject))
    {
        Q_ASSERT_X(false,
            "QUaTypeModel::unbindType",
            "Unsupported type. It must derive from QUaNode.");
        return;
    }
    // check not bound yet
    QString strTypeName = QString(metaObject.className());
    if (!m_connections.contains(strTypeName))
    {
        Q_ASSERT_X(false,
            "QUaTypeModel::unbindType",
            "Type is not bound.");
        return;
    }
    // unbind new children
    QObject::disconnect(m_connections.take(strTypeName));
    // unbind existing children
    this->beginResetModel();
    m_root->children().erase(
	std::remove_if(m_root->children().begin(), m_root->children().end(),
	[this, &strTypeName](QUaModel::QUaNodeWrapper * wrapper) {
        bool isOfType = strTypeName.compare(wrapper->node()->metaObject()->className(), Qt::CaseSensitive) == 0;
        if (isOfType)
        {
            delete wrapper;
        }
        return isOfType;
	}),
    m_root->children().end());
    this->endResetModel();
}

inline void QUaNodeTypeModel::unbindAll()
{
    while (m_connections.count() > 0)
    {
        QObject::disconnect(m_connections.take(m_connections.begin().key()));
    }
    this->clear();
}

#endif // QUATYPEMODEL_H

