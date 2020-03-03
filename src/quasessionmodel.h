#ifndef QUASESSIONMODEL_H
#define QUASESSIONMODEL_H

#include <QUaServer>
#include <QUaTableModel>

class QUaSessionModel : public QUaTableModel<const QUaSession*>
{
public:
    inline explicit QUaSessionModel(QObject* parent = nullptr) : 
        QUaTableModel<const QUaSession*>(parent)
    { }
};

#endif // QUASESSIONMODEL_H
