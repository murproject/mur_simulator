#include "ScriptHandler.h"
#include <QJsonDocument>
#include <QJsonObject>

ScriptHandler::ScriptHandler(QUrho::QUrhoScene *scene, QObject *parent) : QObject(parent)
{
    videoRecording = new ProcessingVideo(scene);

    QObject::connect(scene->GetScriptsOverlay(), &QUrho::ScriptsOverlay::finishEvent, this, &ScriptHandler::stopProcess);
    QObject::connect(this, &ScriptHandler::codeStarted, scene->GetScriptsOverlay(), &QUrho::ScriptsOverlay::StartLuaTimer);
    QObject::connect(this, &ScriptHandler::codeStopped, scene->GetScriptsOverlay(), &QUrho::ScriptsOverlay::UserScriptFinished);
}

void ScriptHandler::startProcess(QString command, QStringList arguments) {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    process = new QProcess(this);

    connect(process, SIGNAL(started()), this, SLOT(processStarted()));
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(readError()));

    process->setProcessEnvironment(env);
    process->start(command, arguments);
    correct_id = process->processId();
}

void ScriptHandler::stopProcess(){
    if (process->state() == QProcess::Running) {
        qDebug() << "QProcess - stopProcess";
        process->kill();
    }
}

void ScriptHandler::processStarted() {
    qDebug() << "QProcess - processStarted";

    QString log_path = mainWindow->LoadPaths("user_log", "/var/tmp/simulator/user.log");
    log_file.setFileName(log_path);

    QString err_path = mainWindow->LoadPaths("user_err", "/var/tmp/simulator/user_err.log");
    err_file.setFileName(err_path);

    emit codeStarted();
}

void ScriptHandler::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    delete videoRecording;

    qDebug() << "QProcess - processFinished" << exitCode << exitStatus;
    closeFile(log_file);
    closeFile(err_file);

    emit codeStopped(exitCode);
}

void ScriptHandler::readOutput() {
    QString output = process->readAllStandardOutput();
    qint64 id = process->processId();

    if (!output.isEmpty() && id == correct_id) {
        saveOutput(output, log_file);
    }
}

void ScriptHandler::readError() {
    QString error = process->readAllStandardError();
    qint64 id = process->processId();

    if (!error.isEmpty() && id == correct_id) {
        saveOutput(error, err_file);
    }
}


void ScriptHandler::saveOutput(QString text, QFile &file){
    if (!file.isOpen()){
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    }

    if (file.size() > size_limit) {
        return;
    }

    if(text.contains("LD_PRELOAD")){
        return;
    }

            file.write(text.toUtf8());
}

void ScriptHandler::closeFile(QFile &file){
    if (file.size() > size_limit) {
        file.resize(size_limit);
    }
    file.close();
}
