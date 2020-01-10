#include "dialog.h"
#include "ui_dialog.h"

#include "quabaseobjectext.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // setup server

    // register extensible object type
    m_server.registerType<QUaBaseObjectExt>("ns=1;s=QUaBaseObjectExt");
    // add root instance of extensible object
    auto root = m_server.objectsFolder()->addChild<QUaBaseObjectExt>("ns=1;s=root");
    root->setDisplayName("Root");
    root->setBrowseName("Root");
    // start server
    m_server.start();

    // setup model - view

    // TODO 

}

Dialog::~Dialog()
{
    delete ui;
}

