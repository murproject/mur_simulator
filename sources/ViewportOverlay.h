#pragma once

#include "QSceneOverlay.h"

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Viewport.h>

#include <QtCore/QObject>
#include <QSharedPointer>
#include <QVector>

namespace QUrho {
    class QUrhoInput;

    class QUrhoScene;

    class Viewport {
    public:
        explicit Viewport(Urho3D::Context *context, Urho3D::Scene *scene, Urho3D::Camera *camera);

        void SetWorldTransform(const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation);

        void SetTransform(const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation);

        void SetRect(Urho3D::IntRect rect);

        void SetManualUpdate(bool flag);

        bool GetManualUpdate();

        virtual void Update(QUrhoInput *input, float timeStep);

        Urho3D::Node *GetNode();

        Urho3D::Camera *GetCamera();

        Urho3D::Viewport *GetViewport();

    protected:
        Urho3D::Node m_cameraNode;
        Urho3D::Camera *m_camera;
        Urho3D::SharedPtr<Urho3D::Viewport> m_viewport;
        bool m_manualUpdate = false;
    };

    class ViewportOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(ViewportOverlay, Urho3D::Object);
    public:
        explicit ViewportOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        virtual void AddViewport(QSharedPointer<Viewport> &viewport);

        void Update(QUrhoInput *input, float timeStep) override;

    private:
        void HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

        void SetupViewports();

        void UpdateViewportsSizes();

        void SelectCurrentViewport(const Urho3D::IntVector2 &mousePosition);

        int m_currentViewport = 0;
        Urho3D::Scene *m_scene;
        QUrhoScene *m_urhoScene;
        QVector<QSharedPointer<Viewport>> m_viewports;
    };
}



