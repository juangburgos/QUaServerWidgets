#ifndef QUANODEMODEL_H
#define QUANODEMODEL_H

#include <QAbstractItemModel>

class QUaNode;

class QUaNodeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QUaNodeModel(QObject *parent = nullptr);

    QUaNode* rootNode() const;
    void     setRootNode(QUaNode * rootNode = nullptr);

    QUaNode* nodeFromIndex(const QModelIndex& index);

    void setColumnDataSource(
        const int& column, 
        const QString &strHeader, 
        std::function<QVariant(QUaNode*)> dataCallback,
        std::function<QMetaObject::Connection(QUaNode*, std::function<void()>)> changeCallback = nullptr,
        std::function<bool(QUaNode*)> editableCallback = nullptr
    );
    void removeColumnDataSource(const int& column);

    // Qt required API:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    class QUaNodeWrapper;
    QUaNodeWrapper* m_root;
    int m_columnCount;

    void bindRoot(QUaNodeWrapper* root);
    void bindRecursivelly(QUaNodeWrapper* node);
    void unbindNodeRecursivelly(QUaNodeWrapper* node);
    void bindChangeCallbackForColumnRecursivelly(const int& column, QUaNodeWrapper* node);

    struct ColumnDataSource
    {
        QString m_strHeader;
        std::function<QVariant(QUaNode*)> m_dataCallback;
        std::function<QMetaObject::Connection(QUaNode*, std::function<void()>)> m_changeCallback;
        std::function<bool(QUaNode*)> m_editableCallback;
    };
    QMap<int, ColumnDataSource> m_mapDataSourceFuncs;
};

#endif // QUANODEMODEL_H