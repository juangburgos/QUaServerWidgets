#include "quabaseobjectext.h"

#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

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
}

void QUaBaseObjectExt::addPropertyChild(QString strName)
{
    auto prop = this->addProperty();
    prop->setDisplayName(strName);
    prop->setBrowseName(strName);
}
