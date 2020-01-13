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

    //bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Fetch data dynamically:
    //bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    //bool canFetchMore(const QModelIndex &parent) const override;
    //void fetchMore(const QModelIndex &parent) override;

    // Editable:
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    //bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    QUaNode* m_rootNode;

    // disconnect signals to and from the given node
    void disconnectNodeRecursivelly(QUaNode* rootNode);
};

#endif // QUANODEMODEL_H
