#ifndef QUASESSIONMODEL_H
#define QUASESSIONMODEL_H

#include <QAbstractTableModel>

class QUaSessionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QUaSessionModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // QUASESSIONMODEL_H
