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

ANTMessage ANTMessage::hrmData(const unsigned char channel, const unsigned char hr)
{

//    channel = message[3];
//    measurementTime = message[8] + (message[9]<<8);
//    heartrateBeats =  message[10];
//    instantHeartrate = message[11];
    // heartrate     heart_rate          0x4e,channel,None,None,None,None,uint16_le_diff:measurement_time,
    //                                                                    uint8_diff:beats,uint8:instant_heart_rate
    static unsigned char specialpagenr = 2;
    static int count = 0;
    static unsigned char togglebit = 0;
    static unsigned char hrcount = 0;

    static uint16_t time = 0;

    ANTMessage m;

    qDebug() << "count: " << count;

    bool specialpage = ((count%65)==0);

    if (count++%4 == 0)
        togglebit ^= 0x80;
    else
        togglebit ^= 0x0;

    if (!specialpage)
    {
        m = ANTMessage(9, ANT_BROADCAST_DATA, channel,  togglebit, 0xff,0xff,0xff, (time & 0xFF), (time & 0xFF00) >> 8, hrcount += (1) , hr+(count%4));
    }
    else
    {
        switch (specialpagenr)
        {
        case 1:
            // dont implement
            break;
        case 2:
            // page2
            specialpagenr++;
            m = ANTMessage(9, ANT_BROADCAST_DATA,channel,  togglebit | 0x02, 0x06,0x06,0x06, (time & 0xFF), (time & 0xFF00) >> 8, hrcount += (1) , hr);
            break;
        case 3:
            //page3
            specialpagenr=2;
            m = ANTMessage(9, ANT_BROADCAST_DATA,channel,  togglebit | 0x03, 0x07,0x07,0x07, (time & 0xFF), (time & 0xFF00) >> 8, hrcount += (1) , hr);
            break;
        }
    }

    time += 100;

    return m;
}
