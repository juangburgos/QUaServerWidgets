#ifndef QUATREEMODEL_H
#define QUATREEMODEL_H

#include <QUaNodeModel>

class QUaTreeModel : public QUaNodeModel
{
    Q_OBJECT

public:
    explicit QUaTreeModel(QObject *parent = nullptr);

    QUaNode* rootNode() const;
    void     setRootNode(QUaNode* rootNode = nullptr);

private:
    void bindRoot(QUaNodeWrapper* root);
    void bindRecursivelly(QUaNodeWrapper* node);
    void unbindNodeRecursivelly(QUaNodeWrapper* node);

    void bindChangeCallbackForColumn(const int& column, QUaNodeWrapper* node) override;
    void bindChangeCallbackForColumnRecursivelly(const int& column, QUaNodeWrapper* node);
};

#endif // QUATREEMODEL_H
