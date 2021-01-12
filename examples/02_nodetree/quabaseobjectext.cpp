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
    this->addChild<QUaBaseObjectExt>(strName);
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
    this->addFolderObject(strName);
}

void QUaBaseObjectExt::addBaseObjectChild(QString strName)
{
    this->addBaseObject(strName);
}

void QUaBaseObjectExt::addBaseDataVariableChild(QString strName)
{
    auto var = this->addBaseDataVariable(strName);
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
    auto prop = this->addProperty(strName);
    prop->setWriteAccess(true);
    prop->setDataType(QMetaType::Double);
}
