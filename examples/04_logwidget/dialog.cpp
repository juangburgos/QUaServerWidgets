#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    // configure log widget
    ui->widgetLog->setMaxEntries(100);
    QObject::connect(&m_server, &QUaServer::logMessage, ui->widgetLog, &QUaLogWidget::addLog);
    // run server
    m_server.start();
}

Dialog::~Dialog()
{
    delete ui;
}
