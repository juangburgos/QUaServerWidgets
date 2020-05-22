#ifndef QUALOGWIDGET_H
#define QUALOGWIDGET_H

#include <QWidget>

#include <QUaServer>
#include <QUaTableModel>
#include <QUaTreeView>

#include <QUaCommonDialog>

namespace Ui {
class QUaLogWidget;
}

typedef QUaTableModel<QUaLog> QUaLogTableModel;
typedef QUaTreeView  <QUaLog> QUaLogTableView;

class QUaLogWidgetSettings;

class QUaLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaLogWidget(QWidget *parent = nullptr);
    ~QUaLogWidget();

    enum class Columns {
        Timestamp = 0,
        Level     = 1,
        Category  = 2,
        Message   = 3,
        Invalid
    };
    Q_ENUM(Columns)

    enum class LogLevelFilter {
        All      = static_cast<int>(QUaLogLevel::Trace) - 1,
		Trace    = static_cast<int>(QUaLogLevel::Trace  ),
		Debug    = static_cast<int>(QUaLogLevel::Debug  ),
		Info     = static_cast<int>(QUaLogLevel::Info   ),
		Warning  = static_cast<int>(QUaLogLevel::Warning),
		Error    = static_cast<int>(QUaLogLevel::Error  ),
		Fatal    = static_cast<int>(QUaLogLevel::Fatal  ),
        Invalid
	};
	Q_ENUM(LogLevelFilter)

    enum class LogCategoryFilter {
        All            = static_cast<int>(QUaLogCategory::Network) - 1,
		Network        = static_cast<int>(QUaLogCategory::Network       ),
		SecureChannel  = static_cast<int>(QUaLogCategory::SecureChannel ),
		Session        = static_cast<int>(QUaLogCategory::Session       ),
		Server         = static_cast<int>(QUaLogCategory::Server        ),
		Client         = static_cast<int>(QUaLogCategory::Client        ),
		UserLand       = static_cast<int>(QUaLogCategory::UserLand      ),
		SecurityPolicy = static_cast<int>(QUaLogCategory::SecurityPolicy),
		Serialization  = static_cast<int>(QUaLogCategory::Serialization ),
		History        = static_cast<int>(QUaLogCategory::History       ),
		Application    = static_cast<int>(QUaLogCategory::Application   ),
        Invalid
	};
    Q_ENUM(LogCategoryFilter)

    quint32 maxEntries() const;
    void setMaxEntries(const quint32& maxEntries);

    QString timeFormat() const;
    void setTimeFormat(const QString& strTimeFormat);

    bool isColumnVisible(const QUaLogWidget::Columns& column) const;
    void setColumnVisible(const QUaLogWidget::Columns& column, const bool &visible);

    QByteArray highlightMessageIfContains() const;
    void setHighlightMessageIfContains(const QByteArray& text);

    QBrush levelColor(const QUaLogLevel &level) const;
    void setLevelColor(const QUaLogLevel& level, const QBrush& color);

public slots:
    void addLog(const QUaLog& log);

private slots:
    void on_pushButtonClear_clicked();

    void on_pushButtonExportCsv_clicked();

    void on_checkBoxFilter_toggled(bool checked);

    void on_pushButtonSettings_clicked();

private:
    Ui::QUaLogWidget *ui;
    quint32 m_maxEntries;
    QString m_timeFormat;
    QByteArray m_byteHighlight;

    QUaLogTableModel     m_modelLogs;
    QUaLambdaFilterProxy m_proxyLogs;

    QMultiMap<QDateTime, QUaLog*> m_logsByDate;
    QSet<QUaLog*> m_logsToHighlight;

    QHash<QUaLogLevel, bool> m_logsToLogByLevel;
    QHash<QUaLogCategory, bool> m_logsToLogByCategory;
    QHash<QUaLogLevel, QBrush> m_logsToPaintByLevel;

    static QMetaEnum m_logLevelMetaEnum;
    static QMetaEnum m_logCategoryMetaEnum;
    static QMetaEnum m_columnsMetaEnum;
    static QMetaEnum m_logLevelFilterMetaEnum;
    static QMetaEnum m_logCategoryFilterMetaEnum;


    friend QUaLogWidgetSettings;

    friend QMetaObject::Connection
        QUaModelItemTraits::DestroyCallback<QUaLog>(
            QUaLog* entry,
            const std::function<void(void)>& callback);

    void setupTable();
    void enforceMaxEntries();
    void purgeLogs();

    bool isFilterVisible() const;
    void setFilterVisible(const bool& isVisible);

    void setupFilterWidgets();

    void showSettingsDialog(QUaCommonDialog& dialog);
};

#endif // QUALOGWIDGET_H
