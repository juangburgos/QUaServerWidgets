#include "qualogmodel.h"

QUaLogModel::QUaLogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int QUaLogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

int QUaLogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

QVariant QUaLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
