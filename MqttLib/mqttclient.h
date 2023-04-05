#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QString>

#include <MQTTAsync.h>

class MQTTClient : public QObject
{
    Q_OBJECT
public:

    MQTTClient(QObject *parent = NULL);
    ~MQTTClient();

    void connectToHost();
    void disconnectFromHost();
    void subscribe(const QString &topic, int qos);
    void publish(const QString &topic, const QByteArray &payload, int qos = 0, bool retrain = false);


    QString clientID() const;
    void setClientID(const QString &clientID);

    QString hostAddress() const;
    void setHostAddress(const QString &hostAddress);

    int port() const;
    void setPort(int port);

    int keepAliveInterval() const;
    void setKeepAliveInterval(int keepAliveInterval);

    bool cleanSession() const;
    void setCleanSession(bool cleanSession);

    bool autoReconnect() const;
    void setAutoReconnect(bool autoReconnect);

    bool willEnabled() const;
    void setWillEnabled(bool willEnabled);

    QString willTopic() const;
    void setWillTopic(const QString &willTopic);

    QString willMessage() const;
    void setWillMessage(const QString &willMessage);

    int willQos() const;
    void setWillQos(int willQos);

    bool willRetain() const;
    void setWillRetain(bool willRetain);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

signals:
    void connected();
    void disconnected();
    void subscribed();
    void sended();
    void received(const QString &topic, const QByteArray &message);

private:
    /** Callbacks from the C library  */
    static void on_connected(void* context, char*);
    static void on_connection_lost(void *context, char *cause);
    static int  on_message_arrived(void* context, char* topicName, int topicLen,
                                   MQTTAsync_message* msg);
    static void on_delivery_complete(void* context, MQTTAsync_token tok);

    //static void onConnectFailure(void* context, MQTTAsync_failureData* response);
    static void on_disconnect(void* context, MQTTAsync_successData* response);
    static void on_subscribe(void* context, MQTTAsync_successData* response);
    static void on_send(void* context, MQTTAsync_successData* response);
    static void on_connect(void* context, MQTTAsync_successData* response);
private:
    MQTTAsync _client;

    QString _clientID;
    QString _username;
    QString _password;
    QString _hostAddress;

    bool    _willEnabled;
    QString _willTopic;
    QString _willMessage;
    int     _willQos;
    bool    _willRetain;

    int     _port;
    int     _keepAliveInterval;
    bool    _cleanSession;
    bool    _autoReconnect;

};

#endif // MQTTCLIENT_H
