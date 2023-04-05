
#include "wicamgpio.h"
#include "config.h"

WiCamGPIO::WiCamGPIO(int wifiButton_GPIO, int wifiIndicator_GPIO,
                     int MQTTIndicator_GPIO, QObject *parent) : QObject(parent)
{
    // Set pin numbers
    m_wifiButton_GPIO = wifiButton_GPIO;
    m_wifiIndicator_GPIO = wifiIndicator_GPIO;
    m_MQTTIndicator_GPIO = MQTTIndicator_GPIO;

    // Initalize GPIO library
    if (gpioInitialise() < 0)
    {
        qFatal("pigpio initialisation failed");
        return;
    }

    // Set GPIO modes
    /*  Set the inputs  */
    gpioSetMode(wifiButton_GPIO, PI_INPUT);

    /*  Set the outputs */
    gpioSetMode(wifiIndicator_GPIO, PI_OUTPUT);
    gpioSetMode(MQTTIndicator_GPIO, PI_OUTPUT);

    // Initalize input reading timer
    m_inputsTimer = new QTimer(this);
    m_inputsTimer->setSingleShot(false);
    m_inputsTimer->setInterval(GPIO_READING_INTERVAL_MS);
    // Connect inputs reading process slot to it's timer
    connect(m_inputsTimer, &QTimer::timeout,
            this, &WiCamGPIO::onInputTimerTimeout);

    m_wifiButtonTimeStamp = QDateTime::currentMSecsSinceEpoch();
    m_wifiRequested = false;
    // Start inputs reader timer

    m_inputsTimer->start();

}

WiCamGPIO::~WiCamGPIO()
{
    // Stop inputs reader timer
    m_inputsTimer->stop();
    /* Stop DMA, release resources */
    gpioTerminate();
}

void WiCamGPIO::onInputTimerTimeout()
{
    // read the wifi button pin
    bool wifiButtonValue = gpioRead(m_wifiButton_GPIO) == BUTTONS_ACTIVE_STATE;
    // if button detected as pressed
    if(wifiButtonValue)
    {
        // if the last button state from last reading was pressed
        if(m_wifiButton)
        {
            // Check the activate time delay for the button
            if(m_wifiButtonTimeStamp + WIFI_BUTTON_PRESS_ACTIVATE_TIME_MS
                    <= QDateTime::currentMSecsSinceEpoch() && !m_wifiRequested)
            {
                // emit the signal
                emit wifiHotspotRequested();
                // clear the button flag to prevent emit the button signal in the next reading
                m_wifiButton = false;
                // set the request flag to true to prevent emit button signal in the next reading
                m_wifiRequested = true;
            }
        }
        else    // if button pressed now
        {
            // initalize the button state flag and it's timestamp
            m_wifiButton = true;
            m_wifiButtonTimeStamp = QDateTime::currentMSecsSinceEpoch();
        }
    }
    else // if button not pressed
    {
        // clear the wifi button flag
        m_wifiButton = false;
        // clear the wifi request flag
        m_wifiRequested = false;
    }
}

/********** Wifi Indicator *********/
void WiCamGPIO::activateWifiIndicator()
{
    gpioWrite(m_wifiIndicator_GPIO, LED_ON_STATE);
}

void WiCamGPIO::deactivateWifiIndicator()
{
    gpioWrite(m_wifiIndicator_GPIO, LED_OFF_STATE);
}

/********** MQTT Indicator *********/
void WiCamGPIO::activateMQTTIndicator()
{
    gpioWrite(m_MQTTIndicator_GPIO, LED_ON_STATE);
}

void WiCamGPIO::deactivateMQTTIndicator()
{
    gpioWrite(m_MQTTIndicator_GPIO, LED_OFF_STATE);
}
