/*
 * Copyright (c) 2009 Mark Rages
 * Copyright (c) 2011 Mark Liversedge (liversedge@gmail.com)
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

#ifndef ANT_H
#define ANT_H

#include <QThread>
#include "LibUsb.h"
#include "antmessage.h"

#include "powerdevice.h"
#include "fecdevice.h"
#include <QMap>

class ANT : public QThread
{
    Q_OBJECT
public:
    ANT(unsigned short deviceId);

public slots:
    void setCurrentPower(quint16 power);
    void setCurrentCadence(quint8 cadence);

private:
    void run();
    LibUsb *m_usb;
    //PowerDevice *m_pd;
    QMap<int, ANTDevice*> m_devices;

    // state machine whilst receiving bytes
    enum States {ST_WAIT_FOR_SYNC, ST_GET_LENGTH, ST_GET_MESSAGE_ID, ST_GET_DATA, ST_VALIDATE_PACKET} m_state;
    void receiveByte(unsigned char byte);
    void processMessage();
    void handleChannelEvent();
    void receiveChannelMessage(unsigned char *ant_message);

    unsigned char rxMessage[ANT_MAX_MESSAGE_SIZE];

    int length;
    int bytes;
    int checksum;
    unsigned short m_deviceId;

signals:
    void newTargetPower(quint32 targetPower);
    void gradeChanged(double grade);
    void fecModeChanged(FECDevice::FecMode mode);

};

#endif // ANT_H
