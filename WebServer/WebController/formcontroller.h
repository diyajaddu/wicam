/**
  @file
  @author Stefan Frings
*/

#ifndef FORMCONTROLLER_H
#define FORMCONTROLLER_H

#include "httprequest.h"
#include "httpresponse.h"
#include "httprequesthandler.h"

#include "WirelessLib/wireless.h"
#include "structures.h"

using namespace stefanfrings;

/**
  This controller displays a HTML form and dumps the submitted input.
*/


class FormController : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(FormController)
public:

    /** Constructor */
    FormController();

    /** Generates the response */
    void service(WiCamSettings *settingsPtr, HttpRequest& request, HttpResponse& response);

signals:
    void settingsChanged(const WiCamSettings & settings);
private:
    Wireless m_wireless;
};

#endif // FORMCONTROLLER_H
