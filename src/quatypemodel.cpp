#include "quatypemodel.h"

QUaTypeModel::QUaTypeModel(QObject *parent)
    : QUaTableModel<QUaNode*>(parent)
{
    
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
    this->clear();
}
