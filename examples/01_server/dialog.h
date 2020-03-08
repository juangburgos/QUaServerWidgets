#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <QUaServer>
#include <QUaTableModel>
#include <QSortFilterProxyModel>

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
