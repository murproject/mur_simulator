#include "ViewportOverlay.h"
#include "../Core/QUrhoInput.h"
#include "../Core/QUrhoScene.h"
#include "AUVOverlay.h"
#include "Urho3D/Graphics/RenderPath.h"

#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Renderer.h>

#include <QApplication>

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
        SetupViewports();
        SubscribeToEvent(Urho3D::E_SCREENMODE, URHO3D_HANDLER(ViewportOverlay, HandleResize));
    }

    void ViewportOverlay::Update(QUrhoInput *input, float timeStep) {
        if (!m_viewport) {
            return;
        }

        if (!m_viewport->GetManualUpdate()) {
            m_viewport->Update(input, timeStep);
        }
    }

    void ViewportOverlay::HandleResize(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
        UpdateViewportsSizes();
    }

    void ViewportOverlay::UpdateSizes(int height, int width){
        alt_height = height;
        alt_width = width;
        UpdateViewportsSizes();
    }

    void ViewportOverlay::SetupViewports() {
        using namespace Urho3D;

        auto graphics = GetSubsystem<Graphics>();
        graphics->SetDefaultTextureFilterMode(TextureFilterMode::FILTER_TRILINEAR);

        renderer_ = GetSubsystem<Urho3D::Renderer>();

        renderer_->SetMaterialQuality(QUALITY_HIGH);
        renderer_->SetTextureFilterMode(FILTER_TRILINEAR);
        renderer_->SetHDRRendering(true);
        renderer_->SetShadowQuality(SHADOWQUALITY_PCF_16BIT);
    }

    void ViewportOverlay::SetTextureQuality(int i) {
        renderer_->SetTextureQuality(static_cast<Urho3D::MaterialQuality>(i));
    }

    void ViewportOverlay::SetShadowsQuality(int i) {
        renderer_->SetShadowMapSize(1024 * qPow(2, i));
    }

    void ViewportOverlay::UpdateShaderParameter(Urho3D::String tag, Urho3D::String shader_param, float value) {
        auto auvOverlay = m_urhoScene->GetAUVOverlay();

        std::vector<Urho3D::Viewport*> viewports = {
            m_viewport->GetViewport(),
            auvOverlay->GetBottomCameraViewport()->GetViewport(),
            auvOverlay->GetFrontCameraViewport()->GetViewport()
        };

        for (auto viewport : viewports) {
            auto renderPath = viewport->GetRenderPath();
            if (!renderPath) continue;

            for (auto& command : renderPath->commands_) {
                if (command.tag_ == tag) {
                    command.shaderParameters_[shader_param] = value;
                }
            }
        }
    }

    void ViewportOverlay::SetFovValue(int i) {
        auto auvOverlay = m_urhoScene->GetAUVOverlay();

        auvOverlay->GetBottomCameraViewport()->GetCamera()->SetFov(static_cast<float>(i));
        auvOverlay->GetFrontCameraViewport()->GetCamera()->SetFov(static_cast<float>(i));
    }

    void ViewportOverlay::SetRenderEffect(QString tag, bool enabled) {
        auto auvOverlay = m_urhoScene->GetAUVOverlay();

        std::vector<Urho3D::Viewport*> viewports = {
            m_viewport->GetViewport(),
            auvOverlay->GetBottomCameraViewport()->GetViewport(),
            auvOverlay->GetFrontCameraViewport()->GetViewport()
        };

        for (auto viewport : viewports) {
            if (auto renderPath = viewport->GetRenderPath()) {
                renderPath->SetEnabled(tag.toStdString().c_str(), enabled);
            }
        }
    }

    void ViewportOverlay::SetDrawShadows(bool i) {
        renderer_->SetDrawShadows(i);
    }

    void ViewportOverlay::UpdateViewportsSizes() {
        if (!m_viewport) {
            return;
        }
        int width, height = 0;

        if (QSysInfo::productType() == "windows") {
            auto graphics = GetSubsystem<Urho3D::Graphics>();
            width = graphics->GetWidth();
            height = graphics->GetHeight();
        } else {
            height = alt_height * 2;
            width = alt_width;
        }

        m_viewport->SetRect(Urho3D::IntRect(0, 0, width, height));
    }

    void ViewportOverlay::AddViewport(QSharedPointer<Viewport> &viewport) {
        m_viewport = viewport;
        GetSubsystem<Urho3D::Renderer>()->SetViewport(0, m_viewport->GetViewport());
        UpdateViewportsSizes();
    }
}
