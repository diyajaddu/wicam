#ifndef NMCLI0990WIRELESS_H
#define NMCLI0990WIRELESS_H

#include "wirelessdriver.h"

class Nmcli0990Wireless : public WirelessDriver
{
public:
    Nmcli0990Wireless(const QString &inteface_dev = QString());

    bool connect(const QString &ssid, const QString &password);

    QString current();

    QStringList scan();

    QStringList interfaces();

    QString interface(const QString &interface_dev = QString());

    void power(bool turnState);

private:
    void _clean(const QString &partial);
    bool _errorInResponse(const QByteArray &response);

    QString _interface;
};

#endif // NMCLI0990WIRELESS_H
