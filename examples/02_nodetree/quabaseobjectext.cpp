#include "quabaseobjectext.h"

#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>

#include <QRandomGenerator>
#include <QElapsedTimer>

QTimer QUaBaseObjectExt::m_timer;

QUaBaseObjectExt::QUaBaseObjectExt(QUaServer *server)
    : QUaBaseObject(server)
{
    if (m_timer.isActive())
    {
        return;
    }
    m_timer.start(50);
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
    // simulate value changes to measure model-view performance
    var->setDataType(QMetaType::Type::Int);
    QElapsedTimer* timer = new QElapsedTimer();
    quint32 timeout = QRandomGenerator::global()->bounded(250, 2000);
    QObject::connect(&m_timer, &QTimer::timeout, var,
    [var, timer, timeout]() {
        static quint32 counter = 0;
        if (timer->elapsed() < timeout)
        {
            return;
        }
        var->setValue(counter++);
        emit var->valueChanged(QVariant());
        timer->restart();
    });
    QObject::connect(var, &QObject::destroyed,
    [timer]() {
        delete timer;
    });
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
