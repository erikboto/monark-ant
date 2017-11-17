QT       += core gui serialport bluetooth network

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

raspberry-pi {
    DEFINES += RASPBERRYPI
}

SOURCES +=  main.cpp \
            MonarkConnection.cpp \
            powerdevice.cpp \
            LibUsb.cpp \
            antmessage.cpp \
            ant.cpp \
            fecdevice.cpp \
            antdevice.cpp \
            btcyclingpowerservice.cpp

HEADERS  += MonarkConnection.h \
            powerdevice.h \
            LibUsb.h \
            antmessage.h \
            ant.h \
            fecdevice.h \
            antdevice.h \
            btcyclingpowerservice.h
