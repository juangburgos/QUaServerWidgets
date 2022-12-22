#ifndef QUASERVERWIDGET_H
#define QUASERVERWIDGET_H

#include <QWidget>

#include <QUaServer>

namespace Ui {
class QUaServerWidget;
}

class QUaServerWidget : public QWidget
{
    Q_OBJECT
    // expose to style
    Q_PROPERTY(bool readOnly   READ readOnly   WRITE setReadOnly   NOTIFY readOnlyChanged)
    Q_PROPERTY(bool allowStart READ allowStart WRITE setAllowStart NOTIFY allowStartChanged)
public:
    explicit QUaServerWidget(QWidget *parent = nullptr);
    ~QUaServerWidget();

    void bindServer(QUaServer* server);

    void clear();

    bool readOnly() const;
    void setReadOnly(const bool& readOnly);

    bool allowStart() const;
    void setAllowStart(const bool& allowStart);

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

signals:
    void readOnlyChanged();
    void allowStartChanged();

private slots:
    void on_pushButtonClearCertificate_clicked();

    void on_pushButtonLoadCertificate_clicked();

#ifdef UA_ENABLE_ENCRYPTION
    void on_pushButtonClearPrivateKey_clicked();

    void on_pushButtonLoadPrivateKey_clicked();
#endif

private:
    Ui::QUaServerWidget *ui;
    bool m_readOnly;
    bool m_allowStart;
    QList<QMetaObject::Connection> m_connections;

    QByteArray m_byteLastCertificate;
#ifdef UA_ENABLE_ENCRYPTION
    QByteArray m_byteLastPrivatekey;
#endif

    static QByteArray readFileData(const QString &strFileName);
};

#endif // QUASERVERWIDGET_H
