#ifndef QUATABLEVIEW_H
#define QUATABLEVIEW_H

#include <QTableView>
#include <QUaView>

template <typename N, int I = 0>
class QUaTableView : public QTableView, public QUaView<QUaTableView<N, I>, N, I>
{
	// to be able to access base class QTableView protected members
	friend class QUaView<QUaTableView, N, I>;
public:
	explicit QUaTableView(QWidget* parent = nullptr);

	void setModel(QAbstractItemModel* model) override;

	// selected on source model
	QModelIndexList selectedIndexesOrigin() const;

	// Qt API:

	// overwrite to ignore some calls to improve performance
	void dataChanged(
		const QModelIndex& topLeft,
		const QModelIndex& bottomRight,
		const QVector<int>& roles = QVector<int>()
	) override;

	// overwrite to handle keyboard events
	void keyPressEvent(QKeyEvent* event) override;
};

template<typename N, int I>
inline QUaTableView<N, I>::QUaTableView(QWidget* parent) :
	QTableView(parent),
	QUaView<QUaTableView, N, I>()
{
}

template<typename N, int I>
inline void QUaTableView<N, I>::setModel(QAbstractItemModel* model)
{
	QUaView<QUaTableView, N, I>
        ::template setModel<QTableView>(model);
}

template<typename N, int I>
inline QModelIndexList QUaTableView<N, I>::selectedIndexesOrigin() const
{
	return QUaView<QUaTableView, N>
		::template selectedIndexesOrigin<QTableView>();
}

template<typename N, int I>
inline void QUaTableView<N, I>::dataChanged(
	const QModelIndex& topLeft,
	const QModelIndex& bottomRight,
	const QVector<int>& roles)
{
	QUaView<QUaTableView, N, I>
        ::template dataChanged<QTableView>(topLeft, bottomRight, roles);
}

template<typename N, int I>
inline void QUaTableView<N, I>::keyPressEvent(QKeyEvent* event)
{
	QUaView<QUaTableView, N, I>
        ::template keyPressEvent<QTableView>(event);
}

#endif // QUATABLEVIEW_H
