#ifndef WICAMMQTTCLIENT_H
#define WICAMMQTTCLIENT_H

#include <QObject>
#include <QFile>

#include "MqttLib/mqttclient.h"
#include "structures.h"

#define HEADER_SIZE 1

enum class UI_MESSAGE_TYPE : uint8_t
{
    SET_SETTINGS    = 0x00
};

enum class DATATRANS_MESSAGE_TYPE : uint8_t
{
    DATA_HEADER    = 0x00,
    DATA_PAYLOAD   = 0x01,
    DATA_FINISH    = 0x02
};

class WiCamMQTTClient : public QObject
{
    Q_OBJECT
public:
    explicit WiCamMQTTClient(QObject *parent = nullptr);
    ~WiCamMQTTClient();

signals:
    void connected();
    void disconnected();
    void captureSettingsReceived(const CaptureInfo &capture_settings);

    void startSystem();
    void stopSystem();
    void powerOffSystem();

private slots:
    void onConnected();
    void onDisconnected();
    void onSended();
    void onReceive(const QString &topic, const QByteArray &message);

private:
    void publishPacket(const QString &topic, uint8_t packetType,
                       const QByteArray &payload);

    void dataSend(const QString &topic,const QByteArray &data);
    void sendDataPayload();
    void sendDataFinish();


public slots:
    void connectToHost();
    void sendImage(QByteArray imgBuff);
    void sendVideo(const QString &fileName);
    void sendSound(const QString &fileName);
    void setSystemState(bool runningState);
    void setSettings(const MQTTSettings &mqtt_settings);


private:
    MQTTClient *m_client;
    QString m_wiCamTopic;

    bool m_connected;
    bool m_currSystemState;

    // data sending variables
    quint64 m_dataSize;
    quint64 m_dataLeft;
    quint64 m_chunkIndex;
    QString m_topic;
    QByteArray m_data;

    bool m_dataTransActive;
    bool m_dataTransProcess;

};

#endif // WICAMMQTTCLIENT_H
