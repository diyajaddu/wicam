#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>

enum class CAPTURE_TYPE : uint8_t
{
    IMAGE_CAPTURE       = 0x00,
    VIDEO_CAPTURE       = 0x01,
    SOUND_CAPTURE       = 0x02
};

typedef struct _MQTTSettings
{
    // Host of Mqtt Server
    QString hostName;
    // MQTT Server port
    int port;
    // User name for MQTT Server
    QString userName;
    // Password for the user on MQTT Server
    QString password;
    // Name of the machine as topic
    QString topic;
} MQTTSettings;

typedef struct _CaptureInfo
{
    // Resolution of the capture
    int cap_width;
    int cap_height;
    // Capture fps
    int fps;
    // seconds to stop recording after movment occur
    int secToStop;
    // Video length in seconds
    int videoLen;
    // Time difference between two captures in seconds
    int imageTimeDiff;
    // Type of the capture
    CAPTURE_TYPE captureType;
    // Write timestamp to capture
    bool showTimestamp;
    // Number of consecutive frames with motion to trigger motion event
    int framesTrigger;

} CaptureInfo;

typedef struct _WIFISettings
{
    // SSID of the wifi
    QString ssid;
    // Password of wifi
    QString password;

} WIFISettings;


typedef struct _WiCamSettings
{
    // All settings that required for WiCam
    CaptureInfo captureSettings;
    MQTTSettings mqttSettings;
    WIFISettings wifiSettings;

} WiCamSettings;

#endif // STRUCTURES_H
