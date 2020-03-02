#include "quasessionmodel.h"

QUaSessionModel::QUaSessionModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int QUaSessionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

int QUaSessionModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return 0;
}

QVariant QUaSessionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    return QVariant();
}
