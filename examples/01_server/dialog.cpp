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
}

Dialog::~Dialog()
{
    delete ui;
}

