#include "QUrhoScene.h"
#include "QUrhoInput.h"
#include "ViewportOverlay.h"
#include "QUrhoHelpers.h"
#include "AUVOverlay.h"
#include "SharingOverlay.h"
#include "PingersOverlay.h"

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

namespace QUrho {
    QUrhoScene::QUrhoScene(Urho3D::Context *context, QUrhoWidget *urhoWidget, QObject *parent) :
            QObject{parent},
            Object{context},
            m_scene{new Urho3D::Scene{context}},
            m_auvOverlay{new AUVOverlay{GetContext(), this, this}},
            m_viewportsOverlay{new ViewportOverlay{GetContext(), this, this}},
            m_sharingOverlay{new SharingOverlay{GetContext(), this, this}},
            m_pingerOverlay{new PingerOverlay{GetContext(), this, this}},
            m_urhoWidget{urhoWidget} {
        SubscribeToEvent(Urho3D::E_UPDATE, URHO3D_HANDLER(QUrhoScene, HandleUpdate));

        m_scene->CreateComponent<Urho3D::Octree>();
        m_scene->CreateComponent<Urho3D::PhysicsWorld>();
        m_scene->CreateComponent<Urho3D::DebugRenderer>();

        AddOverlay(m_viewportsOverlay.data());
        AddOverlay(m_auvOverlay.data());
        AddOverlay(m_sharingOverlay.data());
        AddOverlay(m_pingerOverlay.data());
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

        Urho3D::File file(GetContext());
        file.Open(QtUrhoStringCast(scene));

        bool loadResult = m_scene->LoadXML(file);

        if (loadResult) {
            m_auvOverlay->CreateAUV();
            m_pingerOverlay->CreatePingers();
        }
        m_scene->GetComponent<Urho3D::PhysicsWorld>()->SetGravity(Urho3D::Vector3::DOWN * 1.5);
        return loadResult;
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
        m_pingerOverlay.reset(nullptr);

    }

    PingerOverlay *QUrhoScene::GetPingerOverlay() {
        return m_pingerOverlay.data();
    }

}

