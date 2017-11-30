/*
 * Copyright (c) 2017 Erik Botö (erik.boto@gmail.com)
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


#ifndef DBUSADAPTOR_H
#define DBUSADAPTOR_H

#include <QDBusAbstractAdaptor>
#include "MonarkConnection.h"
#include "gearsimulator.h"

class DBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "se.unixshell.MonarkControl")
    Q_PROPERTY(quint16 targetPower READ targetPower WRITE setTargetPower)
    Q_PROPERTY(double targetKp READ targetKp WRITE setTargetKp)
    Q_PROPERTY(quint8 mode READ mode WRITE setMode)

public:
    DBusAdaptor(MonarkConnection *obj, GearSimulator *gs);

public slots:
    quint16 targetPower();
    void setTargetPower(quint16 targetPower);
    double targetKp();
    void setTargetKp(double targetKp);
    quint8 mode();
    void setMode(quint8 mode);
    void incGear();
    void decGear();

signals:
    void targetPowerChanged(quint16 power);
    void targetKpChanged(double kp);
    void currentPower(quint16 power);
    void currentCadence(quint8 cadence);
    void modeChanged(quint8 mode);

private:
    MonarkConnection *m_monark;
    GearSimulator *m_gearSimulator;

};

#endif // DBUSADAPTOR_H
