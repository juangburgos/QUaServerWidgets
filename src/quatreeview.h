#ifndef QUATREEVIEW_H
#define QUATREEVIEW_H

#include <QTreeView>
#include <QUaView>

template <typename N, int I = 0>
class QUaTreeView : public QTreeView, public QUaView<QUaTreeView<N, I>, N, I>
{
    // to be able to access base class QTreeView protected members
    friend class QUaView<QUaTreeView, N, I>;
public:

    explicit QUaTreeView(QWidget *parent = nullptr);

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
inline QUaTreeView<N, I>::QUaTreeView(QWidget* parent) :
	QTreeView(parent),
	QUaView<QUaTreeView, N, I>()
{
	// NOTE : QTreeView specific
	// set uniform rows for performance by default
	this->setUniformRowHeights(true);
}

template<typename N, int I>
inline void QUaTreeView<N, I>::setModel(QAbstractItemModel* model)
{
    QUaView<QUaTreeView, N, I>
        ::template setModel<QTreeView>(model);
}

template<typename N, int I>
inline QModelIndexList QUaTreeView<N, I>::selectedIndexesOrigin() const
{
	return QUaView<QUaTreeView, N, I>
		::template selectedIndexesOrigin<QTreeView>();
}

template<typename N, int I>
inline void QUaTreeView<N, I>::dataChanged(
    const QModelIndex& topLeft, 
    const QModelIndex& bottomRight, 
    const QVector<int>& roles)
{
	QUaView<QUaTreeView, N, I>
        ::template dataChanged<QTreeView>(topLeft, bottomRight, roles);
}

template<typename N, int I>
inline void QUaTreeView<N, I>::keyPressEvent(QKeyEvent* event)
{
	QUaView<QUaTreeView, N, I>
        ::template keyPressEvent<QTreeView>(event);
}

#endif // QUATREEVIEW_H

