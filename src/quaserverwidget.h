#ifndef QUASERVERWIDGET_H
#define QUASERVERWIDGET_H

#include <QWidget>

class QUaServer;

namespace Ui {
class QUaServerWidget;
}

class QUaServerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QUaServerWidget(QWidget *parent = nullptr);
    ~QUaServerWidget();

    void bindServer(QUaServer* server);

    void clear();

    // NOTE : methods below only read/write from lineedit without any validation
    //        values are set in server only after apply button.
    //        if QUaServerWidget::bindServer is called after this method,
    //        the lineedit values are overwritten.
    QString certificateFile() const;
    void    setCertificateFile(const QString &strFileName);
#ifdef UA_ENABLE_ENCRYPTION
    QString privateKeyFile() const;
    void    setPrivateKeyFile(const QString &strFileName);
#endif

private slots:
    void on_pushButtonClearCertificate_clicked();

    void on_pushButtonLoadCertificate_clicked();

#ifdef UA_ENABLE_ENCRYPTION
    void on_pushButtonClearPrivateKey_clicked();

    void on_pushButtonLoadPrivateKey_clicked();
#endif

private:
    Ui::QUaServerWidget *ui;
    QList<QMetaObject::Connection> m_connections;

    QByteArray m_byteLastCertificate;
#ifdef UA_ENABLE_ENCRYPTION
    QByteArray m_byteLastPrivatekey;
#endif

    void setReadOnly(const bool &readOnly);

    static QByteArray readFileData(const QString &strFileName);
};

#endif // QUASERVERWIDGET_H
