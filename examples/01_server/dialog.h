#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <QUaServer>
#include <QUaLogModel>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;

    QUaServer m_server;
    QUaLogModel m_model;
    QList<QUaLog> m_logs;
};
#endif // DIALOG_H
