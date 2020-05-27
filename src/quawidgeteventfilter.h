#ifndef QUAWIDGETEVENTFILTER_H
#define QUAWIDGETEVENTFILTER_H

#include <QWidget>
#include <QEvent>
#include <QSet>

class QUaWidgetEventFilter : public QObject
{
    Q_OBJECT
public:
    inline explicit QUaWidgetEventFilter(QWidget* parent) :
        QObject(parent)
    {
        parent->installEventFilter(this);
    };

    template<typename M>
    inline QMetaObject::Connection
        installEventCallback(const QEvent::Type& handleType, const M& callback)
    {
        m_handledTypes << handleType;
        return QObject::connect(this, &QUaWidgetEventFilter::handleCallback, this->parent(),
            [handleType, callback](const QEvent::Type& emitType) {
                if (emitType != handleType)
                {
                    return;
                }
                callback();
            });
    }

signals:
    void handleCallback(const QEvent::Type& type, QPrivateSignal);

protected:
    QSet<QEvent::Type> m_handledTypes;
    inline bool eventFilter(QObject* obj, QEvent* event) override
    {
        if (m_handledTypes.contains(event->type()))
        {
            emit this->handleCallback(event->type(), QPrivateSignal());
            return true;
        }
        else {
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
    };
};

#endif // QUAWIDGETEVENTFILTER_H