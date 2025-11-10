#pragma once
#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "../Core/QUrhoScene.h"
#include "../Overlay/ScriptsOverlay.h"
#include "../ProcessingVideo/ProcessingVideo.h"
#include "../Ui/ApplicationWindowWeb.h"

class ScriptHandler : public QObject
{
    Q_OBJECT
public:
    explicit ScriptHandler(QUrho::QUrhoScene *scene, QObject *parent = nullptr);
    void startProcess(QString command, QStringList arguments);
signals:
    void codeStopped(int status);
    void codeStarted();
public slots:
    void stopProcess();
private slots:
    void processStarted();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void readOutput();
    void readError();
private:
    void saveOutput(QString text, QFile &file);
    void closeFile(QFile &file);

    QProcess *process;
    QUrho::ApplicationWindowWeb *mainWindow;

    QFile log_file;
    QFile err_file;
    qint64 size_limit = 15000;
    qint64 correct_id = 0;

    ProcessingVideo *videoRecording;
};
