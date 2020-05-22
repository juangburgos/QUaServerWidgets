#include "qualogwidget.h"
#include "ui_qualogwidget.h"

#include "qualogwidgetsettings.h"

QMetaEnum QUaLogWidget::m_logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum QUaLogWidget::m_logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();

QMetaEnum QUaLogWidget::m_logLevelFilterMetaEnum   ; // = QMetaEnum::fromType<QUaLogWidget::LogLevelFilter>();
QMetaEnum QUaLogWidget::m_logCategoryFilterMetaEnum; // = QMetaEnum::fromType<QUaLogWidget::LogCategoryFilter>();

QUaLogWidget::QUaLogWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaLogWidget)
{
    ui->setupUi(this);
    // defaults
    m_maxEntries = 1000;
    // NOTE : static definition crashes?
    m_logLevelFilterMetaEnum    = QMetaEnum::fromType<LogLevelFilter>();
    m_logCategoryFilterMetaEnum = QMetaEnum::fromType<LogCategoryFilter>();
    // init ui
    this->setupTable();
    this->setupFilterWidgets();
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
    // setup filtering
    m_proxyLogs.setFilterAcceptsRow(
    [this](int sourceRow, const QModelIndex& sourceParent) {
        auto index = m_modelLogs.index(sourceRow, 0, sourceParent);
        if (!index.isValid())
        {
            return true;
        }
        if (!this->isFilterVisible())
        {
            return true;
        }
        auto log = m_modelLogs.nodeFromIndex(index);
        Q_ASSERT(log);
        LogLevelFilter    levelFilter = ui->comboBoxFilterLevel->currentData().value<LogLevelFilter   >();
        LogCategoryFilter categFilter = ui->comboBoxFilterCateg->currentData().value<LogCategoryFilter>();
        QByteArray strMessageFilter = ui->lineEditFilterText->text().toUtf8();
        bool showLevel = levelFilter == LogLevelFilter::AnyLevel || log->level == static_cast<QUaLogLevel>(levelFilter);
        bool showCateg = categFilter == LogCategoryFilter::AnyCategory || log->category == static_cast<QUaLogCategory>(categFilter);
        bool showMessage = strMessageFilter.isEmpty() ? true : log->message.contains(strMessageFilter);
        return showLevel && showCateg && showMessage;
    });
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
    this->setFilterVisible(checked);
}

void QUaLogWidget::on_pushButtonSettings_clicked()
{
    // setup widget
    QUaLogWidgetSettings* widgetSettings = new QUaLogWidgetSettings;
    widgetSettings->readSettings(*this);
    // NOTE : dialog takes ownershit
    QUaCommonDialog dialog(this);
    dialog.setWindowTitle(tr("Settings"));
    dialog.setWidget(widgetSettings);
    // NOTE : call in own method to we can recall it if fails
    this->showSettingsDialog(dialog);
}

bool QUaLogWidget::isFilterVisible() const
{
    return ui->frameFilterOptions->isEnabled();
}

void QUaLogWidget::setFilterVisible(const bool& isVisible)
{
    ui->frameFilterOptions->setEnabled(isVisible);
    ui->frameFilterOptions->setVisible(isVisible);
    if (!isVisible)
    {
        // reset to show all
        int indexAny = ui->comboBoxFilterLevel->findData(QVariant::fromValue(LogLevelFilter::AnyLevel));
        Q_ASSERT(indexAny >= 0); // NOTE : combo index is not enum value
        ui->comboBoxFilterLevel->setCurrentIndex(indexAny);
        indexAny = ui->comboBoxFilterCateg->findData(QVariant::fromValue(LogCategoryFilter::AnyCategory));
        Q_ASSERT(indexAny >= 0); // NOTE : combo index is not enum value
        ui->comboBoxFilterCateg->setCurrentIndex(indexAny);
        // clear filter text
        ui->lineEditFilterText->setText("");
        // refilter
        m_proxyLogs.forceReFilter();
    }
}

void QUaLogWidget::setupFilterWidgets()
{
    // setup combobox level
    for (int i = static_cast<int>(LogLevelFilter::AnyLevel); i < static_cast<int>(LogLevelFilter::Invalid); i++)
    {
        ui->comboBoxFilterLevel->addItem(
            QUaLogWidget::m_logLevelFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<LogLevelFilter>(i))
        );
    }
    // setup combobox category
    for (int i = static_cast<int>(LogCategoryFilter::AnyCategory); i < static_cast<int>(LogCategoryFilter::Invalid); i++)
    {
        ui->comboBoxFilterCateg->addItem(
            QUaLogWidget::m_logCategoryFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<LogCategoryFilter>(i))
        );
    }
    // set default values (initially hidden)
    this->setFilterVisible(false);
    // force refilter when filter changed
    QObject::connect(ui->comboBoxFilterLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this]() {
            m_proxyLogs.forceReFilter();
        });
    QObject::connect(ui->comboBoxFilterCateg, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this]() {
            m_proxyLogs.forceReFilter();
        });
    QObject::connect(ui->lineEditFilterText, &QLineEdit::textChanged, this, 
        [this]() {
            m_proxyLogs.forceReFilter();
        });
}

void QUaLogWidget::showSettingsDialog(QUaCommonDialog& dialog)
{
    int res = dialog.exec();
    if (res != QDialog::Accepted)
    {
        return;
    }
    // get widget
    auto widgetSettings = qobject_cast<QUaLogWidgetSettings*>(dialog.widget());
    Q_CHECK_PTR(widgetSettings);
    widgetSettings->writeSettings(*this);
}