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

class QUaLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaLogWidget(QWidget *parent = nullptr);
    ~QUaLogWidget();

    enum class LogLevelFilter {
        AnyLevel = static_cast<int>(QUaLogLevel::Trace) - 1,
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
        AnyCategory    = static_cast<int>(QUaLogCategory::Network) - 1,
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
    static QMetaEnum m_logLevelMetaEnum;
    static QMetaEnum m_logCategoryMetaEnum;
    static QMetaEnum m_logLevelFilterMetaEnum;
    static QMetaEnum m_logCategoryFilterMetaEnum;

    QUaLogTableModel     m_modelLogs;
    QUaLambdaFilterProxy m_proxyLogs;
    QMultiMap<QDateTime, QUaLog*> m_logsByDate;

    friend QMetaObject::Connection
        QUaModelItemTraits::DestroyCallback<QUaLog>(
            QUaLog* entry,
            const std::function<void(void)>& callback);

    void setupTable();
    void enforceMaxEntries();

    bool isFilterVisible() const;
    void setFilterVisible(const bool& isVisible);

    void setupFilterWidgets();

    void showSettingsDialog(QUaCommonDialog& dialog);
};

#endif // QUALOGWIDGET_H
