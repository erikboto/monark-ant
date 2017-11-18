#include "dbusadaptor.h"

DBusAdaptor::DBusAdaptor(MonarkConnection *obj) : QDBusAbstractAdaptor(obj),
    m_monark(obj)
{
    connect(m_monark, &MonarkConnection::modeChanged, this, &DBusAdaptor::modeChanged);
}

quint8 DBusAdaptor::mode()
{
    return m_monark->mode();
}

void DBusAdaptor::setMode(quint8 mode)
{
    if (m_monark->mode() != mode)
    {
        m_monark->setMode((MonarkConnection::MonarkMode)mode);
        emit modeChanged(mode);
    }
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
