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

#include <QCoreApplication>
#include "LibUsb.h"
#include "ant.h"
#include "MonarkConnection.h"
#include "btcyclingpowerservice.h"
#include <QDebug>
#include <QNetworkInterface>
#include <QDBusConnection>
#include "dbusadaptor.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Use MAC-address of network interface to select a device ID for ANT+
    QString hwaddr;
    for (QNetworkInterface iface: QNetworkInterface::allInterfaces())
    {

        if (!(iface.flags() & QNetworkInterface::IsLoopBack))
        {
            hwaddr = iface.hardwareAddress();
            break;
        }
    }

    hwaddr = hwaddr.remove(":");
    bool ok = false;
    unsigned short devId = hwaddr.right(4).toInt(&ok, 16);
    if (!ok)
        devId = 1137;

    qDebug() << "Using ANT+ device ID: " << devId;

    MonarkConnection *monark = new MonarkConnection();
    ANT * ant = new ANT(devId);

    DBusAdaptor *dbusAdaptor = new DBusAdaptor(monark);
    QDBusConnection::sessionBus().registerObject("/Monark", monark);
    QDBusConnection::sessionBus().registerService("se.unixshell");

    BTCyclingPowerService *btpower = new BTCyclingPowerService();

    QObject::connect(monark, &MonarkConnection::power, ant, &ANT::setCurrentPower);
    QObject::connect(monark, &MonarkConnection::cadence, ant, &ANT::setCurrentCadence);

    QObject::connect(monark, &MonarkConnection::power, btpower, &BTCyclingPowerService::setPower);
    QObject::connect(monark, &MonarkConnection::cadence, btpower, &BTCyclingPowerService::setCadence);

    monark->start();
    ant->start();

    return a.exec();
}

