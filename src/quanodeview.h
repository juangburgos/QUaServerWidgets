#ifndef QUANODEVIEW_H
#define QUANODEVIEW_H

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QUaNodeModel>

class QUaNode;

// https://en.wikipedia.org/wiki/Template_metaprogramming#Static_polymorphism
template <class T>
class QUaNodeView
{
public:
	explicit QUaNodeView();

	template <class B>
	void setModel(QAbstractItemModel* model);

	void setColumnEditor(
		const int& column,
		std::function<QWidget * (QWidget*, QUaNode*)> initEditorCallback,
		std::function<void(QWidget*, QUaNode*)> updateEditorCallback,
		std::function<void(QWidget*, QUaNode*)> updateDataCallback
	);
	void removeColumnEditor(const int& column);

	// overwrite to ignore some calls to improve performance
	template <class B>
	void dataChanged(
		const QModelIndex& topLeft,
		const QModelIndex& bottomRight,
		const QVector<int>& roles = QVector<int>()
	);

protected:
	T* m_thiz;
	// copy to avoid dynamic-casting all the time
	QUaNodeModel<QUaNode>* m_model;
	QSortFilterProxyModel* m_proxy;
	// internal editor callbacks
	struct ColumnEditor
	{
		std::function<QWidget * (QWidget*, QUaNode*)> m_initEditorCallback;
		std::function<void(QWidget*, QUaNode*)>       m_updateEditorCallback;
		std::function<void(QWidget*, QUaNode*)>       m_updateDataCallback;
	};
	QMap<int, ColumnEditor> m_mapEditorFuncs;

	// internal delegate
	class QUaNodeDelegate : public QStyledItemDelegate
	{
		//Q_OBJECT : NOTE : Qt do not support this on nested classes
	public:
		explicit QUaNodeDelegate(QObject* parent = 0);

		// editor factory and intial setup
		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		// populate editor and update editor if value changes while editing
		void setEditorData(QWidget* editor, const QModelIndex& index) const override;

		// update undelying data source (QUaNode) when editor finishes editing
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		//// fix editor size and location inside view
		//void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		QUaNodeView* m_view;
	};
	friend class QUaNodeView::QUaNodeDelegate;
};


template<class T>
inline QUaNodeView<T>::QUaNodeView()
{
	m_model = nullptr;
	m_proxy = nullptr;
	m_thiz = static_cast<T*>(this);
	m_thiz->setItemDelegate(new QUaNodeView<T>::QUaNodeDelegate(m_thiz));
	m_thiz->setAlternatingRowColors(true);
}

template<class T>
template<class B>
inline void QUaNodeView<T>::setModel(QAbstractItemModel* model)
{
	auto nodeModel = dynamic_cast<QUaNodeModel<QUaNode>*>(model);
	if (!nodeModel)
	{
		m_proxy = dynamic_cast<QSortFilterProxyModel*>(model);
		if (m_proxy)
		{
			nodeModel = dynamic_cast<QUaNodeModel<QUaNode>*>(m_proxy->sourceModel());
		}
	}
	else
	{
		m_proxy = nullptr;
	}
	Q_ASSERT_X(nodeModel, "setModel", "QUaView only supports QUaNodeModel models");
	if (!nodeModel)
	{
		return;
	}
	m_model = nodeModel;
	m_thiz->B::setModel(model);
}

template<class T>
template<class B>
inline void QUaNodeView<T>::dataChanged(
	const QModelIndex& topLeft,
	const QModelIndex& bottomRight,
	const QVector<int>& roles)
{
	// NOTE : QUaNodeModel always emits dataChanged with (topLeft == bottomRight)
	Q_ASSERT(topLeft == bottomRight);
	// getting visual rect is expensive but less than QTreeView::dataChanged
	QRect rect = m_thiz->visualRect(topLeft);
	// ignore data update if item out of view
	if (rect.y() < 0 || rect.y() > m_thiz->height())
	{
		return;
	}
	// data update is expensive
	m_thiz->B::dataChanged(topLeft, bottomRight, roles);
}

template<class T>
inline void QUaNodeView<T>::setColumnEditor(
	const int& column,
	std::function<QWidget * (QWidget*, QUaNode*)> initEditorCallback,
	std::function<void(QWidget*, QUaNode*)> updateEditorCallback,
	std::function<void(QWidget*, QUaNode*)> updateDataCallback)
{
	Q_ASSERT(column >= 0);
	if (column < 0)
	{
		return;
	}
	m_mapEditorFuncs.insert(column,
		{
			initEditorCallback,
			updateEditorCallback,
			updateDataCallback
		}
	);
}

template<class T>
inline void QUaNodeView<T>::removeColumnEditor(const int& column)
{
	m_mapEditorFuncs.remove(column);
}

template<class T>
inline QUaNodeView<T>::QUaNodeDelegate::QUaNodeDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	auto view = static_cast<T*>(parent);
	Q_ASSERT_X(view, "QUaNodeDelegate", "Parent must be a T instance");
	m_view = view;
}

template<class T>
inline QWidget* QUaNodeView<T>::QUaNodeDelegate::createEditor(
	QWidget* parent,
	const QStyleOptionViewItem& option,
	const QModelIndex& const_index
) const
{
	QModelIndex index = m_view->m_proxy ? m_view->m_proxy->mapToSource(const_index) : const_index;
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_initEditorCallback)
	{
		return QStyledItemDelegate::createEditor(parent, option, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_initEditorCallback(parent, node);
}

template<class T>
inline void QUaNodeView<T>::QUaNodeDelegate::setEditorData(
	QWidget* editor,
	const QModelIndex& const_index
) const
{
	QModelIndex index = m_view->m_proxy ? m_view->m_proxy->mapToSource(const_index) : const_index;
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_updateEditorCallback)
	{
		return QStyledItemDelegate::setEditorData(editor, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateEditorCallback(editor, node);
}

template<class T>
inline void QUaNodeView<T>::QUaNodeDelegate::setModelData(
	QWidget* editor,
	QAbstractItemModel* model,
	const QModelIndex& const_index
) const
{
	QModelIndex index = m_view->m_proxy ? m_view->m_proxy->mapToSource(const_index) : const_index;
	if (!m_view->m_mapEditorFuncs.contains(index.column()) ||
		!m_view->m_mapEditorFuncs[index.column()].m_updateDataCallback)
	{
		return QStyledItemDelegate::setModelData(editor, model, index);
	}
	QUaNode* node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateDataCallback(editor, node);
}

#endif // QUANODEVIEW_H
