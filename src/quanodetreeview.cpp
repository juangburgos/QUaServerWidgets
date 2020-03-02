#include "quanodetreeview.h"

QUaNodeTreeView::QUaNodeTreeView(QWidget* parent) : 
	QTreeView(parent),
	QUaNodeView<QUaNodeTreeView>()
{
	// NOTE : QTreeView specific
	// set uniform rows for performance by default
	this->setUniformRowHeights(true);
}

void QUaNodeTreeView::setModel(QAbstractItemModel* model)
{
	QUaNodeView<QUaNodeTreeView>
		::setModel<QTreeView>(model);
}

void QUaNodeTreeView::dataChanged(
	const QModelIndex& topLeft, 
	const QModelIndex& bottomRight, 
	const QVector<int>& roles)
{
	QUaNodeView<QUaNodeTreeView>
		::dataChanged<QTreeView>(topLeft, bottomRight, roles);
}

