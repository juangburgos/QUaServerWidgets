QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../src/quaserverwidgets.pri)

SOURCES += \
main.cpp \
dialog.cpp \
    quabaseobjectext.cpp

HEADERS += \
dialog.h \
    quabaseobjectext.h

FORMS += \
dialog.ui

include($$PWD/../../libs/add_qt_path_win.pri)
