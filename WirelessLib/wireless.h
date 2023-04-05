#ifndef WIRELESS_H
#define WIRELESS_H

#include <QString>
#include "wirelessdriver.h"

class Wireless
{
public:
    Wireless(const QString &interface_dev = QString());

    ~Wireless();
    /**
     * @brief connect: method to connect to wifi access point based on ssid and password information
     * @param ssid: the ssid of the wifi access point
     * @param password: password of the wifi access point
     */
    bool connect(const QString &ssid, const QString &password);
    /**
     * @brief current: get the current connected wifi access point
     * @return return the ssid of the connected wifi access point
     */
    QString current();


    QStringList scan();

    /**
     * @brief interfaces: get the wireless adapters available in this computer
     * @param list: the list where to store the adapters names
     */
    QStringList interfaces();
    /**
     * @brief interface: get the current wireless adapter
     * @return name of the current wireless adapter
     */
    QString interface();
    /**
     * @brief power: turn the wireless adapter on or off
     * @param turnState hold the state of the adapter to be set
     */
    void power(bool turnState);
    /**
     * @brief driver get the wireless driver name
     * @return string of the wireless driver name
     */
    QString driver();
private:
    WirelessDriver *_driver;

    QString _driverName;

    QString _detectDriver();

};

#endif // WIRELESS_H
