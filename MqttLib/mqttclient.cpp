#include "mqttclient.h"

#include <exception>
#include <QDebug>

using namespace std;

MQTTClient::MQTTClient(QObject *parent)
    : QObject(parent) , _willEnabled(false), _port(1883)
{

}

MQTTClient::~MQTTClient()
{
    MQTTAsync_destroy(&_client);
}

void MQTTClient::connectToHost()
{
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    //MQTTAsync_token token;
    MQTTAsync_create(&_client, QString("tcp://%1:%2").arg(_hostAddress).arg(_port).toStdString().c_str()
                     , _clientID.toStdString().c_str(),
                     MQTTCLIENT_PERSISTENCE_NONE, NULL);
    int rc = MQTTAsync_setConnected(_client, this, &MQTTClient::on_connected);

    if (rc != MQTTASYNC_SUCCESS) {

        printf("Error to set connect callback , return code %d\n", rc);

    }

    rc = MQTTAsync_setCallbacks(_client, this, &MQTTClient::on_connection_lost, &MQTTClient::on_message_arrived,
                           NULL);

    if (rc != MQTTASYNC_SUCCESS) {
        printf("Error to set callback in connecting , return code %d\n", rc);
    }
    if(_willEnabled)
    {

        MQTTAsync_willOptions willOpt = MQTTAsync_willOptions_initializer;
        if(!_willMessage.isEmpty())
            willOpt.message = _willMessage.toStdString().c_str();
        if(!_willTopic.isEmpty())
            willOpt.topicName = _willTopic.toStdString().c_str();
        willOpt.qos = _willQos;
        willOpt.retained = _willRetain ? 1 : 0;
        conn_opts.will = &willOpt;
    }
    if(!_username.isEmpty())
        conn_opts.username = _username.toStdString().c_str();
    if(!_password.isEmpty())
        conn_opts.password = _password.toStdString().c_str();

    conn_opts.keepAliveInterval = _keepAliveInterval;
    conn_opts.cleansession = _cleanSession ? 1 : 0;
    conn_opts.automaticReconnect = _autoReconnect ? 1 : 0;
    conn_opts.context = this;
    conn_opts.onSuccess = NULL;
    conn_opts.onFailure = NULL;
    //conn_opts.onSuccess = &MQTTClient::on_connect;
    if ((rc = MQTTAsync_connect(_client, &conn_opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
    }
}

void MQTTClient::disconnectFromHost()
{
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    opts.onSuccess = &MQTTClient::on_disconnect;
    opts.context = this;

    if ((rc = MQTTAsync_disconnect(_client, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to disconnect, return code %d\n", rc);
    }
}

void MQTTClient::on_disconnect(void *context, MQTTAsync_successData *)
{
    MQTTClient *client_obj = static_cast<MQTTClient *>(context);
    client_obj->disconnected();
}

void MQTTClient::on_connected(void* context, char* )
{
    MQTTClient *client_obj = static_cast<MQTTClient *>(context);
    client_obj->connected();
}
void MQTTClient::on_connection_lost(void* context, char* )
{
    MQTTClient *client_obj = static_cast<MQTTClient *>(context);
    client_obj->disconnected();
}

int MQTTClient::on_message_arrived(void* context, char* topicName, int topicLen,
                                     MQTTAsync_message* msg)
{
    if (context) {
        MQTTClient *cli_obj = static_cast<MQTTClient*>(context);
        cli_obj->received(QString::fromLatin1(topicName, topicLen), QByteArray((char *)msg->payload, msg->payloadlen));
    }

    MQTTAsync_freeMessage(&msg);
    MQTTAsync_free(topicName);

    // TODO: Should the user code determine the return value?
    // The Java version does doesn't seem to...
    return true;
}

void MQTTClient::subscribe(const QString &topic, int qos)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    opts.onSuccess = on_subscribe;
    opts.context = this;

    if ((rc = MQTTAsync_subscribe(_client, topic.toStdString().c_str(), qos, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
    }
}

void MQTTClient::on_subscribe(void *context, MQTTAsync_successData *)
{
    MQTTClient *cli_obj = static_cast<MQTTClient*>(context);
    cli_obj->subscribed();
}

bool MQTTClient::autoReconnect() const
{
    return _autoReconnect;
}

void MQTTClient::setAutoReconnect(bool autoReconnect)
{
    _autoReconnect = autoReconnect;
}

bool MQTTClient::cleanSession() const
{
    return _cleanSession;
}

void MQTTClient::setCleanSession(bool cleanSession)
{
    _cleanSession = cleanSession;
}

int MQTTClient::keepAliveInterval() const
{
    return _keepAliveInterval;
}

void MQTTClient::setKeepAliveInterval(int keepAliveInterval)
{
    _keepAliveInterval = keepAliveInterval;
}

int MQTTClient::port() const
{
    return _port;
}

void MQTTClient::setPort(int port)
{
    _port = port;
}

QString MQTTClient::hostAddress() const
{
    return _hostAddress;
}

void MQTTClient::setHostAddress(const QString &hostAddress)
{
    _hostAddress = hostAddress;
}

QString MQTTClient::clientID() const
{
    return _clientID;
}

void MQTTClient::setClientID(const QString &clientID)
{
    _clientID = clientID;
}

void MQTTClient::on_send(void *context, MQTTAsync_successData *)
{
    MQTTClient *client_obj = static_cast<MQTTClient *>(context);
    client_obj->sended();
}

QString MQTTClient::password() const
{
    return _password;
}

void MQTTClient::setPassword(const QString &password)
{
    _password = password;
}

QString MQTTClient::username() const
{
    return _username;
}

void MQTTClient::setUsername(const QString &username)
{
    _username = username;
}

bool MQTTClient::willRetain() const
{
    return _willRetain;
}

void MQTTClient::setWillRetain(bool willRetain)
{
    _willRetain = willRetain;
}

int MQTTClient::willQos() const
{
    return _willQos;
}

void MQTTClient::setWillQos(int willQos)
{
    _willQos = willQos;
}

QString MQTTClient::willMessage() const
{
    return _willMessage;
}

void MQTTClient::setWillMessage(const QString &willMessage)
{
    _willMessage = willMessage;
}

QString MQTTClient::willTopic() const
{
    return _willTopic;
}

void MQTTClient::setWillTopic(const QString &willTopic)
{
    _willTopic = willTopic;
}

bool MQTTClient::willEnabled() const
{
    return _willEnabled;
}

void MQTTClient::setWillEnabled(bool willEnabled)
{
    _willEnabled = willEnabled;
}

void MQTTClient::publish(const QString &topic,const QByteArray &payload, int qos, bool retrain)
{
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
    opts.onSuccess = &MQTTClient::on_send;
    opts.context = this;

    pubmsg.payload = (void *)payload.data();
    pubmsg.payloadlen = payload.length();
    pubmsg.qos = qos;
    pubmsg.retained = retrain ? 1 : 0;

    if ((rc = MQTTAsync_sendMessage(_client, topic.toStdString().c_str(), &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
    }
}

/*void MQTTClient::on_connect(void *context, MQTTAsync_successData *)
{
    MQTTClient *cli_obj = static_cast<MQTTClient*>(context);
    cli_obj->connected(QString());
}*/
