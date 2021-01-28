QT += core sql
QT -= gui

CONFIG += c++11

TARGET = 05_historwidget
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
#include($$PWD/../../src/quaserverwidgets.pri)

HISTORPATH = "$$PWD/../../libs/QUaServer.git/examples/10_historizing"

INCLUDEPATH += $$PWD/
INCLUDEPATH += $$HISTORPATH/

HEADERS += $$HISTORPATH/quamultisqlitehistorizer.h

SOURCES += main.cpp
SOURCES += $$HISTORPATH/quamultisqlitehistorizer.cpp

ua_events || ua_alarms_conditions {
	EVENTPATH = "$$PWD/../../libs/QUaServer.git/examples/08_events"

	INCLUDEPATH += $$EVENTPATH/

	SOURCES += \
	$$EVENTPATH/myevent.cpp
	HEADERS += \
	$$EVENTPATH/myevent.h
}

include($$PWD/../../libs/add_qt_path_win.pri)
