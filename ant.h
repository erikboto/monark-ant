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
    ANT();

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

signals:
    void newTargetPower(quint32 targetPower);

};

#endif // ANT_H
