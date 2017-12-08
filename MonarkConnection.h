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

#ifndef _GC_MonarkConnection_h
#define _GC_MonarkConnection_h 1

#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <fecdevice.h>

class MonarkConnection : public QThread
{
    Q_OBJECT

public:
    MonarkConnection();
    void setPollInterval(int interval);
    int pollInterval();
    void setSerialPort(const QString serialPortName);
    static void configurePort(QSerialPort * serialPort);
    bool discover(QString portName);
    enum MonarkMode { MONARK_MODE_WATT=0, MONARK_MODE_KP=1 };
    enum MonarkType { MONARK_UNKNOWN, MONARK_LT2, MONARK_LC, MONARK_LC_NOVO, MONARK_839E };
    bool isFecSimulation();
    bool isFecErg();

public slots:
    void requestAll();
    void requestPower();
    void requestPulse();
    void requestCadence();
    void sendTargetWattOrKp();
    void identifyModel();
    void setLoad(unsigned int load);
    void setKp(double kp);
    void setMode(MonarkMode mode);
    unsigned int load() { return m_loadToWrite; }
    double kp() { return m_kp; }
    MonarkMode mode() { return m_mode; }
    MonarkType type() { return m_type; }
    void setFecMode(FECDevice::FecMode mode);

private:
    QString m_serialPortName;
    QSerialPort *m_serial;
    int m_pollInterval;
    QString m_id;
    void run();
    QTimer *m_timer;
    QString readAnswer(int timeoutMs = -1);
    QMutex m_mutex;
    unsigned int m_load;
    unsigned int m_loadToWrite;
    double m_kp;
    double m_kpToWrite;
    QTimer *m_startupTimer;

    MonarkType m_type;
    MonarkMode m_mode;
    bool canDoLoad();
    bool canDoKp();

    quint16 m_power;
    quint8 m_cadence;
    quint8 m_pulse;
    FECDevice::FecMode m_fecMode;

private slots:
    void identifySerialPort();
signals:
    void pulse(quint8);
    void cadence(quint8);
    void power(quint16);
    void connectionStatus(bool connected);
    void modeChanged(MonarkMode mode);
};

#endif // _GC_MonarkConnection_h
