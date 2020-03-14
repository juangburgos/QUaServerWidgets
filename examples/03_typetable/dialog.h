#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMenu>

#include <QUaServer>
#include <QUaNodeTypeModel>
#include <QUaCategoryModel>
#include <QSortFilterProxyModel>
#include <QUaTableView>
#include <QUaTreeView>

typedef QUaTableView    <QUaNode*> QUaNodeTableView;
typedef QUaTreeView     <QUaNode*> QUaCategoryTreeView;
typedef QUaCategoryModel<QUaNode*> QUaNodeCategoryModel;

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
    void setupTableTypes();
    void setupTreeCategories();

    void addMethods(QUaBaseObject * node, const bool &isObjsFolder = false);

    QUaServer             m_server;
    QUaNodeTypeModel      m_modelTypes;
    QSortFilterProxyModel m_proxyTypes;
    QUaNodeCategoryModel  m_modelCategories;
};

#endif // DIALOG_H

