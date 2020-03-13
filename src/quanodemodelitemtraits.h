#ifndef QUANODEMODELITEMTRAITS_H
#define QUANODEMODELITEMTRAITS_H

#include <QUaNode>
#include <QUaModelItemTraits>

// specialized implementation for QUaNode*

template<>
inline QMetaObject::Connection 
QUaModelItemTraits::DestroyCallback<QUaNode*>(
    QUaNode* node, 
    const std::function<void(void)> &callback)
{
    if (!node)
    {
        return QMetaObject::Connection();
    }
    return QObject::connect(node, &QObject::destroyed,
    [callback]() {
        callback();
    });
}

template<>
inline QMetaObject::Connection
QUaModelItemTraits::NewChildCallback<QUaNode*>(
    QUaNode* node, 
    const std::function<void(QUaNode*)> &callback)
{
    if (!node)
    {
        return QMetaObject::Connection();
    }
    return QObject::connect(node, &QUaNode::childAdded,
    [callback](QUaNode* child) {
        callback(child);
    });
}

template<>
inline QList<QUaNode*> 
QUaModelItemTraits::GetChildren<QUaNode*>(QUaNode* node)
{
    if (!node)
    {
        return QList<QUaNode*>();
    }
    return node->browseChildren();
}

#endif // QUANODEMODELITEMTRAITS_H