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
    static QUaServer * m_pserver;
    static QMetaEnum m_logLevelMetaEnum;
    static QMetaEnum m_logCategoryMetaEnum;
    static QHash<
        QUaLog*,
        QList<std::function<void(void)>>
    > m_hashDestroyLog;
    QUaNodeTreeModel      m_modelNodes;
    QSortFilterProxyModel m_proxyNodes;

    QUaLogTreeModel       m_modelLogs;
    QSortFilterProxyModel m_proxyLogs;

    static QString logToString(const QQueue<QUaLog>& logOut);

    friend QMetaObject::Connection
        QUaModelItemTraits::NewChildCallback<QUaLog>(
            QUaLog* log,
            const std::function<void(QUaLog&)>& callback);
    friend QMetaObject::Connection
        QUaModelItemTraits::DestroyCallback<QUaLog>(
            QUaLog* log,
            const std::function<void(void)>& callback);
};
#endif // DIALOG_H
