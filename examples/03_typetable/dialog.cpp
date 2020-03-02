#include "dialog.h"
#include "ui_dialog.h"

#include <QSpinBox>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QObject::connect(&m_server, &QUaServer::logMessage, this,
    [](const QUaLog &log) {
        qDebug() << "[" << log.level << "]["<< log.category << "] :" << log.message;
    });
    QObject::connect(&m_server, &QUaServer::clientConnected, this,
    [](const QUaSession* session) {
        qDebug() << "[INFO] Client connected" << QString("%1:%2").arg(session->address()).arg(session->port());
        qDebug() << "[INFO] Client connected" << session->applicationName();
        });
    QObject::connect(&m_server, &QUaServer::clientDisconnected, this,
        [](const QUaSession* session) {
            qDebug() << "[INFO] Client disconnected" << QString("%1:%2").arg(session->address()).arg(session->port());
            qDebug() << "[INFO] Client disconnected" << session->applicationName();
    });
    // setup server
    this->setupServer();
    // setup type table
    this->setupTable();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setupServer()
{
    // add methods to create children
    auto objs = m_server.objectsFolder();
    this->addMethods(objs, true);
    // start server
    m_server.start();
}

void Dialog::setupTable()
{
    // setup model
    //m_model.bindType<QUaFolderObject>(&m_server);
    //m_model.bindType<QUaBaseObject>(&m_server);
    m_model.bindType<QUaBaseDataVariable>(&m_server);
    // setup model column data sources
    m_model.setColumnDataSource(0, tr("Display Name"), 
    [](QUaNode * node) {
        return node->displayName();
    }/* second callback is only necessary for data that changes */);
    m_model.setColumnDataSource(1, tr("Node Id"), 
    [](QUaNode * node) {
        return node->nodeId();
    });
    m_model.setColumnDataSource(2, tr("Value"), 
    [](QUaNode * node) {
        QString strType(node->metaObject()->className());
        // only print value for variables
        if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
            strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
        {
            return QVariant();
        }
        auto var = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        return var->value();
    },
    [](QUaNode * node, std::function<void()> changeCallback) {
        QString strType(node->metaObject()->className());
        // only print value for variables
        if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
            strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
        {
            return QMetaObject::Connection();
        }
        auto var = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        return QObject::connect(var, &QUaBaseVariable::valueChanged,
        [changeCallback]() {
            changeCallback();
        });
    },
    [](QUaNode * node) {
        QString strType(node->metaObject()->className());
        // only edit value for variables
        if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
            strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
        {
            return false;
        }
        return true;
    });

    // setup tree editor
    ui->tableView->setColumnEditor(2,
    [](QWidget* parent, QUaNode* node) {
        Q_UNUSED(node);
        // create editor
        auto editor = new QSpinBox(parent);
        editor->setMinimum((std::numeric_limits<int>::min)());
        editor->setMaximum((std::numeric_limits<int>::max)());
        auto var = qobject_cast<QUaBaseVariable*>(node);
        // set current value to editor
        Q_CHECK_PTR(var);
        editor->setValue(var->value().toInt());
        return editor;
    }, 
    [](QWidget* editor, QUaNode* node) {
        // do nothing if value changes while editing, 
        // else user input will be overwritten
        Q_UNUSED(editor);
        Q_UNUSED(node);
    },
    [](QWidget* editor, QUaNode* node) {
        auto sbox = static_cast<QSpinBox*>(editor);
        auto var  = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        var->setValue(sbox->value());
    });

    // allow sorting
    m_proxy.setSourceModel(&m_model);
    ui->tableView->setModel(&m_proxy);
    ui->tableView->setSortingEnabled(true);
}

void Dialog::addMethods(QUaBaseObject* obj, const bool& isObjsFolder)
{
    // NOTE : only objects support ::addMethod
    obj->addMethod("addFolder", [this, obj](QString strName) {
	    if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newFolder = obj->addFolderObject(QString("ns=1;s=%1").arg(strName));
        newFolder->setBrowseName(strName);
        newFolder->setDisplayName(strName);
        this->addMethods(newFolder);
        return QString("Success : %1 created.").arg(strName);
    });
    obj->addMethod("addObject", [this, obj](QString strName) {
        if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newObj = obj->addBaseObject(QString("ns=1;s=%1").arg(strName));
        newObj->setBrowseName(strName);
        newObj->setDisplayName(strName);
        this->addMethods(newObj);
        return QString("Success : %1 created.").arg(strName);
    });
    obj->addMethod("addVariable", [this, obj](QString strName) {
        if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newVar = obj->addBaseDataVariable(QString("ns=1;s=%1").arg(strName));
        newVar->setBrowseName(strName);
        newVar->setDisplayName(strName);
        return QString("Success : %1 created.").arg(strName);
    });
    if (isObjsFolder)
    {
        obj->addMethod("unbindAll", [this]() {
            m_model.unbindAll();
	        return;
        });
        obj->addMethod("unbindQUaFolderObject", [this]() {
            m_model.unbindType<QUaFolderObject>();
	        return;
        });
        obj->addMethod("unbindQUaBaseObject", [this]() {
            m_model.unbindType<QUaBaseObject>();
	        return;
        });
        obj->addMethod("removeFromTable", [this](QString strNodeId) {
            auto node = m_server.nodeById(strNodeId);
            if (!node)
            {
                return QString("Node %1 does not exist.").arg(strNodeId);
            }
            if (!m_model.removeNode(node))
            {
                return QString("Node %1 not in table.").arg(strNodeId);
            }
	        return QString("Success : %1 removed from table.").arg(strNodeId);
        });
        return;
    }
    obj->addMethod("deleteThis", [obj]() {
        obj->deleteLater();
	    return;
    });
}

