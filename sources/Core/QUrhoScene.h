#pragma once

#include <QObject>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include "QUrhoWidget.h"

namespace QUrho {
    class ViewportOverlay;

    class QUrhoInput;

    class QSceneOverlay;

    class AUVOverlay;

    class HydrophoneOverlay;

    class SharingOverlay;

#ifdef WEB_SIM
    class ScriptsOverlay;
#elif DESKTOP_SIM
    class LuaOverlay;
#endif
    class QUrhoScene : public QObject, public Urho3D::Object {
    Q_OBJECT
    URHO3D_OBJECT(QUrhoScene, Urho3D::Object)

    public:
        explicit QUrhoScene(Urho3D::Context *context, QUrhoWidget *urhoWidget, QObject *parent = nullptr);

        bool Load(const QString &scene);

        void AddCustomAssets(const QString& sceneDir);

        void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

        QString GetSceneName();

        ViewportOverlay *GetViewportOverlay();

        Urho3D::Scene *GetScene();

        QString GetSceneDir();

        SharingOverlay *GetNetworkOverlay();

        HydrophoneOverlay* GetHydrophoneOverlay();

#ifdef WEB_SIM
        ScriptsOverlay* GetScriptsOverlay();
#elif DESKTOP_SIM
        LuaOverlay* GetLuaOverlay();
#endif
        void AddOverlay(QSceneOverlay *overlay);

        void RemoveOverlay(QSceneOverlay *overlay);

        AUVOverlay *GetAUVOverlay();

        ~QUrhoScene() override;

    private:
        Urho3D::SharedPtr<Urho3D::Scene> m_scene;

        QScopedPointer<ViewportOverlay> m_viewportsOverlay;
        QScopedPointer<AUVOverlay> m_auvOverlay;
        QScopedPointer<SharingOverlay> m_sharingOverlay;
        QScopedPointer<HydrophoneOverlay> m_hydrophoneOverlay;
#ifdef WEB_SIM
        QScopedPointer<ScriptsOverlay> m_scriptsOverlay;
#elif DESKTOP_SIM
        QScopedPointer<LuaOverlay> m_luaOverlay;
#endif

        QList<QSceneOverlay *> m_overlays;
        QUrhoWidget *m_urhoWidget;
        QString scene_name = "";
        QString scene_dir = "";
    };
}
