#pragma once
#include "QSceneOverlay.h"
#include "../Core/QUrhoHelpers.h"
#include "Urho3D/Core/Variant.h"
#include "../Ui/ApplicationWindowWeb.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>

#include <QObject>

namespace QUrho {
    class QUrhoScene;

    class QUrhoInput;

    class ScriptsOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(ScriptsOverlay, Urho3D::Object)

    public:
        explicit ScriptsOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        void Update(QUrhoInput *input, float timeStep) override;

        void Run(QString script_path);

        void Stop();

        void SendFinalMessage(QString message);

        void StartUserCode();

    signals:
        void finishEvent();
        void scriptRunning(QString output);

    public slots:
        void UserScriptFinished(int exitCode);
        void StartLuaTimer();

    private:
        void handleFinishEvent(Urho3D::StringHash event, Urho3D::VariantMap &args);
        void handleOutputEvent(Urho3D::StringHash event, Urho3D::VariantMap &args);
        QString checkFinishReason(QString lua_output);

        Urho3D::Scene *m_scene = nullptr;
        QUrhoScene *m_urhoScene = nullptr;
        Urho3D::LuaScriptInstance* m_scriptInstance = nullptr;

        ApplicationWindowWeb *mainWindow;

        QString m_lastScriptOutput = "0\n\n\n0\n\n\n0\n\n\n0";
        QString m_finalOutput = "";
        QString lua_path = "";

        bool scriptNotFinished = true;
        bool lua_started = false;

        int finish_code = -1;
    };
}
