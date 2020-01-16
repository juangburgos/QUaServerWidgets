#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMenu>

#include <QUaServer>
#include <QUaNodeModel>

class QUaBaseObjectExt;

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

    void setupServer();
    void setupTree();

    void setupQUaBaseObjectExtMenu(QMenu& menu, QUaBaseObjectExt * extobj);

    QUaServer m_server;
    QUaNodeModel m_model;
};
#endif // DIALOG_H
