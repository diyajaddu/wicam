#include "wpasupplicantwireless.h"

#include <unistd.h>
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

#define WPASUPPLICANT_FILENAME "/etc/wpa_supplicant/wpa_supplicant.conf"

WpasupplicantWireless::WpasupplicantWireless(const QString &interface_dev)
{
    interface(interface_dev);
}

bool WpasupplicantWireless::connect(const QString &ssid, const QString &password)
{
    // attempt to stop any active wpa_supplicant instances
    // ideally we do this just for the interface we care about
    QProcess::execute("sudo killall wpa_supplicant");

    // don't do DHCP for GoPros; can cause dropouts with the server
    QProcess::execute(QString("sudo ifconfig %1 10.5.5.10/24 up").arg(_interface));

    // create configuration file
    QFile wpa_file(WPASUPPLICANT_FILENAME);
    if(!wpa_file.open(QFile::WriteOnly)) {
        qCritical("Cannot open wpa file");
        return false;
    }
    wpa_file.write(QString("country=TR\n"
                           "ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev\n"
                           "update_config=1\n"
                           "network={\n    ssid=\"%1\"\n    psk=\"%2\"\n}\n")
                   .arg(ssid).arg(password).toUtf8());
    wpa_file.close();

    // attempt to connect
    QProcess::execute(QString("sudo wpa_supplicant -i%1 -c%2 -B")
                      .arg(_interface).arg(WPASUPPLICANT_FILENAME));
    // check that the connection was successful
    // i've never seen it take more than 3 seconds for the link to establish
    sleep(5);

    if(this->current() != ssid)
        return false;
    return true;
}

QString WpasupplicantWireless::current()
{
    // get interface status
    QProcess statProcess;
    statProcess.start("iwconfig", QStringList() << interface());
    // the current network is on the first line.
    // ex: wlan0     IEEE 802.11AC  ESSID:"SSID"  Nickname:"<WIFI@REALTEK>"
    if(!statProcess.waitForFinished()) {
        qCritical("iwconfig process timeout in connecting");
        // kill the process
        statProcess.kill();
        return QString();
    }
    QByteArrayList responseLines = statProcess.readAllStandardOutput().split('\n');
    if(responseLines.size() == 0) {
        qCritical("iwconfig output error");
        return QString();
    }
    QByteArray line = responseLines[0];
    // TODO: complete this section  <--------------------------------------------------------------
    QRegularExpression rx("ESSID:\"(.+?)\"");
    QRegularExpressionMatchIterator i = rx.globalMatch(line);

    QStringList match;
     while (i.hasNext()) {
         QRegularExpressionMatch match_regex = i.next();
         QString word = match_regex.captured(1);
         match << word;
     }
    if(match.length() > 0)
    {
        QString network = match.at(0);
        if(network != "off/any")
            return network;
    }
    // return blank string if there is no active connection
    return QString();

}

QStringList WpasupplicantWireless::scan()
{
    QProcess scanProcess;
    // run scan process based on iwlist

    QProcess::execute(QString("ifconfig %1 up").arg(_interface));
    scanProcess.start("bash", QStringList() << "-c" <<
                      QString("iwlist %1 scan | grep \"ESSID\"").arg(_interface));
    // if process stucked for long time
    if(!scanProcess.waitForFinished()) {
        qCritical("iwlist process timeout in scan");
        // kill the process
        scanProcess.kill();
        return QStringList();
    }
    QByteArray response = scanProcess.readAllStandardOutput();
    // use regular expression to catch ssid from output
    /*QByteArrayList responseLines = scanProcess.readAllStandardOutput().split('\n');
    if(responseLines.size() == 0) {
        qCritical("iw output error");
        return QStringList();
    }*/

    QRegularExpression rx("ESSID:\"(.+?)\"");
    QRegularExpressionMatchIterator i = rx.globalMatch(response);

    //QStringList match = rx.capturedTexts();
    QStringList ssidList;
     while (i.hasNext()) {
         QRegularExpressionMatch match_regex = i.next();
         QString word = match_regex.captured(1);
         ssidList << word;
     }

    // return the captured list
    return ssidList;
}

// return the current wireless adapter
QStringList WpasupplicantWireless::interfaces()
{
    QProcess interfaceProcess;
    // get interface status
    interfaceProcess.start("iwconfig");
    // if process stucked for long time
    if(!interfaceProcess.waitForFinished()) {
        qCritical("iwconfig process timeout in get inteface");
        // kill the process
        interfaceProcess.kill();
        return QStringList();
    }
    QByteArrayList responseLines = interfaceProcess.readAllStandardOutput().split('\n');
    QStringList interfacesList = QStringList();
    // parse response
    for(auto & line : responseLines)
        if(line.length() > 0 && !line.startsWith(' ')) {
            //this line contains an interface name
            if(!line.contains("no wireless extensions"))
                interfacesList.append(line.split(' ')[0]);

        }
    // return the interfaces list
    return interfacesList;
}

QString WpasupplicantWireless::interface(const QString &interface_dev)
{
    if(!interface_dev.isEmpty())
        _interface = interface_dev;
    return _interface;
}

// enable/disable wireless networking
void WpasupplicantWireless::power(bool turnState)
{
    // not supported yet
}
