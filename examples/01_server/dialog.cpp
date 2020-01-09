#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    // bind server to widget
    ui->widgetServer->bindServer(&m_server);
}

Dialog::~Dialog()
{
    delete ui;
}

