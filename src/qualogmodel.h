#ifndef QUALOGMODEL_H
#define QUALOGMODEL_H

#include <QUaNode>
#include <QUaNodeModel>

class QUaLogModel : public QUaNodeModel<QUaLog>
{
    Q_OBJECT

public:
    explicit QUaLogModel(QObject* parent = nullptr);
    ~QUaLogModel();
    
    void addNode(QUaLog* node);
    
    void addNodes(const QList<QUaLog*>& nodes);
    
    bool removeNode(QUaLog* node);
    
    void clear();
    
protected:
    void removeWrapper(QUaNodeModel<QUaLog>::QUaNodeWrapper* wrapper);
};

#endif // QUALOGMODEL_H
