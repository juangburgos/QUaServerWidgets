#include "dialog.h"
#include "ui_dialog.h"

#include <QLineEdit>

QMetaEnum logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    
    this->setupServer();
    this->setupLogTable();
    this->setupSessionTable();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setupServer()
{
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

void Dialog::setupLogTable()
{
    QObject::connect(&m_server, &QUaServer::logMessage, &m_modelLog,
    [this](const QUaLog& log) {
        m_modelLog.addNode(QUaLog(log));
    });
    // setup model column data sources
    m_modelLog.setColumnDataSource(0, tr("Timestamp"), 
    [](QUaLog * log, const Qt::ItemDataRole &role) -> QVariant {
        if (role == Qt::DisplayRole)
        {
            return log->timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
        }
        return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(1, tr("Level"), 
    [](QUaLog * log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return logLevelMetaEnum.valueToKey(static_cast<int>(log->level));
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(2, tr("Category"),
    [](QUaLog * log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return logCategoryMetaEnum.valueToKey(static_cast<int>(log->category));
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLog.setColumnDataSource(3, tr("Message"), 
    [](QUaLog * log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return log->message;
		}
		return QVariant();
    },
    nullptr, /* no data changes, but editable just for testing */
    [](QUaLog * log) {
        Q_UNUSED(log);
        return true;
    });

    // log message editable just for testing
    ui->tableViewLogs->setColumnEditor(3, 
    [](QWidget *parent, QUaLog * log) {
        Q_UNUSED(log);
        return new QLineEdit(parent);
    }, 
    [](QWidget* w, QUaLog * log) {
        auto le = qobject_cast<QLineEdit*>(w);
        le->setText(log->message);
    },
    [](QWidget *w, QUaLog * log) {
        auto le = qobject_cast<QLineEdit*>(w);
        log->message = le->text().toUtf8();
    });

    // support delete and copy
    ui->tableViewLogs->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLogs->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableViewLogs->setDeleteCallback(
    [this](QList<QUaLog*> &logs) {
        while (logs.count() > 0)
        {
            m_modelLog.removeNode(logs.takeFirst());
        }
    });
    ui->tableViewLogs->setCopyCallback(
    [](const QList<QUaLog*> &logs) {
        auto mime = new QMimeData();
        for (auto log : logs)
        {
            mime->setText(
                mime->text() + QString("[%1] [%2] [%3] : %4.\n")
                .arg(log->timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
                .arg(logLevelMetaEnum.valueToKey(static_cast<int>(log->level)))
                .arg(logCategoryMetaEnum.valueToKey(static_cast<int>(log->category)))
                .arg(QString(log->message))
            );
        }
        return mime;
    });

    // allow sorting
    m_proxyLog.setSourceModel(&m_modelLog);
    ui->tableViewLogs->setModel(&m_proxyLog);
    ui->tableViewLogs->setSortingEnabled(true);
    ui->tableViewLogs->sortByColumn(0, Qt::DescendingOrder);
}

void Dialog::setupSessionTable()
{
    QObject::connect(&m_server, &QUaServer::clientConnected, &m_modelSession,
    [this](const QUaSession* session) {
        m_modelSession.addNode(session);
    });
    QObject::connect(&m_server, &QUaServer::clientDisconnected, &m_modelSession,
    [this](const QUaSession* session) {
        m_modelSession.removeNode(session);
    });
    // setup model column data sources
    m_modelSession.setColumnDataSource(0, tr("Timestamp"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->timestamp().toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz");
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(1, tr("Id"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->sessionId();
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(2, tr("Name"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->applicationName();
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(3, tr("Address"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->address();
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(4, tr("Port"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->port();
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelSession.setColumnDataSource(5, tr("Username"),
    [](const QUaSession* session, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return session->userName();
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);

    // support copy
    ui->tableViewSessions->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewSessions->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableViewSessions->setCopyCallback(
    [](const QList<const QUaSession*> &sessions) {
        auto mime = new QMimeData();
        for (auto session : sessions)
        {
            mime->setText(
                mime->text() + QString("%1, %2, %3, %4, %5, %6.\n")
                .arg(session->timestamp().toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
                .arg(session->sessionId())
                .arg(session->applicationName())
                .arg(session->address())
                .arg(session->port())
                .arg(session->userName())
            );
        }
        return mime;
    });

    // allow sorting
    m_proxySession.setSourceModel(&m_modelSession);
    ui->tableViewSessions->setModel(&m_proxySession);
    ui->tableViewSessions->setSortingEnabled(true);
    ui->tableViewSessions->sortByColumn(0, Qt::AscendingOrder);
}

void Dialog::on_pushButtonClearLog_clicked()
{
    m_modelLog.clear();
}
