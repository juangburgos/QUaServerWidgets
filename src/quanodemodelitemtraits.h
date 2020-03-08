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

inline QList<QUaNode*> GetChildrenTrait(const QUaNode* node)
{
    return node->browseChildren();
}

inline bool IsValidTrait(const QUaNode* node)
{
    return node;
}

inline QUaNode* GetInvalidTrait()
{
    return nullptr;
}

inline bool IsEqualTrait(const QUaNode* node1, const QUaNode* node2)
{
    return node1 == node2;
}

#endif // QUANODEMODELITEMTRAITS_H