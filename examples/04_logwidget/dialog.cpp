#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QObject::connect(&m_server, &QUaServer::logMessage, ui->widgetLog, &QUaLogWidget::addLog);

    ui->widgetLog->setMaxEntries(15);

    m_server.start();

}

Dialog::~Dialog()
{
    delete ui;
}
