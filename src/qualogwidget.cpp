#include "qualogwidget.h"
#include "ui_qualogwidget.h"

QMetaEnum QUaLogWidget::m_logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum QUaLogWidget::m_logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();

QUaLogWidget::QUaLogWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaLogWidget)
{
    ui->setupUi(this);
    m_maxEntries = 1000;
    this->setupTable();
}

QUaLogWidget::~QUaLogWidget()
{
    delete ui;
}

quint32 QUaLogWidget::maxEntries() const
{
    return m_maxEntries;
}

void QUaLogWidget::setMaxEntries(const quint32& maxEntries)
{
    m_maxEntries = maxEntries;
    this->enforceMaxEntries();
}

void QUaLogWidget::addLog(const QUaLog& log)
{
    m_modelLogs.addNode(log);
}

void QUaLogWidget::setupTable()
{
    // keep map indexed by time to delete oldest if necessary
    m_modelLogs.connectNodeAddedCallback(this, 
    [this](QUaLog * log, const QModelIndex &index) {
        Q_UNUSED(index);
        m_logsByDate.insert(log->timestamp, log);
        Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
        this->enforceMaxEntries();
        Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
    });
    // setup model column data sources
    m_modelLogs.setColumnDataSource(0, tr("Timestamp"),
    [](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
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
			return QUaLogWidget::m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level));
		}
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    m_modelLogs.setColumnDataSource(2, tr("Category"),
    [](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return QUaLogWidget::m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category));
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
            auto log = logs.takeFirst();
            Q_ASSERT(m_logsByDate.contains(log->timestamp, log));
            m_logsByDate.remove(log->timestamp, log);
            m_modelLogs.removeNode(log);
        }
        Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
    });
    ui->treeViewLog->setCopyCallback(
    [](const QList<QUaLog*> & logs) {
        auto mime = new QMimeData();
        for (auto log : logs)
        {
            mime->setText(
                mime->text() + QString("[%1] [%2] [%3] : %4.\n")
                .arg(log->timestamp.toLocalTime().toString("dd.MM.yyyy hh:mm:ss.zzz"))
                .arg(QUaLogWidget::m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level)))
                .arg(QUaLogWidget::m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category)))
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

void QUaLogWidget::enforceMaxEntries()
{
    while (static_cast<quint32>(m_logsByDate.size()) > m_maxEntries)
    {
        auto begin = m_logsByDate.begin();
        m_modelLogs.removeNode(begin.value());
        m_logsByDate.erase(begin);
    }
    Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
}

void QUaLogWidget::on_pushButtonClear_clicked()
{
    m_logsByDate.clear();
    m_modelLogs.clear();
    Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
}

void QUaLogWidget::on_pushButtonExportCsv_clicked()
{

}

void QUaLogWidget::on_checkBoxFilter_toggled(bool checked)
{

}
