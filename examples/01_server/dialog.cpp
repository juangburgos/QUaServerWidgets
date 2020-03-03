#include "dialog.h"
#include "ui_dialog.h"

QMetaEnum logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    
    this->setupServer();
    this->setupLogTable();
    this->setupSessionTable();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setupServer()
{
    // bind server to widget
    ui->widgetServer->bindServer(&m_server);
    // setup test methods
    auto objsFolder = m_server.objectsFolder();
    // 
    objsFolder->addMethod("setPort",
    [this](quint16 intPort) {
        m_server.setPort(intPort);
    });
    // 
    objsFolder->addMethod("setMaxSecureChannels",
    [this](quint16 maxSecureChannels) {
        m_server.setMaxSecureChannels(maxSecureChannels);
    });
    // 
    objsFolder->addMethod("setMaxSessions",
    [this](quint16 maxSessions) {
        m_server.setMaxSessions(maxSessions);
    });
    // 
    objsFolder->addMethod("setApplicationName",
    [this](QString strApplicationName) {
        m_server.setApplicationName(strApplicationName);
    });
    // 
    objsFolder->addMethod("setApplicationUri",
    [this](QString strApplicationUri) {
        m_server.setApplicationUri(strApplicationUri);
    });
    // 
    objsFolder->addMethod("setProductName",
    [this](QString strProductName) {
        m_server.setProductName(strProductName);
    });
    // 
    objsFolder->addMethod("setProductUri",
    [this](QString strProductUri) {
        m_server.setProductUri(strProductUri);
    });
    // 
    objsFolder->addMethod("setManufacturerName",
    [this](QString strManufacturerName) {
        m_server.setManufacturerName(strManufacturerName);
    });
    // 
    objsFolder->addMethod("setSoftwareVersion",
    [this](QString strSoftwareVersion) {
        m_server.setSoftwareVersion(strSoftwareVersion);
    });
    // 
    objsFolder->addMethod("setBuildNumber",
    [this](QString strBuildNumber) {
        m_server.setBuildNumber(strBuildNumber);
    });
}

void Dialog::setupLogTable()
{
    QObject::connect(&m_server, &QUaServer::logMessage, &m_modelLog,
    [this](const QUaLog& log) {
        m_modelLog.addNode(log);
    });
    // setup model column data sources
    m_modelLog.setColumnDataSource(0, tr("Timestamp"), 
    [](QUaLog log) {
        return log.timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(1, tr("Level"), 
    [](QUaLog log) {
        return logLevelMetaEnum.valueToKey(static_cast<int>(log.level));
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(2, tr("Category"),
    [](QUaLog log) {
        return logCategoryMetaEnum.valueToKey(static_cast<int>(log.category));
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(3, tr("Message"), 
    [](QUaLog log) {
        return log.message;
    }/* other callbacks for data that changes or editable */);
    // allow sorting
    m_proxyLog.setSourceModel(&m_modelLog);
    ui->tableViewLogs->setModel(&m_proxyLog);
    ui->tableViewLogs->setSortingEnabled(true);
    ui->tableViewLogs->sortByColumn(0, Qt::DescendingOrder);
}

void Dialog::setupSessionTable()
{
    QObject::connect(&m_server, &QUaServer::clientConnected, &m_modelSession,
    [this](const QUaSession* session) {
        m_modelSession.addNode(session);
    });
    QObject::connect(&m_server, &QUaServer::clientDisconnected, &m_modelSession,
    [this](const QUaSession* session) {
        m_modelSession.removeNode(session);
    });
    // setup model column data sources
    m_modelSession.setColumnDataSource(0, tr("Timestamp"),
    [](const QUaSession* session) {
        return session->timestamp().toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(1, tr("Id"),
    [](const QUaSession* session) {
        return session->sessionId();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(2, tr("Name"),
    [](const QUaSession* session) {
        return session->applicationName();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(3, tr("Address"),
    [](const QUaSession* session) {
        return session->address();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(4, tr("Port"),
    [](const QUaSession* session) {
        return session->port();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(5, tr("Username"),
    [](const QUaSession* session) {
        return session->userName();
    }/* other callbacks for data that changes or editable */);
    // allow sorting
    m_proxySession.setSourceModel(&m_modelSession);
    ui->tableViewSessions->setModel(&m_proxySession);
    ui->tableViewSessions->setSortingEnabled(true);
    ui->tableViewSessions->sortByColumn(0, Qt::AscendingOrder);
}

void Dialog::on_pushButtonClearLog_clicked()
{
    m_modelLog.clear();
}
