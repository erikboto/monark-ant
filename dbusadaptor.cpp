#include "dbusadaptor.h"

DBusAdaptor::DBusAdaptor(MonarkConnection *obj, GearSimulator *gs) : QDBusAbstractAdaptor(obj),
    m_monark(obj),
    m_gearSimulator(gs)
{
    connect(m_monark, &MonarkConnection::modeChanged, this, &DBusAdaptor::modeChanged);
    connect(m_monark, &MonarkConnection::power, this, &DBusAdaptor::currentPower);
    connect(m_monark, &MonarkConnection::cadence, this, &DBusAdaptor::currentCadence);
}

quint8 DBusAdaptor::mode()
{
    return m_monark->mode();
}

void DBusAdaptor::setMode(quint8 mode)
{
    m_monark->setMode((MonarkConnection::MonarkMode)mode);
}

void DBusAdaptor::setTargetPower(quint16 targetPower)
{
    if (targetPower != m_monark->load())
    {
        m_monark->setLoad(targetPower);
        emit targetPowerChanged(targetPower);
    }
}

quint16 DBusAdaptor::targetPower()
{
    return m_monark->load();
}

void DBusAdaptor::setTargetKp(double targetKp)
{
    if (targetKp != m_monark->kp())
    {
        m_monark->setKp(targetKp);
        emit targetKpChanged(targetKp);
    }
}

double DBusAdaptor::targetKp()
{
    return m_monark->kp();
}

void DBusAdaptor::incGear()
{
    if (m_monark->isFecSimulation())
    {
        m_gearSimulator->incGear();
    } else if (m_monark->mode() == MonarkConnection::MONARK_MODE_KP){
        m_monark->setKp(m_monark->kp() + 0.3);
    } else {
        m_monark->setLoad(m_monark->load() + 5);
    }
}

void DBusAdaptor::decGear()
{
    if (m_monark->isFecSimulation())
    {
        m_gearSimulator->decGear();
    } else if (m_monark->mode() == MonarkConnection::MONARK_MODE_KP){
        m_monark->setKp(m_monark->kp() - 0.3);
    } else {
        m_monark->setLoad(m_monark->load() - 5);
    }
}

void DBusAdaptor::incGearLots()
{
    if (m_monark->isFecSimulation())
    {
        m_gearSimulator->incGearLots();
    } else if (m_monark->mode() == MonarkConnection::MONARK_MODE_KP){
        m_monark->setKp(m_monark->kp() + 2);
    } else {
        m_monark->setLoad(m_monark->load() + 100);
    }
}

void DBusAdaptor::decGearLots()
{
    if (m_monark->isFecSimulation())
    {
        m_gearSimulator->decGearLots();
    } else if (m_monark->mode() == MonarkConnection::MONARK_MODE_KP){
        m_monark->setKp(m_monark->kp() - 2);
    } else {
        m_monark->setLoad(m_monark->load() < 100 ? 0: m_monark->load() - 100);
    }
}
