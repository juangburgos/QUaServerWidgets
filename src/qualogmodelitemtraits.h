#ifndef QUALOGMODELITEMTRAITS_H
#define QUALOGMODELITEMTRAITS_H

#include <QUaServer>
#include <QUaModelItemTraits>

// specialized implementation for QUaLog

template<>
inline bool 
QUaModelItemTraits::IsValid<QUaLog>(const QUaLog *node)
{
    // valid if not null
    return !node->message.isNull();
}

template<>
inline bool 
QUaModelItemTraits::IsEqual<QUaLog>(const QUaLog* node1, const QUaLog* node2)
{
    return 
        node1->message   == node2->message  &&
        node1->level     == node2->level    &&
        node1->category  == node2->category &&
        node1->timestamp == node2->timestamp;
}

inline bool operator==(const QUaLog& node1, const QUaLog& node2)
{
    return QUaModelItemTraits::IsEqual<QUaLog>(&node1, &node2);
}

// overload to support default editor (QStyledItemDelegate::setEditorData)
// implement either this or ui->myview->setColumnEditor
// setColumnEditor has preference in case both implemented
template<>
inline bool
QUaModelItemTraits::SetData<QUaLog>(
    QUaLog * node, 
    const int& column, 
    const QVariant& value)
{
    Q_ASSERT_X(column == 3, "SetData<QUaLog>", "Only message column is editable.");
    node->message = value.toByteArray();
    return true;
}

#endif // QUALOGMODELITEMTRAITS_H