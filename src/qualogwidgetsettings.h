#ifndef QUALOGWIDGETSETTINGS_H
#define QUALOGWIDGETSETTINGS_H

#include <QWidget>

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
};

#endif // QUALOGWIDGETSETTINGS_H
