#include <QDebug>
#include "mqttconnection.h"
#include "MonarkConnection.h"

#define TARGET_POWER "monark/target/power"
#define TARGET_SET_POWER "monark/target/set/power"

#define TARGET_KP "monark/target/kp"
#define TARGET_SET_KP "monark/target/set/kp"

#define DETAILS_MODE "monark/details/mode"
#define DETAILS_SET_MODE "monark/details/set/mode"

#define DETAILS_NAME "monark/details/name"
#define DETAILS_MODEL "monark/details/model"
#define CURRENT_POWER "monark/current/power"
#define CURRENT_KP "monark/current/kp"
#define CURRENT_CADENCE "monark/current/cadence"
#define CURRENT_HEARTRATE "monark/current/heartrate"

#define GEARSHIFT "monark/control/gearshift"

MqttConnection::MqttConnection(const QString &name,
                               MonarkConnection *mc,
                               DBusAdaptor * da,
                               QObject *parent) : QObject(parent),
  m_name(name),
  m_mc(mc),
  m_da(da)
{

    // Setup connections
    connect(mc, &MonarkConnection::power, this, &MqttConnection::onCurrentPowerChanged);
    //connect(mc, &MonarkConnection::kp, this, &MqttConnection::onCurrentKpChanged);
    connect(mc, &MonarkConnection::cadence, this, &MqttConnection::onCurrentCadenceChanged);
    connect(mc, &MonarkConnection::pulse, this, &MqttConnection::onCurrentHeartrateChanged);
    connect(mc, &MonarkConnection::modeChanged, this, &MqttConnection::onModeChanged);
    connect(&m_client, &QMqttClient::messageReceived, this, &MqttConnection::onMqttMessageReceived);
    connect(mc, &MonarkConnection::typeIdentified, this, &MqttConnection::onTypeIdentified);

    // Handle connect/disconnect
    connect(&m_client, &QMqttClient::connected, [this](){
        qDebug() << "MqttClient: connected";

        // Publish some retained info messages
        onTypeIdentified();
        onModeChanged(m_mc->mode());
        onTargetKpChanged(m_mc->kp());
        onTargetPowerChanged(m_mc->load());
        m_client.publish(QMqttTopicName(DETAILS_NAME),
                         QString(m_name).toLocal8Bit(),
                         1, // qos
                         true); // retain

        // Setup subscriptions
        m_client.subscribe(m_targetValuesFilter);
        m_client.subscribe(m_deviceValuesFilter);
        m_client.subscribe(QMqttTopicFilter("monark/control/gearshift"));

    });

    connect(&m_client, &QMqttClient::disconnected, [this](){
        qDebug() << "MqttClient: disconnected - reconnecting";
        m_client.connectToHost();
    });

    // Setup broken and connect
    m_client.setPort(1883);
    m_client.setHostname("localhost");
    m_client.setClientId(m_name);
    m_client.connectToHost();
}

void MqttConnection::onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    if (topic.name() == TARGET_SET_POWER)
    {
        bool ok;
        quint16 newTargetPower = QString::fromLocal8Bit(message).toUInt(&ok);
        if (ok)
        {
            m_mc->setLoad(newTargetPower);
        }
    }

    if (topic.name() == TARGET_SET_KP)
    {
        bool ok;
        double newTargetKp = QString::fromLocal8Bit(message).toDouble(&ok);
        if (ok)
        {
            m_mc->setKp(newTargetKp);
        }
    }

    if (topic.name() == DETAILS_SET_MODE)
    {
        if (QString::fromLocal8Bit(message) == "power")
        {
            m_mc->setMode(MonarkConnection::MONARK_MODE_WATT);
        }
        else if (QString::fromLocal8Bit(message) == "kp")
        {
            m_mc->setMode(MonarkConnection::MONARK_MODE_KP);
        }
    }

    if (topic.name() == GEARSHIFT)
    {
        qDebug() << "GEARSHIFT" << QString::fromLocal8Bit(message);
        if (QString::fromLocal8Bit(message) == "inc")
        {
            m_da->incGear();
        } else if (QString::fromLocal8Bit(message) == "inc_lots")
        {
            m_da->incGearLots();
        } else if (QString::fromLocal8Bit(message) == "dec")
        {
            m_da->decGear();
        } else if (QString::fromLocal8Bit(message) == "dec_lots")
        {
            m_da->decGearLots();
        }
    }
}

void MqttConnection::onCurrentKpChanged(double newKp)
{
    m_client.publish(QMqttTopicName(CURRENT_KP),
                     QString::number(newKp, 'f', 1).toLocal8Bit());
}

void MqttConnection::onCurrentPowerChanged(quint16 newPower)
{
    m_client.publish(QMqttTopicName(CURRENT_POWER),
                     QString::number(newPower).toLocal8Bit());
}

void MqttConnection::onCurrentCadenceChanged(quint8 newCadence)
{
    m_client.publish(QMqttTopicName(CURRENT_CADENCE),
                     QString::number(newCadence).toLocal8Bit());
}

void MqttConnection::onCurrentHeartrateChanged(quint8 newHeartrate)
{
    m_client.publish(QMqttTopicName(CURRENT_HEARTRATE),
                     QString::number(newHeartrate).toLocal8Bit());
}

void MqttConnection::onModeChanged(MonarkConnection::MonarkMode newMode)
{
    m_client.publish(QMqttTopicName(DETAILS_MODE),
                     newMode == MonarkConnection::MONARK_MODE_WATT ? QString("power").toLocal8Bit() : QString("kp").toLocal8Bit(),
                     1, // qos
                     true); // retain

}

void MqttConnection::onTypeIdentified()
{
    QString model;
    switch (m_mc->type()) {
    case MonarkConnection::MONARK_LC:
        model = "Monark LCx";
        break;
    case MonarkConnection::MONARK_LT2:
        model = "Monark LT2";
        break;
    case MonarkConnection::MONARK_839E:
        model = "Monark 839e";
        break;
    case MonarkConnection::MONARK_LC_NOVO:
        model = "Monark LC Novo";
        break;
    case MonarkConnection::MONARK_UNKNOWN:
        model = "Unknown";
        break;
    }

    if (!model.isEmpty())
    {
        m_client.publish(QMqttTopicName(DETAILS_MODEL),
                         model.toLocal8Bit(),
                         1, // qos
                         true); // retain
    }
}

void MqttConnection::onTargetKpChanged(double kp)
{
    m_client.publish(QMqttTopicName(TARGET_KP),
                     QString::number(kp, 'f', 1).toLocal8Bit(),
                     1, // qos
                     true); // retain
}

void MqttConnection::onTargetPowerChanged(unsigned int power)
{
    m_client.publish(QMqttTopicName(TARGET_POWER),
                     QString::number(power).toLocal8Bit(),
                     1, // qos
                     true); // retain
}
