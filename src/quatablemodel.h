#ifndef QUATABLEMODEL_H
#define QUATABLEMODEL_H

#include <QUaNodeModel>

class QUaTableModel : public QUaNodeModel
{
    Q_OBJECT

public:
    explicit QUaTableModel(QObject *parent = nullptr);
    ~QUaTableModel();

    void addNode(QUaNode* node);

    void addNodes(const QList<QUaNode*> &nodes);

    bool removeNode(QUaNode* node);

    void clear();

protected:
    void removeWrapper(QUaNodeModel::QUaNodeWrapper * wrapper);
};

#endif // QUATABLEMODEL_H
