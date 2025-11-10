#ifdef WEB_SIM

#include "ApplicationWindowWeb.h"
#include "QAUVSettingsWidgetWeb.h"

#include "../Core/QUrhoWidget.h"
#include "../Core/QUrhoScene.h"

#include "../Overlay/AUVOverlay.h"
#include "../Overlay/SharingOverlay.h"
#include "../Overlay/HydrophonesOverlay.h"

#include "../Overlay/ScriptsOverlay.h"
#include <QProcessEnvironment>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

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

namespace QUrho {

ApplicationWindowWeb::ApplicationWindowWeb(QWidget *parent) :
    ApplicationWindowBase{parent},
    m_settingsWidgetWeb{new QAUVSettingsWidgetWeb{this}} {
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, &ApplicationWindowWeb::RunLuaScript);
}

ApplicationWindowWeb::~ApplicationWindowWeb() = default;

void ApplicationWindowWeb::InitializeEngine() {
    Urho3D::VariantMap parameters;

    QString resource_path = LoadPaths("resource_path", "/usr/share/simulator");
    QString simulator_log_path = LoadPaths("simulator_log", "/var/tmp/simulator/simulator.log");

    parameters[Urho3D::EP_FORCE_GL2] = 0; 
    parameters[Urho3D::EP_RESOURCE_PATHS] = resource_path.toStdString().c_str();
    parameters[Urho3D::EP_RESOURCE_PREFIX_PATHS] = resource_path.toStdString().c_str();
    parameters[Urho3D::EP_RESOURCE_PACKAGES] = "simulator.pck";
    parameters[Urho3D::EP_AUTOLOAD_PATHS] = "";
    parameters[Urho3D::EP_MULTI_SAMPLE] = 0;
    parameters[Urho3D::EP_WINDOW_RESIZABLE] = true;
    parameters[Urho3D::EP_LOG_NAME] = simulator_log_path.toStdString().c_str();
    parameters[Urho3D::EP_LOG_LEVEL] = 3;
    parameters[Urho3D::EP_TEXTURE_FILTER_MODE] = "FILTER_TRILINEAR";
    parameters[Urho3D::EP_TEXTURE_QUALITY] = Urho3D::QUALITY_HIGH;

    m_urhoWidget->InitializeUrho3DEngine(parameters);
}

void ApplicationWindowWeb::closeApplication() {
    cv::destroyAllWindows();
    QApplication::exit();
}

void ApplicationWindowWeb::OnSettingAccepted() {
    if (!m_scene) {
        return;
    }
    auto auv = m_scene->GetAUVOverlay();
    auto hydrophones = m_scene->GetHydrophoneOverlay();
    auv->SetLinearDamping(m_settingsWidgetWeb->GetLinearDamping());
    auv->SetAngularDamping(m_settingsWidgetWeb->GetAngularDamping());
    auv->ShowBottomCameraImage(m_settingsWidgetWeb->ShowBottomCameraImage());
    auv->ShowFrontCameraImage(m_settingsWidgetWeb->ShowFrontCameraImage());
    auv->SetGravity(m_settingsWidgetWeb->GetGravity());
    auv->SetRemote(m_remote);
    hydrophones->SetUpdateTime(m_settingsWidgetWeb->GetHydrophoneUpdateTime());
    hydrophones->SetSignal(m_settingsWidgetWeb->GetHydrophoneSignal());
    hydrophones->SetSpeed(m_settingsWidgetWeb->GetHydrophoneSpeed());
    cv::destroyAllWindows();
}

void ApplicationWindowWeb::OnLuaSettingsAccepted() {
}

void ApplicationWindowWeb::OnSceneLoaded(const QString &scene, const QString &scene_dir) {
    OnSettingAccepted();
    timer->start(2000);
}

void ApplicationWindowWeb::UpdateNotification() {
    qDebug() << "Scene version is too high!";
}

void ApplicationWindowWeb::RunLuaScript(){
    QString lua_path = LoadPaths("lua", "/usr/share/simulator/lua.lua");
    m_scene->GetScriptsOverlay()->Run(lua_path);
}

QString ApplicationWindowWeb::LoadPaths(QString name, QString default_path) {
    QString setting_path = qgetenv("SIMULATOR_SETTING_FILE");
    QSettings settings(setting_path, QSettings::IniFormat);
    QString path = settings.value(name, default_path).toString();
    return path;
}
}

#endif
