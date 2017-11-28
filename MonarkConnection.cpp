/*
 * Copyright (c) 2015 Erik Botö (erik.boto@gmail.com)
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

#include "MonarkConnection.h"

#include <QByteArray>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>

MonarkConnection::MonarkConnection() :
    m_serial(0),
    m_pollInterval(1000),
    m_timer(0),
    m_load(0),
    m_loadToWrite(0),
    m_power(0),
    m_cadence(0),
    m_pulse(0)
{
}

void MonarkConnection::setSerialPort(const QString serialPortName)
{
    if (! this->isRunning())
    {
        m_serialPortName = serialPortName;
    } else {
        qWarning() << "MonarkConnection: Cannot set serialPortName while running";
    }
}

void MonarkConnection::setPollInterval(int interval)
{
    if (interval != m_pollInterval)
    {
        m_pollInterval = interval;
        m_timer->setInterval(m_pollInterval);
    }
}

int MonarkConnection::pollInterval()
{
    return m_pollInterval;
}

/**
 * Private function that reads a complete reply and prepares if for
 * processing by replacing \r with \0
 */
QString MonarkConnection::readAnswer(int timeoutMs)
{
    QByteArray data;

    do
    {
        if (m_serial->waitForReadyRead(timeoutMs))
        {
            data.append(m_serial->readAll());
        } else {
            data.append('\r');
        }
    } while (data.indexOf('\r') == -1);

    data.replace("\r", "\0");
    return QString(data);
}

/**
 * QThread::run()
 *
 * Open the serial port and set it up, then starts polling.
 *
 */
void MonarkConnection::run()
{
    // Open and configure serial port
    m_serial = new QSerialPort();

    m_startupTimer = new QTimer();
    m_startupTimer->setInterval(200);
    m_startupTimer->setSingleShot(true);
    m_startupTimer->start();

    m_timer = new QTimer();

    connect(m_startupTimer, SIGNAL(timeout()), this, SLOT(identifySerialPort()), Qt::DirectConnection);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(requestAll()), Qt::DirectConnection);

    qDebug() << "Started Monark Thread";
    exec();
}

void MonarkConnection::requestAll()
{
    // If something else is blocking mutex, don't start another round of requests
    if (! m_mutex.tryLock())
        return;

    requestPower();
    requestPulse();
    requestCadence();

    if ((m_loadToWrite != m_load) && m_mode == MONARK_MODE_WATT && canDoLoad())
    {
        QString cmd = QString("power %1\r").arg(m_loadToWrite);
        m_serial->write(cmd.toStdString().c_str());
        if (!m_serial->waitForBytesWritten(500))
        {
            // failure to write to device, bail out
            emit connectionStatus(false);
            m_startupTimer->start();
        }
        m_load = m_loadToWrite;
        QByteArray data = m_serial->readAll();
    }

    if ((m_kpToWrite != m_kp) && m_mode == MONARK_MODE_KP && canDoKp())
    {
        QString cmd = QString("kp %1\r").arg(QString::number(m_kpToWrite, 'f', 1 ));
        m_serial->write(cmd.toStdString().c_str());
        if (!m_serial->waitForBytesWritten(500))
        {
            // failure to write to device, bail out
            emit connectionStatus(false);
            m_startupTimer->start();
        }
        m_kp = m_kpToWrite;
        QByteArray data = m_serial->readAll();
    }

    if ((m_mode == MONARK_MODE_KP) && canDoLoad() && !canDoKp())
    {
        // Calculate what wattage to request to simulate selected kp
        // watt = kp * cadence * 0.98
        unsigned int load = (m_kpToWrite * m_cadence) * 0.98;

        QString cmd = QString("power %1\r").arg(load);
        m_serial->write(cmd.toStdString().c_str());
        if (!m_serial->waitForBytesWritten(500))
        {
            // failure to write to device, bail out
            emit connectionStatus(false);
            m_startupTimer->start();
        }
        QByteArray data = m_serial->readAll();
    }

    m_mutex.unlock();
}

void MonarkConnection::requestPower()
{
    // Always empty read buffer first
    m_serial->readAll();

    m_serial->write("power\r");
    if (!m_serial->waitForBytesWritten(500))
    {
        // failure to write to device, bail out
        emit connectionStatus(false);
        m_startupTimer->start();
    }
    QString data = readAnswer(500);
    m_power = data.toInt();
    emit power(m_power);
}

void MonarkConnection::requestPulse()
{
    // Always empty read buffer first
    m_serial->readAll();

    m_serial->write("pulse\r");
    if (!m_serial->waitForBytesWritten(500))
    {
        // failure to write to device, bail out
        emit connectionStatus(false);
        m_startupTimer->start();
    }
    QString data = readAnswer(500);
    m_pulse = data.toInt();
    emit pulse(m_pulse);
}

void MonarkConnection::requestCadence()
{
    // Always empty read buffer first
    m_serial->readAll();

    m_serial->write("pedal\r");
    if (!m_serial->waitForBytesWritten(500))
    {
        // failure to write to device, bail out
        emit connectionStatus(false);
        m_startupTimer->start();
    }
    QString data = readAnswer(500);
    m_cadence = data.toInt();
    emit cadence(m_cadence);
}

void MonarkConnection::identifyModel()
{
    m_serial->write("id\r");
    if (!m_serial->waitForBytesWritten(500))
    {
        // failure to write to device, bail out
        emit connectionStatus(false);
        m_startupTimer->start();
    }
    QString data = readAnswer(500);
    m_id = QString(data);

    if (m_id.toLower().startsWith("lc"))
    {
        m_type = MONARK_LC;
    } else if (m_id.toLower().startsWith("novo")) {
        m_type = MONARK_LC_NOVO;
    } else if (m_id.toLower().startsWith("mec")) {
        m_type = MONARK_839E;
    } else if (m_id.toLower().startsWith("lt")) {
        m_type = MONARK_LT2;
    }

    if (canDoLoad())
    {
        setLoad(100);
    }

    qDebug() << "Connected to bike: " << m_id;
}

void MonarkConnection::setLoad(unsigned int load)
{
    m_loadToWrite = load;

    if (m_mode != MONARK_MODE_WATT)
    {
        m_mode = MONARK_MODE_WATT;
        emit modeChanged(m_mode);
    }
}

void MonarkConnection::setKp(double kp)
{
    if (kp < 0)
        kp = 0;

    if (kp > 7)
        kp = 7;

    m_kpToWrite = kp;

    if (m_mode != MONARK_MODE_KP)
    {
        m_mode = MONARK_MODE_KP;
        emit modeChanged(m_mode);
    }
}

/*
 * Configures a serialport for communicating with a Monark bike.
 */
void MonarkConnection::configurePort(QSerialPort *serialPort)
{
    if (!serialPort)
    {
        qFatal("Trying to configure null port, start debugging.");
    }
    serialPort->setBaudRate(QSerialPort::Baud4800);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::SoftwareControl);
    serialPort->setParity(QSerialPort::NoParity);

    // Send empty \r after configuring port, otherwise first command might not
    // be interpreted correctly
    serialPort->write("\r");
}

/**
 * This functions takes a serial port and tries if it can find a Monark bike connected
 * to it.
 */
bool MonarkConnection::discover(QString portName)
{
    bool found = false;
    QSerialPort sp;

    sp.setPortName(portName);

    if (sp.open(QSerialPort::ReadWrite))
    {
        configurePort(&sp);

        // Discard any existing data
        QByteArray data = sp.readAll();

        // Read id from bike
        sp.write("id\r");
        sp.waitForBytesWritten(2000);

        QByteArray id;
        do
        {
            bool readyToRead = sp.waitForReadyRead(1000);
            if (readyToRead)
            {
                id.append(sp.readAll());
            } else {
                id.append('\r');
            }
        } while ((id.indexOf('\r') == -1));

        id.replace("\r", "\0");

        // Should check for all bike ids known to use this protocol
        if (QString(id).toLower().contains("lt") ||
            QString(id).toLower().contains("lc") ||
            QString(id).toLower().contains("mec") ||
            QString(id).toLower().contains("novo")) {
            found = true;
        }

        qDebug() << "Connected to bike: " << id;
    }

    sp.close();

    return found;
}

void MonarkConnection::identifySerialPort()
{
    qDebug() << __func__;

    bool found = false;

    // Make sure the port is closed to start with
    m_serial->close();

    m_timer->stop();

    do {
        qDebug() << "Refreshing list of serial ports...";

        // Look for test interface

        if (discover("/tmp/monark-test"))
        {
            // found monark
            qDebug() << "FOUND!";
            m_serialPortName = "/tmp/monark-test";
            found = true;
            break;
        }

        QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
        foreach (QSerialPortInfo port, ports)
        {
#ifdef RASPBERRYPI
            if (port.systemLocation() == "/dev/ttyAMA0")
                continue;
#endif
            qDebug() << "Looking for Monark at " << port.systemLocation();
            if (discover(port.systemLocation()))
            {
                // found monark
                qDebug() << "FOUND!";
                m_serialPortName = port.systemLocation();
                found = true;
                break;
            }
        }
        msleep(500);
    } while (!found);

    m_serial->setPortName(m_serialPortName);

    if (!m_serial->open(QSerialPort::ReadWrite))
    {
        qDebug() << "Error opening serial";
        m_startupTimer->start();
    } else {
        configurePort(m_serial);

        // Discard any existing data
        QByteArray data = m_serial->readAll();
    }

    identifyModel();

    m_timer->setInterval(1000);
    m_timer->start();

    emit connectionStatus(true);
}

bool MonarkConnection::canDoLoad()
{
    bool result = false;

    switch (m_type)
    {
    case MONARK_LC: // fall through
    case MONARK_LC_NOVO: // fall through
    case MONARK_839E:
        result = true;
        break;
    case MONARK_LT2: // fall through
    default:
        result = false;
        break;
    }

    return result;
}

bool MonarkConnection::canDoKp()
{
    bool result = false;

    switch (m_type)
    {
    case MONARK_839E: // fall through
    case MONARK_LC_NOVO:
        result = true;
        break;
    case MONARK_LC: // fall through
    case MONARK_LT2: // fall through
    default:
        result = false;
        break;
    }

    return result;
}

void MonarkConnection::setMode(MonarkMode mode)
{
    if (mode == m_mode)
    {
        return;
    }

    if (mode == MONARK_MODE_KP)
    {
        if (m_cadence != 0)
        {
            m_kpToWrite = m_power / m_cadence / 0.98;
        } else {
            m_kpToWrite = 1;
        }

        m_mode = MONARK_MODE_KP;
        emit modeChanged(m_mode);
    }

    if (mode == MONARK_MODE_WATT)
    {
        m_loadToWrite = m_power;
        m_mode = MONARK_MODE_WATT;
        emit modeChanged(m_mode);
    }
}
