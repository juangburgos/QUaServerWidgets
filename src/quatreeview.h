#ifndef QUATREEVIEW_H
#define QUATREEVIEW_H

#include <QTreeView>
#include <QUaView>

template <typename N>
class QUaTreeView : public QTreeView, public QUaView<QUaTreeView<N>, N>
{
    // to be able to access base class QTreeView protected members
    friend class QUaView<QUaTreeView, N>;
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

template<typename N>
inline QUaTreeView<N>::QUaTreeView(QWidget* parent) :
	QTreeView(parent),
	QUaView<QUaTreeView, N>()
{
	// NOTE : QTreeView specific
	// set uniform rows for performance by default
	this->setUniformRowHeights(true);
}

template<typename N>
inline void QUaTreeView<N>::setModel(QAbstractItemModel* model)
{
    QUaView<QUaTreeView, N>
        ::template setModel<QTreeView>(model);
}

template<typename N>
inline QModelIndexList QUaTreeView<N>::selectedIndexesOrigin() const
{
	return QUaView<QUaTreeView, N>
		::template selectedIndexesOrigin<QTreeView>();
}

template<typename N>
inline void QUaTreeView<N>::dataChanged(
    const QModelIndex& topLeft, 
    const QModelIndex& bottomRight, 
    const QVector<int>& roles)
{
	QUaView<QUaTreeView, N>
        ::template dataChanged<QTreeView>(topLeft, bottomRight, roles);
}

template<typename N>
inline void QUaTreeView<N>::keyPressEvent(QKeyEvent* event)
{
	QUaView<QUaTreeView, N>
        ::template keyPressEvent<QTreeView>(event);
}

#endif // QUATREEVIEW_H

