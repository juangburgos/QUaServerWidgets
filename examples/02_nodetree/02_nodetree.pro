QT += core gui xml sql

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

SOURCES += \
$$PWD/../../libs/QUaServer.git/examples/09_serialization/quaxmlserializer.cpp \
$$PWD/../../libs/QUaServer.git/examples/09_serialization/quasqliteserializer.cpp

HEADERS += \
$$PWD/../../libs/QUaServer.git/examples/09_serialization/quaxmlserializer.h \
$$PWD/../../libs/QUaServer.git/examples/09_serialization/quasqliteserializer.h

INCLUDEPATH += $$PWD/../../libs/QUaServer.git/examples/09_serialization

FORMS += \
dialog.ui

include($$PWD/../../libs/add_qt_path_win.pri)
