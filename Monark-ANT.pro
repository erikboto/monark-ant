QT       += core gui serialport bluetooth network dbus mqtt

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb
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
            filelogger.cpp \
            mqttconnection.cpp \
            powerdevice.cpp \
            LibUsb.cpp \
            antmessage.cpp \
            ant.cpp \
            fecdevice.cpp \
            antdevice.cpp \
            dbusadaptor.cpp \
            gearsimulator.cpp \
            bledatabroadcaster.cpp \
            ftmsdevice.cpp

HEADERS  += MonarkConnection.h \
            filelogger.h \
            mqttconnection.h \
            powerdevice.h \
            LibUsb.h \
            antmessage.h \
            ant.h \
            fecdevice.h \
            antdevice.h \
            dbusadaptor.h \
            gearsimulator.h \
            bledatabroadcaster.h \
            ftmsdevice.h
