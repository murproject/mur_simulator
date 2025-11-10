#pragma once

#include "QSceneOverlay.h"
#include "ViewportOverlay.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Texture2D.h>

#include <QObject>
#include <QSharedPointer>
#include <tuple>

namespace QUrho {
    class QUrhoScene;

    class QUrhoInput;

    class HydrophoneOverlay : public QObject, public Urho3D::Object, public QSceneOverlay{
        Q_OBJECT
        URHO3D_OBJECT(HydrophoneOverlay, Urho3D::Object)

    public:
        explicit HydrophoneOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        void Update(QUrhoInput *input, float timeStep) override;

        void CreateHydrophones();

        void SetUpdateTime(float value);

        void SetSpeed(float value);

        void SetSignal(float value);

    private:
        std::tuple<float, float, float, float> CalculateLength(Urho3D::Node *pinger);

        void SignalCalculate(Urho3D::Node *pinger, float signal_time, uint8_t signal_value, float val);

        float m_updateTime = 2.0f;
        float m_updateDelta = 0.0f;

        float m_signalTime = 0.0f;
        float m_speed = 5.0f;
        float m_signalDuration = 0.5f;

        Urho3D::Scene *m_scene = nullptr;
        QUrhoScene *m_urhoScene = nullptr;

        Urho3D::WeakPtr<Urho3D::Node> m_pinger_0;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_1;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_2;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_3;
        Urho3D::WeakPtr<Urho3D::Node> m_pinger_4;
        Urho3D::WeakPtr<Urho3D::Node> m_auv;
        Urho3D::WeakPtr<Urho3D::Node> m_topRightThruster;
        Urho3D::WeakPtr<Urho3D::Node> m_topLeftThruster;
        Urho3D::WeakPtr<Urho3D::Node> m_forwardRightThruster;
    };
}
