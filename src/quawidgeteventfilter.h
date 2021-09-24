#ifndef QUAWIDGETEVENTFILTER_H
#define QUAWIDGETEVENTFILTER_H

#include <QWidget>
#include <QEvent>
#include <QSet>

#include <functional>

typedef std::function<bool(const QEvent* event)> QUaWidgetEventFilterCallback;

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
    inline void
        installEventCallback(const QEvent::Type& handleType, const M& callback)
    {
        m_eventTypesCallbacks[handleType] << [callback](const QEvent* event) -> bool {
            return callback(event);
        };
    }

    /*
    // NOTE : gcc does not like
    template<>
    inline void
        installEventCallback(const QEvent::Type& handleType, const QUaWidgetEventFilterCallback& callback)
    {
        m_eventTypesCallbacks[handleType] << callback;
    }
    */

    inline void clearEventCallbacks(const QEvent::Type& handleType)
    {
        m_eventTypesCallbacks.remove(handleType);
    }

Q_SIGNALS:
    void handleCallback(const QEvent::Type& type, QEvent* event, QPrivateSignal);

protected:
    QHash<QEvent::Type, QList<QUaWidgetEventFilterCallback>> m_eventTypesCallbacks;
    inline bool eventFilter(QObject* obj, QEvent* event) override
    {
        auto type = event->type();
        if (m_eventTypesCallbacks.contains(type))
        {
            // NOTE : filter means do NOT forward it to monitored object
            bool filter = true;
            for (const auto& callback : m_eventTypesCallbacks[type])
            {
                filter &= callback(event);
            }
            return filter;
        }
        else {
            // standard event processing
            return QObject::eventFilter(obj, event);
        }
    };
};

#endif // QUAWIDGETEVENTFILTER_H
