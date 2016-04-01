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
    explicit PowerDevice(LibUsb * usb, const unsigned char channel, QObject *parent = 0);

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
};

#endif // POWERDEVICE_H
