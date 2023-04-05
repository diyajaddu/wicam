#include "wiwebserver.h"

WiWebServer::WiWebServer(WiCamSettings *settings, const QString &IniSettingsFileName, QObject *parent) : QObject(parent)
  , m_sessionStore(nullptr), m_listenerStore(nullptr), m_settingsPtr(settings)
{
    m_serverSettings = new QSettings(IniSettingsFileName,QSettings::IniFormat, this);
}

void WiWebServer::startServer()
{
    // Setup the session store
    m_serverSettings->beginGroup("sessions");
    m_sessionStore = new HttpSessionStore(m_serverSettings, this);
    m_serverSettings->endGroup();
    // Setup the listener
    m_serverSettings->beginGroup("listener");
    RequestMapper *reqMapper = new RequestMapper(m_settingsPtr, this);
    // Connect the setting change signal to the server signal
    connect(reqMapper, &RequestMapper::settingsChanged, this, &WiWebServer::settingsChanged);
    m_listenerStore = new HttpListener(m_serverSettings, reqMapper, this);
    m_serverSettings->endGroup();
}

void WiWebServer::stopServer()
{
    if(m_sessionStore != nullptr)
        delete m_sessionStore;
    if(m_listenerStore != nullptr)
        delete m_listenerStore;
}


