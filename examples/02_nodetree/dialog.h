#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMenu>

#include <QUaServer>
#include <QUaTreeModel>
#include <QUaTreeView>
#include <QSortFilterProxyModel>

#include <QUaNodeModelItemTraits>

typedef QUaTreeModel<QUaNode*> QUaNodeTreeModel;
typedef QUaTreeView <QUaNode*> QUaNodeTreeView;

#include <QUaLogModelItemTraits>

typedef QUaTreeModel<QUaLog> QUaLogTreeModel;
typedef QUaTreeView <QUaLog> QUaLogTreeView;

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
    void setupTreeNodes();
    void setupTreeLogs();

    void setupQUaBaseObjectMenu      (QMenu& menu, QUaBaseObject       *obj    );
    void setupQUaBaseDataVariableMenu(QMenu& menu, QUaBaseDataVariable *datavar);
    void setupQUaPropertyMenu        (QMenu& menu, QUaProperty         *prop   );

    QUaServer m_server;
    QUaNodeTreeModel      m_modelNodes;
    QSortFilterProxyModel m_proxyNodes;

    QUaLogTreeModel       m_modelLogs;
    QSortFilterProxyModel m_proxyLogs;
};
#endif // DIALOG_H
