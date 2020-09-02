INCLUDEPATH += $$PWD/

FORMS += \
    $$PWD/qualogwidget.ui \
    $$PWD/qualogwidgetsettings.ui \
    $$PWD/quaserverwidget.ui \
    $$PWD/quacommondialog.ui

SOURCES += \
    $$PWD/qualogwidget.cpp \
    $$PWD/qualogwidgetsettings.cpp \
    $$PWD/quaserverwidget.cpp \
    $$PWD/quacommondialog.cpp

HEADERS += \
    $$PWD/qualogwidget.h \
    $$PWD/qualogwidgetsettings.h \
    $$PWD/quaserverwidget.h \
    $$PWD/quacommondialog.h

include($$PWD/quamodelview.pri)

HEADERS += \    
    $$PWD/quanodetypemodel.h \
    $$PWD/quacategorymodel.h \
    $$PWD/quamodelitemtraits.h \
    $$PWD/quanodemodelitemtraits.h \
    $$PWD/qualogmodelitemtraits.h
