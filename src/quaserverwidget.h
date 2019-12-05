#ifndef QUASERVERWIDGET_H
#define QUASERVERWIDGET_H

#include <QWidget>

namespace Ui {
class QUaServerWidget;
}

class QUaServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaServerWidget(QWidget *parent = nullptr);
    ~QUaServerWidget();

private:
    Ui::QUaServerWidget *ui;
};

#endif // QUASERVERWIDGET_H
