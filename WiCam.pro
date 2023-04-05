
QT -= gui
QT += multimedia

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    WebServer/wiwebserver.cpp \
    WebServer/WebController/formcontroller.cpp \
    WebServer/requestmapper.cpp \
    wicapturesystem.cpp \
    WirelessLib/nmcli0990wireless.cpp \
    WirelessLib/nmcliwireless.cpp \
    WirelessLib/wireless.cpp \
    WirelessLib/wpasupplicantwireless.cpp \
    wisurveillancesys.cpp \
    MqttLib/mqttclient.cpp \
    wicammqttclient.cpp \
    wicamgpio.cpp \
    wifimodechanger.cpp

HEADERS += \
    WebServer/wiwebserver.h \
    WebServer/WebController/formcontroller.h \
    WebServer/requestmapper.h \
    structures.h \
    wicapturesystem.h \
    config.h \
    WirelessLib/nmcli0990wireless.h \
    WirelessLib/nmcliwireless.h \
    WirelessLib/wireless.h \
    WirelessLib/wirelessdriver.h \
    WirelessLib/wpasupplicantwireless.h \
    wisurveillancesys.h \
    MqttLib/mqttclient.h \
    wicammqttclient.h \
    wicamgpio.h \
    wifimodechanger.h

include(WebServer/QtWebApp/logging/logging.pri)
include(WebServer/QtWebApp/httpserver/httpserver.pri)
include(WebServer/QtWebApp/templateengine/templateengine.pri)

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv

LIBS += -lpaho-mqtt3c -lpaho-mqtt3a -lpigpio

