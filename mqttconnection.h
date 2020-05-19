#ifndef MQTTCONNECTION_H
#define MQTTCONNECTION_H

#include <QObject>
#include <QMqttClient>
#include "MonarkConnection.h"

class MqttConnection : public QObject
{
    Q_OBJECT
public:
    explicit MqttConnection(const QString &name,
                            MonarkConnection * mc,
                            QObject *parent = nullptr);

public slots:
    void onCurrentPowerChanged(quint16 newPower);
    void onCurrentKpChanged(double newKp);
    void onCurrentCadenceChanged(quint8 newCadence);
    void onCurrentHeartrateChanged(quint8 newHeartrate);
    void onTargetPowerChanged(unsigned int power);
    void onTargetKpChanged(double kp);
    void onTypeIdentified();
    void onModeChanged(MonarkConnection::MonarkMode newMode);

    void onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic);

private:
    QString m_name;
    MonarkConnection * m_mc;
    QMqttClient m_client;

    QMqttTopicFilter m_targetValuesFilter{"monark/target/set/#"};
    QMqttTopicFilter m_deviceValuesFilter{"monark/details/set/#"};
};

#endif // MQTTCONNECTION_H
