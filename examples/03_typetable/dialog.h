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

    template<typename T>
    void addCategory();

    QVariant dataCallback_0(QUaNode* node);
    QVariant dataCallback_1(QUaNode* node);
    QVariant dataCallback_2(QUaNode* node);
    
    QMetaObject::Connection changeCallback_2(QUaNode* node, std::function<void(void)>& changeCallback);
    
    bool editableCallback_2(QUaNode* node);


    QUaServer             m_server;
    QUaNodeTypeModel      m_modelTypes;
    QSortFilterProxyModel m_proxyTypes;
    QUaNodeCategoryModel  m_modelCategories;
    QSortFilterProxyModel m_proxyCategories;
};

template<typename T>
inline void Dialog::addCategory()
{
    auto className = T::staticMetaObject.className();
    // T category
    m_modelCategories.addCategory(className);
    // add existing Ts
    auto instances = m_server.typeInstances<T>();
    for (auto instance : instances)
    {
        m_modelCategories.addNodeToCategory(
            className,
            instance
        );
    }
    // add new Ts
    m_server.instanceCreated<T>(
    [this, className](T* instance) {
        m_modelCategories.addNodeToCategory(
            className,
            instance
        );
    });
}

#endif // DIALOG_H