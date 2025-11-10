/*
 * Created by Vladislav Bolotov on 10/11/2019. <vladislav.bolotov@gmail.com>
*/
#include "AUVOverlay.h"
#include "../Core/QUrhoScene.h"
#include "../Core/QUrhoInput.h"
#include "ViewportOverlay.h"
#include "SharingOverlay.h"
#include "../Core/SceneManager.h"

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
#include <Urho3D/Graphics/Material.h>

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
#ifdef WEB_SIM
            const auto cameraDistance = 3.0f;
            Urho3D::Quaternion rotation;
            Urho3D::Vector3 position;
            rotation = Urho3D::Quaternion(70, 0, 0);
            position = m_auvNode->GetPosition() - rotation * Urho3D::Vector3(0.0f, -0.5f, cameraDistance);
#elif DESKTOP_SIM
            const auto rotationSpeed = 0.2f;
            const auto cameraDistance = 1.8f;

            std::pair<int, int> deltaXY;
            static std::pair<int, int> prevXY;

            float yawAngle = m_cameraNode.GetRotation().YawAngle();
            float pitchAngle = m_cameraNode.GetRotation().PitchAngle();

            QCursor cursor;
            int cursorX = cursor.pos().x();
            int cursorY = cursor.pos().y();

            deltaXY.first = cursorX - prevXY.first;
            deltaXY.second = cursorY - prevXY.second;

            prevXY.first = cursorX;
            prevXY.second = cursorY;

            if ((deltaXY.second != 0 || deltaXY.first != 0) &&
                (input->IsMouseButtonDown(Qt::MiddleButton)  || input->IsMouseButtonDown(Qt::RightButton) || input->IsKeyPressed(Qt::Key_Shift))) {
                yawAngle += static_cast<float>(deltaXY.first) * rotationSpeed;
                pitchAngle += static_cast<float>((deltaXY.second)) * rotationSpeed;
            }

            static const float maxPitch = 10.0;
            pitchAngle = qMin(pitchAngle, +90.0 - maxPitch);
            pitchAngle = qMax(pitchAngle, -90.0 + maxPitch);

            Urho3D::Quaternion rotation = Urho3D::Quaternion(pitchAngle, yawAngle, 0);
            Urho3D::Vector3 position = m_auvNode->GetPosition() - rotation * Urho3D::Vector3(0.0f, 0.0f, cameraDistance);
#endif
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
#ifdef WEB_SIM
            m_staticImage(cv::Size(320, 240), CV_8UC4),
#endif
            m_distribution{-5, 5} {
    }

    void AUVOverlay::Update(QUrhoInput *input, float timeStep) {
        m_grabberDelta += timeStep;
        m_shootDelta += timeStep;
        m_dropDelta += timeStep;

        if (!input->IsKeyPressed(Qt::Key_Shift) && !m_remoteEnabled) {
            auto auvBaseSpeed = 1.1f;
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
                bottom->ApplyForce(rotation * Urho3D::Vector3::LEFT * auvBaseSpeed);
            }

            if (input->IsKeyPressed(Qt::Key_X)) {
                auto bottom = m_bottomThruster->GetComponent<Urho3D::RigidBody>();
                bottom->ApplyForce(rotation * Urho3D::Vector3::RIGHT * auvBaseSpeed);
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
                        sceneIsLegacy ? animator->PlayExclusive("Animation/GrabberOpen.ani", 0, false, 0.5f) :
                            animator->PlayExclusive("Animation/OpenGrabber.ani", 0, false, 0.25f);
                    } else {
                        GrabberClose();
                        sceneIsLegacy ? animator->PlayExclusive("Animation/GrabberIDLE.ani", 0, false, 0.5f) :
                            animator->PlayExclusive("Animation/CloseGrabber.ani", 0, false, 0.25f);
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

#ifdef DESKTOP_SIM
        m_viewport->Update(input, timeStep);
#endif
        UpdateCamerasImages();
        emit TelemetryUpdated();
    }

    void AUVOverlay::CreateAUV() {
        sceneIsLegacy = SceneManager::Instance().GetSceneVersion() == 0.0f ? true : false;
        CreateAUVNode();
        CreateGrabberNode();
        CreateThrustersNodes();
        CreateCamerasNodes();
        SetupGravity(-0.05);
#ifdef DESKTOP_SIM
        if (sceneIsLegacy) {
            CreateWater();
        }
#endif
        CreateRenderTextures();
        PrepareInvisibleObjects();
        m_grabberOpened = false;
        m_grabbed = false;
    }


    void AUVOverlay::CreateAUVNode() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();
        m_auvNode = m_scene->CreateChild("AUV");
        auto hullBody = m_auvNode->CreateComponent<Urho3D::RigidBody>();
        hullBody->SetMass(1.5);
        hullBody->SetAngularDamping(0.4f);
        hullBody->SetLinearDamping(0.4f);

        if (sceneIsLegacy) {
            auto hullObject = m_auvNode->CreateComponent<Urho3D::StaticModel>();
            hullObject->SetModel(cache->GetResource<Urho3D::Model>("Models/MiddleAUV.mdl"));
            hullObject->ApplyMaterialList("Materials/AUV/MiddleAUV.txt");
            hullObject->SetCastShadows(true);
            auto hullShape = m_auvNode->CreateComponent<Urho3D::CollisionShape>();
            hullShape->SetBox(Urho3D::Vector3(0.35, 0.110, 0.35));
            hullShape->SetPosition(hullBody->GetPosition() + Urho3D::Vector3(0.0, 0.02, 0.0));
        } else {
            auto hullObject = m_auvNode->CreateComponent<Urho3D::StaticModel>();
            hullObject->SetModel(cache->GetResource<Urho3D::Model>("Models/AUV/MAUV.mdl"));
            hullObject->ApplyMaterialList("Materials/AUV/MAUV.txt");
            hullObject->SetCastShadows(true);
            auto hullShape = m_auvNode->CreateComponent<Urho3D::CollisionShape>();
            hullShape->SetBox(Urho3D::Vector3(0.3, 0.14, 0.325));
            hullShape->SetPosition(hullBody->GetPosition() + Urho3D::Vector3(0.0, 0.0, 0.0));

            auto thruster = m_auvNode->CreateChild("BottomThrust");
            thruster->SetPosition(Urho3D::Vector3(0.0f, -0.11f, 0.0f));

            auto thrusterObject = thruster->CreateComponent<Urho3D::StaticModel>();
            thrusterObject->SetModel(cache->GetResource<Urho3D::Model>("Models/AUV/Thruster.mdl"));
            thrusterObject->ApplyMaterialList("Materials/AUV/Thruster.txt");
            thrusterObject->SetCastShadows(true);
            thrusterObject->SetViewMask(128);

            auto thrusterShape = m_auvNode->CreateComponent<Urho3D::CollisionShape>();
            thrusterShape->SetBox(Urho3D::Vector3(0.075f, 0.075f, 0.075f));
            thrusterShape->SetPosition(Urho3D::Vector3(0.0f, -0.11f, 0.0f));
        }


        const Urho3D::Vector3 defaultPosition(0, 5, -5);
        const Urho3D::Quaternion defaultRotation(45, 0, 0);

#ifdef DESKTOP_SIM
        m_viewport = QSharedPointer<AUVViewport>::create(GetContext(), m_scene, nullptr, m_auvNode);
        m_viewport->SetTransform(defaultPosition, defaultRotation);

        if (sceneIsLegacy) {
            m_viewport->GetViewport()->GetRenderPath()->Append(
                cache->GetResource<Urho3D::XMLFile>("PostProcess/FXAA2.xml"));
        } else {
            m_viewport->GetViewport()->SetRenderPath(
                cache->GetResource<Urho3D::XMLFile>("RenderPaths/MurSimulator_Main.xml")
                );
        }

        m_urhoScene->GetViewportOverlay()->AddViewport(m_viewport);
#endif
    }

    void AUVOverlay::CreateGrabberNode() {
        auto cache = GetSubsystem<Urho3D::ResourceCache>();

        m_grabberNode = m_auvNode->CreateChild("Grabber");
        sceneIsLegacy ? m_grabberNode->SetPosition(m_auvNode->LocalToWorld(Urho3D::Vector3(0.0, -0.04, 0.15))) :
            m_grabberNode->SetPosition(m_auvNode->LocalToWorld(Urho3D::Vector3(0.0, -0.14, 0.13)));

        auto grabberObject = m_grabberNode->CreateComponent<Urho3D::AnimatedModel>();
        auto grabberBody = m_grabberNode->CreateComponent<Urho3D::RigidBody>();
        auto grabberShape = m_grabberNode->CreateComponent<Urho3D::CollisionShape>();
        m_grabberNode->CreateComponent<Urho3D::AnimationController>();

        grabberBody->SetMass(0.05);
        grabberBody->SetCollisionLayer(2);
        grabberBody->SetTrigger(true);

        if (sceneIsLegacy) {
            grabberObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Grabber.mdl"));
            grabberObject->ApplyMaterialList("Materials/AUV/Grabber.txt");
            grabberObject->SetCastShadows(true);
        } else {
            grabberObject->SetModel(cache->GetResource<Urho3D::Model>("Models/AUV/Grabber.mdl"));
            grabberObject->ApplyMaterialList("Materials/AUV/GrabberAUV.txt");
            grabberObject->SetCastShadows(true);
            grabberObject->SetViewMask(128);
        }

        grabberShape->SetBox(Urho3D::Vector3(0.03, 0.05, 0.03));

        auto grabberConstraint = m_auvNode->CreateComponent<Urho3D::Constraint>();
        grabberConstraint->SetConstraintType(Urho3D::CONSTRAINT_HINGE);
        grabberConstraint->SetOtherBody(m_grabberNode->GetComponent<Urho3D::RigidBody>());
        grabberConstraint->SetWorldPosition(m_grabberNode->GetPosition());
        grabberConstraint->SetDisableCollision(true);
    }

    void AUVOverlay::CreateThrustersNodes() {
        auto thrusterCreate = [](Urho3D::Node *parent, const Urho3D::String thrusterName, const Urho3D::Vector3 &position) {
            auto thruster = parent->GetScene()->CreateChild(thrusterName);
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

        m_forwardRightThruster = thrusterCreate(m_auvNode, "Thruster_FR", Urho3D::Vector3(0.12, 0.0095, -0.07));
        m_forwardLeftThruster = thrusterCreate(m_auvNode, "Thruster_FL", Urho3D::Vector3(-0.12, 0.0095, -0.07));
        m_topRightThruster = thrusterCreate(m_auvNode, "Thruster_TR", Urho3D::Vector3(0.12, 0.0095, 0.06));
        m_topLeftThruster = thrusterCreate(m_auvNode, "Thruster_TL", Urho3D::Vector3(-0.12, 0.0095, 0.06));
        m_bottomThruster = thrusterCreate(m_auvNode, "Thruster_B", Urho3D::Vector3(0.0, -0.15, 0.0));
    }

    void AUVOverlay::ApplyBuoyancyForces() {
        auto auvRigidBody = m_auvNode->GetComponent<Urho3D::RigidBody>();
        auto rotation = auvRigidBody->GetRotation();

        if (sceneIsLegacy) {
            auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * 0.03f,
                                     (auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0, 0.025, 0)) +
                                         rotation.Inverse().PitchAngle() * Urho3D::Vector3::FORWARD);

            auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * (rotation.Inverse().RollAngle() * 0.05f),
                                     auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0.600, 0.0, 0));
        } else {
            auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * 0.04f,
                                     (auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0, 0.1, 0)) +
                                         rotation.Inverse().PitchAngle() * Urho3D::Vector3::FORWARD);

            auvRigidBody->ApplyForce(rotation * Urho3D::Vector3::DOWN * (rotation.Inverse().RollAngle() * 0.05f),
                                     auvRigidBody->GetCenterOfMass() - Urho3D::Vector3(0.6, 0.0, 0));
        }

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

#ifdef WEB_SIM
    std::tuple<float, float, float> AUVOverlay::CameraPosition(QString name, QString default_camera_pos) {
        QString camera_pos = mainWindow->LoadPaths(name, default_camera_pos);
        QStringList values = camera_pos.split(" ");

        if (values.length() < 3){
            values = default_camera_pos.split(" ");
        }

        float x = values[0].toFloat();
        float y = values[1].toFloat();
        float z = values[2].toFloat();

        return std::make_tuple(x, y, z);
    }
#endif

    void AUVOverlay::CreateCamerasNodes() {
        auto cameraCreate = [](Urho3D::Node *parent, const Urho3D::Vector3 &position,
                               const Urho3D::Quaternion &rotation) {
            auto cameraNode = parent->CreateChild();
            auto camera = cameraNode->CreateComponent<Urho3D::Camera>();
            camera->SetFarClip(500);
            cameraNode->SetPosition(position);
            cameraNode->Rotate(rotation);

            return cameraNode;
        };

        if (sceneIsLegacy) {
            m_frontCameraNode = cameraCreate(m_auvNode, Urho3D::Vector3(0, 0, 0.14), {});

            m_frontCameraViewport = QSharedPointer<Viewport>::create(GetContext(), m_scene,
                                                                     m_frontCameraNode->GetComponent<Urho3D::Camera>());


            m_bottomCameraNode = cameraCreate(m_auvNode, Urho3D::Vector3(0, -0.005, 0.1),
                                              Urho3D::Quaternion(90.0f, Urho3D::Vector3::RIGHT));

            m_bottomCameraViewport = QSharedPointer<Viewport>::create(GetContext(), m_scene,
                                                                      m_bottomCameraNode->GetComponent<Urho3D::Camera>());
        } else {
            m_frontCameraNode = cameraCreate(
                m_auvNode,
                Urho3D::Vector3(0, 0, 0.04),
                {}
            );

            m_frontCameraViewport = QSharedPointer<Viewport>::create(
                GetContext(),
                m_scene,
                m_frontCameraNode->GetComponent<Urho3D::Camera>()
            );

            m_bottomCameraNode = cameraCreate(
                m_auvNode,
                Urho3D::Vector3(0, 0.03, 0.08),
                Urho3D::Quaternion(90.0f, Urho3D::Vector3::RIGHT)
            );

            m_bottomCameraNode->GetComponent<Urho3D::Camera>()->SetNearClip(0.095f);

            m_bottomCameraViewport = QSharedPointer<Viewport>::create(
                GetContext(),
                m_scene,
                m_bottomCameraNode->GetComponent<Urho3D::Camera>()
            );

            auto cache = GetSubsystem<Urho3D::ResourceCache>();
            auto cameraRenderPath = cache->GetResource<Urho3D::XMLFile>("RenderPaths/MurSimulator_Camera.xml");

            m_frontCameraViewport->GetViewport()->SetRenderPath(cameraRenderPath);
            m_bottomCameraViewport->GetViewport()->SetRenderPath(cameraRenderPath);

            m_bottomCameraViewport->GetCamera()->SetViewMask(0x00000008);
        }
#ifdef WEB_SIM
        auto[pos_x, pos_y, pos_z] = CameraPosition("camera_pos", "0.0 10.0 0.0");
        auto[rot_x, rot_y, rot_z] = CameraPosition("camera_rot", "90.0 0.0 0.0");

        m_staticCameraNode = cameraCreate(m_scene, Urho3D::Vector3(pos_x, pos_y, pos_z),
                                          Urho3D::Quaternion(rot_x, rot_y, rot_z));

        m_staticCameraViewport = QSharedPointer<Viewport>::create(GetContext(), m_scene,
                                                                  m_staticCameraNode->GetComponent<Urho3D::Camera>());

        m_staticCameraViewport->GetCamera()->SetViewMask(0x80000000);
#endif
    }

    void AUVOverlay::SetupGravity(float value) {

        auto setGravity = [](Urho3D::Node *node, float value) {
            auto body = node->GetComponent<Urho3D::RigidBody>();
            body->SetGravityOverride(Urho3D::Vector3::DOWN * (value - 0.009f));
            if (value == 0.0f) {
                body->SetGravityOverride(Urho3D::Vector3::DOWN * -0.009f);
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

        const Urho3D::Quaternion defaultRotation(45, 0, 0);
        m_viewport->GetCamera()->GetNode()->SetRotation(defaultRotation);
    }

    void AUVOverlay::CreateRenderTextures() {
        m_frontCameraTexture = Urho3D::SharedPtr<Urho3D::Texture2D>(new Urho3D::Texture2D(GetContext()));
        m_bottomCameraTexture = Urho3D::SharedPtr<Urho3D::Texture2D>(new Urho3D::Texture2D(GetContext()));
#ifdef WEB_SIM
        m_staticCameraTexture = Urho3D::SharedPtr<Urho3D::Texture2D>(new Urho3D::Texture2D(GetContext()));
#endif
        m_frontCameraTexture->SetSize(320, 240, Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET, 4);
        m_bottomCameraTexture->SetSize(320, 240, Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET, 4);
#ifdef WEB_SIM
        m_staticCameraTexture->SetSize(320, 240, Urho3D::Graphics::GetRGBAFormat(), Urho3D::TEXTURE_RENDERTARGET);
#endif

        auto renderSurface = m_frontCameraTexture->GetRenderSurface();
        renderSurface->SetViewport(0, m_frontCameraViewport->GetViewport());
        renderSurface->SetUpdateMode(Urho3D::SURFACE_UPDATEALWAYS);
        renderSurface->QueueUpdate();

        renderSurface = m_bottomCameraTexture->GetRenderSurface();
        renderSurface->SetViewport(0, m_bottomCameraViewport->GetViewport());
        renderSurface->SetUpdateMode(Urho3D::SURFACE_UPDATEALWAYS);
        renderSurface->QueueUpdate();

#ifdef WEB_SIM
        renderSurface = m_staticCameraTexture->GetRenderSurface();
        renderSurface->SetViewport(0, m_staticCameraViewport->GetViewport());
        renderSurface->SetUpdateMode(Urho3D::SURFACE_UPDATEALWAYS);
        renderSurface->QueueUpdate();
#endif
    }

    void AUVOverlay::UpdateCamerasImages() {
        m_frontCameraTexture->GetData(0, m_frontImage.data);
        cv::cvtColor(m_frontImage, m_frontImage, cv::COLOR_RGBA2BGRA);

        if (m_showFrontImage) {
            cv::imshow("Front camera image", m_frontImage);
        }


        m_bottomCameraTexture->GetData(0, m_bottomImage.data);
        cv::cvtColor(m_bottomImage, m_bottomImage, cv::COLOR_RGBA2BGRA);

        if (m_showBottomImage) {
            cv::imshow("Bottom camera image", m_bottomImage);
        }

#ifdef WEB_SIM
        m_staticCameraTexture->GetData(0, m_staticImage.data);
        cv::cvtColor(m_staticImage, m_staticImage, cv::COLOR_RGBA2BGRA);
        if (m_showStaticImage) {
            cv::imshow("Static camera image", m_staticImage);
        }
#endif
        if (!isWindows) {
            cv::waitKey(1);
        }
    }

#ifdef WEB_SIM
    std::tuple <cv::Mat, cv::Mat, cv::Mat> AUVOverlay::GetCameraImages() {
        cv:: Mat front_img, bottom_img, static_img;

        cv::cvtColor(m_frontImage, front_img, cv::COLOR_BGRA2BGR);
        cv::cvtColor(m_bottomImage, bottom_img, cv::COLOR_BGRA2BGR);
        cv::cvtColor(m_staticImage, static_img, cv::COLOR_BGRA2BGR);

        return {front_img, bottom_img, static_img};
    }
#endif

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

    QSharedPointer<Viewport> AUVOverlay::GetFrontCameraViewport() {
        return m_frontCameraViewport;
    }

    QSharedPointer<Viewport> AUVOverlay::GetBottomCameraViewport() {
        return m_bottomCameraViewport;
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

        forward_left->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvSpeed * static_cast<float>(thrust[0]));
        forward_right->ApplyForce(rotation * Urho3D::Vector3::FORWARD * auvSpeed * static_cast<float>(thrust[1]));
        top_left->ApplyForce(rotation * Urho3D::Vector3::UP * auvSpeed * static_cast<float>(thrust[2]));
        top_right->ApplyForce(rotation * Urho3D::Vector3::UP * auvSpeed * static_cast<float>(thrust[3]));
        bottom->ApplyForce(rotation * Urho3D::Vector3::LEFT * auvSpeed * static_cast<float>(thrust[4]));
    }

    std::vector<unsigned char> AUVOverlay::GetFrontCameraImage() {
        std::vector<unsigned char> compressed;
        cv::imencode(".jpg", m_frontImage, compressed);
        return compressed;
    }

    std::vector<unsigned char> AUVOverlay::GetBottomCameraImage() {
        std::vector<unsigned char> compressed;
        cv::imencode(".jpg", m_bottomImage, compressed);
        return compressed;
    }

#ifdef WEB_SIM
    std::vector<unsigned char> AUVOverlay::GetStaticCameraImage() {
        std::vector<unsigned char> compressed;
        cv::imencode(".jpg", m_staticImage, compressed);
        return compressed;
    }
#endif
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
                animator->PlayExclusive("Animation/OpenGrabber.ani", 0, false, 0.25f);
            } else {
                GrabberClose();
                animator->PlayExclusive("Animation/CloseGrabber.ani", 0, false, 0.25f);
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

        sceneIsLegacy ? boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Colors/Magenta.xml")) :
            boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Objects/Colors/Magenta.xml"));

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
        boxNode->SetPosition(m_auvNode->GetPosition() + m_auvNode->GetRotation() * Urho3D::Vector3(0.0f, -0.09f, 0.1f));
        boxNode->SetScale(0.03f);
        auto boxObject = boxNode->CreateComponent<Urho3D::StaticModel>();
        boxObject->SetModel(cache->GetResource<Urho3D::Model>("Models/Sphere.mdl"));

        sceneIsLegacy ? boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Colors/Cyan.xml")) :
            boxObject->SetMaterial(cache->GetResource<Urho3D::Material>("Materials/Objects/Colors/Cyan.xml"));

        auto body = boxNode->CreateComponent<Urho3D::RigidBody>();
        body->SetMass(3.5f);
        body->SetCollisionLayer(3);
        body->SetFriction(0.5f);
        body->SetRollingFriction(0.75f);
        auto shape = boxNode->CreateComponent<Urho3D::CollisionShape>();
        shape->SetBox(Urho3D::Vector3::ONE);
        body->SetGravityOverride(Urho3D::Vector3(0.0f, -0.5f, 0.0f));
    }

    void AUVOverlay::PrepareInvisibleObjects() {
        Urho3D::PODVector<Urho3D::Node*> invisible_obj;
        m_scene->GetChildrenWithTag(invisible_obj, "i", true);

        for (auto* item : invisible_obj)
        {
            if (auto* staticModel = item->GetComponent<Urho3D::StaticModel>())
                staticModel->SetViewMask(0);
        }
    }
}
