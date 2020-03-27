/*
 * Created by Vladislav Bolotov on 10/11/2019. <vladislav.bolotov@gmail.com>
*/

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

#include <random>

namespace QUrho {
    class QUrhoScene;

    class QUrhoInput;

    class AUVOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(AUVOverlay, Urho3D::Object)

    public:
        explicit AUVOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        void Update(QUrhoInput *input, float timeStep) override;

        void CreateAUV();

        void SetGravity(float value);

        void SetAngularDamping(float value);

        void SetLinearDamping(float value);

        void ResetAUV();

        void ShowFrontCameraImage(bool flag);

        void ShowBottomCameraImage(bool flag);

        Urho3D::Vector3 GetAUVRotations();

        float GetAUVDepth();

        void SetRemote(bool flag);

        std::vector<unsigned char> GetFrontCameraImage();

        std::vector<unsigned char> GetBottomCameraImage();

    signals:

        void TelemetryUpdated();

    private:
        void Shoot();

        void Drop();

        void CreateAUVNode();

        void CreateGrabberNode();

        void CreateThrustersNodes();

        void CreateCamerasNodes();

        void ApplyBuoyancyForces();

        void GrabberOpen();

        void GrabberClose();

        void SetupGravity(float value);

        void CreateWater();

        void CreateRenderTextures();

        void UpdateCamerasImages();

        void UpdateRemoteControl();

        void UpdateRemoteManipulations(float timeStep);

        Urho3D::Node *m_auvNode = nullptr;
        Urho3D::Node *m_grabberNode = nullptr;

        Urho3D::Node *m_frontCameraNode = nullptr;
        Urho3D::Node *m_bottomCameraNode = nullptr;

        QSharedPointer<Viewport> m_bottomCameraViewport;
        QSharedPointer<Viewport> m_frontCameraViewport;

        Urho3D::Node *m_forwardRightThruster = nullptr;
        Urho3D::Node *m_forwardLeftThruster = nullptr;
        Urho3D::Node *m_topLeftThruster = nullptr;
        Urho3D::Node *m_topRightThruster = nullptr;
        Urho3D::Node *m_bottomThruster = nullptr;

        Urho3D::Scene *m_scene = nullptr;
        QUrhoScene *m_urhoScene = nullptr;

        QSharedPointer<Viewport> m_viewport;

        cv::Mat m_frontImage;
        cv::Mat m_bottomImage;

        Urho3D::SharedPtr<Urho3D::Texture2D> m_frontCameraTexture;
        Urho3D::SharedPtr<Urho3D::Texture2D> m_bottomCameraTexture;

        std::default_random_engine m_generator;
        std::uniform_int_distribution<int> m_distribution;

        bool m_grabberOpened = false;
        bool m_grabbed = false;
        bool m_showFrontImage = true;
        bool m_showBottomImage = true;
        bool m_remoteEnabled = true;
        float m_grabberDelta = 0.0f;
        float m_shootDelta = 0.0f;
        float m_dropDelta = 0.0f;
        float m_manipulationsDelta = 0.0f;
        int m_lastGrabValue = 0;
        int m_lastShootValue = 0;
        int m_lastDropValue = 0;
    };
}

