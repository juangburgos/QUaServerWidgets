#ifndef QUAVIEW_H
#define QUAVIEW_H

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QUaModel>

// https://en.wikipedia.org/wiki/Template_metaprogramming#Static_polymorphism
template <typename T, typename N>
class QUaView
{
public:
	explicit QUaView();

	template <class B>
	void setModel(QAbstractItemModel* model);

	void setColumnEditor(
		const int& column,
		std::function<QWidget*(QWidget*, N)> initEditorCallback,
		std::function<void(QWidget*, N)> updateEditorCallback,
		std::function<void(QWidget*, N)> updateDataCallback
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
	QUaModel<N>* m_model;
	QSortFilterProxyModel* m_proxy;
	// internal editor callbacks
	struct ColumnEditor
	{
		std::function<QWidget*(QWidget*, N)> m_initEditorCallback;
		std::function<void(QWidget*, N)>     m_updateEditorCallback;
		std::function<void(QWidget*, N)>     m_updateDataCallback;
	};
	QMap<int, ColumnEditor> m_mapEditorFuncs;

	// internal delegate
	class QUaItemDelegate : public QStyledItemDelegate
	{
		//Q_OBJECT : NOTE : Qt do not support this on nested classes
	public:
		explicit QUaItemDelegate(QObject* parent = 0);

		// editor factory and intial setup
		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		// populate editor and update editor if value changes while editing
		void setEditorData(QWidget* editor, const QModelIndex& index) const override;

		// update undelying data source (<N>) when editor finishes editing
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		//// fix editor size and location inside view
		//void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		QUaView* m_view;
	};
	friend class QUaView::QUaItemDelegate;
};


template<typename T, typename N>
inline QUaView<T, N>::QUaView()
{
	m_model = nullptr;
	m_proxy = nullptr;
	m_thiz  = static_cast<T*>(this);
	m_thiz->setItemDelegate(new QUaView<T, N>::QUaItemDelegate(m_thiz));
	m_thiz->setAlternatingRowColors(true);
}

template<typename T, typename N>
template<class B>
inline void QUaView<T, N>::setModel(QAbstractItemModel* model)
{
	auto nodeModel = dynamic_cast<QUaModel<N>*>(model);
	if (!nodeModel)
	{
		m_proxy = dynamic_cast<QSortFilterProxyModel*>(model);
		if (m_proxy)
		{
			nodeModel = dynamic_cast<QUaModel<N>*>(m_proxy->sourceModel());
		}
	}
	else
	{
		m_proxy = nullptr;
	}
	Q_ASSERT_X(nodeModel, "setModel", "QUaView only supports QUaModel models");
	if (!nodeModel)
	{
		return;
	}
	m_model = nodeModel;
	m_thiz->B::setModel(model);
}

template<typename T, typename N>
template<class B>
inline void QUaView<T, N>::dataChanged(
	const QModelIndex& topLeft,
	const QModelIndex& bottomRight,
	const QVector<int>& roles)
{
	// NOTE : QUaModel always emits dataChanged with (topLeft == bottomRight)
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

template<typename T, typename N>
inline void QUaView<T, N>::setColumnEditor(
	const int& column,
	std::function<QWidget * (QWidget*, N)> initEditorCallback,
	std::function<void(QWidget*, N)> updateEditorCallback,
	std::function<void(QWidget*, N)> updateDataCallback)
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

template<typename T, typename N>
inline void QUaView<T, N>::removeColumnEditor(const int& column)
{
	m_mapEditorFuncs.remove(column);
}

template<typename T, typename N>
inline QUaView<T, N>::QUaItemDelegate::QUaItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{
	auto view = static_cast<T*>(parent);
	Q_ASSERT_X(view, "QUaItemDelegate", "Parent must be a T instance");
	m_view = view;
}

template<typename T, typename N>
inline QWidget* QUaView<T, N>::QUaItemDelegate::createEditor(
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
	N node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_initEditorCallback(parent, node);
}

template<typename T, typename N>
inline void QUaView<T, N>::QUaItemDelegate::setEditorData(
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
	N node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateEditorCallback(editor, node);
}

template<typename T, typename N>
inline void QUaView<T, N>::QUaItemDelegate::setModelData(
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
	N node = m_view->m_model->nodeFromIndex(index);
	return m_view->m_mapEditorFuncs[index.column()].m_updateDataCallback(editor, node);
}

#endif // QUAVIEW_H
