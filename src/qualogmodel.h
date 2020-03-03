#ifndef QUALOGMODEL_H
#define QUALOGMODEL_H

#include <QUaNode>
#include <QUaTableModel>

class QUaLogModel : public QUaTableModel<QUaLog>
{
public:
	inline explicit QUaLogModel(QObject* parent = nullptr) : 
		QUaTableModel<QUaLog>(parent)
	{ }
};

#endif // QUALOGMODEL_H
