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

#ifndef FECDEVICE_H
#define FECDEVICE_H

#include <QObject>
#include <QElapsedTimer>
#include "antmessage.h"
#include "antdevice.h"

class LibUsb;

class FECDevice : public QObject, public ANTDevice
{
    Q_OBJECT
public:
    explicit FECDevice(LibUsb * usb, const unsigned char channel, unsigned short deviceId, QObject *parent = 0);


    // outgoing pages
    ANTMessage fecPage16(bool toggleLap);

    ANTMessage fecPage17(bool toggleLap);

    ANTMessage fecPage21(bool toggleLap);
    ANTMessage fecPage25(bool toggleLap);

    ANTMessage fecPage54(); // send on request

    // Common Pages
    ANTMessage fecPage80();
    ANTMessage fecPage81();
    ANTMessage fecPage71(const unsigned char lastReceivedCommandId,
                         const unsigned char sequenceId,
                         const unsigned char commandStatus,
                         const unsigned char d0,
                         const unsigned char d1,
                         const unsigned char d2,
                         const unsigned char d3);

    // incoming pages
    // page 49 required
    double powerFromFecPage49(const unsigned char * message);

    enum class State {Reserved = 0, Asleep = 1, Ready = 2, InUse = 3, Finished = 4};

    void setState(State newState);
    void setCurrentPower(int power);
    void setTargetPower(quint32 targetPower);
    void setCadence(int cadence);
    void setHeartrate(int heartrate);

    int channel() const {return m_channel;}
    void configureChannel();
signals:
    void newTargetPower(quint32 targetPower);

public slots:
    void channelEvent(unsigned char *ant_message);
    void handleAckData(unsigned char *ant_message);
    void handlePageRequest(unsigned char *message);
    void sendNextPage();
    void setCurrentPower(quint16 power);
    void setCurrentCadence(quint8 cadence);

private:
    QElapsedTimer m_timer;
    LibUsb *m_usb;
    bool m_currLapMarkerHigh;
    State m_state;
    unsigned char m_channel;
    int m_currPower;
    quint32 m_targetPower;
    int m_cadence;
    int m_heartRate;
    int m_lastPage;
    int m_nextPage;
    unsigned short m_deviceId;
};

#endif // FECDEVICE_H
