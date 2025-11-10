#include "QUrhoScene.h"
#include "QUrhoHelpers.h"
#include "../Overlay/ViewportOverlay.h"
#include "../Overlay/AUVOverlay.h"
#include "../Overlay/SharingOverlay.h"
#include "../Overlay/HydrophonesOverlay.h"
#include <Urho3D/Resource/ResourceCache.h>
#include "SceneManager.h"

#ifdef WEB_SIM
#include "../Overlay/ScriptsOverlay.h"
#elif DESKTOP_SIM
#include "../Overlay/LuaOverlay.h"
#endif

#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>

namespace QUrho {
    QUrhoScene::QUrhoScene(Urho3D::Context *context, QUrhoWidget *urhoWidget, QObject *parent) :
            QObject{parent},
            Object{context},
            m_scene{new Urho3D::Scene{context}},
            m_viewportsOverlay{new ViewportOverlay{GetContext(), this, this}},
            m_auvOverlay{new AUVOverlay{GetContext(), this, this}},
            m_sharingOverlay{new SharingOverlay{GetContext(), this, this}},
            m_hydrophoneOverlay{new HydrophoneOverlay{GetContext(), this, this}},
#ifdef WEB_SIM
            m_scriptsOverlay{new ScriptsOverlay{GetContext(), this, this}},
#elif DESKTOP_SIM
            m_luaOverlay{new LuaOverlay{GetContext(), this, this}},
#endif
            m_urhoWidget{urhoWidget} {
        SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(QUrhoScene, HandleUpdate));

        m_scene->CreateComponent<Urho3D::Octree>();
        m_scene->CreateComponent<Urho3D::PhysicsWorld>();
        m_scene->CreateComponent<Urho3D::DebugRenderer>();

        AddOverlay(m_viewportsOverlay.data());
        AddOverlay(m_auvOverlay.data());
        AddOverlay(m_sharingOverlay.data());
        AddOverlay(m_hydrophoneOverlay.data());
#ifdef WEB_SIM
        AddOverlay(m_scriptsOverlay.data());
#elif DESKTOP_SIM
        AddOverlay(m_luaOverlay.data());
#endif
    }

    void QUrhoScene::HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
        const float timeStep = eventData[Urho3D::Update::P_TIMESTEP].GetFloat();
        for (auto overlay : m_overlays) {
            if (!overlay) {
                return;
            }
            overlay->Update(m_urhoWidget->GetUrho3DInput(), timeStep);
        }
    }

    bool QUrhoScene::Load(const QString &scene) {
        QFileInfo fileInfo{scene};

        if (!fileInfo.exists() || !fileInfo.isReadable() || !fileInfo.isFile()) {
            return false;
        }

        scene_name = fileInfo.fileName();
        scene_dir = fileInfo.absolutePath();

        Urho3D::File file(GetContext());
        file.Open(QtUrhoStringCast(scene));

        AddCustomAssets(scene_dir);

        bool loadResult = m_scene->LoadXML(file);

        if (loadResult) {
            m_auvOverlay->CreateAUV();
            m_hydrophoneOverlay->CreateHydrophones();
        }
        m_scene->GetComponent<Urho3D::PhysicsWorld>()->SetGravity(Urho3D::Vector3::DOWN * 1.5);
        return loadResult;
    }

    void QUrhoScene::AddCustomAssets(const QString& sceneDir) {
        auto* cache = GetSubsystem<Urho3D::ResourceCache>();
        static Urho3D::String prevAssetsPath = "";

        if (SceneManager::Instance().GetSceneVersion() == 0.0){
            return;
        }

        cache->ReleaseAllResources(true);

        QString dataDir = sceneDir + "/Data";
        Urho3D::String assetsPath = dataDir.toStdString().c_str();

        if (!cache->GetResourceDirs().Contains(assetsPath))
        {
            cache->RemoveResourceDir(prevAssetsPath);
            cache->AddResourceDir(assetsPath);
            prevAssetsPath = assetsPath;
        }
    }

    void QUrhoScene::AddOverlay(QSceneOverlay *overlay) {
        m_overlays.append(overlay);
    }

    void QUrhoScene::RemoveOverlay(QSceneOverlay *overlay) {
        m_overlays.removeAll(overlay);
    }

    ViewportOverlay *QUrhoScene::GetViewportOverlay() {
        if (m_viewportsOverlay) {
            return m_viewportsOverlay.data();
        }
        return nullptr;
    }

    Urho3D::Scene *QUrhoScene::GetScene() {
        return m_scene;
    }

    QString QUrhoScene::GetSceneName(){
        return scene_name;
    }

    QString QUrhoScene::GetSceneDir(){
        return scene_dir;
    }

    AUVOverlay *QUrhoScene::GetAUVOverlay() {
        if (m_auvOverlay) {
            return m_auvOverlay.data();
        }
        return nullptr;
    }

    SharingOverlay *QUrhoScene::GetNetworkOverlay() {
        return m_sharingOverlay.data();
    }

    QUrhoScene::~QUrhoScene() {
        m_auvOverlay.reset(nullptr);
        m_sharingOverlay.reset(nullptr);
        m_viewportsOverlay.reset(nullptr);
        m_hydrophoneOverlay.reset(nullptr);
#ifdef WEB_SIM
        m_scriptsOverlay.reset(nullptr);
#elif DESKTOP_SIM
        m_luaOverlay.reset(nullptr);
#endif
        

    }

    HydrophoneOverlay *QUrhoScene::GetHydrophoneOverlay()
    {
        return m_hydrophoneOverlay.data();
    }

#ifdef WEB_SIM
    ScriptsOverlay *QUrhoScene::GetScriptsOverlay() {
        return m_scriptsOverlay.data();
    }
#elif DESKTOP_SIM
    LuaOverlay *QUrhoScene::GetLuaOverlay()
    {
        return m_luaOverlay.data();
    }
#endif
}

