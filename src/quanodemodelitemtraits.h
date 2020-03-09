#ifndef QUANODEMODELITEMTRAITS_H
#define QUANODEMODELITEMTRAITS_H

#include <QUaNode>
#include <QUaModelItemTraits>

// specialized implementation for QUaNode*

template<>
inline QMetaObject::Connection 
QUaModelItemTraits::DestroyCallback<QUaNode*>(QUaNode* node, std::function<void(void)> callback)
{
    return QObject::connect(node, &QObject::destroyed,
    [callback]() {
        callback();
    });
}

template<>
inline QList<QUaNode*> 
QUaModelItemTraits::GetChildren<QUaNode*>(QUaNode* node)
{
    return node->browseChildren();
}

#endif // QUANODEMODELITEMTRAITS_H