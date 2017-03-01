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

#include "powerdevice.h"
#include "LibUsb.h"
#include <QDebug>

PowerDevice::PowerDevice(LibUsb * usb, const unsigned char channel, unsigned short deviceId, QObject *parent) : QObject(parent),
    m_usb(usb),
    m_channel(channel),
    m_power(90),
    m_cadence(0),
    m_deviceId(deviceId)
{

}

void PowerDevice::channelEvent(unsigned char *ant_message)
{
    // byte 0 sync
    // byte 1 len
    // byte 2 type (channel event 0x40 if we get it here)
    // byte 3 channel number
    // byte 4 message id (1 for RF events)
    // byte 5 message code

    if (! (ant_message[2] == 0x40))
    {
        qDebug() << "PowerDevice::channelEvent() called with wrong message type";
        return;
    }

    // Make sure we're the right channel
    if (! (ant_message[3] == m_channel))
    {
        qDebug() << "PowerDevice::channelEvent() called with wrong channel";
        return;
    }

    // Make sure it's an RF event
    if (! (ant_message[4] == 1))
    {
        qDebug() << "PowerDevice::channelEvent() called, but not RF event";
        return;
    }

    switch (ant_message[5])
    {
    case RESPONSE_NO_ERROR:
        qDebug() << "PowerDevice::channelEvent" << "RESPONSE_NO_ERROR";
        break;
    case EVENT_RX_SEARCH_TIMEOUT:
        qDebug() << "PowerDevice::channelEvent" << "EVENT_RX_SEARCH_TIMEOUT";
        break;
    case EVENT_RX_FAIL:
        qDebug() << "PowerDevice::channelEvent" << "EVENT_RX_FAIL";
        break;
    case EVENT_TX:
        //qDebug() << "PowerDevice::channelEvent" << "EVENT_TX";
        sendNextPage();
        break;
    case EVENT_TRANSFER_RX_FAILED:
        qDebug() << "PowerDevice::channelEvent" << "EVENT_TRANSFER_RX_FAILED";
        break;
    case EVENT_TRANSFER_TX_COMPLETED:
        qDebug() << "PowerDevice::channelEvent" << "EVENT_TRANSFER_TX_COMPLETED";
        break;
    case EVENT_CHANNEL_COLLISION:
        qDebug() << "PowerDevice::channelEvent" << "EVENT_CHANNEL_COLLISION";
        break;
    default:
        // There's a lot not handled yet here
        qDebug() << "PowerDevice::channelEvent" << " unhandled message";
        break;
    }
}

void PowerDevice::sendNextPage()
{

    // Pages
    // 0x10, 0x50, 0x51 0x01 (calibration)
    ANTMessage m;
    static int patternCounter = 0;
    static int nextCommonPage = 80;

    if (patternCounter++ < 60)
    {

        m = page16();
    } else {
        if (nextCommonPage == 80)
        {
            m = page80();
        } else {
            m = page81();
        }

        nextCommonPage = (nextCommonPage == 80) ? 81 : 80;
        patternCounter = 0;
    }

    m_usb->write((char*)m.data, m.length);
}

ANTMessage PowerDevice::page16()
{
    const unsigned char page = 0x10;    // page 16
    const unsigned char pedalpower = 0xFF; // not used
    static unsigned char eventCount = 0;
    static unsigned short accuPower = 0;

    accuPower += m_power;

    const unsigned char accuPowerLSB = accuPower & 0x00FF;
    const unsigned char accuPowerMSB = accuPower >> 8;

    const unsigned char instPowerLSB = m_power & 0x00FF;
    const unsigned char instPowerMSB = m_power >> 8;

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, eventCount++, pedalpower, m_cadence, accuPowerLSB , accuPowerMSB, instPowerLSB, instPowerMSB);
}

ANTMessage PowerDevice::page80()
{
    const unsigned char page = 0x50; // page 80

    const unsigned char manuMSB = 0x00;
    const unsigned char manuLSB = 0xFF; // 0x00FF is reserved for development

    const unsigned char hwRev = 0x01;

    const unsigned char modMSB = 0x00;
    const unsigned char modLSB = 0x01;

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, 0xFF, hwRev, manuLSB, manuMSB, modLSB, modMSB);
}

ANTMessage PowerDevice::page81()
{
    const unsigned char page = 0x51; // page 80

    const unsigned char swRevSupp = 0xFF; // 0xFF for not used
    const unsigned char swRevMain = 0x01;

    const unsigned char serial = 0xFF; // 0xFFFFFFFF for devices without serial number

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, 0xFF, swRevSupp, swRevMain, serial, serial, serial, serial);
}

ANTMessage PowerDevice::page01()
{
    const unsigned char page = 0x01; // page 01
    const unsigned char success = 0xAC;

    return ANTMessage(9, ANT_BROADCAST_DATA, m_channel, page, success, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00);
}

void PowerDevice::handleAckData(unsigned char *ant_message)
{

    // byte 0 sync
    // byte 1 len
    // byte 2 type (channel event 0x40 if we get it here)
    // byte 3 channel number
    // byte 4 message id

    if (! (ant_message[2] == 0x4F))
    {
        qDebug() << "PowerDevice::handleAckData() called with wrong message type";
        return;
    }

    // Make sure we're the right channel
    if (! (ant_message[3] == m_channel))
    {
        qDebug() << "PowerDevice::handleAckData() called with wrong channel";
        return;
    }

    switch (ant_message[4]) {
    case 0x01: // calibration
        {
            ANTMessage m = page01();
            m_usb->write((char*)m.data, m.length);
        }
        break;

    case 0x46: // Request for a special page, should not be needed.
    {
        //unsigned char * ant_sport_mess = ant_message+4;
        //handlePageRequest(ant_sport_mess);
        break;
    }
    default:
        qDebug() << __func__ << "Unhandled" << ant_message[4] ;
    }
}

void PowerDevice::setCurrentCadence(quint8 cadence)
{
    m_cadence = cadence;
}

void PowerDevice::setCurrentPower(quint16 power)
{
    m_power = power;
}

void PowerDevice::configureChannel()
{
    ANTMessage assignCh = ANTMessage::assignChannel(m_channel, 0x10, 0);
    m_usb->write((char *)assignCh.data,assignCh.length);

    ANTMessage id = ANTMessage::setChannelID(m_channel, m_deviceId, 0x0B, 0x05);
    m_usb->write((char *)id.data, id.length);

    ANTMessage chanPeriod = ANTMessage::setChannelPeriod(m_channel,8182);
    m_usb->write((char *)chanPeriod.data, chanPeriod.length);

    ANTMessage chanFreq = ANTMessage::setChannelFreq(m_channel, 57); // 57 ANT Sport
    m_usb->write((char *)chanFreq.data, chanFreq.length);

    ANTMessage openChan = ANTMessage::open(m_channel);
    m_usb->write((char *)openChan.data, openChan.length);
}
