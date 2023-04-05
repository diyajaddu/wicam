#ifndef NMCLIWIRELESS_H
#define NMCLIWIRELESS_H

#include "wirelessdriver.h"

class NmcliWireless : public WirelessDriver
{
public:
    NmcliWireless(const QString &inteface_dev = QString());

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

#endif // NMCLIWIRELESS_H
