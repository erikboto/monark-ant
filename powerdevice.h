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

#ifndef POWERDEVICE_H
#define POWERDEVICE_H

#include <QObject>
#include "antmessage.h"
#include "antdevice.h"

class LibUsb;

class PowerDevice :  public QObject, public ANTDevice
{
    Q_OBJECT
public:
    explicit PowerDevice(LibUsb * usb, const unsigned char channel, unsigned short deviceId, QObject *parent = 0);

    ANTMessage page16();
    ANTMessage page80();
    ANTMessage page81();
    ANTMessage page01(); // calibration response

    void configureChannel();

    void handleAckData(unsigned char *ant_message);
    int channel() const {return m_channel;}
signals:

public slots:
    void channelEvent(unsigned char *ant_message);
    void sendNextPage();
    void setCurrentPower(quint16 power);
    void setCurrentCadence(quint8 cadence);

private:
    LibUsb *m_usb;
    unsigned char m_channel;
    quint16 m_power;
    unsigned char m_cadence;
    unsigned short m_deviceId;
};

#endif // POWERDEVICE_H
