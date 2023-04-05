#ifndef WIFIMODECHANGER_H
#define WIFIMODECHANGER_H

#include <QObject>
#include <QProcess>

enum class WIFI_MODE : uint8_t
{
    ACCESS_POINT_MODE   = 0x00,
    CLIENT_MODE         = 0x01
};

class WifiModeChanger : public QObject
{
    Q_OBJECT
public:
    explicit WifiModeChanger(QObject *parent = nullptr);

signals:

public slots:
    void changeWifiMode(WIFI_MODE mode, const QString &wirelessDevice);

private:

};

#endif // WIFIMODECHANGER_H
