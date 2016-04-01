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

#ifndef ANTMESSAGE_H
#define ANTMESSAGE_H

#include <cstdint>

// Channel messages
#define RESPONSE_NO_ERROR               0
#define EVENT_RX_SEARCH_TIMEOUT         1
#define EVENT_RX_FAIL                   2
#define EVENT_TX                        3
#define EVENT_TRANSFER_RX_FAILED        4
#define EVENT_TRANSFER_TX_COMPLETED     5
#define EVENT_TRANSFER_TX_FAILED        6
#define EVENT_CHANNEL_CLOSED            7
#define EVENT_CHANNEL_COLLISION         9
#define EVENT_RX_BROADCAST             10
#define EVENT_RX_ACKNOWLEDGED          11
#define EVENT_RX_BURST_PACKET          12
#define CHANNEL_IN_WRONG_STATE         21
#define CHANNEL_NOT_OPENED             22
#define CHANNEL_ID_NOT_SET             24
#define TRANSFER_IN_PROGRESS           31
#define TRANSFER_SEQUENCE_NUMBER_ERROR 32
#define INVALID_MESSAGE                40
#define INVALID_NETWORK_NUMBER         41

// other ANT stuff
#define ANT_SYNC_BYTE        0xA4
#define ANT_MAX_LENGTH       9
#define ANT_KEY_LENGTH       8
#define ANT_MAX_BURST_DATA   8
#define ANT_MAX_MESSAGE_SIZE 12
#define ANT_MAX_CHANNELS     8

// ANT message structure.
#define ANT_OFFSET_SYNC            0
#define ANT_OFFSET_LENGTH          1
#define ANT_OFFSET_ID              2
#define ANT_OFFSET_DATA            3
#define ANT_OFFSET_CHANNEL_NUMBER  3
#define ANT_OFFSET_MESSAGE_ID      4
#define ANT_OFFSET_MESSAGE_CODE    5

// ANT messages
#define ANT_UNASSIGN_CHANNEL   0x41
#define ANT_ASSIGN_CHANNEL     0x42
#define ANT_CHANNEL_ID         0x51
#define ANT_CHANNEL_PERIOD     0x43
#define ANT_SEARCH_TIMEOUT     0x44
#define ANT_CHANNEL_FREQUENCY  0x45
#define ANT_SET_NETWORK        0x46
#define ANT_TX_POWER           0x47
#define ANT_ID_LIST_ADD        0x59
#define ANT_ID_LIST_CONFIG     0x5A
#define ANT_CHANNEL_TX_POWER   0x60
#define ANT_LP_SEARCH_TIMEOUT  0x63
#define ANT_SET_SERIAL_NUMBER  0x65
#define ANT_ENABLE_EXT_MSGS    0x66
#define ANT_ENABLE_LED         0x68
#define ANT_SYSTEM_RESET       0x4A
#define ANT_OPEN_CHANNEL       0x4B
#define ANT_CLOSE_CHANNEL      0x4C
#define ANT_OPEN_RX_SCAN_CH    0x5B
#define ANT_REQ_MESSAGE        0x4D
#define ANT_BROADCAST_DATA     0x4E
#define ANT_ACK_DATA           0x4F
#define ANT_BURST_DATA         0x50
#define ANT_CHANNEL_EVENT      0x40
#define ANT_CHANNEL_STATUS     0x52
#define ANT_CHANNEL_ID         0x51
#define ANT_VERSION            0x3E
#define ANT_CAPABILITIES       0x54
#define ANT_SERIAL_NUMBER      0x61
#define ANT_NOTIF_STARTUP      0x6F
#define ANT_CW_INIT            0x53
#define ANT_CW_TEST            0x48



class ANTMessage
{
public:
    ANTMessage();
    ANTMessage(const unsigned char * message);
    ANTMessage(unsigned char len,
               unsigned char type = '\0',
               unsigned char b3 = '\0',
               unsigned char b4 = '\0',
               unsigned char b5 = '\0',
               unsigned char b6 = '\0',
               unsigned char b7 = '\0',
               unsigned char b8 = '\0',
               unsigned char b9 = '\0',
               unsigned char b10 = '\0',
               unsigned char b11 = '\0'); // encode with values (at least one value must be passed though)

    static ANTMessage assignChannel(const unsigned char channel,
                                    const unsigned char type,
                                    const unsigned char network);

    static ANTMessage setChannelID(const unsigned char channel,
                                   const unsigned short device,
                                   const unsigned char devicetype,
                                   const unsigned char txtype);

    static ANTMessage setChannelPeriod(const unsigned char channel,
                                       const unsigned short period);

    static ANTMessage setChannelFreq(const unsigned char channel,
                                     const unsigned char frequency);

    static ANTMessage open(const unsigned char channel);

    unsigned char data[ANT_MAX_MESSAGE_SIZE+1]; // include sync byte at front
    int length;
    uint8_t sync, type;
};

#endif // ANTMESSAGE_H
