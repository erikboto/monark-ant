#include "mainwindow.h"
#include <QApplication>
#include "LibUsb.h"
#include "ant.h"
#include "MonarkConnection.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    MonarkConnection *monark = new MonarkConnection();
    ANT * ant = new ANT();

    QObject::connect(monark, SIGNAL(power(quint16)), ant, SLOT(setCurrentPower(quint16)));
    QObject::connect(monark, SIGNAL(power(quint16)), &w, SLOT(onCurrentPowerChanged(quint16)));
    QObject::connect(monark, SIGNAL(cadence(quint8)), ant, SLOT(setCurrentCadence(quint8)));
    QObject::connect(&w, SIGNAL(currentLoadChanged(quint32)), monark, SLOT(setLoad(uint)));
    QObject::connect(monark, SIGNAL(connectionStatus(bool)), &w, SLOT(onConnectionStatusChanged(bool)));

    monark->start();
    ant->start();

    return a.exec();
}

