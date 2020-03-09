#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QUaServer>
#include <QUaTableModel>
#include <QSortFilterProxyModel>

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

typedef QUaTableModel<QUaLog> QUaLogModel;
typedef QUaTableModel<const QUaSession*> QUaSessionModel;

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
