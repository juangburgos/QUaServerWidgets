#ifndef QUALOGMODEL_H
#define QUALOGMODEL_H

#include <QAbstractTableModel>

class QUaLogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit QUaLogModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
};

#endif // QUALOGMODEL_H
