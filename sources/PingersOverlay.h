#pragma once

#include "QSceneOverlay.h"
#include "ViewportOverlay.h"
#include "QUrhoHelpers.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Texture2D.h>

#include <QObject>
#include <QSharedPointer>
#include <tuple>
#include <random>

namespace QUrho {
    class QUrhoScene;

    class QUrhoInput;

    class PingerOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(PingerOverlay, Urho3D::Object)

    public:
        explicit PingerOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        void Update(QUrhoInput *input, float timeStep) override;

        void CreatePingers();

        void SetUpdateTime(float value);

    private:
        std::tuple<float, float> CalculatePingerLengthAngle(Urho3D::Node *pinger);

        float m_updateTime = 2.0f;
        float m_updateDelta = 0.0f;
        Urho3D::Scene *m_scene = nullptr;
        QUrhoScene *m_urhoScene = nullptr;

        Urho3D::WeakPtr<Urho3D::Node> m_pinger_0;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_1;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_2;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_3;
        Urho3D::WeakPtr<Urho3D::Node> m_auv;


    };
}