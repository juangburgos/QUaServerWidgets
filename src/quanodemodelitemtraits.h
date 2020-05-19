#ifndef QUANODEMODELITEMTRAITS_H
#define QUANODEMODELITEMTRAITS_H

#include <QUaBaseVariable>
#include <QUaModelItemTraits>

// specialized implementation for QUaNode*

template<> static
inline QMetaObject::Connection 
QUaModelItemTraits::DestroyCallback<QUaNode*, 0>(
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

template<> static
inline QMetaObject::Connection
QUaModelItemTraits::NewChildCallback<QUaNode*, 0>(
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

template<> static
inline QList<QUaNode*> 
QUaModelItemTraits::GetChildren<QUaNode*, 0>(QUaNode* node)
{
    if (!node)
    {
        return QList<QUaNode*>();
    }
    return node->browseChildren();
}

// overload to support default editor (QStyledItemDelegate::setEditorData)
// implement either this or ui->myview->setColumnEditor
// setColumnEditor has preference in case both implemented
template<> static
inline bool
QUaModelItemTraits::SetData<QUaNode*, 0>(
    QUaNode* node,
    const int& column,
    const QVariant& value)
{
    Q_UNUSED(column);
    QString strType(node->metaObject()->className());
    // only set value for variables
    if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
        strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
    {
        return false;
    }
    auto var = qobject_cast<QUaBaseVariable*>(node);
    Q_CHECK_PTR(var);
    var->setValue(value);
    return true;
}

#endif // QUANODEMODELITEMTRAITS_H