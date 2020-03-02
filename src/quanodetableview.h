#ifndef QUANODETABLEVIEW_H
#define QUANODETABLEVIEW_H

#include <QTableView>
#include <QUaNodeView>

class QUaNodeTableView : public QTableView, public QUaNodeView<QUaNodeTableView>
{
    Q_OBJECT
    friend class QUaNodeView<QUaNodeTableView>;
public:
    explicit QUaNodeTableView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel* model) override;

    // Qt API:

    // overwrite to ignore some calls to improve performance
    void dataChanged(
        const QModelIndex& topLeft,
        const QModelIndex& bottomRight,
        const QVector<int>& roles = QVector<int>()
    ) override;

};

#endif // QUANODETABLEVIEW_H
