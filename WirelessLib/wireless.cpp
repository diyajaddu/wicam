#include "wireless.h"

#include <QProcess>
#include <QVersionNumber>
#include <QException>

#include "nmcliwireless.h"
#include "nmcli0990wireless.h"
#include "wpasupplicantwireless.h"

// detect and init appropriate driver
Wireless::Wireless(const QString &interface_dev)
    : _driver(nullptr)
{
    _driverName = this->_detectDriver();
    if(_driverName == "nmcli")
        _driver = new NmcliWireless(interface_dev);
    else if(_driverName == "nmcli0990")
        _driver = new Nmcli0990Wireless(interface_dev);
    else if(_driverName == "wpa_supplicant")
        _driver = new WpasupplicantWireless(interface_dev);
    else
    {
        qCritical("cannot find any wifi devices in the system");
        throw QException();
    }

    // attempt to auto detect the interface if none was provided
    if(_driver->interface().isEmpty())
    {
        QStringList interfacesList = _driver->interfaces();
        if(interfacesList.length() > 0)
            _driver->interface(interfacesList[0]);
    }
    // raise an error if there is still no interface defined
    if(_driver->interface().isEmpty())
    {
        qCritical("Unable to auto-detect the network interface.");
        throw QException();
    }
}

Wireless::~Wireless()
{
    //if(_driver != nullptr)
    //    delete _driver;
}

QString Wireless::_detectDriver()
{
    QProcess detectProcess;
    detectProcess.start("which nmcli");
    if(!detectProcess.waitForFinished()) {
        qCritical("which nmcli timeout");
        // kill the process
        detectProcess.kill();
        return QString();
    }

     QByteArray response = detectProcess.readAllStandardOutput();
     if(response.length() > 0 && !response.contains("not found"))
     {
         detectProcess.start("nmcli --version");

         if(!detectProcess.waitForFinished()) {
             qCritical("which nmcli timeout");
             // kill the process
             detectProcess.kill();
             return QString();
         }
         QByteArrayList responsePartList = detectProcess.readAllStandardOutput().split(' ');
         // The last object is the version sentence
         QVersionNumber nmcli_version = QVersionNumber::fromString(responsePartList.last());
         if(nmcli_version > QVersionNumber::fromString("0.9.9.0"))
             return "nmcli0990";
         else
             return "nmcli";

     }

     // try wpa_supplicant (Ubuntu w/o network-manager)
     detectProcess.start("which wpa_supplicant");
     if(!detectProcess.waitForFinished()) {
         qCritical("which wpa_supplicant timeout");
         // kill the process
         detectProcess.kill();
         return QString();
     }
     response = detectProcess.readAllStandardOutput();
     if(response.length() > 0 && !response.contains("not found"))
        return "wpa_supplicant";
    // if didn't detect any driver then return blank string
    return QString();
}

bool Wireless::connect(const QString &ssid, const QString &password)
{
    return _driver->connect(ssid, password);
}

QString Wireless::current()
{
    return _driver->current();
}

QStringList Wireless::scan()
{
    return _driver->scan();
}

QStringList Wireless::interfaces()
{
    return _driver->interfaces();
}

QString Wireless::interface()
{
    return _driver->interface();
}


