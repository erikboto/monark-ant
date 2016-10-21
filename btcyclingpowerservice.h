#ifndef BTCYCLINGPOWERSERVICE_H
#define BTCYCLINGPOWERSERVICE_H

#include <QObject>
#include <QLowEnergyAdvertisingData>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QLowEnergyServiceData>
#include <QLowEnergyCharacteristicData>
#include <QLowEnergyDescriptorData>
#include <QLowEnergyAdvertisingParameters>
#include <QTimer>

class BTCyclingPowerService : public QObject
{
    Q_OBJECT
public:
    explicit BTCyclingPowerService(QObject *parent = 0);

signals:

public slots:
    void setPower(qint16 power);
    void setCadence(quint8 cadence);

private:
    QLowEnergyCharacteristicData m_featureChar;
    QLowEnergyCharacteristicData m_measurementChar;
    QLowEnergyCharacteristicData m_sensorLocationChar;

    QLowEnergyServiceData m_serviceData;
    QLowEnergyAdvertisingData m_advertisingData;
    QLowEnergyController *m_controller;
    QLowEnergyService *m_service;

    QLowEnergyDescriptorData m_clientConfig;

    QTimer m_updateTimer;

    qint16 m_power;
    quint8 m_cadence;

private slots:
    void transmitMeasurement();
};

#endif // BTCYCLINGPOWERSERVICE_H
