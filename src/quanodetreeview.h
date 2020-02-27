#ifndef QUANODETREEVIEW_H
#define QUANODETREEVIEW_H

#include <QTreeView>
#include <QStyledItemDelegate>

class QUaNode;
class QUaNodeModel;
class QUaNodeDelegate;

class QUaNodeTreeView : public QTreeView
{
    friend class QUaNodeDelegate;
    Q_OBJECT
public:
    explicit QUaNodeTreeView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel* model) override;

    void setColumnEditor(
        const int& column,
        std::function<QWidget*(QWidget*, QUaNode*)> initEditorCallback,
        std::function<void    (QWidget*, QUaNode*)> updateEditorCallback,
        std::function<void    (QWidget*, QUaNode*)> updateDataCallback
    );
    void removeColumnEditor(const int& column);

    // Qt API:

    // overwrite to ignore some calls to improve performance
    void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = QVector<int>()) override;

signals:

public slots:

private:
    // copy to avoid dynamic-casting all the time
    QUaNodeModel* m_model;
    // internal editor callbacks
    struct ColumnEditor
    {
        std::function<QWidget*(QWidget*, QUaNode*)> m_initEditorCallback;
        std::function<void    (QWidget*, QUaNode*)> m_updateEditorCallback;
        std::function<void    (QWidget*, QUaNode*)> m_updateDataCallback;
    };
    QMap<int, ColumnEditor> m_mapEditorFuncs;

};

// internal delegate
class QUaNodeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QUaNodeDelegate(QObject* parent = 0);

    // editor factory and intial setup
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    // populate editor and update editor if value changes while editing
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    // update undelying data source (QUaNode) when editor finishes editing
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

    // fix editor size and location inside view
    //void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QUaNodeTreeView* m_view;
};

#endif // QUANODETREEVIEW_H
