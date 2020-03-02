#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
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

    QObject::connect(&m_server, &QUaServer::logMessage, &m_model,
    [this](const QUaLog& log) {
        m_logs << log;
        m_model.addNode(&m_logs.last());
    });
    // setup model column data sources
    m_model.setColumnDataSource(0, tr("Category"), 
    [](QUaLog* log) {
        return static_cast<int>(log->category);
    }/* second callback is only necessary for data that changes */);
    m_model.setColumnDataSource(1, tr("Message"), 
    [](QUaLog* log) {
        return log->message;
    });
    ui->tableViewLogs->setModel(&m_model);
}

Dialog::~Dialog()
{
    delete ui;
}

