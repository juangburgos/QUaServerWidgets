#ifndef QUABASEOBJECTEXT_H
#define QUABASEOBJECTEXT_H

#include <QUaBaseObject>

class QUaBaseObjectExt : public QUaBaseObject
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QUaBaseObjectExt(QUaServer *server);

    Q_INVOKABLE void addObjectExtChild(QString strName);

    Q_INVOKABLE void remove();

    // built in types

    Q_INVOKABLE void addFolderChild(QString strName);

    Q_INVOKABLE void addBaseObjectChild(QString strName);

    Q_INVOKABLE void addBaseDataVariableChild(QString strName);

    Q_INVOKABLE void addPropertyChild(QString strName);

};

#endif // QUABASEOBJECTEXT_H
