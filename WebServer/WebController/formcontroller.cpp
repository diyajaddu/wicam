/**
  @file
  @author Stefan Frings
*/

#include "formcontroller.h"

FormController::FormController()
    : m_wireless()

{
}

void FormController::service(WiCamSettings *settingsPtr, HttpRequest& request, HttpResponse& response)
{

    WIFISettings &wifi_settings = settingsPtr->wifiSettings;
    CaptureInfo &cap_settings = settingsPtr->captureSettings;
    MQTTSettings &mqtt_settings = settingsPtr->mqttSettings;
    response.setHeader("Content-Type", "text/html; charset=ISO-8859-1");

    /*if (request.getParameter("action")=="show")
    {
        response.write("<html><body>");
        response.write("Name = ");
        response.write(request.getParameter("name"));
        response.write("<br>City = ");
        response.write(request.getParameter("city"));
        QStringList wifiList = m_wireless.scan();
        for(auto &it : wifiList )
        {
            response.write(it.toUtf8());
            response.write("<br>");
        }
        response.write("</body></html>",true);

    }
    else
    {
        response.write("<html><body>");
        response.write("<form method='post'>");
        response.write("  <input type='hidden' name='action' value='show'>");
        response.write("  Name: <input type='text' name='name'><br>");
        response.write("  City: <input type='text' name='city'><br>");
        response.write("  <input type='submit'>");
        response.write("</form>");
        response.write("</body></html>",true);
    }*/
    if(request.getParameter("action") == "save")
    {
        qDebug("Save");
        WiCamSettings wiCamConfigs;
        // Have some references to make implementation easy
        WIFISettings &wifiConfig = wiCamConfigs.wifiSettings;
        CaptureInfo &cap_config = wiCamConfigs.captureSettings;
        MQTTSettings &mqtt_config = wiCamConfigs.mqttSettings;
        // Wifi settings
        wifiConfig.ssid = request.getParameter("ssid");
        wifiConfig.password = request.getParameter("pass");

        // Capture settings
        cap_config.cap_width = request.getParameter("wres").toInt();
        cap_config.cap_height = request.getParameter("hres").toInt();
        cap_config.fps = request.getParameter("fps").toInt();
        QString captureTypeStr = request.getParameter("captype");
        if(captureTypeStr == "image")
            cap_config.captureType = CAPTURE_TYPE::IMAGE_CAPTURE;
        else if(captureTypeStr == "video")
            cap_config.captureType = CAPTURE_TYPE::VIDEO_CAPTURE;
        else
            cap_config.captureType = CAPTURE_TYPE::SOUND_CAPTURE;
        cap_config.secToStop = request.getParameter("secstop").toInt();
        cap_config.videoLen = request.getParameter("videolen").toInt();
        cap_config.imageTimeDiff = request.getParameter("imgrate").toInt();
        cap_config.framesTrigger = request.getParameter("framestrig").toInt();
        cap_config.showTimestamp = request.getParameter("showtime").toInt();

        // MQTT settings
        mqtt_config.hostName = request.getParameter("mqtthost");
        mqtt_config.port = request.getParameter("mqttport").toInt();
        mqtt_config.userName = request.getParameter("mqttuser");
        mqtt_config.password = request.getParameter("mqttpass");
        mqtt_config.topic = request.getParameter("mqtttopic");

        // Emit the resulting signal
        emit settingsChanged(wiCamConfigs);
    }

    response.write("<html><head>");
    response.write("<style>"
                              "div,fieldset,input,select{padding:5px;font-size:1em;}"
                              "input{width:100%;box-sizing:border-box;-webkit-box-sizing:border-box;-moz-box-sizing:border-box;}"
                              "select{width:100%;}"
                              "textarea{resize:none;width:98%;height:318px;padding:5px;overflow:auto;}"
                              "body{text-align:center;font-family:verdana;}"
                              "td{padding:0px;}"
                              "button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;-webkit-transition-duration:0.4s;transition-duration:0.4s;cursor:pointer;}"
                              "button:hover{background-color:#0e70a4;}"
                              ".bred{background-color:#d43535;}"
                              ".bred:hover{background-color:#931f1f;}"
                              ".bgrn{background-color:#47c266;}"
                              ".bgrn:hover{background-color:#5aaf6f;}"
                              "a{text-decoration:none;}"
                              ".p{float:left;text-align:left;}"
                              ".q{float:right;text-align:right;}"
                              "</style>");
    response.write("<script> function autoRef(){"
                   "var x = document.getElementById('action');"
                   "x.value = 'refresh';"
                   "document.getElementById('settings').submit(); }"
                   "function setVal(idtag){\n"
                   "var z= document.getElementById('ssid');\n"
                   "z.value = document.getElementById(idtag).name;\n"
                   "console.log(z);}\n"
                   "</script></head><body><div style='text-align:left;display:inline-block;min-width:340px;'>");
    response.write("<form method='post' id='settings'>");
    response.write(" <input  id='action' type='hidden' name='action' value='save'>");
    if(request.getParameter("action") == "refresh")
    {
        qDebug("Refresh");
        QStringList wifiList = m_wireless.scan();
        for(int i = 0; i < wifiList.size(); i++)
        {
            response.write(QString("<a id='%1' href='#' onClick='setVal(id)' name='%2' >").arg(i).arg(wifiList[i]).toUtf8());
            response.write(wifiList[i].toUtf8());
            response.write("<a/>");
            response.write("<br/>");
        }
    }
    response.write(QString("  SSID: <input type='text' name='ssid' id='ssid' value='%1' /><br/>").arg(
                       wifi_settings.ssid).toUtf8());
    response.write(QString("  Password: <input type='password' name='pass' value='%1' /><br/>").arg(
                       wifi_settings.password).toUtf8());
    response.write("  <button name='wiref' onClick='autoRef()'>Refresh</button><br/>");
    response.write(QString("  Image resolution: <div>Width: <input type='text' name='wres' value='%1' />"
                           "Height: <input type'text' name='hres' value='%2' /></div><br/>").arg(
                       cap_settings.cap_width).arg(cap_settings.cap_height).toUtf8());
    response.write(QString("  Capture rate(Hz): <input type='text' name='fps' value='%1' /><br/>").arg(
                       cap_settings.fps).toUtf8());
    response.write("  Capture type: <select name='captype' >"
                       "<option value='image' ");
    if(cap_settings.captureType == CAPTURE_TYPE::IMAGE_CAPTURE)
        response.write("selected='selected'");
    response.write(">Image</option>"
                       "<option value='video'");
    if(cap_settings.captureType == CAPTURE_TYPE::VIDEO_CAPTURE)
        response.write("selected='selected'");
    response.write(">Video</option>"
                        "<option value='sound'");
    if(cap_settings.captureType == CAPTURE_TYPE::SOUND_CAPTURE)
        response.write("selected='selected'");
    response.write(">Sound</option>"
                       "</select><br/>");
    response.write(QString("  Seconds to stop after no motion: <input type='text' name='secstop' value='%1'/><br/>").arg(
                       cap_settings.secToStop).toUtf8());
    response.write(QString("  Maximum video/sound duration(sec): <input type='text' name='videolen' value='%1'/><br/>").arg(
                       cap_settings.videoLen).toUtf8());
    response.write(QString("  Image send rate(sec): <input type='text' name='imgrate' value='%1' /><br/>").arg(
                       cap_settings.imageTimeDiff).toUtf8());
    response.write(QString("  Consecutive frames with motion to trigger motion event(frame count): <input type='text' name='framestrig'"
                           "value='%1'/><br/>").arg(cap_settings.framesTrigger).toUtf8());
    response.write(QString("  Show time: <input type='text' name='showtime'"
                           "value='%1'/><br/>").arg(cap_settings.showTimestamp ? 1 : 0).toUtf8());

    response.write(QString("  MQTT host: <input type='text' name='mqtthost' value='%1'/><br/>").arg(
                       mqtt_settings.hostName).toUtf8());
    response.write(QString("  MQTT port: <input type='text' name='mqttport' value='%1'/><br/>").arg(
                       mqtt_settings.port).toUtf8());
    response.write(QString("  MQTT username: <input type='text' name='mqttuser' value='%1'/><br/>").arg(
                       mqtt_settings.userName).toUtf8());
    response.write(QString("  MQTT password: <input type='password' name='mqttpass' value='%1'/><br/>").arg(
                       mqtt_settings.password).toUtf8());
    response.write(QString("  MQTT topic: <input type='text' name='mqtttopic' value='%1'/><br/>").arg(
                       mqtt_settings.topic).toUtf8());


    response.write("  <button type='submit' >Submit</button>");
    response.write("</form></div>");
    response.write("</body></html>",true);

}

