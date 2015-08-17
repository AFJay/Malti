#-------------------------------------------------
#
# Project created by QtCreator 2015-08-16T03:27:17
#
#-------------------------------------------------

QT       += webkitwidgets network widgets

TARGET = malti-view
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  = mainwindow.h

RESOURCES += \
    jquery.qrc

DISTFILES +=

#Raspberry Pi Stuff
target.path = /home/pi/malti-view
TARGET = malti-view
INSTALLS += target
