#ifndef BLEDATABROADCASTER_H
#define BLEDATABROADCASTER_H

#include <QObject>
#include <QLowEnergyAdvertisingData>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyServiceData>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyDescriptorData>
#include <QLowEnergyAdvertisingParameters>
#include <QTimer>

class BLEDataBroadcaster : public QObject
{
    Q_OBJECT
public:
    explicit BLEDataBroadcaster(QObject *parent = 0);

signals:

public slots:
    void setPower(qint16 power);
    void setCadence(quint8 cadence);
    void setHeartRate(quint8 heartRate);

private:
    // For CyclingPowerService
    QLowEnergyCharacteristicData m_cpsFeatureChar;
    QLowEnergyCharacteristicData m_cpsMeasurementChar;
    QLowEnergyCharacteristicData m_cpsSensorLocationChar;
    QLowEnergyServiceData m_cpsServiceData;
    QLowEnergyDescriptorData m_cpsClientConfig;
    qint16 m_power;
    quint8 m_cadence;
    QLowEnergyService *m_cpsService;

    // For HeartRate
    quint8 m_heartRate;
    QLowEnergyCharacteristicData m_hrChar;
    QLowEnergyServiceData m_hrServiceData;
    QLowEnergyDescriptorData m_hrClientConfig;
    QLowEnergyService *m_hrService;

    // Common
    QLowEnergyAdvertisingData m_advertisingData;
    QLowEnergyController *m_controller;
    QTimer m_updateTimer;

private slots:
    void transmitCpsMeasurement();
    void transmitHrMeasurement();
};

#endif // BLEDATABROADCASTER_H
