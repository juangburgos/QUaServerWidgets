#include "quaserverwidget.h"
#include "ui_quaserverwidget.h"

QUaServerWidget::QUaServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QUaServerWidget)
{
    ui->setupUi(this);
}

QUaServerWidget::~QUaServerWidget()
{
    delete ui;
}
