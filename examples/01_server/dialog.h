#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QUaServer>
#include <QUaTableModel>
#include <QSortFilterProxyModel>

// activate specialized implementation for QUaLog
template<>
struct QUaHasModelItemTraits<QUaLog> {
    static const bool value = true;
};

inline QMetaObject::Connection DestroyCallbackTrait(
    QUaLog &node, 
    std::function<void(void)> callback
)
{
    Q_UNUSED(node);
    Q_UNUSED(callback);
    return QMetaObject::Connection();
}

inline QList<QUaLog> GetChildrenTrait(const QUaLog &node)
{
    Q_UNUSED(node);
    return QList<QUaLog>();
}

inline bool IsValidTrait(const QUaLog &node)
{
    // valid if not null
    return !node.message.isNull();
}

inline QUaLog GetInvalidTrait()
{
    return QUaLog({
        QByteArray(), // null message
        QUaLogLevel::Info,
        QUaLogCategory::Server
    });
}

inline bool IsEqualTrait(const QUaLog& node1, const QUaLog& node2)
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
