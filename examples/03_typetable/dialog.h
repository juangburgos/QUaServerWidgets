#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMenu>

#include <QUaServer>
#include <QUaTypeModel>
#include <QSortFilterProxyModel>
#include <QUaTableView>

typedef QUaTableView<QUaNode*> QUaNodeTableView;

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
    void setupTable();

    void addMethods(QUaBaseObject * node, const bool &isObjsFolder = false);

    QUaServer m_server;
    QUaTypeModel m_model;
    QSortFilterProxyModel m_proxy;
};

#endif // DIALOG_H

