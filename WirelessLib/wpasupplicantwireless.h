#ifndef WPASUPPLICANTWIRELESS_H
#define WPASUPPLICANTWIRELESS_H

#include "wirelessdriver.h"

#include <QString>

class WpasupplicantWireless : public WirelessDriver
{
public:
    WpasupplicantWireless(const QString &interface_dev);

    bool connect(const QString &ssid, const QString &password);

    QString current();

    QStringList scan();

    QStringList interfaces();

    QString interface(const QString &interface_dev = QString());

    void power(bool turnState);

private:
    QString _interface;
};

#endif // WPASUPPLICANTWIRELESS_H
