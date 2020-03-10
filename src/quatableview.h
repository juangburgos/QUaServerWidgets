#ifndef QUATABLEVIEW_H
#define QUATABLEVIEW_H

#include <QTableView>
#include <QUaView>

template <typename N>
class QUaTableView : public QTableView, public QUaView<QUaTableView<N>, N>
{
	// to be able to access base class QTableView protected members
	friend class QUaView<QUaTableView, N>;
public:
	explicit QUaTableView(QWidget* parent = nullptr);

	void setModel(QAbstractItemModel* model) override;

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

template<typename N>
inline QUaTableView<N>::QUaTableView(QWidget* parent) :
	QTableView(parent),
	QUaView<QUaTableView, N>()
{
}

template<typename N>
inline void QUaTableView<N>::setModel(QAbstractItemModel* model)
{
	QUaView<QUaTableView, N>
		::setModel<QTableView>(model);
}

template<typename N>
inline void QUaTableView<N>::dataChanged(
	const QModelIndex& topLeft,
	const QModelIndex& bottomRight,
	const QVector<int>& roles)
{
	QUaView<QUaTableView, N>
		::dataChanged<QTableView>(topLeft, bottomRight, roles);
}

template<typename N>
inline void QUaTableView<N>::keyPressEvent(QKeyEvent* event)
{
	QUaView<QUaTableView, N>
		::keyPressEvent<QTableView>(event);
}

#endif // QUATABLEVIEW_H
