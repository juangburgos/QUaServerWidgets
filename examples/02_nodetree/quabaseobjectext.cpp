#include "quabaseobjectext.h"

#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

#include <QTimer>
#include <QRandomGenerator>

QUaBaseObjectExt::QUaBaseObjectExt(QUaServer *server)
    : QUaBaseObject(server)
{

}

void QUaBaseObjectExt::addObjectExtChild(QString strName)
{
    auto child = this->addChild<QUaBaseObjectExt>();
    child->setDisplayName(strName);
    child->setBrowseName(strName);
}

void QUaBaseObjectExt::addMulitpleObjectExtChild(QString strBaseName, quint32 numChildren)
{
    for (quint32 i = 0; i < numChildren; i++)
    {
        this->addObjectExtChild(QString("%1%2").arg(strBaseName).arg(i));
    }
}

void QUaBaseObjectExt::remove()
{
    this->deleteLater();
}

void QUaBaseObjectExt::addFolderChild(QString strName)
{
    auto folder = this->addFolderObject();
    folder->setDisplayName(strName);
    folder->setBrowseName(strName);
}

void QUaBaseObjectExt::addBaseObjectChild(QString strName)
{
    auto obj = this->addBaseObject();
    obj->setDisplayName(strName);
    obj->setBrowseName(strName);
}

void QUaBaseObjectExt::addBaseDataVariableChild(QString strName)
{
    auto var = this->addBaseDataVariable();
    var->setDisplayName(strName);
    var->setBrowseName(strName);
    var->setWriteAccess(true);
    // TODO : remove when editing is supported
    var->setDataType(QMetaType::Type::Int);
    QTimer * timer = new QTimer(var);
    QObject::connect(timer, &QTimer::timeout, var, 
    [var]() {
        var->setValue(QRandomGenerator::global()->generate());
        emit var->valueChanged(QVariant());
    });
    timer->start(QRandomGenerator::global()->bounded(250, 2000));
}

void QUaBaseObjectExt::addMultipleBaseDataVariableChild(QString strBaseName, quint32 numChildren)
{
    for (quint32 i = 0; i < numChildren; i++)
    {
        this->addBaseDataVariableChild(QString("%1%2").arg(strBaseName).arg(i));
    }
}

void QUaBaseObjectExt::addPropertyChild(QString strName)
{
    auto prop = this->addProperty();
    prop->setDisplayName(strName);
    prop->setBrowseName(strName);
    prop->setWriteAccess(true);
}
