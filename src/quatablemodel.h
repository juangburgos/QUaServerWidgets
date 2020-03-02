#ifndef QUATABLEMODEL_H
#define QUATABLEMODEL_H

#include <QUaNode>
#include <QUaNodeModel>

class QUaTableModel : public QUaNodeModel<QUaNode>
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
    void removeWrapper(QUaNodeModel<QUaNode>::QUaNodeWrapper * wrapper);
};

#endif // QUATABLEMODEL_H
