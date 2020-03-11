#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QUaServer>
#include <QUaTableModel>
#include <QSortFilterProxyModel>
#include <QUaTableView>

template<>
inline bool 
QUaModelItemTraits::IsValid<QUaLog>(const QUaLog &node)
{
    // valid if not null
    return !node.message.isNull();
}

template<>
inline QUaLog 
QUaModelItemTraits::GetInvalid<QUaLog>()
{
    return QUaLog({
        QByteArray(), // null message
        QUaLogLevel::Info,
        QUaLogCategory::Server
    });
}

template<>
inline bool 
QUaModelItemTraits::IsEqual<QUaLog>(const QUaLog& node1, const QUaLog& node2)
{
    return 
        node1.message   == node2.message  &&
        node1.level     == node2.level    &&
        node1.category  == node2.category &&
        node1.timestamp == node2.timestamp;
}

inline bool operator==(const QUaLog& node1, const QUaLog& node2)
{
    return QUaModelItemTraits::IsEqual<QUaLog>(node1, node2);
}

// overload to support default editor (QStyledItemDelegate::setEditorData)
// implement either this or ui->tableViewLogs->setColumnEditor
// setColumnEditor has preference in case both implemented
template<>
inline bool
QUaModelItemTraits::SetData<QUaLog>(
    QUaLog & node, 
    const int& column, 
    const QVariant& value)
{
    Q_ASSERT_X(column == 3, "SetData<QUaLog>", "Only message column is editable.");
    node.message = value.toByteArray();
    return true;
}

typedef QUaTableModel<QUaLog> QUaLogModel;
typedef QUaTableModel<const QUaSession*> QUaSessionModel;

typedef QUaTableView<QUaLog> QUaLogTableView;
typedef QUaTableView<const QUaSession*> QUaSessionTableView;

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void setupServer();
    void setupLogTable();
    void setupSessionTable();

private slots:
    void on_pushButtonClearLog_clicked();

private:
    Ui::Dialog *ui;

    QUaServer             m_server;
    QUaLogModel           m_modelLog;
    QSortFilterProxyModel m_proxyLog;
    QUaSessionModel       m_modelSession;
    QSortFilterProxyModel m_proxySession;
};
#endif // DIALOG_H
