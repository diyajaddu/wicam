#ifndef WIRELESSDRIVER_H
#define WIRELESSDRIVER_H

#include <QString>

class WirelessDriver
{
public:
    virtual bool connect(const QString &ssid, const QString &password) = 0;

    virtual QString current() = 0;

    virtual QStringList scan() = 0;

    virtual QStringList interfaces() = 0;

    virtual QString interface(const QString &interface_dev = QString()) = 0;

    virtual void power(bool turnState) = 0;
};

#endif // WIRELESSDRIVER_H
