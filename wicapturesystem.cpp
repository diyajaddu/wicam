#include <vector>
#include <QDateTime>
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QAudioRecorder>
#include <QUrl>

#include "wicapturesystem.h"
#include "config.h"

#define DEBUG_VIEW

WiCaptureSystem::WiCaptureSystem(QObject *parent) : QThread(parent),
    m_doStop(false), m_alive(false), m_newSettings(false), m_motionCaptured(false)
{
    // Initalize raspiCam v4l2 driver
    QProcess::startDetached("modprobe bcm2835-v4l2");
    // Wait until it's become valid
    QThread::msleep(200);
}

void WiCaptureSystem::stopThread()
{
    // if the thread is running
    if(m_alive)
        // request stopping the thread
        m_doStop = true;
}

void WiCaptureSystem::setCaptureConfigurations(const CaptureInfo &capInfo)
{
    QMutexLocker lck(&m_captureMutex);
    // set The requested resolution
    m_capInfo = capInfo;
    // raise the new settings flag
    m_newSettings = true;
}

bool WiCaptureSystem::alive() const
{
    return m_alive;
}

void WiCaptureSystem::run()
{
    // Raise alive flag for the thread
    m_alive = true;
    qDebug("Capture thread started.");
    // Open capture device (Camera)
    cv::VideoCapture cap;
    // raise first frame capture flag
    if(!cap.open(CAMERA_DEVICE_NUMBER))
    {
        qFatal(qPrintable("Can't open capture device"));
        return;
    }
    
    
    else
    {
        QAudioRecorder audioRecorder;
        QAudioEncoderSettings recorderSettings;
        audioRecorder.setAudioInput(SOUND_RECORDER_DEVICE);
        recorderSettings.setCodec(SOUND_RECORDER_CODEC);
        recorderSettings.setSampleRate(SOUND_RECORDER_SAMPLE_RATE);
        recorderSettings.setChannelCount(SOUND_RECORDER_CHANNEL_COUNT);
        recorderSettings.setQuality(static_cast<QMultimedia::EncodingQuality>(SOUND_RECORDER_QUALITY));
        audioRecorder.setEncodingSettings(recorderSettings,
                                          QVideoEncoderSettings(), SOUND_RECORDER_CONTAINER);
        cv::Mat frame, oldFrame, gray, gaus, deltaFrame, binaryFrame, dilatedFrame;
        cv::Mat element = cv::getStructuringElement(
                    cv::MORPH_RECT, cv::Size(7,7), cv::Point(-1,-1));
        cv::VideoWriter vWriter;
        std::vector<uchar> imgBuff;
        int frames_motion = 0;
        int motion = 0;
        int fps = cap.get(CV_CAP_PROP_FPS);

        double t2 = 0;
        double t3 = 0;
        double t4 = 0;
        double motionCaptureTimeStamp = 0;

        qint64 now = 0;
        qint64 startframe = 0;
        qint64 nextframe = 0;
        int numframe = 0;

        m_firstFrame = true;

        // The thread loop
        for(;;)
        {
            // If stop thread requested
            if(m_doStop)
            {
                // clear the flag
                m_doStop = false;
                // escape the while loop
                break;
            }
            // Lock the capture time mutex
            QMutexLocker lck(&m_captureMutex);
            // If capture settings changed
            if(m_newSettings)
            {
                // Close capture device
                cap.release();
                // wait until all resources released
                QThread::msleep(3000);
                if(!cap.open(CAMERA_DEVICE_NUMBER))
                {
                    qFatal(qPrintable("Can't open capture device"));
                    return;
                }
                // Set The requested capture resolution
                cap.set(CV_CAP_PROP_FRAME_WIDTH, m_capInfo.cap_width);
                cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_capInfo.cap_height);
                // Set The requested capture frame rate
                //cap.set(CV_CAP_PROP_FPS, m_capInfo.fps);
                fps = cap.get(CV_CAP_PROP_FPS);
                // limit the fps if the requested fps exceeds the limit of capture device
                if(m_capInfo.fps > fps && fps > 0)
                    m_capInfo.fps = fps;
                // ReInitalize the old frame to prevent resolution mismatch
                m_firstFrame = true;
                // close video file if it's opened and delete the file
                if(vWriter.isOpened())
                {
                    vWriter.release();
                    if(!QFile(m_videoFileName).remove())
                        qWarning("Can't delete video file");
                }
                // Close the sound file if it's still recording and delete the file
                if(audioRecorder.state() != QAudioRecorder::StoppedState)
                {
                    audioRecorder.stop();
                    if(!QFile(m_soundFileName).remove())
                        qWarning("Can't delete sound file");
                }
                // Reinitalize motion variables
                t2 = 0;
                m_motionCaptured = false;
                // Clear the new settings flag
                m_newSettings = false;
            }
            // Grab frame from capture device
            now = QDateTime::currentMSecsSinceEpoch();
            cap >> frame;
            // If frame is invalid
            if(frame.empty())
                continue;
            // Process just requested fps
            if (fps > m_capInfo.fps || fps < 1)
            {
                if (startframe == 0)
                    startframe = now;
                if (now < nextframe) {
                    //do not process this frame
                    continue;
                }
                numframe++;
                nextframe = startframe + ((1000/m_capInfo.fps)*numframe);
                if (numframe > m_capInfo.fps)
                {
                    numframe = 0;
                    startframe = startframe + ((1000/m_capInfo.fps)*(1+m_capInfo.fps));
                }
            }
            // Convert to grayscale
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            // Apply gaussian blur to elminate noise
            cv::GaussianBlur(gray, gaus, cv::Size(MOTION_DETECTION_NOISE, MOTION_DETECTION_NOISE), 5, 5);
            // If this is first frame
            if (m_firstFrame)
            {
                // Copy this frame to old frame
                gaus.copyTo(oldFrame);
                // raise that first frame is initalized
                m_firstFrame = false;
            }
            // Calculate the absolute difference between the two frames
            cv::absdiff(oldFrame, gaus, deltaFrame);
            // Copy the gaus frame to old frame
            gaus.copyTo(oldFrame);
            // Threshold the difference to binary frame
            cv::threshold(deltaFrame, binaryFrame, 15, 255, cv::THRESH_BINARY);
            // TODO: Add Mask implemntation
            // cv::bitwise_and(bin_mask, binaryImage, binaryResolution);
            // Zoomin the white pixels of binary frame
            cv::dilate(binaryFrame, dilatedFrame, element, cv::Point(-1, -1), 2);
            // Get Contours of the resulted image
            std::vector<std::vector < cv::Point > > contours0;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(dilatedFrame, contours0, hierarchy,
                             cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            //Add timestamp to frame if enabled
            if (m_capInfo.showTimestamp)
                cv::putText(frame, QTime::currentTime().toString().toStdString()
                            , cv::Point2f(15, m_capInfo.cap_height - 15), cv::FONT_HERSHEY_PLAIN,
                            1,  cv::Scalar(0,0,255,255), 2);
            for (size_t k = 0; k < contours0.size(); k++)
            {
                if (cv::contourArea(contours0[k]) < MOTION_DETECTION_MIN_AREA)
                {
                    frames_motion = 0;
                    continue;
                }
                frames_motion++;
                if (frames_motion < m_capInfo.framesTrigger)
                {
                    /* Not enough frames to trigger motion yet */
                    continue;
                }
                motion++; /* Some contour big enough to be movement */
                t2 = 0;

                if(!m_motionCaptured)
                {
                    qDebug("Motion start");
                    m_motionCaptured = true;
                    switch(m_capInfo.captureType)
                    {
                    case CAPTURE_TYPE::IMAGE_CAPTURE:
                    {
                        cv::imencode(".jpg", frame, imgBuff);
                        QByteArray buff((const char *)imgBuff.data(),
                                                  imgBuff.size());
                        emit motionDetectedImage(buff);
                    }
                        break;

                    case CAPTURE_TYPE::VIDEO_CAPTURE:
                        m_videoFileName = QString("/tmp/%1.avi").arg(
                                    QDateTime::currentDateTime().toString("dd_MM_yyyy_HH_mm_ss_z"));
                        if(!vWriter.open(m_videoFileName.toStdString(), CV_FOURCC('X','V','I','D'), m_capInfo.fps,
                                         cv::Size(m_capInfo.cap_width, m_capInfo.cap_height)))
                        {
                            qCritical() << "Can't open video file for writing" << m_videoFileName;
                            continue;
                        }
                        break;
                    case CAPTURE_TYPE::SOUND_CAPTURE:
                        m_soundFileName = QString("/tmp/%1.ogg").arg(
                                    QDateTime::currentDateTime().toString("dd_MM_yyyy_HH_mm_ss_z"));
                        audioRecorder.setOutputLocation(QUrl::fromLocalFile(m_soundFileName));
                        //audioRecorder.setEncodingSettings(recorderSettings,
                        //                                  QVideoEncoderSettings(), SOUND_RECORDER_CONTAINER);

                        // start recording sound
                        audioRecorder.record();
                        break;
                    }
                    // Get start of motion timestamp
                    t4 = motionCaptureTimeStamp = cv::getTickCount();
                }
            }
            if (motion == 0 && m_motionCaptured && t2 == 0)
            {
                /* If no motion in this frame, stop recording */
                t2 = cv::getTickCount();
                frames_motion = 0;
            }

            motion = 0; /* Reset motion detected for the new frame */

            if (m_motionCaptured)
            {
                t3 = cv::getTickCount();

                // For sound and video capture
                if(m_capInfo.captureType != CAPTURE_TYPE::IMAGE_CAPTURE)
                {
                    // Calculate the full video length limit to stop the video
                    if((t3 - motionCaptureTimeStamp) / (double) cv::getTickFrequency()
                            >= m_capInfo.videoLen)
                    {
                        // Stop the motion state
                        m_motionCaptured = false;
                        t2 = 0;
                        qDebug("Motion Stop");
                        // Close the video file
                        if(m_capInfo.captureType == CAPTURE_TYPE::VIDEO_CAPTURE)
                        {
                            vWriter.release();
                            // Emit the signal for finished video
                            emit motionDetectedVideo(m_videoFileName);
                        }
                        else // SOUND_CAPTURE
                        {
                            // Stop recording
                            audioRecorder.stop();
                            // Empty the buffer
                            while(audioRecorder.state() != QMediaRecorder::StoppedState);
                            audioRecorder.setOutputLocation(QUrl());
                            audioRecorder.record();
                            audioRecorder.stop();
                            // Empty the buffer
                            // wait until write finish
                            //QThread::sleep(2);
                            // Emit the signal for finished sound recording
                            emit motionDetectedSound(m_soundFileName);
                        }
                    }
                    else
                    {
                        // Write this frame to the video
                        vWriter << frame;
                    }
                }
                else
                {
                    // Calculate the image diff time to capture new image
                    if((t3 - t4) / (double)cv::getTickFrequency() >= m_capInfo.imageTimeDiff)
                    {
                        // Encode the frame into jpeg encoding to the memory
                        cv::imencode(".jpg", frame, imgBuff);
                        //emit the signal with the encoded image
                        QByteArray buff((const char *)imgBuff.data(),
                                                  imgBuff.size());

                        emit motionDetectedImage(buff);
                        // just for testing purposes
                        //cv::imwrite(QString("/tmp/%1.jpg").arg(
                        //                QDateTime::currentDateTime().toString("dd_MM_yyyy_HH_mm_ss_z")).toStdString(),
                        //            frame);

                        // reset the timestamp for next image
                        t4 = t3;
                    }
                }
                // Calculate sec to stop after motion stops
                if(t2 != 0)
                {
                    if((t3 - t2) / (double)cv::getTickFrequency() >= m_capInfo.secToStop)
                    {
                        // Stop the motion state
                        m_motionCaptured = false;
                        t2 = 0;
                        qDebug("Motion Stop");
                        switch(m_capInfo.captureType)
                        {
                        case CAPTURE_TYPE::VIDEO_CAPTURE:
                            // Close the video file
                            vWriter.release();
                            // Emit the signal for finished video
                            emit motionDetectedVideo(m_videoFileName);
                            break;
                        case CAPTURE_TYPE::SOUND_CAPTURE:
                            // stop recording
                            audioRecorder.stop();
                            while(audioRecorder.state() != QMediaRecorder::StoppedState);

                            // Empty the buffer
                            audioRecorder.setOutputLocation(QUrl());
                            audioRecorder.record();
                            audioRecorder.stop();

                            // wait until write finish
                            //while(audioRecorder.status() != QAudioRecorder::LoadedStatus);
                            //QThread::sleep(2);
                            // Emit the signal for finished sound
                            emit motionDetectedSound(m_soundFileName);
                            break;
                        case CAPTURE_TYPE::IMAGE_CAPTURE:
                            // no need for image capture
                            break;

                        }
                    }
                }
            }
        }
        // Raise finish flag for thread
        m_alive = false;
        qDebug("Capture thread stopped.");
    }
}
