/**
  @file
  @author Stefan Frings
*/

#include <QCoreApplication>
#include "requestmapper.h"
#include "WebController/formcontroller.h"

RequestMapper::RequestMapper(WiCamSettings *wiCamSettingsPtr, QObject* parent)
    :HttpRequestHandler(parent), m_wiCamSettingsPtr(wiCamSettingsPtr)
{
    qDebug("RequestMapper: created");
}


RequestMapper::~RequestMapper()
{
    qDebug("RequestMapper: deleted");
}


void RequestMapper::service(HttpRequest& request, HttpResponse& response)
{
    QByteArray path=request.getPath();
    qDebug("RequestMapper: path=%s",path.data());
    FormController frmCtrl;
    connect(&frmCtrl, &FormController::settingsChanged, this, &RequestMapper::settingsChanged);

    // For the following pathes, each request gets its own new instance of the related controller.

    frmCtrl.service(m_wiCamSettingsPtr, request, response);

    qDebug("RequestMapper: finished request");
/*
    // Clear the log buffer
    if (logger)
    {
        logger->clear();
    }
    */
}
