#include "dialog.h"
#include "ui_dialog.h"

#include "quabaseobjectext.h"

#include <QUaNodeModel>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    // setup server

    // register extensible object type
    m_server.registerType<QUaBaseObjectExt>("ns=1;s=QUaBaseObjectExt");
    // add root instance of extensible object
    auto objs = m_server.objectsFolder();
    auto root  = objs->addChild<QUaBaseObjectExt>("ns=1;s=root");
    root->setDisplayName("root");
    root->setBrowseName("root");
    // start server
    m_server.start();

    // setup model - view
    auto model = new QUaNodeModel(this);
    model->bindRootNode(root);

    ui->treeView->setModel(model);
}

Dialog::~Dialog()
{
    delete ui;
}

