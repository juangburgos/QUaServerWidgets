#include "qualogwidgetsettings.h"
#include "ui_qualogwidgetsettings.h"

#include <QUaLogWidget>

QUaLogWidgetSettings::QUaLogWidgetSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaLogWidgetSettings)
{
    ui->setupUi(this);
}

QUaLogWidgetSettings::~QUaLogWidgetSettings()
{
    delete ui;
}

void QUaLogWidgetSettings::readSettings(const QUaLogWidget& logWidget)
{
    // general
    ui->spinBoxMaxEntries->setValue(logWidget.maxEntries());


}

void QUaLogWidgetSettings::writeSettings(QUaLogWidget& logWidget) const
{
    // general
    logWidget.setMaxEntries(ui->spinBoxMaxEntries->value());

}
