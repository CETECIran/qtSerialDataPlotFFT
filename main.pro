#-------------------------------------------------
#
# Project created by QtCreator 2014-10-20T04:57:52
#
#-------------------------------------------------

QT       += core gui

#CONFIG += extserialport
#LIBS += -lfftw3
#LIBS +=  -L /usr/lib/libfftw3.so

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = main
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    qledindicator.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    qledindicator.h

FORMS    += mainwindow.ui


#LIBS += -L /usr/local/Qt-5.3.2/lib/libQt5ExtSerialPort.so
#INCLUDEPATH += -I /usr/local/Qt-5.3.2/include/QtExtSerialPort/
INCLUDEPATH += -I /usr/local/include

include(qextserialport/src/qextserialport.pri)

OTHER_FILES += \
    ../../../../Downloads/icons/at-sign-document.png
