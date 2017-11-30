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

#include "fecdevice.h"
#include "LibUsb.h"
#include <QDebug>

#define FEC_STATE_MASK 0xF0
#define FEC_CAPS_MASK 0x0F

FECDevice::FECDevice(LibUsb *usb, const unsigned char channel, unsigned short deviceId, QObject *parent) : QObject(parent),
    m_usb(usb),
    m_currLapMarkerHigh(false),
    m_state(State::Ready),
    m_channel(channel),
    m_currPower(100),
    m_targetPower(0),
    m_grade(0),
    m_cadence(0),
    m_heartRate(0),
    m_lastPage(-1),
    m_nextPage(16),
    m_deviceId(deviceId)
{
    m_timer.start();

}

ANTMessage FECDevice::fecPage16(bool toggleLap)
{
    const unsigned char page = 0x10;    // page 16
    const unsigned char eqType = 0x19;  // trainer
    //const unsigned char eqType = 0x15;  // stationary bike
    const unsigned char distance = 0xFF; // not used due to our capabilities field, value doesn't matter
    const unsigned char speedMSB = 0x0; // set speed to zero, it's a required field for trainers
    const unsigned char speedLSB = 0x0;
    const unsigned char heartRate = 0xFF; // set to invalid, not required value

    const unsigned char capabilities = 0x0; // Bit 0-3 No HR source, No distance or speed

    if (toggleLap)
    {
        m_currLapMarkerHigh = !m_currLapMarkerHigh;
    }

    unsigned char lap = m_currLapMarkerHigh ? (1<<7) : 0;

    // calculated elapsed time (0.25s with 64s rollaround)
    qint64 elapsedMilliSec = m_timer.elapsed();

    const unsigned char time = qint64(elapsedMilliSec/250) % 256;


    const unsigned char caps_and_state = ( ((((unsigned char)m_state) << 4) | lap) & FEC_STATE_MASK) | (capabilities & FEC_CAPS_MASK);

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, eqType, time, distance, speedLSB, speedMSB, heartRate, caps_and_state);
}

ANTMessage FECDevice::fecPage17(bool toggleLap)
{
    const unsigned char page = 0x11; // page 17

    if (toggleLap)
    {
        m_currLapMarkerHigh = !m_currLapMarkerHigh;
    }

    // current resistance (% of max, 0.5% steps)
    const unsigned char resistance = (m_targetPower / 700);

    unsigned char lap = m_currLapMarkerHigh ? (1<<7) : 0;

    const unsigned char caps_and_state = ((((unsigned char)m_state) << 4) | lap);

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, resistance, caps_and_state);
}

ANTMessage FECDevice::fecPage54()
{
    const unsigned char page = 0x36; // page 54
    const unsigned char caps = 0b00000110; // support target power mode and simulation

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, caps);
}

double FECDevice::powerFromFecPage49(const unsigned char *message)
{
    // Verify page type
    if (message[0] != 0x31)
    {
        // wrong page
        qDebug() << "powerFromFecPage49: Trying to interpret wrong page type";
        return 0;
    }

    return (message[7]<<8 | message[6])/4;

}

double FECDevice::gradeFromFecPage51(const unsigned char *message)
{
    // Verify page type
    if (message[0] != 0x33)
    {
        // wrong page
        qDebug() << "gradeFromFecPage51: Trying to interpret wrong page type";
        return 0;
    }
    // Byte 5 and 6 contains value in unit 0.01%, with an offset of 200
    return ((message[6]<<8 | message[5])/100)-200;
}

ANTMessage FECDevice::fecPage80()
{
    const unsigned char page = 0x50; // page 80

    const unsigned char manuMSB = 0x00;
    const unsigned char manuLSB = 0xFF; // 0x00FF is reserved for development

    const unsigned char hwRev = 0x01;

    const unsigned char modMSB = 0x00;
    const unsigned char modLSB = 0x01;

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, 0xFF, hwRev, manuLSB, manuMSB, modLSB, modMSB);
}

ANTMessage FECDevice::fecPage81()
{
    const unsigned char page = 0x51; // page 80

    const unsigned char swRevSupp = 0xFF; // 0xFF for not used
    const unsigned char swRevMain = 0x01;

    const unsigned char serial = 0xFF; // 0xFFFFFFFF for devices without serial number

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, swRevSupp, swRevMain, serial, serial, serial, serial);
}

ANTMessage FECDevice::fecPage71(const unsigned char lastReceivedCommandId,
                                const unsigned char sequenceId,
                                const unsigned char commandStatus,
                                const unsigned char d0,
                                const unsigned char d1,
                                const unsigned char d2,
                                const unsigned char d3)
{
    const unsigned char page = 0x47; // page 71

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, lastReceivedCommandId, sequenceId, commandStatus, d0, d1, d2, d3);
}

ANTMessage FECDevice::fecPage21(bool toggleLap)
{
    const unsigned char page = 0x15; // page 21 (stationary bike specific main page)

    if (toggleLap)
    {
        m_currLapMarkerHigh = !m_currLapMarkerHigh;
    }

    const unsigned char cadence = m_cadence;
    const unsigned short power = m_currPower;

    unsigned char lap = m_currLapMarkerHigh ? (1<<7) : 0;
    const unsigned char caps_and_state = ((((unsigned char)m_state) << 4) | lap);

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, 0xFF, 0xFF, cadence, power & 0x00FF, (power & 0xFF00) >> 8, caps_and_state);

}

ANTMessage FECDevice::fecPage25(bool toggleLap)
{
    static unsigned char eventCount = 0;
    const unsigned char page = 0x19; // page 25 (trainer specific main page)

    static unsigned short accuPower = 0;

    accuPower += m_currPower;

    const unsigned char accuPowerLSB = accuPower & 0x00FF;
    const unsigned char accuPowerMSB = accuPower >> 8;

    const unsigned char instPowerLSB = m_currPower & 0x00FF;
    const unsigned char instPowerMSB = m_currPower >> 8;

    if (toggleLap)
    {
        m_currLapMarkerHigh = !m_currLapMarkerHigh;
    }

    const unsigned char cadence = m_cadence;

    const unsigned char flags_and_status = 0;

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, eventCount++, cadence, accuPowerLSB, accuPowerMSB, instPowerLSB, instPowerMSB, flags_and_status);

}

void FECDevice::setState(State newState)
{
    m_state = newState;
}

void FECDevice::setCadence(int cadence)
{
    m_cadence = cadence;
}

void FECDevice::setCurrentPower(int power)
{
    m_currPower = power;
}

void FECDevice::setHeartrate(int heartrate)
{
    m_heartRate = heartrate;
}

void FECDevice::setTargetPower(quint32 targetPower)
{
    if (m_targetPower != targetPower)
    {
        m_targetPower = targetPower;
        emit newTargetPower(m_targetPower);
        qDebug() << "New target power: " << m_targetPower;
    }
}

void FECDevice::setGrade(double grade)
{
    if (m_grade != grade)
    {
        m_grade = grade;
        emit gradeChanged(m_grade);
        qDebug() << "New grade: " << m_grade;
    }
}

void FECDevice::channelEvent(unsigned char *ant_message)
{
    // byte 0 sync
    // byte 1 len
    // byte 2 type (channel event 0x40 if we get it here)
    // byte 3 channel number
    // byte 4 message id (1 for RF events)
    // byte 5 message code

    if (! (ant_message[2] == 0x40))
    {
        qDebug() << "FECDevice::channelEvent() called with wrong message type";
        return;
    }

    // Make sure we're the right channel
    if (! (ant_message[3] == m_channel))
    {
        qDebug() << "FECDevice::channelEvent() called with wrong channel";
        return;
    }

    // Make sure it's an RF event
    if (! (ant_message[4] == 1))
    {
        qDebug() << "FECDevice::channelEvent() called, but not RF event";
        return;
    }

    switch (ant_message[5])
    {
    case RESPONSE_NO_ERROR:
        qDebug() << "FECDevice::channelEvent" << "RESPONSE_NO_ERROR";
        break;
    case EVENT_RX_SEARCH_TIMEOUT:
        qDebug() << "FECDevice::channelEvent" << "EVENT_RX_SEARCH_TIMEOUT";
        break;
    case EVENT_RX_FAIL:
        qDebug() << "FECDevice::channelEvent" << "EVENT_RX_FAIL";
        break;
    case EVENT_TX:
        //qDebug() << "FECDevice::channelEvent" << "EVENT_TX";
        sendNextPage();
        break;
    case EVENT_TRANSFER_RX_FAILED:
        qDebug() << "FECDevice::channelEvent" << "EVENT_TRANSFER_RX_FAILED";
        break;
    case EVENT_TRANSFER_TX_COMPLETED:
        qDebug() << "FECDevice::channelEvent" << "EVENT_TRANSFER_TX_COMPLETED";
        break;
    case EVENT_CHANNEL_COLLISION:
        qDebug() << "FECDevice::channelEvent" << "EVENT_CHANNEL_COLLISION";
        break;
    default:
        // There's a lot not handled yet here
        qDebug() << "FECDevice::channelEvent" << " unhandled message";
        break;
    }
}

void FECDevice::sendNextPage()
{
    ANTMessage m;
    static int patternCounter = 0;
    static int commonCounter = 0;
    static int nextCommonPage = 80;

    if (patternCounter < 64)
    {

        // This is somwhat according to recommended pattern from ANT+ FE-C docs (expect for not using 18)
        switch (patternCounter++%8)
        {
        case 0:
        case 1:
        case 4:
        case 5:
            m = fecPage16(false);
            break;
        case 2:
        case 6:
            m = fecPage25(false);
            // m = fecPage21(false);
            break;
        case 3:
        case 7:
            m = fecPage17(false);
            break;
        }
    } else {
        if (nextCommonPage == 80)
        {
            m = fecPage80();
        } else {
            m = fecPage81();
        }

        if (++commonCounter == 2)
        {
            nextCommonPage = (nextCommonPage == 80) ? 81 : 80;
            commonCounter = 0;
            patternCounter = 0;
        }
    }

    m_usb->write((char*)m.data, m.length);
}

void FECDevice::handleAckData(unsigned char *ant_message)
{

    // byte 0 sync
    // byte 1 len
    // byte 2 type (channel event 0x40 if we get it here)
    // byte 3 channel number
    // byte 4 message id

    if (! (ant_message[2] == 0x4F))
    {
        qDebug() << "FECDevice::handleAckData() called with wrong message type";
        return;
    }

    // Make sure we're the right channel
    if (! (ant_message[3] == m_channel))
    {
        qDebug() << "FECDevice::handleAckData() called with wrong channel";
        return;
    }

    switch (ant_message[4]) {
    case 0x31: // power
        {
            unsigned char * ant_sport_mess = ant_message+4;
            setTargetPower(powerFromFecPage49(ant_sport_mess));
        }
        break;
    case 0x32: // wind resistance
        qDebug() << "Got wind resistance page 0x32";
        break;
    case 0x33: // track resistance
        qDebug() << "Got track resistance page 0x33";
        {
            unsigned char * ant_sport_mess = ant_message+4;
            setGrade(gradeFromFecPage51(ant_sport_mess));
        }

        break;
    case 0x46: // Request for a special page, we should only get req for page 54
    {
        unsigned char * ant_sport_mess = ant_message+4;
        handlePageRequest(ant_sport_mess);
        break;
    }
    default:
        qDebug() << __func__ << "Unhandled" << ant_message[4] ;
    }
}

void FECDevice::handlePageRequest(unsigned char *message)
{
    const unsigned char descriptorByte1 = message[3];
    const unsigned char descriptorByte2 = message[4];
    const unsigned char numberOfTimes = message[5] & 0x7F;
    bool replyUsingAck = message[5]>>7 == 1;
    const unsigned char requestedPage = message[6];
    const unsigned char commandType = message[7];

    if (replyUsingAck)
        qDebug() << "FECDevice::handlePageRequest - replyUsingAck requested but not supported";


    if (commandType == 1) // Request Data Page
    {
        switch(requestedPage)
        {
        case 54:
        {
            ANTMessage m = fecPage54();
            m_usb->write((char*)m.data, m.length);
        }
            break;
        default:
            qDebug() << "FECDevice::handlePageRequest - unhandled request for page " << requestedPage;
        }
    }
}

void FECDevice::configureChannel()
{
    ANTMessage assignCh = ANTMessage::assignChannel(m_channel, 0x10, 0);
    m_usb->write((char *)assignCh.data,assignCh.length);

    ANTMessage id = ANTMessage::setChannelID(m_channel, m_deviceId, 0x11, 0x05);
    m_usb->write((char *)id.data, id.length);

    ANTMessage chanPeriod = ANTMessage::setChannelPeriod(m_channel,8192);
    m_usb->write((char *)chanPeriod.data, chanPeriod.length);

    ANTMessage chanFreq = ANTMessage::setChannelFreq(m_channel, 57); // 57 ANT Sport
    m_usb->write((char *)chanFreq.data, chanFreq.length);

    ANTMessage openChan = ANTMessage::open(m_channel);
    m_usb->write((char *)openChan.data, openChan.length);
}

void FECDevice::setCurrentCadence(quint8 cadence)
{
    m_cadence = cadence;
}

void FECDevice::setCurrentPower(quint16 power)
{
    m_currPower = power;
}
