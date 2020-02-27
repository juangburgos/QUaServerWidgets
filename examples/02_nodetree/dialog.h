#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMenu>

#include <QUaServer>
#include <QUaTreeModel>
#include <QSortFilterProxyModel>

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

    void setupQUaBaseObjectMenu      (QMenu& menu, QUaBaseObject       *obj    );
    void setupQUaBaseDataVariableMenu(QMenu& menu, QUaBaseDataVariable *datavar);
    void setupQUaPropertyMenu        (QMenu& menu, QUaProperty         *prop   );

    QUaServer m_server;
    QUaTreeModel m_model;
    QSortFilterProxyModel m_proxy;
};
#endif // DIALOG_H
