#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaNodeModelItemTraits>
#include <QUaModel>

class QUaTreeModel : public QUaModel<QUaNode*>
{
    Q_OBJECT

public:
    explicit QUaTreeModel(QObject *parent = nullptr);
    ~QUaTreeModel();

    QUaNode* rootNode() const;
    void     setRootNode(QUaNode* rootNode = nullptr);

private:
    void bindRoot(QUaModel<QUaNode*>::QUaNodeWrapper* root);
    void bindRecursivelly(QUaModel<QUaNode*>::QUaNodeWrapper* wrapper);
    void unbindNodeRecursivelly(QUaModel<QUaNode*>::QUaNodeWrapper* wrapper);
};

#endif // QUATREEMODEL_H
