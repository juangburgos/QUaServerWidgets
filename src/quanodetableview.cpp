#include "quanodetableview.h"

QUaNodeTableView::QUaNodeTableView(QWidget *parent) : 
	QTableView(parent),
	QUaNodeView<QUaNodeTableView>()
{

}

void QUaNodeTableView::setModel(QAbstractItemModel* model)
{
	QUaNodeView<QUaNodeTableView>
		::setModel<QTableView>(model);
}

void QUaNodeTableView::dataChanged(
	const QModelIndex& topLeft,
	const QModelIndex& bottomRight,
	const QVector<int>& roles)
{
	QUaNodeView<QUaNodeTableView>
		::dataChanged<QTableView>(topLeft, bottomRight, roles);
}