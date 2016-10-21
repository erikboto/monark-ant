QT       += core gui serialport widgets bluetooth

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

TARGET = Monark-ANT
TEMPLATE = app

disable-ant-power {
    DEFINES += DISABLE_ANT_POWER
}

disable-ant-fec {
    DEFINES += DISABLE_ANT_FEC
}

SOURCES +=  main.cpp \
            mainwindow.cpp \
            MonarkConnection.cpp \
            powerdevice.cpp \
            LibUsb.cpp \
            antmessage.cpp \
            ant.cpp \
            fecdevice.cpp \
            antdevice.cpp \
            btcyclingpowerservice.cpp

HEADERS  += mainwindow.h \
            MonarkConnection.h \
            powerdevice.h \
            LibUsb.h \
            antmessage.h \
            ant.h \
            fecdevice.h \
            antdevice.h \
            btcyclingpowerservice.h
