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
        // objects, objectsext and folders are all objects
        if (strType.compare("QUaProperty", Qt::CaseInsensitive) == 0)
        {
            auto prop = dynamic_cast<QUaProperty*>(node);
            Q_CHECK_PTR(prop);
            this->setupQUaPropertyMenu(contextMenu, prop);
        }
        else if (strType.compare("QUaBaseDataVariable", Qt::CaseInsensitive) == 0)
        {
            auto datavar = dynamic_cast<QUaBaseDataVariable*>(node);
            Q_CHECK_PTR(datavar);
            this->setupQUaBaseDataVariableMenu(contextMenu, datavar);
        }
        else
        {
            auto obj = dynamic_cast<QUaBaseObject*>(node);
            Q_CHECK_PTR(obj);
            this->setupQUaBaseObjectMenu(contextMenu, obj);
        }
        // exec
        contextMenu.exec(ui->treeView->viewport()->mapToGlobal(point));
    });
}

void Dialog::setupQUaBaseObjectMenu(QMenu& menu, QUaBaseObject* obj)
{
    menu.addAction(tr("Add QUaBaseObjectExt"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto objext = obj->addChild<QUaBaseObjectExt>();
        objext->setDisplayName(name);
        objext->setBrowseName(name);
	});
    menu.addAction(tr("Add Folder"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Folder to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto folder = obj->addFolderObject();
        folder->setDisplayName(name);
        folder->setBrowseName(name);
	});
    menu.addAction(tr("Add BaseObject"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add base BaseObject to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto nobj = obj->addBaseObject();
        nobj->setDisplayName(name);
        nobj->setBrowseName(name);
	});
    menu.addAction(tr("Add DataVariable"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add DataVariable to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto basevar = obj->addBaseDataVariable();
        basevar->setDisplayName(name);
        basevar->setBrowseName(name);
	});
    menu.addAction(tr("Add Property"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Property to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto prop = obj->addProperty();
        prop->setDisplayName(name);
        prop->setBrowseName(name);
	});
    menu.addSeparator();
    QString strType(obj->metaObject()->className());
    // objectsext can add multiple children at once
    if (strType.compare("QUaBaseObjectExt", Qt::CaseInsensitive) == 0)
    {
        menu.addAction(tr("Add Multiple QUaBaseObjectExt"), this,
	    [this, obj]() {
            auto objext = dynamic_cast<QUaBaseObjectExt*>(obj);
            Q_CHECK_PTR(objext);
            bool ok;
            QString name = QInputDialog::getText(this, tr("Add Multiple QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Children Base Name:"), QLineEdit::Normal, "", &ok);
            if (!ok) { return; }
            int i = QInputDialog::getInt(this, tr("Add Multiple QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Number of children:"), 1, 0, 2147483647, 10, &ok);
            if (!ok) { return; }
            objext->addMulitpleObjectExtChild(name, i);
	    });
        menu.addSeparator();
    }
    menu.addAction(tr("Delete \"%1\"").arg(obj->displayName()), this,
	[this, obj]() {
        auto res = QMessageBox::question(this, tr("Delete %1").arg(obj->displayName()), tr("Are you sure you want to delete %1?").arg(obj->displayName()));
        if (res != QMessageBox::Yes) { return; }
        obj->deleteLater();
	});
}

void Dialog::setupQUaBaseDataVariableMenu(QMenu& menu, QUaBaseDataVariable* datavar)
{
    menu.addAction(tr("Add QUaBaseObjectExt"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add QUaBaseObjectExt to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto objext = datavar->addChild<QUaBaseObjectExt>();
        objext->setDisplayName(name);
        objext->setBrowseName(name);
	});
    menu.addAction(tr("Add Folder"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Folder to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto folder = datavar->addFolderObject();
        folder->setDisplayName(name);
        folder->setBrowseName(name);
	});
    menu.addAction(tr("Add BaseObject"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add base BaseObject to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto nobj = datavar->addBaseObject();
        nobj->setDisplayName(name);
        nobj->setBrowseName(name);
	});
    menu.addAction(tr("Add DataVariable"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add DataVariable to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto basevar = datavar->addBaseDataVariable();
        basevar->setDisplayName(name);
        basevar->setBrowseName(name);
	});
    menu.addAction(tr("Add Property"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Property to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto prop = datavar->addProperty();
        prop->setDisplayName(name);
        prop->setBrowseName(name);
	});
    menu.addSeparator();
    menu.addAction(tr("Delete \"%1\"").arg(datavar->displayName()), this,
	[this, datavar]() {
        auto res = QMessageBox::question(this, tr("Delete %1").arg(datavar->displayName()), tr("Are you sure you want to delete %1?").arg(datavar->displayName()));
        if (res != QMessageBox::Yes) { return; }
        datavar->deleteLater();
	});
}

void Dialog::setupQUaPropertyMenu(QMenu& menu, QUaProperty* prop)
{
    menu.addAction(tr("Delete \"%1\"").arg(prop->displayName()), this,
	[this, prop]() {
        auto res = QMessageBox::question(this, tr("Delete %1").arg(prop->displayName()), tr("Are you sure you want to delete %1?").arg(prop->displayName()));
        if (res != QMessageBox::Yes) { return; }
        prop->deleteLater();
	});
}

