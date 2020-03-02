#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaNode>
#include <QUaNodeModel>

class QUaTreeModel : public QUaNodeModel<QUaNode>
{
    Q_OBJECT

public:
    explicit QUaTreeModel(QObject *parent = nullptr);
    ~QUaTreeModel();

    QUaNode* rootNode() const;
    void     setRootNode(QUaNode* rootNode = nullptr);

private:
    void bindRoot(QUaNodeModel<QUaNode>::QUaNodeWrapper* root);
    void bindRecursivelly(QUaNodeModel<QUaNode>::QUaNodeWrapper* wrapper);
    void unbindNodeRecursivelly(QUaNodeModel<QUaNode>::QUaNodeWrapper* wrapper);
};

#endif // QUATREEMODEL_H
