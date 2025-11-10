#include "LuaOverlay.h"
#include "../Core/QUrhoScene.h"
#include <QDebug>
#include <Urho3D/LuaScript/LuaScriptInstance.h>
#include <Urho3D/LuaScript/LuaFile.h>
#include <Urho3D/LuaScript/LuaScript.h>
#include <QDir>

namespace QUrho {
    LuaOverlay::LuaOverlay(Urho3D::Context *context, QUrho::QUrhoScene *scene, QObject *parent) :
            QObject{parent},
            Urho3D::Object{context},
            m_scene{scene->GetScene()},
            m_urhoScene{scene} {
    }

    void LuaOverlay::GetPath(){
        lua_path = m_urhoScene->GetSceneDir() + "/Script.lua";
    }

    void LuaOverlay::Run(QString script_path) {
        m_isScriptRunning = true;

        GetContext()->RegisterSubsystem(new Urho3D::LuaScript(GetContext()));
        Urho3D::LuaFile* scriptFile = new Urho3D::LuaFile(GetContext());
        scriptFile->LoadFile(QtUrhoStringCast(script_path));

        m_scriptInstance = m_scene->CreateComponent<Urho3D::LuaScriptInstance>();
        m_scriptInstance->CreateObject(scriptFile, "Script");

    }

    void LuaOverlay::Update(QUrhoInput *input, float timeStep) {
        if (QFile::exists(lua_path)) {
            scriptExists = true;
        }

        if (!m_isScriptRunning && scriptExists){
            this->Run(lua_path);
        }
    }

    void LuaOverlay::RestartScript() {
        m_isScriptRunning = false;
        scriptExists = false;
    }

    void LuaOverlay::DisableNode(Urho3D::String name, bool enable)
    {
        Urho3D::Node* particleNode = m_scene->GetChild(name, true);

        if (particleNode)
            particleNode->SetEnabled(enable);
    }

}

