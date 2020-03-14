#ifndef QUAVIEW_H
#define QUAVIEW_H

#include <QApplication>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QMimeData>
#include <QUaModel>

// SFINAE on members
// https://stackoverflow.com/questions/25492589/can-i-use-sfinae-to-selectively-define-a-member-variable-in-a-template-class
template <typename N, typename Enable = void>
class QUaViewBase {};

// pointer type specialization
template<typename N>
class QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
{
public:
	template<
		typename M1 = const std::function<QWidget*(QWidget*, N)>&,
		typename M2 = const std::function<void(QWidget*, N)>&,
		typename M3 = const std::function<void(QWidget*, N)&>
	>
	void setColumnEditor(
		const int& column,
		M1 &&initEditorCallback,
		M2 &&updateEditorCallback,
		M3 &&updateDataCallback
	);
	void removeColumnEditor(const int& column);

	// signature : void(QList<N>&)
	template <typename M = const std::function<void(QList<N>&)>&>
	void setDeleteCallback(M &&deleteCallback);
	// signature : QMimeData*(const QList<const N>&)
	template <typename M = const std::function<QMimeData*(QList<N>&)>&>
	void setCopyCallback(M &&copyCallback);
	// signature : void(QList<N>&, const QMimeData*)
	template <typename M = const std::function<void(QList<N>&, const QMimeData*)>&>
	void setPasteCallback(M &&pasteCallback);

protected:
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

	// internal keyboard events callbacks
	std::function<void(QList<N>&)>                   m_funcHandleDelete;
	std::function<QMimeData*(QList<N>&)>             m_funcHandleCopy;
	std::function<void(QList<N>&, const QMimeData*)> m_funcHandlePaste;

	QList<N> nodesFromIndexes(const QModelIndexList& indexes) const;
};

template<typename N>
template<typename M1, typename M2, typename M3>
inline void QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
::setColumnEditor(
	const int& column,
	M1&& initEditorCallback,
	M2&& updateEditorCallback,
	M3&& updateDataCallback)
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

template<typename N>
inline void QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
	::removeColumnEditor(const int& column)
{
	m_mapEditorFuncs.remove(column);
}

template<typename N>
inline QList<N> QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
	::nodesFromIndexes(const QModelIndexList& indexes) const
{
	QList<N> nodes;
	for (auto index : indexes)
	{
		index = m_proxy ? m_proxy->mapToSource(index) : index;
		auto node = m_model->nodeFromIndex(index);
		if (nodes.contains(node))
		{
			continue;
		}
		nodes << node;
	}
	return nodes;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
	::setDeleteCallback(M &&deleteCallback)
{
	m_funcHandleDelete = deleteCallback;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
	::setCopyCallback(M &&copyCallback)
{
	m_funcHandleCopy = copyCallback;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<std::is_pointer<N>::value>::type>
	::setPasteCallback(M &&pasteCallback)
{
	m_funcHandlePaste = pasteCallback;
}


// instance specialization
template<typename N>
class QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
{
public:
	template<
		typename M1 = const std::function<QWidget*(QWidget*, N*)>&,
		typename M2 = const std::function<void(QWidget*, N*)>&,
		typename M3 = const std::function<void(QWidget*, N*)&>
	>
	void setColumnEditor(
		const int& column,
		M1 &&initEditorCallback,
		M2 &&updateEditorCallback,
		M3 &&updateDataCallback
	);
	void removeColumnEditor(const int& column);

	// signature : 
	template <typename M = const std::function<void(QList<N*>&)>&>
	void setDeleteCallback(M &&deleteCallback);
	// signature : 
	template <typename M = const std::function<QMimeData*(QList<N*>&)>&>
	void setCopyCallback(M &&copyCallback);
	// signature : 
	template <typename M = const std::function<void(QList<N*>&, const QMimeData*)>&>
	void setPasteCallback(M &&pasteCallback);

protected:
	// copy to avoid dynamic-casting all the time
	QUaModel<N>* m_model;
	QSortFilterProxyModel* m_proxy;

	// internal editor callbacks
	struct ColumnEditor
	{
		std::function<QWidget*(QWidget*, N*)> m_initEditorCallback;
		std::function<void(QWidget*, N*)>     m_updateEditorCallback;
		std::function<void(QWidget*, N*)>     m_updateDataCallback;
	};
	QMap<int, ColumnEditor> m_mapEditorFuncs;

	// internal keyboard events callbacks
	std::function<void(QList<N*>&)>                   m_funcHandleDelete;
	std::function<QMimeData*(QList<N*>&)>             m_funcHandleCopy;
	std::function<void(QList<N*>&, const QMimeData*)> m_funcHandlePaste;

	QList<N*> nodesFromIndexes(const QModelIndexList& indexes) const;
};

template<typename N>
template<typename M1, typename M2, typename M3>
inline void QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
::setColumnEditor(
	const int& column,
	M1 &&initEditorCallback,
	M2 &&updateEditorCallback,
	M3 &&updateDataCallback
)
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

template<typename N>
inline void QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
	::removeColumnEditor(const int& column)
{
	m_mapEditorFuncs.remove(column);
}

template<typename N>
inline QList<N*> QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
	::nodesFromIndexes(const QModelIndexList& indexes) const
{
	QList<N*> nodes;
	for (auto index : indexes)
	{
		index = m_proxy ? m_proxy->mapToSource(index) : index;
		auto node = m_model->nodeFromIndex(index);
		if (nodes.contains(node))
		{
			continue;
		}
		nodes << node;
	}
	return nodes;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
	::setDeleteCallback(M &&deleteCallback)
{
	m_funcHandleDelete = deleteCallback;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
	::setCopyCallback(M &&copyCallback)
{
	m_funcHandleCopy = copyCallback;
}

template<typename N>
template<typename M>
inline void QUaViewBase<N, typename std::enable_if<!std::is_pointer<N>::value>::type>
	::setPasteCallback(M &&pasteCallback)
{
	m_funcHandlePaste = pasteCallback;
}

// https://en.wikipedia.org/wiki/Template_metaprogramming#Static_polymorphism
template <typename T, typename N>
class QUaView : public QUaViewBase<N>
{
public:
	explicit QUaView();

	template <class B>
	void setModel(QAbstractItemModel* model);

	
	void clearDeleteCallback();
	void clearCopyCallback();
	void clearPasteCallback();

	// Inheirted class - Qt API:

	// overwrite to ignore some calls to improve performance
	template <class B>
	void dataChanged(
		const QModelIndex& topLeft,
		const QModelIndex& bottomRight,
		const QVector<int>& roles = QVector<int>()
	);

	// overwrite to handle keyboard events
	template <class B>
	void keyPressEvent(QKeyEvent* event);

protected:
	T* m_thiz;

	// internal delegate
	class QUaItemDelegate : public QStyledItemDelegate
	{
		//Q_OBJECT : NOTE : Qt do not support this on nested classes
	public:
        explicit QUaItemDelegate(QObject* parent = nullptr);

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
	m_funcHandleCopy  = nullptr;
	m_funcHandlePaste = nullptr;
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
inline void QUaView<T, N>::clearDeleteCallback()
{
	m_funcHandleDelete = nullptr;
}

template<typename T, typename N>
inline void QUaView<T, N>::clearCopyCallback()
{
	m_funcHandleCopy = nullptr;
}

template<typename T, typename N>
inline void QUaView<T, N>::clearPasteCallback()
{
	m_funcHandlePaste = nullptr;
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
template<class B>
inline void QUaView<T, N>::keyPressEvent(QKeyEvent* event)
{
	auto indexes = m_thiz->selectedIndexes();
	if (indexes.isEmpty())
	{
		// call base class method
		m_thiz->B::keyPressEvent(event);
		return;
	}
	// check if delete
	if (event->key() == Qt::Key_Delete)
	{
		if (!m_funcHandleDelete)
		{
			// call base class method
			m_thiz->B::keyPressEvent(event);
			return;
		}
		// call delete callback and exit
		auto nodes = this->nodesFromIndexes(indexes);
		m_funcHandleDelete(nodes);
		return;
	}
	// check if copy
	if (event->matches(QKeySequence::Copy))
	{
		if (!m_funcHandleCopy)
		{
			// call base class method
			m_thiz->B::keyPressEvent(event);
			return;
		}
		// call copy callback and exit
		auto nodes = this->nodesFromIndexes(indexes);
		auto mime = m_funcHandleCopy(nodes);
		if (mime)
		{
			QApplication::clipboard()->setMimeData(mime);
		}
		return;
	}
	// check if paste
	if (event->matches(QKeySequence::Paste))
	{
		if (!m_funcHandlePaste)
		{
			// call base class method
			m_thiz->B::keyPressEvent(event);
			return;
		}
		// call paste callback and exit
		auto nodes = this->nodesFromIndexes(indexes);
		m_funcHandlePaste(
			nodes,
			QApplication::clipboard()->mimeData()
		);
		return;
	}
	// call base class method
	m_thiz->B::keyPressEvent(event);
	return;
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
    return m_view->m_mapEditorFuncs[index.column()]
        .m_initEditorCallback(parent, m_view->m_model->nodeFromIndex(index));
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
    m_view->m_mapEditorFuncs[index.column()]
		.m_updateEditorCallback(editor, m_view->m_model->nodeFromIndex(index));
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
		return QStyledItemDelegate::setModelData(editor, model, const_index);
	}
    m_view->m_mapEditorFuncs[index.column()]
		.m_updateDataCallback(editor, m_view->m_model->nodeFromIndex(index));
}

#endif // QUAVIEW_H

