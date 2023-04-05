#ifndef FILEWORKER_H
#define FILEWORKER_H

#include "chatworker.h"


#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>

class fileWorker : public QObject
{
    Q_OBJECT

public:

    explicit fileWorker(QObject *parent = 0);

    enum listen_t{LT_LISTEN,LT_UNLISTEN};
    enum update_t{UT_SHOW,UT_HIDE,UT_SETMAX,UT_SETVALUE};

    bool startListen(void);
    void startSend(void);
    void stopWorker(void);

    bool setSendFile(QString path);
    void setArgs(QString ip,QString port);

    listen_t status();

private:

    listen_t currentListenType;

    QString IP,PORT;
    const QString DEFAULT_FILE_STORE = "D:\\";

    QTcpServer * fileServer;

    /* --- File Send --- */
    QString filePath;
    qint8 sendTimes;
    QTcpSocket * sendSocket;
    QFile * sendFile;
    QString sendFileName;
    qint64 sendFileTotalSize,sendFileLeftSize,sendFileEachTransSize;
    QByteArray sendFileBlock;

    /* --- File Receive --- */
    QTcpSocket * receiveSocket;
    QFile * receiveFile;
    QString receiveFileName;
    qint64 receiveFileTotalSize,receiveFileTransSize;
    QByteArray receiveFileBlock;

signals:

    void messageShowReady(chatWorker::message_t type, QString hint, QString content);
    void progressBarUpdateReady(fileWorker::update_t type, qint64 number);

private slots:

    void acceptConnection();
    void readConnection();
    void sendFileInfo();
    void continueToSend(qint64 size);
};

#endif // FILEWORKER_H
