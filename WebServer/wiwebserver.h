#ifndef WIWEBSERVER_H
#define WIWEBSERVER_H

#include <QSettings>

// WebApp Qt Library
#include "httpsessionstore.h"
#include "httplistener.h"

#include "WebServer/requestmapper.h"
#include "structures.h"

class WiWebServer : public QObject
{
    Q_OBJECT
public:
    explicit WiWebServer(WiCamSettings *settings, const QString &IniSettingsFileName, QObject *parent = nullptr);

public slots:
    void startServer();
    void stopServer();

signals:
    // if user inserted new settings
    void settingsChanged(const WiCamSettings & settings);

private:
    // Server settings
    QSettings *m_serverSettings;
    HttpSessionStore* m_sessionStore;
    HttpListener *m_listenerStore;
    WiCamSettings *m_settingsPtr;

};

#endif // WIWEBSERVER_H
