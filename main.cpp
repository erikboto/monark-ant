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
#include "bledatabroadcaster.h"
#include "ftmsdevice.h"
#include <QDebug>
#include <QNetworkInterface>
#include <QDBusConnection>
#include "dbusadaptor.h"
#include "gearsimulator.h"
#include "mqttconnection.h"
#include <QFile>

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

    GearSimulator *gearSimu = new GearSimulator(monark);
    DBusAdaptor *dbusAdaptor = new DBusAdaptor(monark, gearSimu);
    MqttConnection *mqttConnection = new MqttConnection(QString("Monark %1").arg(devId),
                                                        monark,
                                                        dbusAdaptor);

    Q_UNUSED(dbusAdaptor)
    QDBusConnection::sessionBus().registerObject("/Monark", monark);
    QDBusConnection::sessionBus().registerService("se.unixshell");

    //BLEDataBroadcaster *btpower = new BLEDataBroadcaster(devId);
    FTMSDevice *ftmsDevice = new FTMSDevice(devId);
    QObject::connect(monark, &MonarkConnection::typeIdentified, ftmsDevice, [ftmsDevice, monark](){
        ftmsDevice->setControllable(monark->canDoLoad());
        ftmsDevice->initialize();
        qDebug() << "canDoLoad: " << monark->canDoLoad() << " initializing";
    }, Qt::QueuedConnection);

    QObject::connect(monark, &MonarkConnection::power, ant, &ANT::setCurrentPower);
    QObject::connect(monark, &MonarkConnection::cadence, ant, &ANT::setCurrentCadence);

    QObject::connect(ant, &ANT::newTargetPower, monark, &MonarkConnection::setLoad);
    QObject::connect(ant, &ANT::gradeChanged, gearSimu, &GearSimulator::onGradeChanged);
    QObject::connect(ant, &ANT::fecModeChanged, monark, &MonarkConnection::setFecMode);

    QObject::connect(monark, &MonarkConnection::power, ftmsDevice, &FTMSDevice::setCurrentPower);
    QObject::connect(monark, &MonarkConnection::cadence, ftmsDevice, &FTMSDevice::setCurrentCadence);
    QObject::connect(ftmsDevice, &FTMSDevice::newTargetKp, monark, &MonarkConnection::setKp);
    QObject::connect(ftmsDevice, &FTMSDevice::newTargetPower, monark, &MonarkConnection::setLoad);
    QObject::connect(ftmsDevice, &FTMSDevice::newGrade, gearSimu, &GearSimulator::onGradeChanged);
    QObject::connect(ftmsDevice, &FTMSDevice::simulationModeChanged, monark, &MonarkConnection::setFecMode);

    QObject::connect(monark, &MonarkConnection::targetKpChanged, mqttConnection, &MqttConnection::onTargetKpChanged);
    QObject::connect(monark, &MonarkConnection::targetPowerChanged, mqttConnection, &MqttConnection::onTargetPowerChanged);

    // Control the status led on Raspberry Pi
    QObject::connect(monark, &MonarkConnection::connectionStatus, [](bool connected){
        QFile led("/sys/class/leds/led0/brightness");
        if ( led.open(QIODevice::WriteOnly) )
        {
            led.write(connected ? "1" : "0");
        }
    });

    monark->start();
    ant->start();

    return a.exec();
}

