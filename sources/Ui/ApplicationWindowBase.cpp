#include "ApplicationWindowBase.h"
#include "GraphicsSettingsWidget.h"

#include "../Core/QUrhoWidget.h"
#include "../Core/QUrhoInput.h"
#include "../Core/QUrhoScene.h"
#include "../Core/SceneManager.h"

#include "../Overlay/AUVOverlay.h"
#include "../Overlay/SharingOverlay.h"
#include "../Overlay/HydrophonesOverlay.h"
#include "../Overlay/LuaOverlay.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/Constraint.h>
#include <Urho3D/Graphics/RenderPath.h>

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QFileDialog>
#include <QApplication>

#include <opencv2/opencv.hpp>
#include "zip.c"

namespace QUrho {

ApplicationWindowBase::ApplicationWindowBase(QWidget *parent) :
    QMainWindow{parent},
    m_mdiWidget{new QMdiArea{this}},
    m_urhoWidget{new QUrhoWidget{m_mdiWidget.data()}},
    m_graphicsWidget{new GraphicsSettingsWidget{this}} {
}

ApplicationWindowBase::~ApplicationWindowBase() {
    QDir dir(scene_cache_path);
    dir.removeRecursively();
}

void ApplicationWindowBase::OnGraphicsSettingAccepted() {
    auto viewport = m_scene->GetViewportOverlay();

    float fisheye_value = static_cast<float>(m_graphicsWidget->GetSliderValue("fisheye value"))  / 100.0f;
    float chromabber_value = static_cast<float>(m_graphicsWidget->GetSliderValue("chromabber value")) / 10.0f;
    float noise_value = static_cast<float>(m_graphicsWidget->GetSliderValue("noise value")) / 10.0f;

    viewport->UpdateShaderParameter("Fisheye", "DistortionFactor", fisheye_value);
    viewport->UpdateShaderParameter("ChromaticAberration", "AberrationStrength", chromabber_value);
    viewport->UpdateShaderParameter("Noise", "NoiseStrength", noise_value);

    viewport->SetTextureQuality(m_graphicsWidget->GetSliderValue("texture quality"));
    viewport->SetShadowsQuality(m_graphicsWidget->GetSliderValue("shadows quality"));
    viewport->SetFovValue(m_graphicsWidget->GetSliderValue("fov value"));

    viewport->SetRenderEffect("Bloom", m_graphicsWidget->GetCheckBoxValue("render bloom"));
    viewport->SetRenderEffect("FXAA3", m_graphicsWidget->GetCheckBoxValue("render fxaa3"));
    viewport->SetRenderEffect("FocusBlur", m_graphicsWidget->GetCheckBoxValue("render focus blur"));
    viewport->SetRenderEffect("Noise", m_graphicsWidget->GetCheckBoxValue("render noise"));;
    viewport->SetRenderEffect("Fisheye", m_graphicsWidget->GetCheckBoxValue("render fish eye"));
    viewport->SetDrawShadows(m_graphicsWidget->GetCheckBoxValue("shadows draw"));

    OnLuaSettingsAccepted();
}

void ApplicationWindowBase::OpenFile(const QString &file_path) {
    if (file_path.endsWith(".mur_scene", Qt::CaseInsensitive)) {
        if (!QFile::exists(file_path)) {
            return;
        }

        QString extraction_path = scene_cache_path + QFileInfo(file_path).completeBaseName();

        if (zip_extract(file_path.toStdString().c_str(),
                        extraction_path.toStdString().c_str(),
                        nullptr, nullptr) != 0) {
            return;
        }

        QDir dir(extraction_path);
        QStringList xml_files = dir.entryList(QStringList("*.xml"));
        QString scene_path = xml_files.isEmpty() ? QString() : dir.filePath(xml_files.first());

        OpenScene(scene_path, file_path);
    } else if (file_path.endsWith(".xml", Qt::CaseInsensitive)) {
        OpenScene(file_path);
    }
}


void ApplicationWindowBase::OpenScene(const QString &scene, const QString &scene_dir) {
    auto& scene_mgr = SceneManager::Instance();

    scene_mgr.LoadJSONConfig(scene);

    if (scene_mgr.GetSceneVersion() > scene_mgr.GetActualSceneVersion()) {
        UpdateNotification();
        return;
    }

    if (scene.isEmpty() || scene.size() < 3) {
        return;
    }

    QFileInfo info(scene);
    if (!info.exists() || !info.isFile()) {
        return;
    }

    if (!m_scene) {
        m_scene.reset(new QUrhoScene{m_urhoWidget->GetUrho3DContext(), m_urhoWidget.data(), this});
    }

    m_scene->GetScene()->Clear();

    if (m_scene->Load(scene)) {
        OnSceneLoaded(scene, scene_dir);
    }
}

}
