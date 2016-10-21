/*
 * Copyright (c) 2016 Erik Botö (erik.boto@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "mainwindow.h"
#include <QApplication>
#include "LibUsb.h"
#include "ant.h"
#include "MonarkConnection.h"
#include "btcyclingpowerservice.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    MonarkConnection *monark = new MonarkConnection();
    ANT * ant = new ANT();

    BTCyclingPowerService *btpower = new BTCyclingPowerService();

    QObject::connect(monark, SIGNAL(power(quint16)), ant, SLOT(setCurrentPower(quint16)));
    QObject::connect(monark, SIGNAL(power(quint16)), &w, SLOT(onCurrentPowerChanged(quint16)));
    QObject::connect(monark, SIGNAL(cadence(quint8)), ant, SLOT(setCurrentCadence(quint8)));
    QObject::connect(&w, SIGNAL(currentLoadChanged(quint32)), monark, SLOT(setLoad(uint)));
    QObject::connect(monark, SIGNAL(connectionStatus(bool)), &w, SLOT(onConnectionStatusChanged(bool)));
    QObject::connect(ant, SIGNAL(newTargetPower(quint32)), &w, SLOT(setCurrentLoad(quint32)));

    QObject::connect(monark, &MonarkConnection::power, btpower, &BTCyclingPowerService::setPower);
    QObject::connect(monark, &MonarkConnection::cadence, btpower, &BTCyclingPowerService::setCadence);

    monark->start();
    ant->start();

    return a.exec();
}

