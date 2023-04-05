#ifndef WICAPTURESYSTEM_H
#define WICAPTURESYSTEM_H

#include <QThread>
#include <QMutex>
#include <opencv2/opencv.hpp>

#include "structures.h"

class WiCaptureSystem : public QThread
{
    Q_OBJECT
public:
    explicit WiCaptureSystem(QObject *parent = nullptr);

    bool alive() const;

signals:
    void motionDetectedImage(QByteArray encodedImage);
    void motionDetectedVideo(const QString &fileName);
    void motionDetectedSound(const QString &fileName);

protected:
    virtual void run();

public slots:
    void stopThread();

    void setCaptureConfigurations(const CaptureInfo &capInfo);

private:
    volatile bool m_doStop;
    volatile bool m_alive;

    bool m_newSettings;
    bool m_firstFrame;
    bool m_motionCaptured;

    QString m_videoFileName;
    QString m_soundFileName;
    // Capture info
    CaptureInfo m_capInfo;
    // Capture time mutex
    QMutex m_captureMutex;
};

#endif // WICAPTURESYSTEM_H
