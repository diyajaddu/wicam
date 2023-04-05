#ifndef WISURVEILLANCESYS_H
#define WISURVEILLANCESYS_H

#include <QObject>

#include "wicapturesystem.h"
#include "wicammqttclient.h"
#include "wicamgpio.h"
#include "wifimodechanger.h"
#include "WebServer/wiwebserver.h"

class WiSurveillanceSys : public QObject
{
    Q_OBJECT
public:
    explicit WiSurveillanceSys(QObject *parent = nullptr);
private:
    void saveSettings(const WiCamSettings &wicam_settings);
    void loadSettings(WiCamSettings &wicam_settings);

signals:

private slots:
    void onSettingsChanged(const WiCamSettings &camSettings);
    void onCaptureSettingsReceived(const CaptureInfo &captureSettings);

    void onMQTTConnect();
    void onMQTTDisconnect();
    void onWifiAPRequested();

    void onStartSystemRequested();
    void onStopSystemRequested();
    void onPowerOffSystemRequested();
public slots:

private:
    WiCaptureSystem *m_capSys;
    WiCamMQTTClient *m_mqttClient;
    WiWebServer *m_webServer;
    WiCamGPIO *m_GPIO;
    WifiModeChanger *m_wifiModeChanger;
    WiCamSettings m_wiCamSettings;
};

#endif // WISURVEILLANCESYS_H
