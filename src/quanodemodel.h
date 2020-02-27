#ifndef QUANODEMODEL_H
#define QUANODEMODEL_H

#include <QAbstractItemModel>

class QUaNode;

class QUaNodeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QUaNodeModel(QObject *parent = nullptr);

    QUaNode* nodeFromIndex(const QModelIndex& index) const;

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

protected:
    class QUaNodeWrapper
    {
    public:
        explicit QUaNodeWrapper(QUaNode* node, QUaNodeModel::QUaNodeWrapper* parent = nullptr);
        ~QUaNodeWrapper();

        QUaNode* node() const;

        QModelIndex index() const;
        void setIndex(const QModelIndex &index);

        QUaNodeModel::QUaNodeWrapper* parent() const;

        // NOTE : return by reference
        QList<QUaNodeModel::QUaNodeWrapper*> & children();
        QList<QMetaObject::Connection> & connections();

        std::function<void()> getChangeCallbackForColumn(const int& column, QAbstractItemModel* model);

    private:
        // internal data
        QUaNode* m_node;
        // NOTE : need index to support model manipulation 
        //        cannot use createIndex outside Qt's API
        //        (doesnt work even inside a QAbstractItemModel member)
        //        and for beginRemoveRows and beginInsertRows
        //        we can only use the indexes provided by the model
        //        else random crashes occur when manipulating model
        //        btw do not use QPersistentModelIndex, they get corrupted
        QModelIndex m_index;
        // members for tree structure
        QUaNodeWrapper* m_parent;
        QList<QUaNodeWrapper*> m_children;
        QList<QMetaObject::Connection> m_connections;
    };

    QUaNodeWrapper* m_root;
    int m_columnCount;

    virtual void bindChangeCallbackForColumn(const int& column, QUaNodeWrapper* node) = 0;

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