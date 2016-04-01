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
