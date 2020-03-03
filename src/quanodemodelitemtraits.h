#ifndef QUANODEMODELITEMTRAITS_H
#define QUANODEMODELITEMTRAITS_H

#include <QUaModelItemTraits>
#include <QUaNode>

// activate specialized implementation for QUaNode*
template<>
struct QUaHasModelItemTraits<QUaNode*> {
    static const bool value = true;
};

inline QMetaObject::Connection DestroyCallbackTrait(QUaNode* node, std::function<void(void)> callback)
{
    return QObject::connect(node, &QObject::destroyed,
    [callback]() {
        callback();
    });
}

inline QList<QUaNode*> GetChildrenTrait(QUaNode* node)
{
    return node->browseChildren();
}

#endif // QUANODEMODELITEMTRAITS_H