/*
 * Copyright (c) 2011 Darren Hague & Eric Brandt
 *               Modified to support Linux and OSX by Mark Liversedge
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

#include <QString>
#include <QDebug>

#ifndef Q_CC_MSVC
#include <unistd.h>
#endif
#include "LibUsb.h"

#define LINUX 0
#define WINDOWS 1
#define OSX 2

#ifdef Q_OS_MAC
#define OperatingSystem 2
#elif defined Q_OS_WIN32
#define OperatingSystem 1
#elif defined Q_OS_LINUX
#define OperatingSystem 0
#endif

LibUsb::LibUsb(int type) : type(type)
{

    intf = NULL;
    readBufIndex = 0;
    readBufSize = 0;

    // Initialize the library.
    usb_init();
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();
}

int LibUsb::open()
{
    // reset counters
    intf = NULL;
    readBufIndex = 0;
    readBufSize = 0;

    // Find all busses.
    usb_find_busses();

    // Find all connected devices.
    usb_find_devices();

    switch (type) {

    // Search USB busses for USB2 ANT+ stick host controllers
    default:
    case TYPE_ANT: device = OpenAntStick();
              break;
    }

    if (device == NULL) return -1;

    // Clear halt is needed, but ignore return code
    usb_clear_halt(device, writeEndpoint);
    usb_clear_halt(device, readEndpoint);

    return 0;
}

bool LibUsb::find()
{
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();

    switch (type) {

    // Search USB busses for USB2 ANT+ stick host controllers
    default:
    case TYPE_ANT: return findAntStick();
              break;
    }
}

void LibUsb::close()
{
    if (device) {
        // stop any further write attempts whilst we close down
        usb_dev_handle *p = device;
        device = NULL;

        usb_release_interface(p, interface);
        //usb_reset(p);
        usb_close(p);
    }
}

int LibUsb::read(char *buf, int bytes)
{
    // check it isn't closed already
    if (!device) return -1;

    // The USB2 stick really doesn't like you reading 1 byte when more are available
    // so we use a local buffered read
    int bufRemain = readBufSize - readBufIndex;

    // Can we entirely satisfy the request from the buffer?
    if (bufRemain >= bytes)
    {
        // Yes, so do it
        memcpy(buf, readBuf+readBufIndex, bytes);
        readBufIndex += bytes;
        return bytes;
    }

    // No, so partially satisfy by emptying the buffer, then refill the buffer for the rest
    // !!! CHECK bufRemain > 0 rather than non-zero fixes annoying bug with ANT+
    //     devices not working again after restart "sometimes"
    if (bufRemain > 0) {
        memcpy(buf, readBuf+readBufIndex, bufRemain);
    }

    readBufSize = 0;
    readBufIndex = 0;

    int rc = usb_bulk_read(device, readEndpoint, readBuf, 64, 125);
    if (rc < 0)
    {
        // don't report timeouts - lots of noise so commented out
        //qDebug()<<"usb_bulk_read Error reading: "<<rc<< usb_strerror();
        return rc;
    }
    readBufSize = rc;

    int bytesToGo = bytes - bufRemain;
    if (bytesToGo < readBufSize)
    {
        // If we have enough bytes in the buffer, return them
        memcpy(buf+bufRemain, readBuf, bytesToGo);
        readBufIndex += bytesToGo;
        rc = bytes;
    } else {
        // Otherwise, just return what we can
        memcpy(buf+bufRemain, readBuf, readBufSize);
        rc = bufRemain + readBufSize;
        readBufSize = 0;
        readBufIndex = 0;
    }

    return rc;
}

int LibUsb::write(char *buf, int bytes)
{
    // check it isn't closed
    if (!device) return -1;

    int rc;
    if (OperatingSystem == WINDOWS) {
        rc = usb_interrupt_write(device, writeEndpoint, buf, bytes, 1000);
    } else {
        // we use a non-interrupted write on Linux/Mac since the interrupt
        // write block size is incorrectly implemented in the version of
        // libusb we build with. It is no less efficient.
        rc = usb_bulk_write(device, writeEndpoint, buf, bytes, 125);
    }

    if (rc < 0)
    {
        // Report timeouts - previously we ignored -110 errors. This masked a serious
        // problem with write on Linux/Mac, the USB stick needed a reset to avoid
        // these error messages, so we DO report them now
        qDebug()<<"usb_interrupt_write Error writing ["<<rc<<"]: "<< usb_strerror();
    }

    return rc;
}



bool LibUsb::findAntStick()
{
    struct usb_bus* bus;
    struct usb_device* dev;
    bool found = false;
    for (bus = usb_get_busses(); bus; bus = bus->next) {

        for (dev = bus->devices; dev; dev = dev->next) {

            if (dev->descriptor.idVendor == GARMIN_USB2_VID && 
                (dev->descriptor.idProduct == GARMIN_USB2_PID || dev->descriptor.idProduct == GARMIN_OEM_PID)) {
                found = true;
            }
        }
    }
    return found;
}

struct usb_dev_handle* LibUsb::OpenAntStick()
{
    struct usb_bus* bus;
    struct usb_device* dev;
    struct usb_dev_handle* udev;

// for Mac and Linux we do a bus reset on it first...
#ifndef WIN32
    for (bus = usb_get_busses(); bus; bus = bus->next) {

        for (dev = bus->devices; dev; dev = dev->next) {

            if (dev->descriptor.idVendor == GARMIN_USB2_VID &&
                (dev->descriptor.idProduct == GARMIN_USB2_PID || dev->descriptor.idProduct == GARMIN_OEM_PID)) {

                if ((udev = usb_open(dev))) {
                    usb_reset(udev);
                    usb_close(udev);
                }
            }
        }
    }
#endif

    for (bus = usb_get_busses(); bus; bus = bus->next) {

        for (dev = bus->devices; dev; dev = dev->next) {

            if (dev->descriptor.idVendor == GARMIN_USB2_VID && 
                (dev->descriptor.idProduct == GARMIN_USB2_PID || dev->descriptor.idProduct == GARMIN_OEM_PID)) {

                //Avoid noisy output
                qDebug() << "Found a Garmin USB2 ANT+ stick" << bus->dirname << " " << dev->filename;

                if ((udev = usb_open(dev))) {

                    if (dev->descriptor.bNumConfigurations) {

                        if ((intf = usb_find_interface(&dev->config[0])) != NULL) {

#ifdef Q_OS_LINUX
                            usb_detach_kernel_driver_np(udev, interface);
#endif

                            int rc = usb_set_configuration(udev, 1);
                            if (rc < 0) {
                                qDebug()<<"usb_set_configuration Error: "<< usb_strerror();
                                if (OperatingSystem == LINUX) {
                                    // looks like the udev rule has not been implemented
                                    qDebug()<<"check permissions on:"<<QString("/dev/bus/usb/%1/%2").arg(bus->dirname).arg(dev->filename);
                                    qDebug()<<"did you remember to setup a udev rule for this device?";
                                }
                            }

                            rc = usb_claim_interface(udev, interface);
                            if (rc < 0) qDebug()<<"usb_claim_interface Error: "<< usb_strerror();

                            if (OperatingSystem != OSX) {
                                // fails on Mac OS X, we don't actually need it anyway
                                rc = usb_set_altinterface(udev, alternate);
                                if (rc < 0) qDebug()<<"usb_set_altinterface Error: "<< usb_strerror();
                            }

                            return udev;
                        }
                    }

                    usb_close(udev);
                }
            }
        }
    }
    return NULL;
}

struct usb_interface_descriptor* LibUsb::usb_find_interface(struct usb_config_descriptor* config_descriptor)
{

    struct usb_interface_descriptor* intf;

    readEndpoint = -1;
    writeEndpoint = -1;
    interface = -1;
    alternate = -1;

    if (!config_descriptor) return NULL;

    if (!config_descriptor->bNumInterfaces) return NULL;

    if (!config_descriptor->interface[0].num_altsetting) return NULL;

    intf = &config_descriptor->interface[0].altsetting[0];

    if (intf->bNumEndpoints != 2) return NULL;

    interface = intf->bInterfaceNumber;
    alternate = intf->bAlternateSetting;

    for (int i = 0 ; i < 2; i++)
    {
        if (intf->endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
            readEndpoint = intf->endpoint[i].bEndpointAddress;
        else
            writeEndpoint = intf->endpoint[i].bEndpointAddress;
    }

    if (readEndpoint < 0 || writeEndpoint < 0)
        return NULL;

    return intf;
}
