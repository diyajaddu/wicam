#ifndef WICAMGPIO_H
#define WICAMGPIO_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <pigpio.h>

#define LED_ON_STATE PI_HIGH
#define LED_OFF_STATE PI_LOW

#define BUTTONS_ACTIVE_STATE PI_LOW

class WiCamGPIO : public QObject
{
    Q_OBJECT
public:
    explicit WiCamGPIO(int wifiButton_GPIO, int wifiIndicator_GPIO,
                       int MQTTIndicator_GPIO, QObject *parent = nullptr);
    ~WiCamGPIO();
signals:
    void wifiHotspotRequested();

public slots:
    void activateWifiIndicator();
    void deactivateWifiIndicator();

    void activateMQTTIndicator();
    void deactivateMQTTIndicator();

private slots:
    void onInputTimerTimeout();

private:
    int m_wifiButton_GPIO;
    int m_wifiIndicator_GPIO;
    int m_MQTTIndicator_GPIO;

    /* Flag to store wifi button value and request
     * state and wifi button press timestamp */
    bool m_wifiButton;
    bool m_wifiRequested;
    qint64 m_wifiButtonTimeStamp;

    QTimer *m_inputsTimer;
};

#endif // WICAMGPIO_H
