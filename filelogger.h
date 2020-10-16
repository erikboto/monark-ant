#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QObject>
#include <QFile>

class FileLogger : public QObject
{
    Q_OBJECT
public:
    explicit FileLogger(QObject *parent = nullptr);

signals:
public slots:
    void onKp(double kp);
    void onPower(quint16 power);
    void onCadence(quint8 cadence);

private:
    QFile m_file;

    double m_kp;
    quint16 m_power;
    quint8 m_cadence;
};

#endif // FILELOGGER_H
