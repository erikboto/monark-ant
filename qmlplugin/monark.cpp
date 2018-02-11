#include "monark.h"

Monark::Monark(QQuickItem *parent):
    QQuickItem(parent),
    m_currentPower(0),
    m_currentCadence(0)
{

    m_mci = new SeUnixshellMonarkControlInterface("se.unixshell", "/Monark", QDBusConnection::sessionBus(), this);
    connect(m_mci, &SeUnixshellMonarkControlInterface::currentPower, this, &Monark::updateCurrentPower);
    connect(m_mci, &SeUnixshellMonarkControlInterface::currentCadence, this, &Monark::updateCurrentCadence);
    connect(m_mci, &SeUnixshellMonarkControlInterface::modeChanged, this, &Monark::modeChanged);
}

Monark::~Monark()
{
}

unsigned int Monark::currentPower()
{
    return m_currentPower;
}

unsigned int Monark::currentCadence()
{
    return m_currentCadence;
}

void Monark::updateCurrentPower(ushort newCurrentPower)
{
    if (newCurrentPower != m_currentPower)
    {
        m_currentPower = newCurrentPower;
        emit currentPowerChanged();
    }
}

void Monark::updateCurrentCadence(unsigned char newCurrentCadence)
{
    if (newCurrentCadence != m_currentCadence)
    {
        m_currentCadence = newCurrentCadence;
        emit currentCadenceChanged();
    }
}

Monark::Mode Monark::mode()
{
    Mode m;
    switch(m_mci->mode()) {
    case 0:
        m = Mode::Kp;
        break;
    case 1:
        m = Mode::Power;
        break;
    default:
        qWarning() << "Got invalid mode from Monark-ANT";
    }
    return m;
}

void Monark::setMode(Mode newMode)
{
    m_mci->setMode((unsigned char)newMode);
}
