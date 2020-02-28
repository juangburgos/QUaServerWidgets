#include "quatypemodel.h"

QUaTypeModel::QUaTypeModel(QObject *parent)
    : QUaNodeModel(parent)
{
    m_root = new QUaNodeWrapper(nullptr);
}

QUaTypeModel::~QUaTypeModel()
{
    if (m_root)
    {
        this->unbindAll();
        delete m_root;
        m_root = nullptr;
    }
}

void QUaTypeModel::unbindAll()
{
    while (m_connections.count() > 0)
    {
        QObject::disconnect(m_connections.take(m_connections.begin().key()));
    }
    this->beginResetModel();
    while(m_root->children().count() > 0)
    {
        auto wrapper = m_root->children().takeFirst();
        this->disconnect(wrapper->node());
        wrapper->node()->disconnect(this);
        delete wrapper;
    }
    this->endResetModel();
}
