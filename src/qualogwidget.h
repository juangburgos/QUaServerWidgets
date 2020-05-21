#ifndef QUALOGWIDGET_H
#define QUALOGWIDGET_H

#include <QWidget>

#include <QUaServer>
#include <QUaTableModel>
#include <QUaTreeView>
#include <QSortFilterProxyModel>

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

    quint32 maxEntries() const;
    void setMaxEntries(const quint32& maxEntries);

public slots:
    void addLog(const QUaLog& log);

private slots:
    void on_pushButtonClear_clicked();

    void on_pushButtonExportCsv_clicked();

    void on_checkBoxFilter_toggled(bool checked);

private:
    Ui::QUaLogWidget *ui;
    quint32 m_maxEntries;
    static QMetaEnum m_logLevelMetaEnum;
    static QMetaEnum m_logCategoryMetaEnum;

    QUaLogTableModel      m_modelLogs;
    QSortFilterProxyModel m_proxyLogs;
    QMultiMap<QDateTime, QUaLog*> m_logsByDate;

    friend QMetaObject::Connection
        QUaModelItemTraits::DestroyCallback<QUaLog>(
            QUaLog* entry,
            const std::function<void(void)>& callback);

    void setupTable();
    void enforceMaxEntries();
};

#endif // QUALOGWIDGET_H
