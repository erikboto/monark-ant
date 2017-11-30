#ifndef GEARSIMULATOR_H
#define GEARSIMULATOR_H

#include <QObject>
#include <MonarkConnection.h>

class GearSimulator : public QObject
{
    Q_OBJECT
public:
    explicit GearSimulator(MonarkConnection *mc);

signals:

public slots:
    void incGear();
    void decGear();
    void onGradeChanged(double grade);

private:
    void recalculateKp();

    MonarkConnection *m_monark;
    double m_grade;
    double m_kpOffset;
};

#endif // GEARSIMULATOR_H
