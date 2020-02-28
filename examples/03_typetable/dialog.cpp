#include "dialog.h"
#include "ui_dialog.h"

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
    m_model.bindType<QUaFolderObject>(&m_server);
    m_model.bindType<QUaBaseObject>(&m_server);
    // setup model column data sources
    m_model.setColumnDataSource(0, tr("Display Name"), 
    [](QUaNode * node) {
        return node->displayName();
    }/* second callback is only necessary for data that changes */);
    m_model.setColumnDataSource(1, tr("Node Id"), 
    [](QUaNode * node) {
        return node->nodeId();
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
        auto newFolder = obj->addFolderObject();
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
        auto newObj = obj->addBaseObject();
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
        auto newVar = obj->addBaseDataVariable();
        newVar->setBrowseName(strName);
        newVar->setDisplayName(strName);
        return QString("Success : %1 created.").arg(strName);
    });
    if (isObjsFolder)
    {
        obj->addMethod("unbindTable", [this]() {
            m_model.unbindAll();
	        return;
        });
        return;
    }
    obj->addMethod("deleteThis", [obj]() {
        obj->deleteLater();
	    return;
    });
}

