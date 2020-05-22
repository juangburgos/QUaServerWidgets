#ifndef QUALOGWIDGETSETTINGS_H
#define QUALOGWIDGETSETTINGS_H

#include <QWidget>
#include <QUaServer>

class QUaLogWidget;

namespace Ui {
class QUaLogWidgetSettings;
}

class QUaLogWidgetSettings : public QWidget
{
    Q_OBJECT

public:
    explicit QUaLogWidgetSettings(QWidget *parent = nullptr);
    ~QUaLogWidgetSettings();

    void readSettings(const QUaLogWidget& logWidget);
    void writeSettings(QUaLogWidget& logWidget) const;

private:
    Ui::QUaLogWidgetSettings *ui;
    QHash<QUaLogLevel, bool> m_logsToLogByLevel;
    QHash<QUaLogCategory, bool> m_logsToLogByCategory;
    QHash<QUaLogLevel, QBrush> m_logsToPaintByLevel;
};

#endif // QUALOGWIDGETSETTINGS_H
