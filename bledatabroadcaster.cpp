#include "bledatabroadcaster.h"

#include <QDataStream>

BLEDataBroadcaster::BLEDataBroadcaster(unsigned short devId, QObject *parent) : QObject(parent),
    m_cpsClientConfig(QLowEnergyDescriptorData(QBluetoothUuid::ClientCharacteristicConfiguration,
                                               QByteArray(2,0))),
    m_power(0),
    m_cadence(0),
    m_heartRate(0),
    m_hrClientConfig(QLowEnergyDescriptorData(QBluetoothUuid::ClientCharacteristicConfiguration,
                                              QByteArray(2,0)))
{

    m_controller = QLowEnergyController::createPeripheral();

    // Set up HR
    m_hrChar.setUuid(QBluetoothUuid::HeartRateMeasurement);
    m_hrChar.setValue(QByteArray(2, 0));
    m_hrChar.setProperties(QLowEnergyCharacteristic::Notify);
    m_hrChar.addDescriptor(m_hrClientConfig);

    m_hrServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    m_hrServiceData.setUuid(QBluetoothUuid::HeartRate);
    m_hrServiceData.addCharacteristic(m_hrChar);

    m_hrService = m_controller->addService(m_hrServiceData);

    connect(&m_updateTimer, &QTimer::timeout, this, &BLEDataBroadcaster::transmitHrMeasurement);

    // Set up CPS
    m_cpsMeasurementChar.setUuid(QBluetoothUuid::CyclingPowerMeasurement);
    m_cpsMeasurementChar.setValue(QByteArray(4,0));
    m_cpsMeasurementChar.setProperties(QLowEnergyCharacteristic::Notify);
    m_cpsMeasurementChar.addDescriptor(m_cpsClientConfig);

    m_cpsFeatureChar.setUuid(QBluetoothUuid::CyclingPowerFeature);
    QByteArray value;
    QDataStream ds(&value, QIODevice::ReadWrite);
    ds.setByteOrder(QDataStream::LittleEndian);
    //                10987654321098765432109876543210
    quint32 flags = 0b00000000000000000000000000000000;
    ds << flags;
    m_cpsFeatureChar.setValue(value);
    m_cpsFeatureChar.setProperties(QLowEnergyCharacteristic::Read);

    m_cpsSensorLocationChar.setUuid(QBluetoothUuid::SensorLocation);
    m_cpsSensorLocationChar.setValue(QByteArray(1,15)); //rear hub?
    m_cpsSensorLocationChar.setProperties(QLowEnergyCharacteristic::Read);

    m_cpsServiceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    m_cpsServiceData.setUuid(QBluetoothUuid::CyclingPower);
    m_cpsServiceData.addCharacteristic(m_cpsMeasurementChar);
    m_cpsServiceData.addCharacteristic(m_cpsFeatureChar);
    m_cpsServiceData.addCharacteristic(m_cpsSensorLocationChar);

    m_cpsService = m_controller->addService(m_cpsServiceData);
    connect(&m_updateTimer, &QTimer::timeout, this, &BLEDataBroadcaster::transmitCpsMeasurement);


    // Set up advertising data, services and controllers.
    QString localName = QString("MonarkBLE %1").arg(devId);
    m_advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    m_advertisingData.setIncludePowerLevel(true);
    m_advertisingData.setLocalName(localName);
    m_advertisingData.setServices(QList<QBluetoothUuid>() << QBluetoothUuid::HeartRate << QBluetoothUuid::CyclingPower);

    m_controller->startAdvertising(QLowEnergyAdvertisingParameters(),
                                   m_advertisingData,
                                   m_advertisingData);

    m_updateTimer.setInterval(500);
    m_updateTimer.start();

    // Fix readvertisement after disconnect
    auto restartAdvertising = [this]() {
        this->m_controller->startAdvertising(QLowEnergyAdvertisingParameters(),
                                       this->m_advertisingData,
                                       this->m_advertisingData);
    };
    QObject::connect(m_controller, &QLowEnergyController::disconnected, restartAdvertising);

}

void BLEDataBroadcaster::transmitCpsMeasurement()
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

    ds << flags << m_power << accumulatedCrankRevs << lastCrankEventTime;
    QLowEnergyCharacteristic measChar = m_cpsService->characteristic(QBluetoothUuid::CyclingPowerMeasurement);
    m_cpsService->writeCharacteristic(measChar, value);
}

void BLEDataBroadcaster::transmitHrMeasurement()
{
    QByteArray value;
    value.append(char(0)); // flags
    value.append(char(m_heartRate));
    QLowEnergyCharacteristic characteristic = m_hrService->characteristic(QBluetoothUuid::HeartRateMeasurement);

    m_hrService->writeCharacteristic(characteristic, value);
}

void BLEDataBroadcaster::setCadence(quint8 cadence)
{
    m_cadence = cadence;
}

void BLEDataBroadcaster::setPower(qint16 power)
{
    m_power = power;
}

void BLEDataBroadcaster::setHeartRate(quint8 heartRate)
{
    m_heartRate = heartRate;
}
