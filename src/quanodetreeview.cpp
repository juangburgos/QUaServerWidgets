#include "quanodetreeview.h"
#include <QSortFilterProxyModel>
#include <QUaNodeModel>

QUaNodeDelegate::QUaNodeDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	QUaNodeTreeView* view = dynamic_cast<QUaNodeTreeView*>(parent);
	Q_ASSERT_X(view, "QUaNodeDelegate", "Parent must be a QUaNodeTreeView instance");
	m_view = view;
}

QWidget* QUaNodeDelegate::createEditor(
	QWidget* parent, 
	const QStyleOptionViewItem& option, 
	const QModelIndex& index
) const
{
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_initEditorCallback)
	{
		return QStyledItemDelegate::createEditor(parent, option, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_initEditorCallback(parent, node);
}

void QUaNodeDelegate::setEditorData(
	QWidget* editor, 
	const QModelIndex& index
) const
{
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_updateEditorCallback)
	{
		return QStyledItemDelegate::setEditorData(editor, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateEditorCallback(editor, node);
}

void QUaNodeDelegate::setModelData(
	QWidget* editor, 
	QAbstractItemModel* model, 
	const QModelIndex& index
) const
{
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_updateDataCallback)
	{
		return QStyledItemDelegate::setModelData(editor, model, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateDataCallback(editor, node);
}

QUaNodeTreeView::QUaNodeTreeView(QWidget *parent) : QTreeView(parent)
{
	m_model = nullptr;
	this->setItemDelegate(new QUaNodeDelegate(this));
	this->setAlternatingRowColors(true);
	// set uniform rows for performance by default
	this->setUniformRowHeights(true);
}

void QUaNodeTreeView::setModel(QAbstractItemModel* model)
{
	auto nodeModel = dynamic_cast<QUaNodeModel*>(model);
	if (!nodeModel)
	{
		auto proxyModel = dynamic_cast<QSortFilterProxyModel*>(model);
		if (proxyModel)
		{
			nodeModel = dynamic_cast<QUaNodeModel*>(proxyModel->sourceModel());
		}
	}
	Q_ASSERT_X(nodeModel, "setModel", "QUaNodeTreeView only supports QUaNodeModel models");
	if (!nodeModel)
	{
		return;
	}
	m_model = nodeModel;
	QTreeView::setModel(model);
}

void QUaNodeTreeView::setColumnEditor(
	const int& column, 
	std::function<QWidget*(QWidget*, QUaNode*)> getEditorCallback, 
	std::function<void    (QWidget*, QUaNode*)> populateEditorCallback, 
	std::function<void    (QWidget*, QUaNode*)> updateDataCallback
)
{
	Q_ASSERT(column >= 0);
	if (column < 0)
	{
		return;
	}
	m_mapEditorFuncs.insert(column,
		{
			getEditorCallback,
			populateEditorCallback,
			updateDataCallback
		}
	);
}

void QUaNodeTreeView::removeColumnEditor(const int& column)
{
	m_mapEditorFuncs.remove(column);
}

void QUaNodeTreeView::dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
	// NOTE : QUaNodeModel always emits dataChanged with (topLeft == bottomRight)
	Q_ASSERT(topLeft == bottomRight);
	// getting visual rect is expensive but less than QTreeView::dataChanged
	QRect rect = this->visualRect(topLeft);
	// ignore data update if item out of view
	if (rect.y() < 0 || rect.y() > this->height())
	{
		return;
	}
	// data update is expensive
	QTreeView::dataChanged(topLeft, bottomRight, roles);
}

