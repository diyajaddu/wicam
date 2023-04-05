#include "nmcli0990wireless.h"

#include <QProcess>
#include <QString>
#include <QRegularExpression>

Nmcli0990Wireless::Nmcli0990Wireless(const QString &inteface_dev)
{
    this->interface(inteface_dev);
}

void Nmcli0990Wireless::_clean(const QString &partial)
{
    QProcess cleanProcess;
    // start cleanining process
    cleanProcess.start("bash", QStringList() << "-c" << QString("nmcli --fields UUID,NAME con show | grep %1")
                       .arg(partial));
    // wait until process finished
    if(!cleanProcess.waitForFinished()) {
        qCritical("nmcli process timeout");
        cleanProcess.kill();
        return;
    }

    QByteArrayList responseLines = cleanProcess.readAllStandardOutput().split('\n');
    for(auto & line : responseLines)
        if(line.length() > 0)
            QProcess::execute(QString("nmcli con delete uuid %1").arg(QString(line.split(' ')[0])));
}

bool Nmcli0990Wireless::_errorInResponse(const QByteArray &response)
{
    // if there is no response then return false
    if(response.length() == 0)
        return false;
    // split the response
    QByteArrayList responseLines = response.split('\n');

    // check for Error word in the begining
    for(auto & line : responseLines)
        if(line.startsWith("Error"))
            return true;
    // if we didn't find any error in the response return false
    return false;
}

bool Nmcli0990Wireless::connect(const QString &ssid, const QString &password)
{
    // first clean the other connections
    this->_clean(this->current());
    // initalize connect process
    QProcess connectProcess;
    // start connect process
    connectProcess.start(QString("nmcli dev wifi connect %1 password %2 iface %3").arg(ssid).arg(password).arg(_interface));
    // if process stucked for long time
    if(!connectProcess.waitForFinished()) {
        qCritical("nmcli process timeout in connecting");
        // kill the process
        connectProcess.kill();
        return false;
    }
    // return false if there is error in the response
    return _errorInResponse(connectProcess.readAllStandardOutput());
}

QString Nmcli0990Wireless::current()
{
    QProcess statProcess;
    // start the info command
    statProcess.start("bash", QStringList() << "-c" << QString("nmcli con | grep %1").arg(_interface));
    // if process stucked for long time
    if(!statProcess.waitForFinished()) {
        qCritical("nmcli process timeout in get current status");
        // kill the process
        statProcess.kill();
        return QString();
    }
    // get the command output and split it so seperate lines
    QByteArrayList responseLines = statProcess.readAllStandardOutput().split('\n');
    for(auto & line : responseLines)
        // if there is output then
        if(line.length() > 0)
            // return the output
            return line.split(' ')[0];
    // if there is no output then return empty string
    return QString();
}

QStringList Nmcli0990Wireless::scan()
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


QStringList Nmcli0990Wireless::interfaces()
{
    QProcess interfaceProcess;
    interfaceProcess.start("nmcli dev");
    // if process stucked for long time
    if(!interfaceProcess.waitForFinished()) {
        qCritical("nmcli process timeout in get inteface");
        // kill the process
        interfaceProcess.kill();
        return QStringList();
    }
    QByteArrayList responseLines = interfaceProcess.readAllStandardOutput().split('\n');
    QStringList interfacesList = QStringList();
    for(auto & line : responseLines)
        if(line.contains("wifi"))
            interfacesList.append(line.split(' ')[0]);
    return interfacesList;
}

QString Nmcli0990Wireless::interface(const QString &interface_dev)
{
    if(!interface_dev.isEmpty())
        _interface = interface_dev;
    return _interface;
}

void Nmcli0990Wireless::power(bool turnState)
{
    if(turnState)
        QProcess::execute("nmcli r wifi on");
    else
        QProcess::execute("nmcli r wifi off");
}
