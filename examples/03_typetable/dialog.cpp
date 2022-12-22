#include "dialog.h"
#include "ui_dialog.h"

#include <QSpinBox>

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QObject::connect(&m_server, &QUaServer::logMessage, this,
    [](const QUaLog &log) {
        qDebug() << "[" << log.level << "]["<< log.category << "] :" << log.message;
    });
    QObject::connect(&m_server, &QUaServer::clientConnected, this,
    [](const QUaSession* session) {
        qDebug() << "[INFO] Client connected" << QString("%1:%2").arg(session->address()).arg(session->port());
        qDebug() << "[INFO] Client connected" << session->applicationName();
        });
    QObject::connect(&m_server, &QUaServer::clientDisconnected, this,
        [](const QUaSession* session) {
            qDebug() << "[INFO] Client disconnected" << QString("%1:%2").arg(session->address()).arg(session->port());
            qDebug() << "[INFO] Client disconnected" << session->applicationName();
    });
    // setup server
    this->setupServer();
    // setup type table
    this->setupTableTypes();
    // setup categories tree
    this->setupTreeCategories();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setupServer()
{
    // add methods to create children
    auto objs = m_server.objectsFolder();
    this->addMethods(objs, true);
    // start server
    m_server.start();
}

void Dialog::setupTableTypes()
{
    // setup model
    m_modelTypes.bindType<QUaFolderObject>(&m_server);
    m_modelTypes.bindType<QUaBaseObject>(&m_server);
    m_modelTypes.bindType<QUaBaseDataVariable>(&m_server);
    // setup model column data sources
    m_modelTypes.setColumnDataSource(0, tr("Display Name"), 
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_0(node);
		}
		return QVariant();
    }/* second callback is only necessary for data that changes */);
    m_modelTypes.setColumnDataSource(1, tr("Node Id"),
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_1(node);
		}
		return QVariant();
    });
    m_modelTypes.setColumnDataSource(2, tr("Value"), 
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_2(node);
		}
		return QVariant();
    },
    [this](QUaNode * node, std::function<void()> changeCallback) {
        return QList<QMetaObject::Connection>() <<
            this->changeCallback_2(node, changeCallback);
    },
    [this](QUaNode * node) {
        return this->editableCallback_2(node);
    });

    // setup tree editor
    ui->tableViewTypes->setColumnEditor(2,
    [](QWidget* parent, QUaNode* node) {
        Q_UNUSED(node);
        // create editor
        auto editor = new QSpinBox(parent);
        editor->setMinimum((std::numeric_limits<int>::min)());
        editor->setMaximum((std::numeric_limits<int>::max)());
        auto var = qobject_cast<QUaBaseVariable*>(node);
        // set current value to editor
        Q_CHECK_PTR(var);
        editor->setValue(var->value().toInt());
        return editor;
    }, 
    [](QWidget* editor, QUaNode* node) {
        // do nothing if value changes while editing, 
        // else user input will be overwritten
        Q_UNUSED(editor);
        Q_UNUSED(node);
    },
    [](QWidget* editor, QUaNode* node) {
        auto sbox = static_cast<QSpinBox*>(editor);
        auto var  = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        auto value = sbox->value();
        var->setValue(value);
    });

    // support delete, copy-paste
    ui->tableViewTypes->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewTypes->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableViewTypes->setDeleteCallback(
    [](QList<QUaNode*> &nodes) {
        while (!nodes.isEmpty())
        {
            auto node = nodes.takeFirst();
            if (node == node->server()->objectsFolder())
            {
                continue;
            }
            delete node;
        }
    });
    ui->tableViewTypes->setCopyCallback(
    [](const QList<QUaNode*> &nodes) {
        auto mime = new QMimeData();
        qDebug() << "copy callback";
        for (auto node : nodes)
        {
            qDebug() << "copy" << node->nodeId();
            mime->setText(
                mime->text().isEmpty() ?
                (QString)node->nodeId() :
                mime->text() + ", " + node->nodeId()
            );
        }
        return mime;
    });
    ui->tableViewTypes->setPasteCallback(
    [](const QList<QUaNode*> &nodes, const QMimeData* mime) {
        qDebug() << "paste callback :" 
                 << (mime ? mime->text() : "no data");
        for (auto node : nodes)
        {
            qDebug() << "paste target" << node->nodeId();
        }
    });

    // allow sorting
    m_proxyTypes.setSourceModel(&m_modelTypes);
    ui->tableViewTypes->setModel(&m_proxyTypes);
    ui->tableViewTypes->setSortingEnabled(true);
}

void Dialog::setupTreeCategories()
{
    // setup model
    m_modelCategories.setColumnDataSource(0, tr("Display Name"),
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_0(node);
		}
		return QVariant();
    }/* second callback is only necessary for data that changes */);
    m_modelCategories.setColumnDataSource(1, tr("Node Id"),
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_1(node);
		}
		return QVariant();
    });
    m_modelCategories.setColumnDataSource(2, tr("Value"),
    [this](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return this->dataCallback_2(node);
		}
		return QVariant();
    },
    [this](QUaNode * node, std::function<void()> changeCallback) {
        return  QList<QMetaObject::Connection>() <<
            this->changeCallback_2(node, changeCallback);
    },
    [this](QUaNode * node) {
        return this->editableCallback_2(node);
    });

    // folders category
	this->addCategory<QUaFolderObject>();
    // objects category
    this->addCategory<QUaBaseObject>();
    // variables category
    this->addCategory<QUaBaseDataVariable>();

    // support delete
    ui->treeViewCategories->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeViewCategories->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeViewCategories->setDeleteCallback(
    [this](QList<QUaNode*> &nodes) {
        while (!nodes.isEmpty())
        {
            auto node = nodes.takeFirst();
            // empty node means category selected
            if (!node)
            {
                auto indexes = ui->treeViewCategories->selectedIndexesOrigin();
                auto categories = m_modelCategories.indexesToCategories(indexes);
                for (auto strCategory : categories)
                {
                    m_modelCategories.removeCategory(strCategory);
                }
                continue;
            }
            // NOTE : removed from model, not deleted
            m_modelCategories.removeNode(node);
        }
    });

    // support clear in tree context menu
    ui->treeViewCategories->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->treeViewCategories, &QTreeView::customContextMenuRequested, this,
    [this](const QPoint& point) {
        QMenu contextMenu(ui->treeViewCategories);
        contextMenu.addAction(tr("Clear"), this,
	    [this]() {
                m_modelCategories.clear();
	    });
        contextMenu.exec(ui->treeViewCategories->viewport()->mapToGlobal(point));
    });

    // allow sorting
    m_proxyCategories.setSourceModel(&m_modelCategories);
    ui->treeViewCategories->setModel(&m_proxyCategories);
    ui->treeViewCategories->setSortingEnabled(true);
}

void Dialog::addMethods(QUaBaseObject* obj, const bool& isObjsFolder)
{
    // NOTE : only objects support ::addMethod
    obj->addMethod("addFolder", [this, obj](QString strName) {
	    if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newFolder = obj->addFolderObject(strName, QString("ns=0;s=%1.%2").arg(obj->browseName().name()).arg(strName));
        this->addMethods(newFolder);
        return QString("Success : %1 created.").arg(strName);
    });
    obj->addMethod("addObject", [this, obj](QString strName) {
        if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newObj = obj->addBaseObject(strName, QString("ns=0;s=%1.%2").arg(obj->browseName().name()).arg(strName));
        this->addMethods(newObj);
        return QString("Success : %1 created.").arg(strName);
    });
    obj->addMethod("addVariable", [this, obj](QString strName) {
        if (obj->browseChild(strName))
        {
            return QString("Error : %1 already exists.").arg(strName);
        }
        auto newVar = obj->addBaseDataVariable(strName, QString("ns=0;s=%1.%2").arg(obj->browseName().name()).arg(strName));
        newVar->setWriteAccess(true);
        return QString("Success : %1 created.").arg(strName);
    });
    if (isObjsFolder)
    {
        obj->addMethod("unbindAll", [this]() {
            m_modelTypes.unbindAll();
	        return;
        });
        obj->addMethod("unbindQUaFolderObject", [this]() {
            m_modelTypes.unbindType<QUaFolderObject>();
	        return;
        });
        obj->addMethod("unbindQUaBaseObject", [this]() {
            m_modelTypes.unbindType<QUaBaseObject>();
	        return;
        });
        obj->addMethod("removeFromTable", [this](QString strNodeId) {
            auto node = m_server.nodeById(strNodeId);
            if (!node)
            {
                return QString("Node %1 does not exist.").arg(strNodeId);
            }
            if (!m_modelTypes.removeNode(node))
            {
                return QString("Node %1 not in table.").arg(strNodeId);
            }
	        return QString("Success : %1 removed from table.").arg(strNodeId);
        });
        return;
    }
    obj->addMethod("deleteThis", [obj]() {
        obj->deleteLater();
	    return;
    });
}

QVariant Dialog::dataCallback_0(QUaNode* node)
{
    return node->displayName().text();
}

QVariant Dialog::dataCallback_1(QUaNode* node)
{
    return (QString)node->nodeId();
}

QVariant Dialog::dataCallback_2(QUaNode* node)
{
    QString strType(node->metaObject()->className());
    // only print value for variables
    if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
        strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
    {
        return QVariant();
    }
    auto var = qobject_cast<QUaBaseVariable*>(node);
    Q_CHECK_PTR(var);
    return var->value();
}

QMetaObject::Connection Dialog::changeCallback_2(QUaNode* node, std::function<void(void)>& changeCallback)
{
    QString strType(node->metaObject()->className());
    // only print value for variables
    if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
        strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
    {
        return QMetaObject::Connection();
    }
    auto var = qobject_cast<QUaBaseVariable*>(node);
    Q_CHECK_PTR(var);
    return QObject::connect(var, &QUaBaseVariable::valueChanged,
    [changeCallback]() {
        changeCallback();
    });
}

bool Dialog::editableCallback_2(QUaNode* node)
{
    QString strType(node->metaObject()->className());
    // only edit value for variables
    if (strType.compare("QUaProperty", Qt::CaseSensitive) != 0 &&
        strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) != 0)
    {
        return false;
    }
    return true;
}

