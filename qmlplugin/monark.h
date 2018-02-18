#ifndef MONARK_H
#define MONARK_H

#include <QQuickItem>
#include "monarkcontrol.h"

class Monark : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(Monark)

    Q_PROPERTY(unsigned int currentPower READ currentPower NOTIFY currentPowerChanged)
    Q_PROPERTY(unsigned int currentCadence READ currentCadence NOTIFY currentCadenceChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(unsigned int targetPower READ targetPower WRITE setTargetPower NOTIFY targetPowerChanged)
    Q_PROPERTY(double targetKp READ targetKp WRITE setTargetKp NOTIFY targetKpChanged)

public:
    Monark(QQuickItem *parent = nullptr);
    ~Monark();
    unsigned int currentPower();
    unsigned int currentCadence();
    unsigned int targetPower();
    void setTargetPower(unsigned int newTargetPower);
    double targetKp();
    void setTargetKp(double newTargetKp);

    enum Mode {
        Power,
        Kp
    };
    Q_ENUM(Mode)

    Mode mode();
    void setMode(Mode newMode);

private:
    SeUnixshellMonarkControlInterface *m_mci;
    unsigned int m_currentPower;
    unsigned int m_currentCadence;

private slots:
    void updateCurrentPower(ushort newCurrentPower);
    void updateCurrentCadence(unsigned char newCurrentCadence);

signals:
    void currentPowerChanged();
    void currentCadenceChanged();
    void modeChanged();
    void targetPowerChanged();
    void targetKpChanged();
};

#endif // MONARK_H
