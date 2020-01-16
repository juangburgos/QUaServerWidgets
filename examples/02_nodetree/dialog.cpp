#include "dialog.h"
#include "ui_dialog.h"

#include "quabaseobjectext.h"

#include <QMetaObject>
#include <QInputDialog>
#include <QMessageBox>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , m_model(this)
{
    ui->setupUi(this);
    // setup server
    this->setupServer();
    // setup node tree
    this->setupTree();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setupServer()
{
    // register extensible object type
    m_server.registerType<QUaBaseObjectExt>("ns=1;s=QUaBaseObjectExt");
    // add root instance of extensible object
    auto objs = m_server.objectsFolder();
    auto root = objs->addChild<QUaBaseObjectExt>("ns=1;s=root");
    root->setDisplayName("root");
    root->setBrowseName("root");
    // start server
    m_server.start();
}

void Dialog::setupTree()
{
    auto objs = m_server.objectsFolder();
    auto root = objs->browseChild("root");
    Q_CHECK_PTR(root);
    // setup model
    m_model.setRootNode(root);
    // setup view
    ui->treeView->setModel(&m_model);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->treeView, &QTreeView::customContextMenuRequested, this,
    [this](const QPoint& point) {
        QModelIndex index = ui->treeView->indexAt(point);
        QMenu contextMenu(ui->treeView);
        auto node = m_model.nodeFromIndex(index);
        Q_CHECK_PTR(node);
        QString strType(node->metaObject()->className());
        if (strType.compare("QUaBaseDataVariable", Qt::CaseInsensitive) == 0)
        {
            auto datavar = dynamic_cast<QUaBaseDataVariable*>(node);
            Q_CHECK_PTR(datavar);
        }
        if (strType.compare("QUaBaseObject", Qt::CaseInsensitive) == 0)
        {
            auto obj = dynamic_cast<QUaBaseObject*>(node);
            Q_CHECK_PTR(obj);
        }
        if (strType.compare("QUaFolderObject", Qt::CaseInsensitive) == 0)
        {
            auto folder = dynamic_cast<QUaFolderObject*>(node);
            Q_CHECK_PTR(folder);
        }
        if (strType.compare("QUaBaseObjectExt", Qt::CaseInsensitive) == 0)
        {
            auto extobj = dynamic_cast<QUaBaseObjectExt*>(node);
            Q_CHECK_PTR(extobj);
            this->setupQUaBaseObjectExtMenu(contextMenu, extobj);
        }
        if (strType.compare("QUaProperty", Qt::CaseInsensitive) == 0)
        {
            auto prop = dynamic_cast<QUaProperty*>(node);
            Q_CHECK_PTR(prop);
        }
        // exec
        contextMenu.exec(ui->treeView->viewport()->mapToGlobal(point));
    });
}

void Dialog::setupQUaBaseObjectExtMenu(QMenu& menu, QUaBaseObjectExt* extobj)
{
    menu.addAction(tr("Add QUaBaseObjectExt"), this,
	[this, extobj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add QUaBaseObjectExt to %1").arg(extobj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        extobj->addObjectExtChild(name);
	});
    menu.addSeparator();
    menu.addAction(tr("Add Folder"), this,
	[this, extobj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Folder to %1").arg(extobj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        extobj->addFolderChild(name);
	});
    menu.addAction(tr("Add BaseObject"), this,
	[this, extobj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add base BaseObject to %1").arg(extobj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        extobj->addBaseObjectChild(name);
	});
    menu.addAction(tr("Add DataVariable"), this,
	[this, extobj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add DataVariable to %1").arg(extobj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        extobj->addBaseDataVariableChild(name);
	});
    menu.addAction(tr("Add Property"), this,
	[this, extobj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Property to %1").arg(extobj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        extobj->addPropertyChild(name);
	});
    menu.addSeparator();
    menu.addAction(tr("Delete \"%1\"").arg(extobj->displayName()), this,
	[this, extobj]() {
        auto res = QMessageBox::question(this, tr("Delete %1").arg(extobj->displayName()), tr("Are you sure you want to delete %1?").arg(extobj->displayName()));
        if (res != QMessageBox::Yes) { return; }
        extobj->deleteLater();
	});
}

