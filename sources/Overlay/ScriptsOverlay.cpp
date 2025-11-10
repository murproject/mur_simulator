#include "ScriptsOverlay.h"

#include "../Core/QUrhoScene.h"

#include <Urho3D/IO/File.h>
#include <Urho3D/LuaScript/LuaFile.h>
#include <Urho3D/LuaScript/LuaFunction.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <QSettings>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include "SharingOverlay.h"
#include "../ScriptHandler/ScriptHandler.h"
#include "Urho3D/Math/StringHash.h"

namespace QUrho {

    ScriptsOverlay::ScriptsOverlay(Urho3D::Context *context, QUrho::QUrhoScene *scene, QObject *parent) :
            QObject{parent},
            Urho3D::Object{context},
            m_scene{scene->GetScene()},
            m_urhoScene{scene} {

        SubscribeToEvent(
            "SOLUTION_CHECKER_OUTPUT",
            URHO3D_HANDLER(ScriptsOverlay, handleOutputEvent)
        );

        SubscribeToEvent(
            "SOLUTION_CHECKER_FINISH",
            URHO3D_HANDLER(ScriptsOverlay, handleFinishEvent)
        );
    }


    void ScriptsOverlay::Run(QString script_path) {
        GetContext()->RegisterSubsystem(new Urho3D::LuaScript(GetContext()));
        Urho3D::LuaFile* scriptFile = new Urho3D::LuaFile(GetContext());
        scriptFile->LoadFile(QtUrhoStringCast(script_path));

        m_scriptInstance = m_scene->CreateComponent<Urho3D::LuaScriptInstance>();
        m_scriptInstance->CreateObject(scriptFile, "SolutionChecker");

        lua_started = true;
        qDebug() << "ScriptsOverlay - lua script started";

        StartUserCode();
    }

    void ScriptsOverlay::Stop() {
        m_scriptInstance->SetEnabled(false);
        m_scriptInstance->Remove();
        scriptNotFinished = false;
        qDebug() << "ScriptsOverlay - lua script stopped";
    }

    void ScriptsOverlay::handleOutputEvent(Urho3D::StringHash event, Urho3D::VariantMap &args) {
        m_lastScriptOutput = QString(args["OUTPUT"].GetString().CString());
    }

    void ScriptsOverlay::handleFinishEvent(Urho3D::StringHash event, Urho3D::VariantMap &args) {
        m_finalOutput = QString(args["FINAL"].GetString().CString());
        emit finishEvent();
    }

    void ScriptsOverlay::Update(QUrhoInput *input, float timeStep) {
        if (scriptNotFinished && lua_started) {
            emit scriptRunning(m_lastScriptOutput);
        }
    }

    void ScriptsOverlay::StartLuaTimer(){
        Urho3D::VariantMap arg;
        SendEvent("SOLUTION_CHECKER_START", arg);
    }


    void ScriptsOverlay::StartUserCode(){
        ScriptHandler *script = new ScriptHandler(m_urhoScene);
        QString python_script = mainWindow->LoadPaths("script", "/usr/share/simulator/script.py");
        script->startProcess("nice", {"-n", "19", "sudo", "-E", "-u", "nobody", "python3", "-u", python_script});
    }

    void ScriptsOverlay::UserScriptFinished(int exitCode){
        this->Stop();
        if (m_finalOutput == ""){
            m_finalOutput = m_lastScriptOutput;
        }
        finish_code = exitCode;

        SendFinalMessage(m_finalOutput);
    }

    void ScriptsOverlay::SendFinalMessage(QString message){
        QString file_path = mainWindow->LoadPaths("final_log", "/var/tmp/simulator/final_log.json");
        QFile fileJson(file_path);
        fileJson.open(QIODevice::WriteOnly | QIODevice::Truncate);

        QStringList lines = message.split("\n\n\n");
        QJsonObject jsonObj;

        jsonObj["score"] = lines[0].toInt();
        jsonObj["time"] = lines[1].toInt();
        jsonObj["log"] = lines[2];
        jsonObj["finish"] = checkFinishReason(lines[3]);

        QJsonDocument jsonDoc(jsonObj);
        qDebug() << "ScriptsOverlay - final msg" << jsonDoc;

        fileJson.write(jsonDoc.toJson());
        fileJson.close();

        mainWindow->closeApplication();
    }

    QString ScriptsOverlay::checkFinishReason(QString lua_output) {
        QString finish_reason = "";

        if (finish_code == 0){
            finish_reason = "Код завершился";
        } else if (finish_code == 1) {
            finish_reason = "Код завершился с ошибкой";
        } else {
            finish_reason = lua_output;
        }

        return finish_reason;
    }
}
