#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaNodeModel>

class QUaTreeModel : public QUaNodeModel
{
    Q_OBJECT

public:
    explicit QUaTreeModel(QObject *parent = nullptr);
    ~QUaTreeModel();

    QUaNode* rootNode() const;
    void     setRootNode(QUaNode* rootNode = nullptr);

private:
    void bindRoot(QUaNodeWrapper* root);
    void bindRecursivelly(QUaNodeWrapper* wrapper);
    void unbindNodeRecursivelly(QUaNodeWrapper* wrapper);    
};

#endif // QUATREEMODEL_H
