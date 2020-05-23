#include "qualogwidget.h"
#include "ui_qualogwidget.h"

#include "qualogwidgetsettings.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

QMetaEnum QUaLogWidget::m_logLevelMetaEnum    = QMetaEnum::fromType<QUaLogLevel>();
QMetaEnum QUaLogWidget::m_logCategoryMetaEnum = QMetaEnum::fromType<QUaLogCategory>();

QMetaEnum QUaLogWidget::m_columnsMetaEnum;
QMetaEnum QUaLogWidget::m_logLevelFilterMetaEnum   ;
QMetaEnum QUaLogWidget::m_logCategoryFilterMetaEnum;

QUaLogWidget::QUaLogWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaLogWidget)
{
    ui->setupUi(this);
    // defaults
    m_maxEntries = 1000;
    m_strCsvSeparator = ", ";
    m_timeFormat = "dd.MM.yyyy hh:mm:ss.zzz";
    m_strCsvFormat = QString("%1%5%2%5%3%5%4\n");
    // NOTE : static definition crashes?
    m_columnsMetaEnum           = QMetaEnum::fromType<Columns>();
    m_logLevelFilterMetaEnum    = QMetaEnum::fromType<LogLevelFilter>();
    m_logCategoryFilterMetaEnum = QMetaEnum::fromType<LogCategoryFilter>();
    // init ui
    for (int i = static_cast<int>(LogLevelFilter::All) + 1; i < static_cast<int>(LogLevelFilter::Invalid); i++)
    {
        // log all levels by default
        m_logsToLogByLevel[static_cast<QUaLogLevel>(i)] = true;
        m_logsToPaintByLevel[static_cast<QUaLogLevel>(i)] = QBrush(Qt::black);
    }
    for (int i = static_cast<int>(LogCategoryFilter::All) + 1; i < static_cast<int>(LogCategoryFilter::Invalid); i++)
    {
        // log all categories by default
        m_logsToLogByCategory[static_cast<QUaLogCategory>(i)] = true;
    }
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

QString QUaLogWidget::csvSeparator() const
{
    return m_strCsvSeparator;
}

void QUaLogWidget::setCsvSeparator(const QString& csvSeparator)
{
    m_strCsvSeparator = csvSeparator;
}

QString QUaLogWidget::timeFormat() const
{
    return m_timeFormat;
}

void QUaLogWidget::setTimeFormat(const QString& strTimeFormat)
{
    m_timeFormat = strTimeFormat;
}

bool QUaLogWidget::isColumnVisible(const QUaLogWidget::Columns& column) const
{
    return !ui->treeViewLog->isColumnHidden(static_cast<int>(column));
}

void QUaLogWidget::setColumnVisible(const QUaLogWidget::Columns& column, const bool& visible)
{
    ui->treeViewLog->setColumnHidden(static_cast<int>(column), !visible);
    
    if (column == Columns::Level)
    {
        ui->comboBoxFilterLevel->setVisible(visible);
        if (!visible)
        {
            // release filter
            int indexAny = ui->comboBoxFilterLevel->findData(QVariant::fromValue(LogLevelFilter::All));
            Q_ASSERT(indexAny >= 0); // NOTE : combo index is not enum value
            ui->comboBoxFilterLevel->setCurrentIndex(indexAny);         
        }
    }
    if (column == Columns::Category)
    {
        ui->comboBoxFilterCateg->setVisible(visible);
        if (!visible)
        {
            // release filter
            int indexAny = ui->comboBoxFilterCateg->findData(QVariant::fromValue(LogCategoryFilter::All));
            Q_ASSERT(indexAny >= 0); // NOTE : combo index is not enum value
            ui->comboBoxFilterCateg->setCurrentIndex(indexAny);
        }
    }
    if (column == Columns::Message)
    {
        ui->lineEditFilterText->setVisible(visible);
        if (!visible)
        {
            ui->lineEditFilterText->clear();
        }
    }
}

QByteArray QUaLogWidget::highlightMessageIfContains() const
{
    return m_byteHighlight;
}

void QUaLogWidget::setHighlightMessageIfContains(const QByteArray& text)
{
    m_byteHighlight = text;
}

QBrush QUaLogWidget::levelColor(const QUaLogLevel& level) const
{
    return m_logsToPaintByLevel[level];
}

void QUaLogWidget::setLevelColor(const QUaLogLevel& level, const QBrush& color)
{
    m_logsToPaintByLevel[level] = color;
}

bool QUaLogWidget::isFilterVisible() const
{
    return ui->frameFilter->isEnabled();
}

void QUaLogWidget::setFilterVisible(const bool& visible)
{
    ui->frameFilter->setVisible(visible);
    ui->frameFilter->setEnabled(visible);
    ui->labelSpacer->setSizePolicy(
        visible ? QSizePolicy::Minimum : QSizePolicy::Expanding,
        QSizePolicy::Minimum
    );
    this->updateSpacerLabelVisible();
}

bool QUaLogWidget::isSettingsVisible() const
{
    return ui->pushButtonSettings->isEnabled();
}

void QUaLogWidget::setSettingsVisible(const bool& visible)
{
    ui->pushButtonSettings->setVisible(visible);
    ui->pushButtonSettings->setEnabled(visible);
    this->updateSpacerLabelVisible();
}

bool QUaLogWidget::isExportCsvVisible() const
{
    return ui->pushButtonExportCsv->isEnabled();
}

void QUaLogWidget::setExportCsvVisible(const bool& visible)
{
    ui->pushButtonExportCsv->setVisible(visible);
    ui->pushButtonExportCsv->setEnabled(visible);
    this->updateSpacerLabelVisible();
}

bool QUaLogWidget::isClearVisible() const
{
    return ui->pushButtonClear->isEnabled();
}

void QUaLogWidget::setClearVisible(const bool& visible)
{
    ui->pushButtonClear->setVisible(visible);
    ui->pushButtonClear->setEnabled(visible);
    this->updateSpacerLabelVisible();
}

void QUaLogWidget::updateSpacerLabelVisible()
{
    bool visible =
        this->isFilterVisible() ||
        this->isSettingsVisible() ||
        this->isExportCsvVisible() ||
        this->isClearVisible();
    ui->labelSpacer->setVisible(visible);
}

void QUaLogWidget::addLog(const QUaLog& log)
{
    // check if needs to be logged first
    if (!m_logsToLogByLevel[log.level])
    {
        return;
    }
    if (!m_logsToLogByCategory[log.category])
    {
        return;
    }
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
    int timestampColumn = static_cast<int>(Columns::Timestamp);
    m_modelLogs.setColumnDataSource(timestampColumn, QUaLogWidget::m_columnsMetaEnum.valueToKey(timestampColumn),
    [this](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return log->timestamp.toLocalTime().toString(m_timeFormat);
		}
        if (role == Qt::FontRole && m_logsToHighlight.contains(log))
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        if (role == Qt::ForegroundRole)
        {
            return m_logsToPaintByLevel[log->level];
        }
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    int levelColumn = static_cast<int>(Columns::Level);
    m_modelLogs.setColumnDataSource(levelColumn, QUaLogWidget::m_columnsMetaEnum.valueToKey(levelColumn),
    [this](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return QUaLogWidget::m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level));
		}
        if (role == Qt::FontRole && m_logsToHighlight.contains(log))
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        if (role == Qt::ForegroundRole)
        {
            return m_logsToPaintByLevel[log->level];
        }
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    int categoryColumn = static_cast<int>(Columns::Category);
    m_modelLogs.setColumnDataSource(categoryColumn, QUaLogWidget::m_columnsMetaEnum.valueToKey(categoryColumn),
    [this](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return QUaLogWidget::m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category));
		}
        if (role == Qt::FontRole && m_logsToHighlight.contains(log))
        {
            QFont font;
            font.setBold(true);
            return font;
        }
        if (role == Qt::ForegroundRole)
        {
            return m_logsToPaintByLevel[log->level];
        }
		return QVariant();
    }/* other callbacks for data that changes or editable */);
    int messageColumn = static_cast<int>(Columns::Message);
    m_modelLogs.setColumnDataSource(messageColumn, QUaLogWidget::m_columnsMetaEnum.valueToKey(messageColumn),
    [this](QUaLog* log, const Qt::ItemDataRole& role) -> QVariant {
		if (role == Qt::DisplayRole)
		{
			return log->message;
		}
        if (role == Qt::FontRole)
        {
            QFont font;
            if (!m_byteHighlight.isEmpty() && log->message.contains(m_byteHighlight))
            {
                font.setBold(true);
                m_logsToHighlight << log;
            }
            else
            {
                m_logsToHighlight.remove(log);
            }
            return font;
        }
        if (role == Qt::ForegroundRole)
        {
            return m_logsToPaintByLevel[log->level];
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
            m_logsToHighlight.remove(log);
            Q_ASSERT(m_logsByDate.contains(log->timestamp, log));
            m_logsByDate.remove(log->timestamp, log);
            m_modelLogs.removeNode(log);
        }
        Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
    });
    ui->treeViewLog->setCopyCallback(
    [this](const QList<QUaLog*> & logs) {
        auto mime = new QMimeData();
        for (auto log : logs)
        {
            mime->setText(
                mime->text() + m_strCsvFormat
                .arg(log->timestamp.toLocalTime().toString(m_timeFormat))
                .arg(QUaLogWidget::m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level)))
                .arg(QUaLogWidget::m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category)))
                .arg(QString(log->message))
                .arg(m_strCsvSeparator)
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
        if (!this->isFilterShown())
        {
            return true;
        }
        auto log = m_modelLogs.nodeFromIndex(index);
        Q_ASSERT(log);
        LogLevelFilter    levelFilter = ui->comboBoxFilterLevel->currentData().value<LogLevelFilter   >();
        LogCategoryFilter categFilter = ui->comboBoxFilterCateg->currentData().value<LogCategoryFilter>();
        QByteArray strMessageFilter = ui->lineEditFilterText->text().toUtf8();
        bool showLevel = levelFilter == LogLevelFilter::All || log->level == static_cast<QUaLogLevel>(levelFilter);
        bool showCateg = categFilter == LogCategoryFilter::All || log->category == static_cast<QUaLogCategory>(categFilter);
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
        m_logsToHighlight.remove(begin.value());
        m_logsByDate.erase(begin);
    }
    Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
}

void QUaLogWidget::purgeLogs()
{
    auto iter = m_logsByDate.begin();
    while(iter != m_logsByDate.end())
    {
        QDateTime timestamp = iter.key();
        auto log = iter.value();
        iter++;
        if (m_logsToLogByLevel[log->level] && m_logsToLogByCategory[log->category])
        {
            continue;
        }
        m_modelLogs.removeNode(log);
        m_logsToHighlight.remove(log);
        m_logsByDate.remove(timestamp, log);
    };
}

void QUaLogWidget::on_pushButtonClear_clicked()
{
    m_logsToHighlight.clear();
    m_logsByDate.clear();
    m_modelLogs.clear();
    Q_ASSERT(m_modelLogs.count() == m_logsByDate.size());
}

void QUaLogWidget::on_pushButtonExportCsv_clicked()
{
    // select file
    QString strSaveFile = QFileDialog::getSaveFileName(this, tr("Save File"),
            m_strLastPathUsed.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) : m_strLastPathUsed,
            tr("CSV (*.csv *.txt)"));
    // ignore if empty
    if (strSaveFile.isEmpty() || strSaveFile.isNull())
    {
        return;
    }
    // create CSV
    QString strCsv;
    auto iter = m_logsByDate.begin();
    while (iter != m_logsByDate.end())
    {
        auto log = iter.value();
        iter++;
        strCsv = strCsv + m_strCsvFormat
            .arg(log->timestamp.toLocalTime().toString(m_timeFormat))
            .arg(QUaLogWidget::m_logLevelMetaEnum.valueToKey(static_cast<int>(log->level)))
            .arg(QUaLogWidget::m_logCategoryMetaEnum.valueToKey(static_cast<int>(log->category)))
            .arg(QString(log->message))
            .arg(m_strCsvSeparator);
    };
    // save to file
    QFile file(strSaveFile);
    if (file.open(QIODevice::ReadWrite | QFile::Truncate))
    {
        // save last path used
        m_strLastPathUsed = QFileInfo(file).absoluteFilePath();
        // write
        QTextStream stream(&file);
        stream << strCsv;
    }
    else
    {
        QMessageBox::critical(
            this,
            tr("Error"),
            tr("Error opening file %1 for write operations.").arg(strSaveFile)
        );
    }
    // close file
    file.close();
}

void QUaLogWidget::on_checkBoxFilter_toggled(bool checked)
{
    this->setFilterShown(checked);
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

bool QUaLogWidget::isFilterShown() const
{
    return ui->frameFilterOptions->isEnabled();
}

void QUaLogWidget::setFilterShown(const bool& isShown)
{
    ui->frameFilterOptions->setEnabled(isShown);
    ui->frameFilterOptions->setVisible(isShown);
    if (!isShown)
    {
        // reset to show all
        int indexAny = ui->comboBoxFilterLevel->findData(QVariant::fromValue(LogLevelFilter::All));
        Q_ASSERT(indexAny >= 0); // NOTE : combo index is not enum value
        ui->comboBoxFilterLevel->setCurrentIndex(indexAny);
        indexAny = ui->comboBoxFilterCateg->findData(QVariant::fromValue(LogCategoryFilter::All));
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
    for (int i = static_cast<int>(LogLevelFilter::All); i < static_cast<int>(LogLevelFilter::Invalid); i++)
    {
        ui->comboBoxFilterLevel->addItem(
            QUaLogWidget::m_logLevelFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<LogLevelFilter>(i))
        );
    }
    // setup combobox category
    for (int i = static_cast<int>(LogCategoryFilter::All); i < static_cast<int>(LogCategoryFilter::Invalid); i++)
    {
        ui->comboBoxFilterCateg->addItem(
            QUaLogWidget::m_logCategoryFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<LogCategoryFilter>(i))
        );
    }
    // set default values (initially hidden)
    this->setFilterShown(false);
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