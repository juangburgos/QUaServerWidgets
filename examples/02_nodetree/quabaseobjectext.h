#ifndef QUABASEOBJECTEXT_H
#define QUABASEOBJECTEXT_H

#include <QUaBaseObject>
#include <QTimer>

class QUaBaseObjectExt : public QUaBaseObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QUaBaseObjectExt(QUaServer *server);

    Q_INVOKABLE void addObjectExtChild(QString strName);

    Q_INVOKABLE void addMulitpleObjectExtChild(QString strBaseName, quint32 numChildren);

    Q_INVOKABLE void remove();

    // built in types

    Q_INVOKABLE void addFolderChild(QString strName);

    Q_INVOKABLE void addBaseObjectChild(QString strName);

    Q_INVOKABLE void addBaseDataVariableChild(QString strName);

    Q_INVOKABLE void addMultipleBaseDataVariableChild(QString strBaseName, quint32 numChildren);

    Q_INVOKABLE void addPropertyChild(QString strName);

private:
    static QTimer m_timer;
};

#endif // QUABASEOBJECTEXT_H
