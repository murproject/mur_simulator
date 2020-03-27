#include "ViewportOverlay.h"
#include "QUrhoInput.h"
#include "QUrhoScene.h"

#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>

#include <QApplication>
#include <iostream>

namespace QUrho {

    Viewport::Viewport(Urho3D::Context *context, Urho3D::Scene *scene, Urho3D::Camera *camera) :
            m_cameraNode{context},
            m_camera{camera ? camera : m_cameraNode.CreateComponent<Urho3D::Camera>()},
            m_viewport{new Urho3D::Viewport{context, scene, m_camera}} {
    }

    void Viewport::SetWorldTransform(const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation) {
        m_cameraNode.SetWorldPosition(position);
        m_cameraNode.SetWorldRotation(rotation);
    }

    void Viewport::SetTransform(const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation) {
        m_cameraNode.SetPosition(position);
        m_cameraNode.SetRotation(rotation);
    }

    void Viewport::SetRect(Urho3D::IntRect rect) {
        m_viewport->SetRect(rect);
    }

    void Viewport::Update(QUrhoInput *input, float timeStep) {
        if (input->IsKeyPressed(Qt::Key_Shift)) {
            const auto cameraBaseSpeed = 2.0f;
            const auto cameraBaseRotationSpeed = 0.05f;
            if (input->IsKeyPressed(Qt::Key_W)) {
                m_cameraNode.Translate(Urho3D::Vector3(0, 0, cameraBaseSpeed) * timeStep);
            }

            if (input->IsKeyPressed(Qt::Key_A)) {
                m_cameraNode.Translate(Urho3D::Vector3(-cameraBaseSpeed, 0, 0) * timeStep);
            }

            if (input->IsKeyPressed(Qt::Key_S)) {
                m_cameraNode.Translate(Urho3D::Vector3(0, 0, -cameraBaseSpeed) * timeStep);
            }

            if (input->IsKeyPressed(Qt::Key_D)) {
                m_cameraNode.Translate(Urho3D::Vector3(cameraBaseSpeed, 0, 0) * timeStep);
            }

            if (input->IsKeyPressed(Qt::Key_Q)) {
                m_cameraNode.Translate(Urho3D::Vector3(0, -cameraBaseSpeed, 0) * timeStep);
            }

            if (input->IsKeyPressed(Qt::Key_E)) {
                m_cameraNode.Translate(Urho3D::Vector3(0, cameraBaseSpeed, 0) * timeStep);
            }
            if (input->IsMouseButtonPressed(Qt::MiddleButton)) {
                const auto mouseMove = input->GetMouseMove();
                if (mouseMove.y_ != 0 || mouseMove.x_ != 0) {

                    auto yawAngle = m_cameraNode.GetRotation().YawAngle() +
                                    static_cast<float>(mouseMove.x_) * cameraBaseRotationSpeed;

                    auto pitchAngle = m_cameraNode.GetRotation().PitchAngle() +
                                    static_cast<float>(mouseMove.y_) * cameraBaseRotationSpeed;

                    auto direction = m_cameraNode.GetDirection();
                    direction.Normalize();
                    auto distance = m_cameraNode.GetPosition().Length();

                    auto rotation = Urho3D::Quaternion(pitchAngle, yawAngle, 0);
                    auto position = -direction * distance;

                    SetWorldTransform(position, rotation);
                    m_cameraNode.SetWorldRotation(rotation);
                    m_cameraNode.SetWorldPosition(-direction * distance);
                }
            }
        }
    }

    Urho3D::Node *Viewport::GetNode() {
        return &m_cameraNode;
    }

    Urho3D::Camera *Viewport::GetCamera() {
        return m_camera;
    }

    Urho3D::Viewport *Viewport::GetViewport() {
        return m_viewport;
    }

    void Viewport::SetManualUpdate(bool flag) {
        m_manualUpdate = flag;
    }

    bool Viewport::GetManualUpdate() {
        return m_manualUpdate;
    }


    ViewportOverlay::ViewportOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent) :
            QObject(parent),
            Object(context),
            m_scene(scene->GetScene()),
            m_urhoScene(scene) {
        SubscribeToEvent(Urho3D::E_SCREENMODE, URHO3D_HANDLER(ViewportOverlay, HandleResize));
    }

    void ViewportOverlay::Update(QUrhoInput *input, float timeStep) {
        if (m_viewports.empty()) {
            return;
        }

        if (!input->IsMouseButtonDown(Qt::LeftButton)
            && !input->IsMouseButtonDown(Qt::RightButton) && !input->IsMouseButtonDown(Qt::MiddleButton)) {
            SelectCurrentViewport(input->GetMousePosition());
        }

        Viewport &currentViewport = *m_viewports[m_currentViewport];
        if (!currentViewport.GetManualUpdate()) {
            currentViewport.Update(input, timeStep);
        }
    }

    void ViewportOverlay::HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
        UpdateViewportsSizes();
    }

    void ViewportOverlay::SetupViewports() {
        const Urho3D::Vector3 defaultPosition(0, 5, -5);
        const Urho3D::Quaternion defaultRotation(45, 0, 0);

        auto viewport = QSharedPointer<Viewport>::create(GetContext(), m_scene, nullptr);
        viewport->SetWorldTransform(defaultPosition, defaultRotation);
        GetSubsystem<Urho3D::Renderer>()->SetViewport(0, viewport->GetViewport());
        m_viewports.push_back(std::move(viewport));

        UpdateViewportsSizes();
    }

    void ViewportOverlay::UpdateViewportsSizes() {
        if (m_viewports.empty()) {
            return;
        }

        auto graphics = GetSubsystem<Urho3D::Graphics>();
        auto width = graphics->GetWidth();
        auto height = graphics->GetHeight();
        auto halfWidth = width / 2;
        auto halfHeight = height / 2;

        if (m_viewports.size() == 1) {
            m_viewports[0]->SetRect(Urho3D::IntRect(0, 0, width, height));
        }

        if (m_viewports.size() == 2) {
            m_viewports[0]->SetRect(Urho3D::IntRect(0, 0, halfWidth, height));
            m_viewports[1]->SetRect(Urho3D::IntRect(halfWidth, 0, width, height));
        }

        if (m_viewports.size() == 3) {
            m_viewports[0]->SetRect(Urho3D::IntRect(0, 0, halfWidth, height));
            m_viewports[1]->SetRect(Urho3D::IntRect(halfWidth, 0, width, halfHeight));
            m_viewports[2]->SetRect(Urho3D::IntRect(halfWidth, halfHeight, width, height));
        }

        if (m_viewports.size() == 4) {
            m_viewports[0]->SetRect(Urho3D::IntRect(0, 0, halfWidth, halfHeight));
            m_viewports[1]->SetRect(Urho3D::IntRect(halfWidth, 0, width, halfHeight));
            m_viewports[2]->SetRect(Urho3D::IntRect(0, halfHeight, halfWidth, height));
            m_viewports[3]->SetRect(Urho3D::IntRect(halfWidth, halfHeight, width, height));
        }
    }

    void ViewportOverlay::SelectCurrentViewport(const Urho3D::IntVector2 &mousePosition) {
        if (m_viewports.empty()) {
            return;
        }

        Urho3D::IntVector2 localPosition;
        for (int i = 0; i < m_viewports.size(); ++i) {
            const auto viewport = m_viewports[i]->GetViewport();
            const Urho3D::IntRect rect = viewport->GetRect();
            if (rect.Size() == Urho3D::IntVector2::ZERO || rect.IsInside(mousePosition) != Urho3D::OUTSIDE) {
                m_currentViewport = i;
                break;
            }
        }
    }

    void ViewportOverlay::AddViewport(QSharedPointer<Viewport> &viewport) {
        if (m_viewports.size() < 4) {
            GetSubsystem<Urho3D::Renderer>()->SetViewport(m_viewports.size(), viewport->GetViewport());
            m_viewports.push_back(viewport);
        } else {
            std::cerr << "Unable to add more then 4 viewports" << std::endl;
        }
        UpdateViewportsSizes();
    }

}