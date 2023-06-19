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

    class PingerOverlay;

    class HydrophoneOverlay;

    class SharingOverlay;

    class QUrhoScene : public QObject, public Urho3D::Object {
    Q_OBJECT
    URHO3D_OBJECT(QUrhoScene, Urho3D::Object)

    public:
        explicit QUrhoScene(Urho3D::Context *context, QUrhoWidget *urhoWidget, QObject *parent = nullptr);

        bool Load(const QString &scene);

        void HandleUpdate(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

        ViewportOverlay *GetViewportOverlay();

        Urho3D::Scene *GetScene();

        SharingOverlay *GetNetworkOverlay();

        PingerOverlay* GetPingerOverlay();

        HydrophoneOverlay* GetHydrophoneOverlay();

        void AddOverlay(QSceneOverlay *overlay);

        void RemoveOverlay(QSceneOverlay *overlay);

        AUVOverlay *GetAUVOverlay();

        ~QUrhoScene() override;

    private:
        Urho3D::SharedPtr<Urho3D::Scene> m_scene;

        QScopedPointer<ViewportOverlay> m_viewportsOverlay;
        QScopedPointer<AUVOverlay> m_auvOverlay;
        QScopedPointer<SharingOverlay> m_sharingOverlay;
        QScopedPointer<PingerOverlay> m_pingerOverlay;
        QScopedPointer<HydrophoneOverlay> m_hydrophoneOverlay;

        QList<QSceneOverlay *> m_overlays;
        QUrhoWidget *m_urhoWidget;
    };
}




