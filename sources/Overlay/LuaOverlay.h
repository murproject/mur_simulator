#pragma once

#include "QSceneOverlay.h"
#include "../Core/QUrhoHelpers.h"
#include "../Ui/ApplicationWindowDesktop.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/LuaScript/LuaScriptInstance.h>

#include <QObject>

namespace QUrho {
    class QUrhoScene;

    class QUrhoInput;

    class LuaOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(LuaOverlay, Urho3D::Object)

    public:
        explicit LuaOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        void Update(QUrhoInput *input, float timeStep) override;

        void GetPath();

        void Run(QString);

        void Stop();

        void DisableNode(Urho3D::String, bool);

        void RestartScript();

    private:
        bool m_isScriptRunning = false;
        bool scriptExists = false;
        Urho3D::Scene *m_scene = nullptr;
        QUrhoScene *m_urhoScene = nullptr;
        Urho3D::LuaScriptInstance* m_scriptInstance = nullptr;
        QString lua_path;
    };
}
