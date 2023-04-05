#include <QCoreApplication>
#include <QByteArray>
#include "WebServer/wiwebserver.h"
#include "WirelessLib/wireless.h"
#include "wicapturesystem.h"
#include "wisurveillancesys.h"
#include "structures.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qRegisterMetaType<QByteArray>("QByteArray");
    qRegisterMetaType<WiCamSettings>("WiCamSettings");

    //QProcess::execute("rfkill unblock wifi");

    //QAudioRecorder arec(&a);

    //ui->audioDeviceBox->addItem(tr("Default"), QVariant(QString()));
    //foreach (const QString &device, arec.audioInputs()) {
    //    qDebug() << device;
    //}

    //WiWebServer webServer(&camSettings, "webserver.ini", &a);
    //webServer.startServer();

    //QObject::connect(&webServer, &WiWebServer::settingsChanged, &)

    WiSurveillanceSys sur(&a);

/*
    WiCaptureSystem capSys(&a);
    capSys.setCaptureConfigurations(capInfo);
    capSys.start(QThread::HighestPriority);*/
    /*
    Wireless wls;
    qDebug() << wls.scan();
    */

    return a.exec();
}
