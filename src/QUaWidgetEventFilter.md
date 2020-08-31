## Handle widget events without sub-classing

Just pass in the widget as the parent to the `QUaWidgetEventFilter` instance and call the `installEventCallback` method to add a callback for an specific `QEvent::` event:

<https://doc.qt.io/qt-5/qevent.html#Type-enum>

```c++
// set double click event handler for lineedit
auto lineEditEventHandler = new QUaWidgetEventFilter(ui->lineEditSource);
lineEditEventHandler->installEventCallback(QEvent::MouseButtonDblClick,
    [this](QEvent* event) {
        Q_UNUSED(event);
        this->openFile();
    });
// set double click event handler for scroll area
auto scrollEventHandler = new QUaWidgetEventFilter(ui->scrollAreaQml);
scrollEventHandler->installEventCallback(QEvent::KeyPress,
    [this](QEvent* event) {
        // update pressed keys
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        Q_ASSERT(keyEvent);
        auto key = static_cast<Qt::Key>(keyEvent->key());
        m_keysPressed << key;
        // check for shortcuts
        if (m_keysPressed.contains(Qt::Key_Control) && m_keysPressed.contains(Qt::Key_Alt) &&
            m_keysPressed.contains(Qt::Key_V) && m_keysPressed.contains(Qt::Key_R))
        {
            this->toggleFullScreen();
            // NOTE for some reason, after toggle to minimize, 
            // key release events are not handled, so need to force clean
            m_keysPressed.clear();
        }
        if (m_keysPressed.contains(Qt::Key_F5))
        {
            this->reloadFile();
            // NOTE do not allow multiple executions
            m_keysPressed.clear();
        }
    });
scrollEventHandler->installEventCallback(QEvent::KeyRelease,
    [this](QEvent* event) {
        // update pressed keys
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        Q_ASSERT(keyEvent);
        auto key = static_cast<Qt::Key>(keyEvent->key());
        m_keysPressed.remove(key);
    });
```