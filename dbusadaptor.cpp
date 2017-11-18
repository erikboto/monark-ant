#include "dbusadaptor.h"

DBusAdaptor::DBusAdaptor(MonarkConnection *obj) : QDBusAbstractAdaptor(obj),
    m_monark(obj)
{

}

quint8 DBusAdaptor::mode()
{
    return m_mode;
}

void DBusAdaptor::setMode(quint8 mode)
{
    if (m_mode != mode)
    {
        m_mode = mode;
        emit modeChanged(m_mode);
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
