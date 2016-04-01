#ifndef ANTDEVICE_H
#define ANTDEVICE_H

#include <QtGlobal>

class ANTDevice
{
public:
    ANTDevice();
    virtual int channel() const = 0;
    virtual void channelEvent(unsigned char *ant_message) = 0;
    virtual void handleAckData(unsigned char *ant_message) = 0;
    virtual void configureChannel() = 0;
    virtual void setCurrentPower(quint16 power) = 0;
    virtual void setCurrentCadence(quint8 cadence) = 0;
};

#endif // ANTDEVICE_H
