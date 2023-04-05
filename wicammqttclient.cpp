
#include <QUuid>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include <QDir>

#include "wicammqttclient.h"
#include "config.h"

WiCamMQTTClient::WiCamMQTTClient(QObject *parent) : QObject(parent)
  , m_connected(false), m_currSystemState(false), m_dataTransActive(false),
    m_dataTransProcess(false)
{
    m_client = new MQTTClient(this);
/*
    // MQTT Server info settings
    m_client->setHostAddress(mqtt_settings.hostName);
    m_client->setPort(mqtt_settings.port);
    m_client->setUsername(mqtt_settings.userName);
    m_client->setPassword(mqtt_settings.password);

    // Connection Settings and auto reconnection
    m_client->setAutoReconnect(true);
    m_client->setCleanSession(false);
    m_client->setClientID(QUuid::createUuid().toString());
    m_client->setKeepAliveInterval(MQTT_KEEP_ALIVE_INTERVAL_SEC);

    // Will Topic as availabilty of WiCam
    m_client->setWillEnabled(true);
    m_client->setWillQos(2);
    m_client->setWillRetain(true);
    m_client->setWillTopic(mqtt_settings.topic + MQTT_ONLINE_PREFIX);
    m_client->setWillMessage(MQTT_OFFLINE_MESSAGE);

    // Save the topic
    m_wiCamTopic = mqtt_settings.topic;
    */
    // Connect signals
    connect(m_client, &MQTTClient::connected, this, &WiCamMQTTClient::onConnected);
    connect(m_client, &MQTTClient::disconnected, this, &WiCamMQTTClient::onDisconnected);
    connect(m_client, &MQTTClient::received, this, &WiCamMQTTClient::onReceive);
    connect(m_client, &MQTTClient::sended, this, &WiCamMQTTClient::onSended);
}

WiCamMQTTClient::~WiCamMQTTClient()
{
}
void WiCamMQTTClient::onConnected()
{
    // Publish online status to the server
    m_client->publish(m_wiCamTopic + MQTT_ONLINE_PREFIX, MQTT_ONLINE_MESSAGE, 2, true);
    // Put a flag of connected to true
    m_connected = true;
    // Publish last state of the system
    setSystemState(m_currSystemState);
    // Subscribe to setting topic
    m_client->subscribe(m_wiCamTopic + MQTT_SETTINGS_PREFIX, 1);
    // Subscribe to control topic
    m_client->subscribe(m_wiCamTopic + MQTT_CONTROL_PREFIX, 1);
    // Emit the connected signal
    emit connected();
}

void WiCamMQTTClient::onDisconnected()
{
    // Put a flag of connected to false
    m_connected = false;
    // Emit the connected signal
    emit disconnected();
}

void WiCamMQTTClient::onReceive(const QString &topic, const QByteArray &message)
{
    // Check if message has some data
    if(message.size() < 1)
        // if not ignore this message
        return;
    if(topic.endsWith(MQTT_SETTINGS_PREFIX))
    {
        QJsonObject captureObj(QJsonDocument::fromJson(message).object());
        //QJsonObject captureObj(jsonObj["Capture"].toObject());
        //QJsonObject mqttObj(jsonObj["MQTT"].toObject());
        CaptureInfo capture_settings;

        capture_settings.cap_width = captureObj.value("XRes").toInt(X_RES_DEFAULT);
        capture_settings.cap_height = captureObj.value("YRes").toInt(Y_RES_DEFAULT);
        capture_settings.fps = captureObj.value("FPS").toInt(FPS_DEFAULT);
        capture_settings.captureType = static_cast<CAPTURE_TYPE>(
                    captureObj.value("CaptureType").toInt(CAPTURE_TYPE_DEFAULT));
        capture_settings.framesTrigger = captureObj.value(
                    "FramesTrigger").toInt(FRAMES_TRIGGER_DEFAULT);
        capture_settings.imageTimeDiff = captureObj.value(
                    "ImageTimeDiff").toInt(IMAGE_TIME_DIFF_DEFAULT);
        capture_settings.secToStop = captureObj.value(
                    "SecToStop").toInt(SEC_TO_STOP_DEFAULT);
        capture_settings.showTimestamp = captureObj.value(
                    "showTime").toBool(SHOW_TIME_DEFAULT);
        capture_settings.videoLen = captureObj.value(
                    "videoLength").toInt(VIDEO_LENGTH_DEFAULT);

        emit captureSettingsReceived(capture_settings);
    }
    else if(topic.endsWith(MQTT_CONTROL_PREFIX))
    {
        // Change the message to upper case
        QByteArray message_upper = message.toUpper();
        // compare the message with state strings
        if(message_upper == MQTT_CONTROL_ON_MESSAGE)
            emit startSystem();
        else if(message_upper == MQTT_CONTROL_OFF_MESSAGE)
            emit stopSystem();
        else if(message_upper == MQTT_CONTROL_POWER_OFF_MESSAGE)
            emit powerOffSystem();
    }
}

void WiCamMQTTClient::setSystemState(bool runningState)
{
    if(m_connected)
    {
        // Publish the running state of system
        m_client->publish(m_wiCamTopic + MQTT_SYSTEM_STATE_PREFIX,
                          (runningState ? MQTT_STATE_RUNNING_MESSAGE :
                                          MQTT_STATE_STOPPED_MESSAGE), 2, true);
    }
    m_currSystemState = runningState;
}

void WiCamMQTTClient::onSended()
{
    if(m_dataTransActive)
        sendDataPayload();
}

void WiCamMQTTClient::connectToHost()
{
    // Just forward the connect command
    m_client->connectToHost();
}

void WiCamMQTTClient::publishPacket(const QString &topic, uint8_t packetType,
                                    const QByteArray &payload)
{
    if (m_connected)
    {
        QByteArray packet;
        packet.resize(HEADER_SIZE);
        packet[0] = packetType;
        packet.append(payload);
        m_client->publish(topic, packet, 1, false);
    }
}

void WiCamMQTTClient::dataSend(const QString &topic, const QByteArray &data)
{
    // Raise the data process transmittion flag
    m_dataTransProcess = true;

    m_dataSize = data.size();
    QJsonObject jsonObj (QJsonObject::fromVariantMap({
                                                         {"size", m_dataSize},
                                                         {"timestamp",
                                                          QDateTime::currentMSecsSinceEpoch()},
                                                         {"checksum",
                                                          QString(QCryptographicHash::hash(data,
                                                          QCryptographicHash::Sha1).toHex())}}));
    QByteArray headerData( QJsonDocument(jsonObj).toJson() );

    m_chunkIndex = 0;
    m_dataLeft = m_dataSize;
    m_topic = topic;
    m_data = data;

    publishPacket(topic , (uint8_t)DATATRANS_MESSAGE_TYPE::DATA_HEADER, headerData);
    m_dataTransActive = true;
}

void WiCamMQTTClient::sendDataPayload()
{
    // If no data left when calling this method then exit it
    if(!m_dataLeft)
        return;

    // Choose the payload from the data buffer and care with the final chunk
    QByteArray dataPayload =
            QByteArray::fromRawData(m_data.data() + (m_chunkIndex * MQTT_DATA_PACKET_LENGTH),
                                    m_dataLeft < MQTT_DATA_PACKET_LENGTH ? m_dataLeft : MQTT_DATA_PACKET_LENGTH);
    //QByteArray dataPayload = m_data.mid(m_chunkIndex * MQTT_DATA_PACKET_LENGTH,
    //                                    m_dataLeft < MQTT_DATA_PACKET_LENGTH ? m_dataLeft : MQTT_DATA_PACKET_LENGTH);

    // Increase the chunk counter by one
    m_chunkIndex++;
    // Decrease the bytes counter by the sended bytes count
    m_dataLeft -= dataPayload.size();
    // send the packet
    publishPacket(m_topic, (uint8_t)DATATRANS_MESSAGE_TYPE::DATA_PAYLOAD, dataPayload);
    // if no bytes left then send finish signal
    if(!m_dataLeft)
        sendDataFinish();
}

void WiCamMQTTClient::sendDataFinish()
{
    // Disable the data transmittion
    m_dataTransActive = false;
    // Publish the finish signal
    publishPacket(m_topic, (uint8_t)DATATRANS_MESSAGE_TYPE::DATA_FINISH, QByteArray());
    // Raise clearness of data transmittion process
    m_dataTransProcess = false;
}

void WiCamMQTTClient::sendImage(QByteArray imgBuff)
{
    // Exit if there is active sending process
    if(m_dataTransProcess)
        return;
    // send image buffer
    dataSend(m_wiCamTopic + MQTT_IMAGE_PREFIX, imgBuff);
}

void WiCamMQTTClient::sendVideo(const QString &fileName)
{
    // Exit if there is active sending process
    if(m_dataTransProcess)
        return;
    QFile videoFile(fileName);
    // open video file at requested path
    if(!videoFile.open(QIODevice::ReadOnly))
    {
        // If error happen in opening file prompt and return the function
        qCritical() << "Can't open video file at " << fileName;
        return;
    }
    // send the file contents
    dataSend(m_wiCamTopic + MQTT_VIDEO_PREFIX, videoFile.readAll());
    // close the file
    videoFile.close();
    // Delete the file to free memory
    videoFile.remove();
}

void WiCamMQTTClient::sendSound(const QString &fileName)
{
    // Exit if there is active sending process
    if(m_dataTransProcess)
        return;
    QFile soundFile(fileName);
    // open video file at requested path
    if(!soundFile.open(QIODevice::ReadOnly))
    {
        // If error happen in opening file prompt and return the function
        qCritical() << "Can't open sound file at " << fileName;
        return;
    }
    // send the file contents
    dataSend(m_wiCamTopic + MQTT_SOUND_PREFIX, soundFile.readAll());
    // close the file
    soundFile.close();
    // Delete the file to free memory
    soundFile.remove();

}

void WiCamMQTTClient::setSettings(const MQTTSettings &mqtt_settings)
{
    bool lastConnectState = m_connected;
    // If already connected
    if(lastConnectState)
        // Disconnect from host
        m_client->disconnectFromHost();

    // MQTT Server info settings
    m_client->setHostAddress(mqtt_settings.hostName);
    m_client->setPort(mqtt_settings.port);
    m_client->setUsername(mqtt_settings.userName);
    m_client->setPassword(mqtt_settings.password);

    // Connection Settings and auto reconnection
    m_client->setAutoReconnect(true);
    m_client->setCleanSession(false);
    m_client->setClientID(QUuid::createUuid().toString());
    m_client->setKeepAliveInterval(MQTT_KEEP_ALIVE_INTERVAL_SEC);

    // Will Topic as availabilty of WiCam
    m_client->setWillEnabled(true);
    m_client->setWillQos(2);
    m_client->setWillRetain(true);
    m_client->setWillTopic(mqtt_settings.topic + MQTT_ONLINE_PREFIX);
    m_client->setWillMessage(MQTT_OFFLINE_MESSAGE);
    // Save the topic
    m_wiCamTopic = mqtt_settings.topic;
    // If client was connected then reconnect
    if(lastConnectState)
        m_client->connectToHost();

}
