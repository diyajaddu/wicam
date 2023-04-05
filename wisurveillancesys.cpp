#include "wisurveillancesys.h"
#include "config.h"
#include <QSettings>
#include <unistd.h>
#include <sys/reboot.h>
#include "WirelessLib/wireless.h"

WiSurveillanceSys::WiSurveillanceSys(QObject *parent) : QObject(parent)
{
    // Initalize the main functionality
    m_capSys = new WiCaptureSystem(this);
    m_GPIO = new WiCamGPIO(WIFI_BUTTON_PIN_NUMBER, WIFI_INDICATOR_PIN_NUMBER,
                           MQTT_INDICATOR_PIN_NUMBER, this);

    m_webServer = new WiWebServer(&m_wiCamSettings, "webserver.ini", this);
/*
    CaptureInfo &capInfo = m_wiCamSettings.captureSettings;
    capInfo.captureType = CAPTURE_TYPE::IMAGE_CAPTURE;
    capInfo.cap_width = 640;
    capInfo.cap_height = 480;
    capInfo.fps = 5;
    capInfo.framesTrigger = 3;
    capInfo.secToStop = 2;
    capInfo.showTimestamp = true;
    capInfo.videoLen = 10;
    capInfo.imageTimeDiff = 5;

    MQTTSettings &mqttSettings = m_wiCamSettings.mqttSettings;
    mqttSettings.hostName = "136.144.216.100";
    mqttSettings.port = 1883;
    mqttSettings.userName = "mikroelectron";
    mqttSettings.password =  "abcdABCD1234";
    mqttSettings.topic = "cam";
*/
    m_mqttClient = new WiCamMQTTClient(this);

    m_wifiModeChanger = new WifiModeChanger(this);

    connect(m_GPIO, &WiCamGPIO::wifiHotspotRequested, this, &WiSurveillanceSys::onWifiAPRequested);
    connect(m_mqttClient, &WiCamMQTTClient::connected, m_GPIO, &WiCamGPIO::activateMQTTIndicator);
    connect(m_mqttClient, &WiCamMQTTClient::disconnected, m_GPIO, &WiCamGPIO::deactivateMQTTIndicator);
    connect(m_mqttClient, &WiCamMQTTClient::captureSettingsReceived, this, &WiSurveillanceSys::onCaptureSettingsReceived);
    connect(m_mqttClient, &WiCamMQTTClient::startSystem, this, &WiSurveillanceSys::onStartSystemRequested);
    connect(m_mqttClient, &WiCamMQTTClient::stopSystem, this, &WiSurveillanceSys::onStopSystemRequested);
    connect(m_mqttClient, &WiCamMQTTClient::powerOffSystem, this, &WiSurveillanceSys::onPowerOffSystemRequested);

    // Connect slots
    connect(m_webServer, &WiWebServer::settingsChanged, this, &WiSurveillanceSys::onSettingsChanged);
    connect(m_capSys, &WiCaptureSystem::motionDetectedImage, m_mqttClient,
            &WiCamMQTTClient::sendImage, Qt::QueuedConnection);
    connect(m_capSys, &WiCaptureSystem::motionDetectedVideo, m_mqttClient,
            &WiCamMQTTClient::sendVideo, Qt::QueuedConnection);
    connect(m_capSys, &WiCaptureSystem::motionDetectedSound, m_mqttClient,
            &WiCamMQTTClient::sendSound, Qt::QueuedConnection);
    connect(m_mqttClient, &WiCamMQTTClient::connected, this, &WiSurveillanceSys::onMQTTConnect);
    connect(m_mqttClient, &WiCamMQTTClient::disconnected, this, &WiSurveillanceSys::onMQTTDisconnect);


    loadSettings(m_wiCamSettings);
    // Load Capture settings
    m_capSys->setCaptureConfigurations(m_wiCamSettings.captureSettings);
    // Load MQTT settings
    m_mqttClient->setSettings(m_wiCamSettings.mqttSettings);

    m_mqttClient->connectToHost();
    m_webServer->startServer();

}

void WiSurveillanceSys::onSettingsChanged(const WiCamSettings &camSettings)
{
    Wireless wls;
    m_capSys->setCaptureConfigurations(camSettings.captureSettings);
    m_mqttClient->setSettings(camSettings.mqttSettings);

    qDebug() << "Changing to Client mode";
    m_wifiModeChanger->changeWifiMode(WIFI_MODE::CLIENT_MODE, wls.interfaces()[0]);

    // Try to connect to requested wifi
    wls.connect(camSettings.wifiSettings.ssid, camSettings.wifiSettings.password);
    // Save the settings in the settings file
    saveSettings(camSettings);
    // Set current settings
    m_wiCamSettings = camSettings;
}

void WiSurveillanceSys::saveSettings(const WiCamSettings &wicam_settings)
{
    QSettings settings(SETTINGS_FILENAME, QSettings::IniFormat);
    const CaptureInfo &capSettings = wicam_settings.captureSettings;
    const MQTTSettings &mqttSettings = wicam_settings.mqttSettings;

    // Capture settings store
    settings.beginGroup("Capture");
    settings.setValue("XRes", capSettings.cap_width);
    settings.setValue("YRes", capSettings.cap_height);
    settings.setValue("FPS", capSettings.fps);
    settings.setValue("CaptureType", (int)capSettings.captureType);
    settings.setValue("FramesTrigger", capSettings.framesTrigger);
    settings.setValue("ImageTimeDiff", capSettings.imageTimeDiff);
    settings.setValue("SecToStop", capSettings.secToStop);
    settings.setValue("showTime", capSettings.showTimestamp);
    settings.setValue("videoLength", capSettings.videoLen);
    settings.endGroup();

    // MQTT Settings store
    settings.beginGroup("MQTT");
    settings.setValue("hostName", mqttSettings.hostName);
    settings.setValue("port", mqttSettings.port);
    settings.setValue("userName", mqttSettings.userName);
    settings.setValue("password", mqttSettings.password);
    settings.setValue("topic", mqttSettings.topic);
    settings.endGroup();

    // Wifi settings store
    settings.beginGroup("Wireless");
    settings.setValue("ssid", wicam_settings.wifiSettings.ssid);
    settings.setValue("password", wicam_settings.wifiSettings.password);
    settings.endGroup();
    // Syncronise to write the settings
    settings.sync();

}

void WiSurveillanceSys::loadSettings(WiCamSettings &wicam_settings)
{
    QSettings settings(SETTINGS_FILENAME, QSettings::IniFormat);
    CaptureInfo &capSettings = wicam_settings.captureSettings;
    MQTTSettings &mqttSettings = wicam_settings.mqttSettings;

    // Capture settings store
    settings.beginGroup("Capture");
    capSettings.cap_width = settings.value("XRes", X_RES_DEFAULT).toInt();
    capSettings.cap_height = settings.value("YRes", Y_RES_DEFAULT).toInt();
    capSettings.fps = settings.value("FPS", FPS_DEFAULT).toInt();
    capSettings.captureType = static_cast<CAPTURE_TYPE>(settings.value("CaptureType", CAPTURE_TYPE_DEFAULT).toInt());
    capSettings.framesTrigger = settings.value("FramesTrigger", FRAMES_TRIGGER_DEFAULT).toInt();
    capSettings.imageTimeDiff = settings.value("ImageTimeDiff", IMAGE_TIME_DIFF_DEFAULT).toInt();
    capSettings.secToStop = settings.value("SecToStop", SEC_TO_STOP_DEFAULT).toInt();
    capSettings.showTimestamp = settings.value("showTime", SHOW_TIME_DEFAULT).toBool();
    capSettings.videoLen = settings.value("videoLength", VIDEO_LENGTH_DEFAULT).toInt();
    settings.endGroup();

    // MQTT Settings store
    settings.beginGroup("MQTT");
    mqttSettings.hostName = settings.value("hostName", MQTT_HOST_NAME_DEFAULT).toString();
    mqttSettings.port = settings.value("port", MQTT_PORT_DEFAULT).toInt();
    mqttSettings.userName = settings.value("userName", MQTT_USER_NAME_DEFAULT).toString();
    mqttSettings.password = settings.value("password", MQTT_PASSWORD_DEFAULT).toString();
    mqttSettings.topic = settings.value("topic", MQTT_TOPIC_DEFAULT).toString();

    settings.endGroup();

    // Wifi settings store
    settings.beginGroup("Wireless");
    wicam_settings.wifiSettings.ssid = settings.value("ssid", "").toString();
    wicam_settings.wifiSettings.password = settings.value("password", "").toString();
    settings.endGroup();
}

void WiSurveillanceSys::onCaptureSettingsReceived(const CaptureInfo &capSettings)
{
    QSettings settings(SETTINGS_FILENAME, QSettings::IniFormat);

    // Capture settings store
    settings.beginGroup("Capture");
    settings.setValue("XRes", capSettings.cap_width);
    settings.setValue("YRes", capSettings.cap_height);
    settings.setValue("FPS", capSettings.fps);
    settings.setValue("CaptureType", (int)capSettings.captureType);
    settings.setValue("FramesTrigger", capSettings.framesTrigger);
    settings.setValue("ImageTimeDiff", capSettings.imageTimeDiff);
    settings.setValue("SecToStop", capSettings.secToStop);
    settings.setValue("showTime", capSettings.showTimestamp);
    settings.setValue("videoLength", capSettings.videoLen);
    settings.endGroup();

    m_capSys->setCaptureConfigurations(capSettings);

    m_wiCamSettings.captureSettings = capSettings;

    settings.sync();

}


void WiSurveillanceSys::onMQTTConnect()
{
    qDebug() << "MQTT Connected!";
}

void WiSurveillanceSys::onMQTTDisconnect()
{
    qDebug() << "MQTT Disconnected";
}

void WiSurveillanceSys::onWifiAPRequested()
{
    Wireless wls;
    qDebug() << "Changing to AP mode";
    m_wifiModeChanger->changeWifiMode(WIFI_MODE::ACCESS_POINT_MODE,
                                      wls.interfaces()[0]);
}

void WiSurveillanceSys::onStartSystemRequested()
{
    if(!m_capSys->alive())
    {
        m_capSys->start(QThread::HighPriority);
        m_mqttClient->setSystemState(true);
    }
}

void WiSurveillanceSys::onStopSystemRequested()
{
    if(m_capSys->alive())
    {
        m_capSys->stopThread();
        m_mqttClient->setSystemState(false);
    }
}

void WiSurveillanceSys::onPowerOffSystemRequested()
{
    onStopSystemRequested();
    sync();
    reboot(RB_POWER_OFF);
}
