#include "gearsimulator.h"

#define BASE_KP 0

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

void GearSimulator::incGearLots()
{
    m_kpOffset += 3;
    recalculateKp();
}

void GearSimulator::decGearLots()
{
    m_kpOffset -= 3;
    recalculateKp();
}

void GearSimulator::onGradeChanged(double grade)
{
    m_grade = grade;
    recalculateKp();
}

void GearSimulator::recalculateKp()
{
    m_monark->setKp(BASE_KP + gradeToKp(m_grade) + m_kpOffset);
}

double GearSimulator::gradeToKp(double grade)
{
    return (grade + 8)/4;
}
