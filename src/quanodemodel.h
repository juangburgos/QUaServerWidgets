#ifndef QUANODEMODEL_H
#define QUANODEMODEL_H

#include <QAbstractItemModel>

class QUaNode;

class QUaNodeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QUaNodeModel(QObject *parent = nullptr);

    void bindRootNode(QUaNode * rootNode = nullptr);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    class QUaNodeWrapper;
    QUaNodeWrapper* m_root;

    // disconnect signals to and from the given node
    void bindRoot(QUaNodeWrapper* root);
    void bindRecursivelly(QUaNodeWrapper* node);
    void unbindNodeRecursivelly(QUaNodeWrapper* node);
};

#endif // QUANODEMODEL_H
