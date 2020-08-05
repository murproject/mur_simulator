/*
 * Created by Vladislav Bolotov on 10/11/2019. <vladislav.bolotov@gmail.com>
*/
#include "AUVOverlay.h"
#include "QUrhoScene.h"
#include "QUrhoInput.h"
#include "ViewportOverlay.h"
#include "SharingOverlay.h"

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/Constraint.h>
#include <Urho3D/Graphics/RenderPath.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Graphics.h>

namespace QUrho {

    class AUVViewport : public Viewport {
    public:
        explicit AUVViewport(Urho3D::Context *context, Urho3D::Scene *scene, Urho3D::Camera *camera,
                             Urho3D::Node *auvNode) :
                Viewport{context, scene, camera},
                m_auvNode{auvNode} {
            SetManualUpdate(true);
        }

        void Update(QUrhoInput *input, float timeStep) override {
            const auto rotationSpeed = 0.2f;
            const auto cameraDistance = 1.8f;
            Urho3D::Quaternion rotation;
            Urho3D::Vector3 position;
            const auto mouseMove = input->GetMouseMove();
            if ((mouseMove.y_ != 0 || mouseMove.x_ != 0) &&
                (input->IsMouseButtonPressed(Qt::MiddleButton) || input->IsKeyPressed(Qt::Key_Shift))) {
                auto yawAngle =
                        m_cameraNode.GetRotation().YawAngle() + static_cast<float>(mouseMove.x_) * rotationSpeed;
                auto pitchAngle =
                        m_cameraNode.GetRotation().PitchAngle() + static_cast<float>(mouseMove.y_) * rotationSpeed;
                rotation = Urho3D::Quaternion(pitchAngle, yawAngle, 0);
                position = m_auvNode->GetPosition() - rotation * Urho3D::Vector3(0.0f, 0.0f, cameraDistance);
            } else {
                auto yawAngle = m_cameraNode.GetRotation().YawAngle();
                auto pitchAngle = m_cameraNode.GetRotation().PitchAngle();
                rotation = Urho3D::Quaternion(pitchAngle, yawAngle, 0);
                position = m_auvNode->GetPosition() - rotation * Urho3D::Vector3(0.0f, 0.0f, cameraDistance);
            }
            m_cameraNode.SetPosition(position);
            m_cameraNode.SetRotation(rotation);
        }

    private:
        Urho3D::Node *m_auvNode = nullptr;
    };


    AUVOverlay::AUVOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent) :
            QObject{parent},
            Object{context},
            m_scene{scene->GetScene()},
            m_urhoScene{scene},
            m_frontImage(cv::Size(320, 240), CV_8UC4),
            m_bottomImage(cv::Size(320, 240), CV_8UC4),
            m_distribution{-5, 5} {
    }

    void AUVOverlay::Update(QUrhoInput *input, float timeStep) {
        m_grabberDelta += timeStep;
        m_shootDelta += timeStep;
        m_dropDelta += timeStep;

        if (!input->IsKeyPressed(Qt::Key_Shift) && !m_remoteEnabled) {
            auto auvBaseSpeed = 1.5f;
            auto rigidBody = m_auvNode->GetComponent<Urho3D::RigidBody>();
            auto rotation = rigidBody->GetRotation();

            if (input->IsKeyPressed(Qt::Key_W)) {
                auto left = m_forwardLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_forwardRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_S)) {
                auto left = m_forwardLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_forwardRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::BACK * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::BACK * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_A)) {
                auto left = m_forwardLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_forwardRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::BACK * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_D)) {
                auto left = m_forwardLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_forwardRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::BACK * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_Q)) {
                auto left = m_topLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_topRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::UP * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::UP * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_E)) {
                auto left = m_topLeftThruster->GetComponent<Urho3D::RigidBody>();
                auto right = m_topRightThruster->GetComponent<Urho3D::RigidBody>();
                left->ApplyForce(rotation * Urho3D::Vector3::DOWN * auvBaseSpeed);
                right->ApplyForce(rotation * Urho3D::Vector3::DOWN * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_Z)) {
                auto bottom = m_bottomThruster->GetComponent<Urho3D::RigidBody>();
                bottom->ApplyForce(rotation * Urho3D::Vector3::LEFT * auvBaseSpeed * 2.0f);
            }

            if (input->IsKeyPressed(Qt::Key_X)) {
                auto bottom = m_bottomThruster->GetComponent<Urho3D::RigidBody>();
                bottom->ApplyForce(rotation * Urho3D::Vector3::RIGHT * auvBaseSpeed * 2.0f);
            }

            if (input->IsKeyPressed(Qt::Key_R) && input->IsKeyPressed(Qt::Key_Control)) {
                ResetAUV();
            }

            if (input->IsKeyPressed(Qt::Key_F) && m_shootDelta >= 1.5) {
                Shoot();
                m_shootDelta = 0.0f;
            }

            if (input->IsKeyPressed(Qt::Key_G) && m_dropDelta >= 1.5) {
                Drop();
                m_dropDelta = 0.0f;
            }


            if (input->IsKeyPressed(Qt::Key_Space)) {
                if (m_grabberDelta > 0.5f) {
                    m_grabberOpened = !m_grabberOpened;
                    auto animator = m_grabberNode->GetComponent<Urho3D::AnimationController>(true);
                    if (m_grabberOpened) {
                        GrabberOpen();
                        animator->PlayExclusive("Animation/GrabberOpen.ani", 0, false, 0.5f);
                    } else {
                        GrabberClose();
                        animator->PlayExclusive("Animation/GrabberIDLE.ani", 0, false, 0.5f);
                    }
                    m_grabberDelta = 0.0f;
                }
            }
        }
        ApplyBuoyancyForces();

        if (m_remoteEnabled) {
            UpdateRemoteControl();
            UpdateRemoteManipulations(timeStep);
        }

        m_viewport->Update(input, timeStep);
        UpdateCamerasImages();
        emit TelemetryUpdated();
    }

    void AUVOverlay::CreateAUV() {
        CreateAUVNode();
        CreateGrabberNode();
        CreateThrustersNodes();
        CreateCamerasNodes();
        SetupGravity(-0.05);
        //CreateWater();
        CreateRenderTextures();
    }


    void AUVOverlay::CreateAUVNode() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();
        m_auvNode = m_scene->CreateChild("AUV");
        auto hullBody = m_auvNode->CreateComponent<Urho3D::RigidBody>();
        hullBody->SetMass(1.5);
        hullBody->SetAngularDamping(0.4f);
        hullBody->SetLinearDamping(0.4f);

        auto hullObject = m_auvNode->CreateComponent<Urho3D::StaticModel>();
        hullObject->SetModel(cache->GetResource<Urho3D::Model>("Models/MiddleAUV.mdl"));
        hullObject->ApplyMaterialList("Materials/AUV/MiddleAUV.txt");
        hullObject->SetCastShadows(true);
        auto hullShape = m_auvNode->CreateComponent<Urho3D::CollisionShape>();
        hullShape->SetBox(Urho3D::Vector3(0.35, 0.110, 0.35));
        hullShape->SetPosition(hullBody->GetPosition() + Urho3D::Vector3(0.0, 0.02, 0.0));

        const Urho3D::Vector3 defaultPosition(0, 5, -5);
        const Urho3D::Quaternion defaultRotation(45, 0, 0);

        m_viewport = QSharedPointer<AUVViewport>::create(GetContext(), m_scene, nullptr, m_auvNode);
        m_viewport->SetTransform(defaultPosition, defaultRotation);
        m_viewport->GetViewport()->GetRenderPath()->Append(
                cache->GetResource<Urho3D::XMLFile>("PostProcess/FXAA2.xml"));

        m_urhoScene->GetViewportOverlay()->AddViewport(m_viewport);
    }

    void AUVOverlay::CreateGrabberNode() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();

        m_grabberNode = m_auvNode->CreateChild("Grabber");
        m_grabberNode->SetPosition(m_auvNode->LocalToWorld(Urho3D::Vector3(0.0, -0.04, 0.15)));

        auto grabberObject = m_grabberNode->CreateComponent<Urho3D::AnimatedModel>();
        auto grabberBody = m_grabberNode->CreateComponent<Urho3D::RigidBody>();
        auto grabberShape = m_grabberNode->CreateComponent<Urho3D::CollisionShape>();
        m_grabberNode->CreateComponent<Urho3D::AnimationController>();

        grabberBody->SetMass(0.05);
        grabberBody->SetCollisionLayer(2);
        grabberBody->SetTrigger(true);

        grabberObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Grabber.mdl"));
        grabberObject->ApplyMaterialList("Materials/AUV/Grabber.txt");
        grabberObject->SetCastShadows(true);

        grabberShape->SetBox(Urho3D::Vector3(0.03, 0.05, 0.03));

        auto grabberConstraint = m_auvNode->CreateComponent<Urho3D::Constraint>();
        grabberConstraint->SetConstraintType(Urho3D::CONSTRAINT_HINGE);
        grabberConstraint->SetOtherBody(m_grabberNode->GetComponent<Urho3D::RigidBody>());
        grabberConstraint->SetWorldPosition(m_grabberNode->GetPosition());
        grabberConstraint->SetDisableCollision(true);
    }

    void AUVOverlay::CreateThrustersNodes() {
        auto thrusterCreate = [](Urho3D::Node *parent, const Urho3D::Vector3 &position) {
            auto thruster = parent->GetScene()->CreateChild();
            auto thrusterBody = thruster->CreateComponent<Urho3D::RigidBody>();
            auto thrusterShape = thruster->CreateComponent<Urho3D::CollisionShape>();
            auto thrusterConstraint = thruster->CreateComponent<Urho3D::Constraint>();

            thruster->SetPosition(parent->LocalToWorld(position));
            thrusterBody->SetMass(0.05);
            thrusterBody->SetTrigger(true);
            thrusterShape->SetBox(Urho3D::Vector3(0.05, 0.05, 0.05));
            thrusterConstraint->SetConstraintType(Urho3D::CONSTRAINT_HINGE);
            thrusterConstraint->SetOtherBody(parent->GetComponent<Urho3D::RigidBody>());
            thrusterConstraint->SetWorldPosition(thruster->GetPosition());
            thrusterConstraint->SetDisableCollision(true);
            return thruster;
        };

        m_forwardRightThruster = thrusterCreate(m_auvNode, Urho3D::Vector3(0.12, 0.0095, -0.07));
        m_forwardLeftThruster = thrusterCreate(m_auvNode, Urho3D::Vector3(-0.12, 0.0095, -0.07));
        m_topRightThruster = thrusterCreate(m_auvNode, Urho3D::Vector3(0.12, 0.0095, 0.06));
        m_topLeftThruster = thrusterCreate(m_auvNode, Urho3D::Vector3(-0.12, 0.0095, 0.06));
        m_bottomThruster = thrusterCreate(m_auvNode, Urho3D::Vector3(0.0, -0.07, 0.0));
    }

    void AUVOverlay::ApplyBuoyancyForces() {
        auto auvRigidBody = m_auvNode->GetComponent<Urho3D::RigidBody>();
        auto rotation = auvRigidBody->GetRotation();

        auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * 0.03f,
                                 (auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0, 0.025, 0)) +
                                 rotation.Inverse().PitchAngle() * Urho3D::Vector3::FORWARD);

        auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * (rotation.Inverse().RollAngle() * 0.05f),
                                 auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0.600, 0.0, 0));

        if (m_auvNode->GetPosition().y_ > -0.05) {
            auvRigidBody->ApplyForce(Urho3D::Vector3::DOWN * 10);
        }

    }

    void AUVOverlay::GrabberOpen() {
        if (m_grabbed) {
            auto grabberConstraint = m_grabberNode->GetComponent<Urho3D::Constraint>();
            auto otherBody = grabberConstraint->GetOtherBody();
            otherBody->SetMass(15.0f);
            otherBody->SetTrigger(false);
            m_grabberNode->RemoveComponent<Urho3D::Constraint>();
            m_grabbed = false;
        }
    }

    void AUVOverlay::GrabberClose() {

        Urho3D::PODVector<Urho3D::RigidBody *> bodies;
        auto grabberBody = m_grabberNode->GetComponent<Urho3D::RigidBody>();
        grabberBody->GetCollidingBodies(bodies);


        for (auto &otherBody : bodies) {
            if (otherBody->GetCollisionLayer() == 3 && !m_grabbed) {
                auto grabberConstraint = m_grabberNode->CreateComponent<Urho3D::Constraint>();
                grabberConstraint->SetConstraintType(Urho3D::CONSTRAINT_HINGE);
                grabberConstraint->SetOtherBody(otherBody);
                grabberConstraint->SetWorldPosition(otherBody->GetNode()->LocalToWorld(Urho3D::Vector3(0.0, 0.0, 0.0)));
                grabberConstraint->SetDisableCollision(true);
                otherBody->SetMass(0.001);
                otherBody->GetNode()->SetPosition(otherBody->GetNode()->LocalToWorld(Urho3D::Vector3(0.0, 0.0, 0.0)));
                m_grabbed = true;
            }
        }
    }

    void AUVOverlay::CreateCamerasNodes() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();

        auto cameraCreate = [](Urho3D::Node *parent, const Urho3D::Vector3 &position,
                               const Urho3D::Quaternion &rotation) {
            auto cameraNode = parent->CreateChild();
            auto camera = cameraNode->CreateComponent<Urho3D::Camera>();
            camera->SetFarClip(500);
            cameraNode->SetPosition(position);
            cameraNode->Rotate(rotation);

            return cameraNode;
        };

        m_frontCameraNode = cameraCreate(m_auvNode, Urho3D::Vector3(0, 0, 0.14), {});

        m_frontCameraViewport = QSharedPointer<Viewport>::create(GetContext(), m_scene,
                                                                 m_frontCameraNode->GetComponent<Urho3D::Camera>());


        m_bottomCameraNode = cameraCreate(m_auvNode, Urho3D::Vector3(0, -0.005, 0.1),
                                          Urho3D::Quaternion(90.0f, Urho3D::Vector3::RIGHT));

        m_bottomCameraViewport = QSharedPointer<Viewport>::create(GetContext(), m_scene,
                                                                  m_bottomCameraNode->GetComponent<Urho3D::Camera>());
    }

    void AUVOverlay::SetupGravity(float value) {

        auto setGravity = [](Urho3D::Node *node, float value) {
            auto body = node->GetComponent<Urho3D::RigidBody>();
            body->SetGravityOverride(Urho3D::Vector3::DOWN * value);
            if (value == 0.0f) {
                body->SetGravityOverride(Urho3D::Vector3::DOWN * 0.0005);
            }
        };

        setGravity(m_auvNode, value);
        setGravity(m_grabberNode, value);
        setGravity(m_forwardRightThruster, value);
        setGravity(m_forwardLeftThruster, value);
        setGravity(m_topLeftThruster, value);
        setGravity(m_topRightThruster, value);
        setGravity(m_bottomThruster, value);
    }

    void AUVOverlay::CreateWater() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();

        auto waterNode = m_scene->CreateChild();
        waterNode->SetScale(Urho3D::Vector3(20.0f, 0.05f, 20.0f));
        waterNode->SetPosition(Urho3D::Vector3(0.0f, -0.05f, 0.0f));
        auto water = waterNode->CreateComponent<Urho3D::StaticModel>();
        water->SetModel(cache->GetResource<Urho3D::Model>("Models/Box.mdl"));
        water->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Water/Water.xml"));
        water->SetViewMask(0x80000000);
    }

    void AUVOverlay::SetGravity(float value) {
        SetupGravity(value);
    }

    void AUVOverlay::SetLinearDamping(float value) {
        auto boyd = m_auvNode->GetComponent<Urho3D::RigidBody>();
        boyd->SetLinearDamping(Urho3D::Clamp(value, 0.0f, 1.0f));
    }

    void AUVOverlay::SetAngularDamping(float value) {
        auto boyd = m_auvNode->GetComponent<Urho3D::RigidBody>();
        boyd->SetAngularDamping(Urho3D::Clamp(value, 0.0f, 1.0f));

    }

    void AUVOverlay::ResetAUV() {
        auto body = m_auvNode->GetComponent<Urho3D::RigidBody>();
        body->SetMass(0);

        auto resetForces = [](Urho3D::Node *node) {
            auto body = node->GetComponent<Urho3D::RigidBody>();
            body->SetTrigger(true);
            body->ResetForces();
            node->SetPosition(Urho3D::Vector3(0.0f, 0.0f, 0.0f));
            node->SetRotation(Urho3D::Quaternion{});
        };

        auto resetRigidBody = [](Urho3D::Node *node) {
            auto body = node->GetComponent<Urho3D::RigidBody>();
            body->SetTrigger(false);
            body->SetMass(1.5);
        };

        resetForces(m_auvNode);
        resetForces(m_grabberNode);
        resetForces(m_forwardRightThruster);
        resetForces(m_forwardLeftThruster);
        resetForces(m_topLeftThruster);
        resetForces(m_topRightThruster);
        resetForces(m_bottomThruster);

        body->SetMass(0);
        resetRigidBody(m_auvNode);

        while (auto node = m_scene->GetChild("DropObject", true)) {
            node->Remove();
        }
        while (auto node = m_scene->GetChild("ShootObject", true)) {
            node->Remove();
        }
        m_lastDropValue = 0;
        m_lastShootValue = 0;
    }

    void AUVOverlay::CreateRenderTextures() {
        m_frontCameraTexture = Urho3D::SharedPtr<Urho3D::Texture2D>(new Urho3D::Texture2D(GetContext()));
        m_bottomCameraTexture = Urho3D::SharedPtr<Urho3D::Texture2D>(new Urho3D::Texture2D(GetContext()));

        m_frontCameraTexture->SetSize(320, 240, Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET);
        m_bottomCameraTexture->SetSize(320, 240, Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET);

        auto renderSurface = m_frontCameraTexture->GetRenderSurface();
        renderSurface->SetViewport(0, m_frontCameraViewport->GetViewport());
        renderSurface->SetUpdateMode(Urho3D::SURFACE_UPDATEALWAYS);
        renderSurface->QueueUpdate();

        renderSurface = m_bottomCameraTexture->GetRenderSurface();
        renderSurface->SetViewport(0, m_bottomCameraViewport->GetViewport());
        renderSurface->SetUpdateMode(Urho3D::SURFACE_UPDATEALWAYS);
        renderSurface->QueueUpdate();
    }

    void AUVOverlay::UpdateCamerasImages() {
        m_frontCameraTexture->GetData(0, m_frontImage.data);
        m_bottomCameraTexture->GetData(0, m_bottomImage.data);

        cv::Mat image_front;
        cv::Mat image_bottom;

        cv::cvtColor(m_frontImage, m_frontImage, cv::COLOR_RGBA2BGRA);
        cv::cvtColor(m_bottomImage, m_bottomImage, cv::COLOR_RGBA2BGRA);

        if (m_showFrontImage) {
            cv::imshow("Front camera image", m_frontImage);
        }

        if (m_showBottomImage) {
            cv::imshow("Bottom camera image", m_bottomImage);
        }
    }

    void AUVOverlay::ShowFrontCameraImage(bool flag) {
        m_showFrontImage = flag;
    }

    void AUVOverlay::ShowBottomCameraImage(bool flag) {
        m_showBottomImage = flag;
    }

    Urho3D::Vector3 AUVOverlay::GetAUVRotations() {
        return m_auvNode->GetRotation().EulerAngles();
    }

    float AUVOverlay::GetAUVDepth() {
        return m_auvNode->GetPosition().y_ * -1.0f;
    }

    void AUVOverlay::SetRemote(bool flag) {
        m_remoteEnabled = flag;
    }

    void AUVOverlay::UpdateRemoteControl() {
        if (!m_remoteEnabled) {
            return;
        }
        auto auvSpeed = 1.1f / 100.0f;

        auto rigidBody = m_auvNode->GetComponent<Urho3D::RigidBody>();
        auto rotation = rigidBody->GetRotation();
        auto thrust = m_urhoScene->GetNetworkOverlay()->GetControl().thrusters;

        auto forward_left = m_forwardLeftThruster->GetComponent<Urho3D::RigidBody>();
        auto forward_right = m_forwardRightThruster->GetComponent<Urho3D::RigidBody>();
        auto top_left = m_topLeftThruster->GetComponent<Urho3D::RigidBody>();
        auto top_right = m_topRightThruster->GetComponent<Urho3D::RigidBody>();
        auto bottom = m_bottomThruster->GetComponent<Urho3D::RigidBody>();

//        for (auto i = 0; i < 8; ++i) {
//            if (thrust[i] != 0 && std::abs(thrust[i]) > 5) {
//                thrust[i] += m_distribution(m_generator);
//            }
//        }

        forward_left->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvSpeed * static_cast<float>(thrust[0]));
        forward_right->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvSpeed * static_cast<float>(thrust[1]));
        top_left->ApplyForce(rotation * Urho3D::Vector3::UP * auvSpeed * static_cast<float>(thrust[2]));
        top_right->ApplyForce(rotation * Urho3D::Vector3::UP * auvSpeed * static_cast<float>(thrust[3]));
        bottom->ApplyForce(rotation * Urho3D::Vector3::LEFT * auvSpeed * static_cast<float>(thrust[4]));
    }

    std::vector<unsigned char> AUVOverlay::GetFrontCameraImage() {
        std::vector<unsigned char> compressed;
        cv::imencode(".jpg", m_frontImage, compressed);
        return std::move(compressed);
    }

    std::vector<unsigned char> AUVOverlay::GetBottomCameraImage() {
        std::vector<unsigned char> compressed;
        cv::imencode(".jpg", m_bottomImage, compressed);
        return std::move(compressed);
    }

    void AUVOverlay::UpdateRemoteManipulations(float timeStep) {
        m_manipulationsDelta += timeStep;
        if (m_manipulationsDelta < 1) {
            return;
        }

        auto grabber = m_urhoScene->GetNetworkOverlay()->GetControl().colorR;
        auto shoot = m_urhoScene->GetNetworkOverlay()->GetControl().colorG;
        auto drop = m_urhoScene->GetNetworkOverlay()->GetControl().colorB;

        if (shoot == 0) {
            m_lastShootValue = 0;
        }

        if (drop == 0) {
            m_lastDropValue = 0;
        }
        if (shoot != 0 && m_lastShootValue != shoot) {
            Shoot();
            m_lastShootValue = shoot;
        }

        if (drop != 0 && m_lastDropValue != drop) {
            Drop();
            m_lastDropValue = drop;
        }
        if (grabber != m_lastGrabValue) {
            m_grabberOpened = static_cast<bool>(grabber);
            auto animator = m_grabberNode->GetComponent<Urho3D::AnimationController>(true);
            if (m_grabberOpened) {
                GrabberOpen();
                animator->PlayExclusive("Animation/GrabberOpen.ani", 0, false, 0.5f);
            } else {
                GrabberClose();
                animator->PlayExclusive("Animation/GrabberIDLE.ani", 0, false, 0.5f);
            }
            m_lastGrabValue = grabber;
        }
        m_manipulationsDelta = 0.0f;
    }

    void AUVOverlay::Shoot() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();

        auto boxNode = m_scene->CreateChild("ShootObject");
        boxNode->SetPosition(m_auvNode->GetPosition() + m_auvNode->GetRotation() * Urho3D::Vector3(0.0f, 0.0, 0.3f));
        boxNode->SetScale(Urho3D::Vector3(0.03, 0.05, 0.03));
        boxNode->SetRotation(m_auvNode->GetRotation() * Urho3D::Quaternion(-90.0f, 0.0f, 0.0f));
        auto boxObject = boxNode->CreateComponent<Urho3D::StaticModel>();
        boxObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Cylinder.mdl"));
        boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Colors/Magenta.xml"));

        auto body = boxNode->CreateComponent<Urho3D::RigidBody>();
        body->SetRollingFriction(0.75f);
        body->SetMass(3.5f);
        body->SetFriction(0.75f);
        body->SetLinearDamping(0.2f);
        body->SetCollisionLayer(3);
        auto shape = boxNode->CreateComponent<Urho3D::CollisionShape>();
        shape->SetBox(Urho3D::Vector3::ONE);

        const float velocity = 0.5f;
        body->SetGravityOverride(Urho3D::Vector3(0.0f, -0.5f, 0.0f));
        body->SetLinearVelocity(m_auvNode->GetRotation() * Urho3D::Vector3(0.0f, 0.0f, 3.0f) * velocity);
    }

    void AUVOverlay::Drop() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();
        auto boxNode = m_scene->CreateChild("DropObject");
        boxNode->SetPosition(m_auvNode->GetPosition() + m_auvNode->GetRotation() * Urho3D::Vector3(0.0f, -0.05f, 0.1f));
        boxNode->SetScale(0.03f);
        auto boxObject = boxNode->CreateComponent<Urho3D::StaticModel>();
        boxObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Sphere.mdl"));
        boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Colors/Cyan.xml"));
        auto body = boxNode->CreateComponent<Urho3D::RigidBody>();
        body->SetMass(3.5f);
        body->SetCollisionLayer(3);
        body->SetFriction(0.5f);
        body->SetRollingFriction(0.75f);
        auto shape = boxNode->CreateComponent<Urho3D::CollisionShape>();
        shape->SetBox(Urho3D::Vector3::ONE);
        body->SetGravityOverride(Urho3D::Vector3(0.0f, -0.5f, 0.0f));
    }
}