#include "btcyclingpowerservice.h"

#include <QDataStream>

BTCyclingPowerService::BTCyclingPowerService(QObject *parent) : QObject(parent),
    m_clientConfig(QLowEnergyDescriptorData(QBluetoothUuid::ClientCharacteristicConfiguration,
                                            QByteArray(2,0))),
    m_power(50),
    m_cadence(67)
{
    m_measurementChar.setUuid(QBluetoothUuid::CyclingPowerMeasurement);
    m_measurementChar.setValue(QByteArray(4,0));
    m_measurementChar.setProperties(QLowEnergyCharacteristic::Notify);
    m_measurementChar.addDescriptor(m_clientConfig);

    m_featureChar.setUuid(QBluetoothUuid::CyclingPowerFeature);
    QByteArray value;
    QDataStream ds(&value, QIODevice::ReadWrite);
    ds.setByteOrder(QDataStream::LittleEndian);
    quint16 flags = 0b0000000000100000;
    ds << flags << m_power << (quint16)0 << (quint16)0;
    m_featureChar.setValue(value);
    m_featureChar.setProperties(QLowEnergyCharacteristic::Read);

    m_sensorLocationChar.setUuid(QBluetoothUuid::SensorLocation);
    m_sensorLocationChar.setValue(QByteArray(1,15)); //rear hub?
    m_sensorLocationChar.setProperties(QLowEnergyCharacteristic::Read);

    m_serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    m_serviceData.setUuid(QBluetoothUuid::CyclingPower);
    m_serviceData.addCharacteristic(m_measurementChar);
    m_serviceData.addCharacteristic(m_featureChar);
    m_serviceData.addCharacteristic(m_sensorLocationChar);

    m_advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    m_advertisingData.setIncludePowerLevel(true);
    m_advertisingData.setLocalName("MonarkPower");
    m_advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::CyclingPower);

    m_controller = QLowEnergyController::createPeripheral();
    m_service = m_controller->addService(m_serviceData);

    m_controller->startAdvertising(QLowEnergyAdvertisingParameters(),
                                   m_advertisingData,
                                   m_advertisingData);

    connect(&m_updateTimer, &QTimer::timeout, this, &BTCyclingPowerService::transmitMeasurement);
    m_updateTimer.setInterval(500);
    m_updateTimer.start();

    auto restartAdvertising = [this]() {
        this->m_controller->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       this->m_advertisingData,
                                       this->m_advertisingData);
    };
    QObject::connect(m_controller, &QLowEnergyController::disconnected, restartAdvertising);

}

void BTCyclingPowerService::transmitMeasurement()
{
    QByteArray value;
    QDataStream ds(&value, QIODevice::ReadWrite);
    ds.setByteOrder(QDataStream::LittleEndian);
    quint16 flags = 0b0000000000100000;

    static quint16 accumulatedCrankRevs = 0;
    static quint16 lastCrankEventTime = 0;
    static double crankResidue = 0;

    //                     left overs  +       revs/s         *         seconds passed
    double newCrankRevs = crankResidue + (double)m_cadence/60 * m_updateTimer.interval()/1000;
    crankResidue =  newCrankRevs - ((long)newCrankRevs);

    //                            seconds / rev       *        revs           * convert to 1/1024th s
    quint32 timeUpdate = (double)60/(double)m_cadence * (quint16)newCrankRevs * 1024;

    lastCrankEventTime += timeUpdate;
    accumulatedCrankRevs += newCrankRevs;

    ds << flags << m_power++ << accumulatedCrankRevs << lastCrankEventTime;
    QLowEnergyCharacteristic measChar = m_service->characteristic(QBluetoothUuid::CyclingPowerMeasurement);
    m_service->writeCharacteristic(measChar, value);
}

void BTCyclingPowerService::setCadence(quint8 cadence)
{
    m_cadence = cadence;
}

void BTCyclingPowerService::setPower(qint16 power)
{
    m_power = power;
}
