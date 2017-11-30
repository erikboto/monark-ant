#include "gearsimulator.h"

#define BASE_KP 2.5

GearSimulator::GearSimulator(MonarkConnection *mc) :
    m_monark(mc),
    m_grade(0),
    m_kpOffset(0)
{
}

void GearSimulator::incGear()
{
    m_kpOffset += 0.3;
    recalculateKp();
}

void GearSimulator::decGear()
{
    m_kpOffset -= 0.3;
    recalculateKp();
}

void GearSimulator::onGradeChanged(double grade)
{
    m_grade = grade;
    recalculateKp();
}

void GearSimulator::recalculateKp()
{
    m_monark->setKp(BASE_KP + m_grade + m_kpOffset);
}
