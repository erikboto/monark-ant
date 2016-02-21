#-------------------------------------------------
#
# Project created by QtCreator 2016-02-19T18:02:08
#
#-------------------------------------------------

QT       += core gui serialport

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb
}

win32 {
    LIBUSB_INSTALL = ""
    LIBUSB_LIBS = -L$${LIBUSB_INSTALL}/lib/gcc -lusb
    INCLUDEPATH += $${LIBUSB_INSTALL}/include
    LIBS        += $${LIBUSB_LIBS}
}

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Monark-ANT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    MonarkConnection.cpp \
    powerdevice.cpp \
    LibUsb.cpp \
    antmessage.cpp \
    ant.cpp

HEADERS  += mainwindow.h \
    MonarkConnection.h \
    powerdevice.h \
    LibUsb.h \
    antmessage.h \
    ant.h
