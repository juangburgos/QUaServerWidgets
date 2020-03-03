#ifndef QUANODETREEVIEW_H
#define QUANODETREEVIEW_H

#include <QTreeView>
#include <QUaNodeView>

class QUaNodeTreeView : public QTreeView, public QUaNodeView<QUaNodeTreeView>
{
    Q_OBJECT
    // to be able to access base class QTreeView protected members
    friend class QUaNodeView<QUaNodeTreeView>;
public:
    explicit QUaNodeTreeView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel* model) override;

    // Qt API:

    // overwrite to ignore some calls to improve performance
    void dataChanged(
		const QModelIndex& topLeft, 
		const QModelIndex& bottomRight, 
		const QVector<int>& roles = QVector<int>()
	) override;

};

#endif // QUANODETREEVIEW_H

