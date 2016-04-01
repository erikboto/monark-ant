/*
 * Copyright (c) 2011 Mark Liversedge (liversedge@gmail.com)
 * Copyright (c) 2009 Mark Rages (Quarq)
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

#include "antmessage.h"

#include <QDebug>

ANTMessage::ANTMessage() :
    length(-1)
{

}

// construct a message with all data passed (except sync and checksum)
ANTMessage::ANTMessage(const unsigned char len,
                       const unsigned char type,
                       const unsigned char b3,
                       const unsigned char b4,
                       const unsigned char b5,
                       const unsigned char b6,
                       const unsigned char b7,
                       const unsigned char b8,
                       const unsigned char b9,
                       const unsigned char b10,
                       const unsigned char b11)
{

    // encode the message
    data[0] = ANT_SYNC_BYTE;
    data[1] = len; // message payload length
    data[2] = type; // message type
    data[3] = b3;
    data[4] = b4;
    data[5] = b5;
    data[6] = b6;
    data[7] = b7;
    data[8] = b8;
    data[9] = b9;
    data[10] = b10;
    data[11] = b11;

    // compute the checksum and place after data
    unsigned char crc = 0;
    int i=0;
    for (; i< (len+3); i++) crc ^= data[i];
    data[i] = crc;

    length = i+1;
}

ANTMessage::ANTMessage(const unsigned char * message)
{
    // Verify sync
    if (! (message[0] == ANT_SYNC_BYTE))
    {
        qDebug() << "Message with invalid sync";
    }

    length = message[1];
    type = message[2];

    memcpy(data, message, ANT_MAX_MESSAGE_SIZE);

    switch(type) {
    case ANT_CHANNEL_EVENT:
        qDebug() << "Channel event";
        break;
    default:
        qDebug() << "Unhandled message";
    }
}

ANTMessage ANTMessage::assignChannel(const unsigned char channel,
                                     const unsigned char type,
                                     const unsigned char network)
{
    return ANTMessage(3, ANT_ASSIGN_CHANNEL, channel, type, network);
}

ANTMessage ANTMessage::setChannelID(const unsigned char channel,
                                    const unsigned short device,
                                    const unsigned char devicetype,
                                    const unsigned char txtype)
{
    return ANTMessage(5, ANT_CHANNEL_ID, channel, device&0xff, (device>>8)&0xff, devicetype, txtype);
}

ANTMessage ANTMessage::setChannelPeriod(const unsigned char channel,
                                        const unsigned short period)
{
    return ANTMessage(3, ANT_CHANNEL_PERIOD, channel, period&0xff, (period>>8)&0xff);
}

ANTMessage ANTMessage::setChannelFreq(const unsigned char channel,
                                      const unsigned char frequency)
{
    // Channel Frequency = 2400 MHz + Channel RF Frequency Number * 1.0 MHz
    // The range is 0-124 with a default value of 66
    // ANT_SPORT_FREQ = 57
    // ANT_KICKR_FREQ = 52
    return ANTMessage(2, ANT_CHANNEL_FREQUENCY, channel, frequency);
}

ANTMessage ANTMessage::open(const unsigned char channel)
{
    return ANTMessage(1, ANT_OPEN_CHANNEL, channel);
}
