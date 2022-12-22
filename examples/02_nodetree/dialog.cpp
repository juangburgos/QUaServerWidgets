#include "dialog.h"
#include "ui_dialog.h"

#include <limits>

#include "quabaseobjectext.h"

#include <QMetaObject>
#include <QInputDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QDateTimeEdit>

#include "quaxmlserializer.h"
#include "quasqliteserializer.h"

QUaServer* Dialog::m_pserver;
QMetaEnum Dialog::m_logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum Dialog::m_logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();
QHash<
    QUaLog*,
    QList<std::function<void(void)>>
> Dialog::m_hashDestroyLog;

QString Dialog::logToString(const QQueue<QUaLog>& logOut)
{
    QString strLog;
    for (auto &log : logOut)
    {
        // construct string
        strLog += QString("[%1] : %2\n")
            .arg(Dialog::m_logLevelMetaEnum.valueToKey(static_cast<int>(log.level)))
            .arg(log.message.constData());
        // also emit to show in log tree
        emit Dialog::m_pserver->logMessage(log);
    }
    return strLog;
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , m_modelNodes(this)
{
    ui->setupUi(this);
    Dialog::m_pserver = &m_server;
    // setup node tree
    this->setupTreeNodes();
    // setup log tree
    this->setupTreeLogs();
    // setup server
    this->setupServer();
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
    // add extra method to test clear tree
    objs->addMethod("clearTree",
    [this]() {
        //NOTE : if use m_modelNodes.setRootNode(nullptr); then dont forget to handle nodeFromIndex accordingly
        m_modelNodes.clear();
    });
    // test serialize
    objs->addMethod("SerializeXML", [objs](QString strFileName) {
	    QUaXmlSerializer serializer;
	    QQueue<QUaLog> logOut;
	    if (!serializer.setXmlFileName(strFileName, logOut))
	    {
		    return logToString(logOut);
	    }
	    if (!objs->serialize(serializer, logOut))
	    {
            return logToString(logOut);
	    }
        for (auto& log : logOut)
        {
            // also emit to show in log tree
            emit objs->server()->logMessage(log);
        }
	    return QString("Success : Serialized to %1 file.").arg(strFileName);
    });
    objs->addMethod("SerializeSQL", [objs](QString strFileName) {
	    QUaSqliteSerializer serializer;
	    QQueue<QUaLog> logOut;
	    if (!serializer.setSqliteDbName(strFileName, logOut))
	    {
            return logToString(logOut);
	    }
	    if (!objs->serialize(serializer, logOut))
	    {
            return logToString(logOut);
	    }
        for (auto& log : logOut)
        {
            // also emit to show in log tree
            emit objs->server()->logMessage(log);
        }
	    return QString("Success : Serialized to %1 file.").arg(strFileName);
    });
    // test deserialize
    objs->addMethod("DeserializeXML", [objs](QString strFileName) {
        QUaXmlSerializer serializer;
	    QQueue<QUaLog> logOut;
	    if (!serializer.setXmlFileName(strFileName, logOut))
	    {
            return logToString(logOut);
	    }
	    if (!objs->deserialize(serializer, logOut))
	    {
            return logToString(logOut);
	    }
        for (auto& log : logOut)
        {
            // also emit to show in log tree
            emit objs->server()->logMessage(log);
        }
	    return QString("Success : Deserialized from %1 file.").arg(strFileName);
    });
    objs->addMethod("DeserializeSQL", [objs](QString strFileName) {
        QUaSqliteSerializer serializer;
	    QQueue<QUaLog> logOut;
	    if (!serializer.setSqliteDbName(strFileName, logOut))
	    {
            return logToString(logOut);
	    }
	    if (!objs->deserialize(serializer, logOut))
	    {
            return logToString(logOut);
	    }
        for (auto& log : logOut)
        {
            // also emit to show in log tree
            emit objs->server()->logMessage(log);
        }
	    return QString("Success : Deserialized from %1 file.").arg(strFileName);
    });
    // start server
    m_server.start();
}

void Dialog::setupTreeNodes()
{
    auto objs = m_server.objectsFolder();
    // setup model into tree
    m_modelNodes.setRootNode(objs);

    // setup tree context menu
    ui->treeViewNodes->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->treeViewNodes, &QTreeView::customContextMenuRequested, this,
    [this](const QPoint& point) {
        QModelIndex index = m_proxyNodes.mapToSource(ui->treeViewNodes->indexAt(point));
        QMenu contextMenu(ui->treeViewNodes);
        auto node = m_modelNodes.nodeFromIndex(index);
        if (!node)
        {
            Q_ASSERT_X(node, "Tree context menu", "Use model clear() instead of setRootNode(nullptr)");
            return;
        }
        QString strType(node->metaObject()->className());
        // objects, objectsext and folders are all objects
        if (strType.compare("QUaProperty", Qt::CaseSensitive) == 0)
        {
            auto prop = qobject_cast<QUaProperty*>(node);
            Q_CHECK_PTR(prop);
            this->setupQUaPropertyMenu(contextMenu, prop);
        }
        else if (strType.compare("QUaBaseDataVariable", Qt::CaseSensitive) == 0)
        {
            auto datavar = qobject_cast<QUaBaseDataVariable*>(node);
            Q_CHECK_PTR(datavar);
            this->setupQUaBaseDataVariableMenu(contextMenu, datavar);
        }
        else
        {
            auto obj = qobject_cast<QUaBaseObject*>(node);
            Q_CHECK_PTR(obj);
            this->setupQUaBaseObjectMenu(contextMenu, obj);
        }
        // exec
        contextMenu.exec(ui->treeViewNodes->viewport()->mapToGlobal(point));
    });

    // setup model column data sources
    m_modelNodes.setColumnDataSource(0, tr("Display Name"), 
    [](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return node->displayName().toXmlString();
		}
		return QVariant();
    }/* second callback is only necessary for data that changes */);
    m_modelNodes.setColumnDataSource(1, tr("Node Id"), 
    [](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return node->nodeId().toXmlString();
		}
		return QVariant();
    });
    m_modelNodes.setColumnDataSource(2, tr("Value"), 
    [](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			auto var = qobject_cast<QUaBaseVariable*>(node);
			if (!var) { return QVariant(); }
			return var->value();
		}
		return QVariant();
    },
    [](QUaNode * node, std::function<void(void)> changeCallback) {
        QList<QMetaObject::Connection> retList;
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return retList; }
        retList <<
        QObject::connect(var, &QUaBaseVariable::valueChanged,
        [changeCallback]() {
            changeCallback();
        });
        return retList;
    },
    [](QUaNode * node) {
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return false; }
        return true;
    });
    m_modelNodes.setColumnDataSource(3, tr("Source Timestamp"), 
    [](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			auto var = qobject_cast<QUaBaseVariable*>(node);
			if (!var) { return QString(); }
			return var->sourceTimestamp().toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		}
		return QVariant();
    },
    [](QUaNode * node, std::function<void(void)> changeCallback) {
        QList<QMetaObject::Connection> retList;
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return retList; }
        retList <<
        QObject::connect(var, &QUaBaseVariable::sourceTimestampChanged,
        [changeCallback]() {
            changeCallback();
        });
        return retList;
    },
    [](QUaNode * node) {
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return false; }
        return true;
    });
    m_modelNodes.setColumnDataSource(4, tr("Server Timestamp"), 
    [](QUaNode * node, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			auto var = qobject_cast<QUaBaseVariable*>(node);
			if (!var) { return QString(); }
			return var->serverTimestamp().toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		}
        return QVariant();
    },
    [](QUaNode * node, std::function<void(void)> changeCallback) {
        QList<QMetaObject::Connection> retList;
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return retList; }
        retList <<
        QObject::connect(var, &QUaBaseVariable::serverTimestampChanged,
        [changeCallback]() {
            changeCallback();
        });
        return retList;
    },
    [](QUaNode * node) {
        auto var = qobject_cast<QUaBaseVariable*>(node);
        if (!var) { return false; }
        return true;
    });

    // setup tree editors
    ui->treeViewNodes->setColumnEditor(2,
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
        var->setValue(sbox->value());
    });
    ui->treeViewNodes->setColumnEditor(3,
    [](QWidget* parent, QUaNode* node) {
        Q_UNUSED(node);
        // create editor
        auto editor = new QDateTimeEdit(parent);
        auto var = qobject_cast<QUaBaseVariable*>(node);
        // set current value to editor
        Q_CHECK_PTR(var);
        editor->setDateTime(var->sourceTimestamp());
        editor->setDisplayFormat("dd.MM.yyyy hh:mm:ss.zzz");
        return editor;
    }, 
    [](QWidget* editor, QUaNode* node) {
        Q_UNUSED(editor);
        Q_UNUSED(node);
    },
    [](QWidget* editor, QUaNode* node) {
        auto dedit = static_cast<QDateTimeEdit*>(editor);
        auto var   = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        var->setSourceTimestamp(dedit->dateTime());
    });
    ui->treeViewNodes->setColumnEditor(4,
    [](QWidget* parent, QUaNode* node) {
        Q_UNUSED(node);
        // create editor
        auto editor = new QDateTimeEdit(parent);
        auto var = qobject_cast<QUaBaseVariable*>(node);
        // set current value to editor
        Q_CHECK_PTR(var);
        editor->setDateTime(var->serverTimestamp());
        editor->setDisplayFormat("dd.MM.yyyy hh:mm:ss.zzz");
        return editor;
    }, 
    [](QWidget* editor, QUaNode* node) {
        Q_UNUSED(editor);
        Q_UNUSED(node);
    },
    [](QWidget* editor, QUaNode* node) {
        auto dedit = static_cast<QDateTimeEdit*>(editor);
        auto var   = qobject_cast<QUaBaseVariable*>(node);
        Q_CHECK_PTR(var);
        var->setServerTimestamp(dedit->dateTime());
    });

    // support copy-paste
    ui->treeViewNodes->setCopyCallback(
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
    ui->treeViewNodes->setPasteCallback(
    [](const QList<QUaNode*> &nodes, const QMimeData* mime) {
        qDebug() << "paste callback :" 
                 << (mime ? mime->text() : "no data");
        for (auto node : nodes)
        {
            qDebug() << "paste target" << node->nodeId();
        }
    });
    // allow sorting
    m_proxyNodes.setSourceModel(&m_modelNodes);
    ui->treeViewNodes->setModel(&m_proxyNodes);
    ui->treeViewNodes->setSortingEnabled(true);
}

template<>
inline QMetaObject::Connection
QUaModelItemTraits::NewChildCallback<QUaLog>(
    QUaLog *log, 
    const std::function<void(QUaLog&)> &callback)
{
    // valid logs do not have children
    //if (QUaModelItemTraits::IsValid<QUaLog>(log))
    // TODO : VS2017 does not calls specialized template above from QUaLogModelItemTraits for some reason
    if (!log->message.isNull()) 
    {
        return QMetaObject::Connection();
    }
    // invalid logs (only one; root) have as children all valid logs
    return QObject::connect(Dialog::m_pserver, &QUaServer::logMessage,
    [callback](const QUaLog& log) {
        QUaLog nlog = log;
        callback(nlog);
    });
}

template<>
inline QMetaObject::Connection
QUaModelItemTraits::DestroyCallback<QUaLog>(
    QUaLog* log,
    const std::function<void(void)> &callback)
{
    Dialog::m_hashDestroyLog[log] << callback;
    return QMetaObject::Connection();
}

void Dialog::setupTreeLogs()
{
    m_modelLogs.setRootNode(QUaLog());
    // setup model column data sources
    m_modelLogs.setColumnDataSource(0, tr("Timestamp"),
    [](QUaLog * log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return log->timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLogs.setColumnDataSource(1, tr("Level"),
    [](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level));
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLogs.setColumnDataSource(2, tr("Category"),
    [](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category));
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLogs.setColumnDataSource(3, tr("Message"),
    [](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return log->message;
		}
		return QVariant();
    });
    // support delete and copy
    ui->treeViewLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeViewLog->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeViewLog->setDeleteCallback(
    [this](QList<QUaLog*> &logs) {
        while (logs.count() > 0)
        {
            Q_ASSERT(m_hashDestroyLog.contains(logs.first()));
            for (auto callback : m_hashDestroyLog.take(logs.takeFirst()))
            {
                callback();
            }
        }
    });
    ui->treeViewLog->setCopyCallback(
    [](const QList<QUaLog*> &logs) {
        auto mime = new QMimeData();
        for (auto log : logs)
        {
            mime->setText(
                mime->text() + QString("[%1] [%2] [%3] : %4.\n")
                .arg(log->timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
                .arg(m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level)))
                .arg(m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category)))
                .arg(QString(log->message))
            );
        }
        return mime;
    });
    // allow sorting
    m_proxyLogs.setSourceModel(&m_modelLogs);
    ui->treeViewLog->setModel(&m_proxyLogs);
    ui->treeViewLog->setSortingEnabled(true);
    ui->treeViewLog->sortByColumn(0, Qt::DescendingOrder);
}

void Dialog::setupQUaBaseObjectMenu(QMenu& menu, QUaBaseObject* obj)
{
    menu.addAction(tr("Add QUaBaseObjectExt"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        obj->addChild<QUaBaseObjectExt>(name);
	});
    menu.addAction(tr("Add Folder"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Folder to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        obj->addFolderObject(name);
	});
    menu.addAction(tr("Add BaseObject"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add base BaseObject to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        obj->addBaseObject(name);
	});
    menu.addAction(tr("Add DataVariable"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add DataVariable to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto basevar = obj->addBaseDataVariable(name);
        basevar->setWriteAccess(true);
        basevar->setDataType(QMetaType::Double);
	});
    menu.addAction(tr("Add Property"), this,
	[this, obj]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Property to %1").arg(obj->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto prop = obj->addProperty(name);
        prop->setWriteAccess(true);
        prop->setDataType(QMetaType::Double);
	});
    menu.addSeparator();
    QString strType(obj->metaObject()->className());
    // objectsext can add multiple children at once
    if (strType.compare("QUaBaseObjectExt", Qt::CaseSensitive) == 0)
    {
        menu.addAction(tr("Add Multiple QUaBaseObjectExt"), this,
	    [this, obj]() {
            auto objext = qobject_cast<QUaBaseObjectExt*>(obj);
            Q_CHECK_PTR(objext);
            bool ok;
            QString name = QInputDialog::getText(this, tr("Add Multiple QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Children Base Name:"), QLineEdit::Normal, "", &ok);
            if (!ok) { return; }
            int i = QInputDialog::getInt(this, tr("Add Multiple QUaBaseObjectExt to %1").arg(obj->displayName()), tr("Number of children:"), 1, 0, 2147483647, 10, &ok);
            if (!ok) { return; }
            objext->addMulitpleObjectExtChild(name, i);
	    });
        menu.addAction(tr("Add Multiple DataVariable"), this,
	    [this, obj]() {
            auto objext = qobject_cast<QUaBaseObjectExt*>(obj);
            Q_CHECK_PTR(objext);
            bool ok;
            QString name = QInputDialog::getText(this, tr("Add Multiple DataVariable to %1").arg(obj->displayName()), tr("Children Base Name:"), QLineEdit::Normal, "", &ok);
            if (!ok) { return; }
            int i = QInputDialog::getInt(this, tr("Add Multiple DataVariable to %1").arg(obj->displayName()), tr("Number of children:"), 1, 0, 2147483647, 10, &ok);
            if (!ok) { return; }
            objext->addMultipleBaseDataVariableChild(name, i);
	    });
        menu.addSeparator();
    }
    // cannot delete objects folder
    if (obj == m_server.objectsFolder())
    {
        return;
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
        datavar->addChild<QUaBaseObjectExt>(name);
	});
    menu.addAction(tr("Add Folder"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Folder to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        datavar->addFolderObject(name);
	});
    menu.addAction(tr("Add BaseObject"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add base BaseObject to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        datavar->addBaseObject(name);
	});
    menu.addAction(tr("Add DataVariable"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add DataVariable to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto basevar = datavar->addBaseDataVariable(name);
        basevar->setWriteAccess(true);
        basevar->setDataType(QMetaType::Double);
	});
    menu.addAction(tr("Add Property"), this,
	[this, datavar]() {
        bool ok;
        QString name = QInputDialog::getText(this, tr("Add Property to %1").arg(datavar->displayName()), tr("Child Name:"), QLineEdit::Normal, "", &ok);
        if (!ok) { return; }
        auto prop = datavar->addProperty(name);
        prop->setWriteAccess(true);
        prop->setDataType(QMetaType::Double);
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

