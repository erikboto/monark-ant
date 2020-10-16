#include "filelogger.h"

#include <QDateTime>
#include <QTextStream>

FileLogger::FileLogger(QObject *parent) : QObject(parent)
{
    QString tstamp = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    m_file.setFileName(QString("monarklog-%1").arg(tstamp));
    m_file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream ts(&m_file);
    ts << "kp;cadence;power\n";


}

void FileLogger::onKp(double kp)
{
    m_kp = kp;
}

void FileLogger::onPower(quint16 power)
{
    m_power = power;
}

void FileLogger::onCadence(quint8 cadence)
{
    m_cadence = cadence;

    QTextStream ts(&m_file);
    ts << QString("%1;%2;%3\n").arg(QString::number(m_kp, 'f', 1)).arg(m_cadence).arg(m_power);
    m_file.flush();
}
