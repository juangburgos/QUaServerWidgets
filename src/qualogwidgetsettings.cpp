#include "qualogwidgetsettings.h"
#include "ui_qualogwidgetsettings.h"

#include <QUaLogWidget>
#include <QColorDialog>

QUaLogWidgetSettings::QUaLogWidgetSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaLogWidgetSettings)
{
    ui->setupUi(this);
    // setup combobox level
    for (int i = static_cast<int>(QUaLogWidget::LogLevelFilter::All)+1; i < static_cast<int>(QUaLogWidget::LogLevelFilter::Invalid); i++)
    {
        ui->comboBoxLevel->addItem(
            QUaLogWidget::m_logLevelFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<QUaLogWidget::LogLevelFilter>(i))
        );
    }
    // setup combobox category
    for (int i = static_cast<int>(QUaLogWidget::LogCategoryFilter::All)+1; i < static_cast<int>(QUaLogWidget::LogCategoryFilter::Invalid); i++)
    {
        ui->comboBoxCategory->addItem(
            QUaLogWidget::m_logCategoryFilterMetaEnum.valueToKey(i),
            QVariant::fromValue(static_cast<QUaLogWidget::LogCategoryFilter>(i))
        );
    }
    // interactions
    QObject::connect(ui->comboBoxLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this]() {
            QSignalBlocker blocker(ui->checkLevelTypeLog);
            QUaLogLevel level = ui->comboBoxLevel->currentData().value<QUaLogLevel>();
            ui->checkLevelTypeLog->setChecked(m_logsToLogByLevel[level]);
            QPalette palette = ui->labelLevelColorDisplay->palette();
            palette.setColor(ui->labelLevelColorDisplay->backgroundRole(), m_logsToPaintByLevel[level].color());
            ui->labelLevelColorDisplay->setAutoFillBackground(true);
            ui->labelLevelColorDisplay->setPalette(palette);
        });
    QObject::connect(ui->comboBoxCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
        [this]() {
            QSignalBlocker blocker(ui->checkCategoryTypeLog);
            QUaLogCategory categ = ui->comboBoxCategory->currentData().value<QUaLogCategory>();
            ui->checkCategoryTypeLog->setChecked(m_logsToLogByCategory[categ]);
        });
    QObject::connect(ui->checkLevelTypeLog, &QCheckBox::toggled, this,
        [this]() {
            QUaLogLevel level = ui->comboBoxLevel->currentData().value<QUaLogLevel>();
            m_logsToLogByLevel[level] = ui->checkLevelTypeLog->isChecked();;
        });
    QObject::connect(ui->checkCategoryTypeLog, &QCheckBox::toggled, this,
        [this]() {
            QUaLogCategory categ = ui->comboBoxCategory->currentData().value<QUaLogCategory>();
            m_logsToLogByCategory[categ] = ui->checkCategoryTypeLog->isChecked();;
        });
    QObject::connect(ui->pushButtonLevelColor, &QPushButton::clicked, this,
        [this]() {
            QUaLogLevel level = ui->comboBoxLevel->currentData().value<QUaLogLevel>();
            QColorDialog diagColor(this);
            diagColor.setCurrentColor(m_logsToPaintByLevel[level].color());
            // exec color diag
            if (diagColor.exec() == QDialog::Rejected)
            {
                return;
            }
            m_logsToPaintByLevel[level] = QBrush(diagColor.currentColor());
            QPalette palette = ui->labelLevelColorDisplay->palette();
            palette.setColor(ui->labelLevelColorDisplay->backgroundRole(), m_logsToPaintByLevel[level].color());
            ui->labelLevelColorDisplay->setAutoFillBackground(true);
            ui->labelLevelColorDisplay->setPalette(palette);
        });
}

QUaLogWidgetSettings::~QUaLogWidgetSettings()
{
    delete ui;
}

void QUaLogWidgetSettings::readSettings(const QUaLogWidget& logWidget)
{
    // general
    ui->spinBoxMaxEntries->setValue(logWidget.maxEntries());
    // time
    ui->lineEditTimestampFormat->setText(logWidget.timeFormat());
    ui->checkTimestampShow->setChecked(logWidget.isColumnVisible(QUaLogWidget::Columns::Timestamp));
    // level
    ui->checkLevelShow->setChecked(logWidget.isColumnVisible(QUaLogWidget::Columns::Level));
    m_logsToLogByLevel   = logWidget.m_logsToLogByLevel;
    m_logsToPaintByLevel = logWidget.m_logsToPaintByLevel;
    // category
    ui->checkCategoryShow->setChecked(logWidget.isColumnVisible(QUaLogWidget::Columns::Category));
    m_logsToLogByCategory = logWidget.m_logsToLogByCategory;
    // message
    ui->checkMessageShow->setChecked(logWidget.isColumnVisible(QUaLogWidget::Columns::Message));
    ui->lineEditMessageHighlightIf->setText(logWidget.highlightMessageIfContains());
    // force level and categ specifics
    ui->comboBoxLevel->setCurrentIndex(ui->comboBoxLevel->currentIndex() +1);
    ui->comboBoxCategory->setCurrentIndex(ui->comboBoxCategory->currentIndex() +1);
    ui->comboBoxLevel->setCurrentIndex(ui->comboBoxLevel->currentIndex() -1);
    ui->comboBoxCategory->setCurrentIndex(ui->comboBoxCategory->currentIndex() -1);
}

void QUaLogWidgetSettings::writeSettings(QUaLogWidget& logWidget) const
{
    // general
    logWidget.setMaxEntries(ui->spinBoxMaxEntries->value());
    // time
    logWidget.setTimeFormat(ui->lineEditTimestampFormat->text());
    logWidget.setColumnVisible(QUaLogWidget::Columns::Timestamp, ui->checkTimestampShow->isChecked());
    // level
    logWidget.setColumnVisible(QUaLogWidget::Columns::Level, ui->checkLevelShow->isChecked());
    logWidget.m_logsToLogByLevel = m_logsToLogByLevel;
    logWidget.m_logsToPaintByLevel = m_logsToPaintByLevel;
    // category
    logWidget.setColumnVisible(QUaLogWidget::Columns::Category, ui->checkCategoryShow->isChecked());
    logWidget.m_logsToLogByCategory = m_logsToLogByCategory;
    // message
    logWidget.setColumnVisible(QUaLogWidget::Columns::Message, ui->checkMessageShow->isChecked());
    logWidget.setHighlightMessageIfContains(ui->lineEditMessageHighlightIf->text().toUtf8());

    logWidget.purgeLogs();
}
