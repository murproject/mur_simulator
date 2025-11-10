#ifdef DESKTOP_SIM

#include "ApplicationWindowDesktop.h"
#include "QAUVSettingsWidgetDesktop.h"
#include "GraphicsSettingsWidget.h"

#include "../Core/QUrhoWidget.h"
#include "../Core/QUrhoInput.h"
#include "../Core/QUrhoScene.h"

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
#include <QMessageBox>
#include <QStyle>
#include <QFile>

#include <opencv2/opencv.hpp>

namespace QUrho {

ApplicationWindowDesktop::ApplicationWindowDesktop(QWidget *parent) :
    ApplicationWindowBase{parent},
    m_settingsWidgetDesktop{new QAUVSettingsWidgetDesktop{this}},
    m_toolBar{new QToolBar{this}},
    m_yawLabel{new QLabel{"Yaw: 0.0 "}},
    m_pitchLabel{new QLabel{"Pitch: 0.0 "}},
    m_rollLabel{new QLabel{"Roll: 0.0 "}},
    m_depthLabel{new QLabel{"Depth: 0.0 "}},
    m_FPSLabel{new QLabel{"FPS: 0"}} {
    isWindows = QSysInfo::productType() == "windows";
    InitializeMainWindow();

    if (isWindows) {
        resize(1024, 768);
    } else {
        auto[width, height] = getScreenSize();
        int size_toolbars = menuBar()->height() + m_toolBar->height();
        resize(width, height - size_toolbars); 
    }

    connect(m_settingsWidgetDesktop.data(), &QAUVSettingsWidgetDesktop::accepted, this, &ApplicationWindowDesktop::OnSettingAccepted);
    connect(m_graphicsWidget.data(), &GraphicsSettingsWidget::accepted, this, &ApplicationWindowDesktop::OnGraphicsSettingAccepted);
}

void ApplicationWindowDesktop::InitializeEngine() {
    Urho3D::VariantMap parameters;
    parameters[Urho3D::EP_RESOURCE_PREFIX_PATHS] = "";
    parameters[Urho3D::EP_RESOURCE_PATHS] = "";
    parameters[Urho3D::EP_AUTOLOAD_PATHS] = "";
    parameters[Urho3D::EP_WINDOW_RESIZABLE] = true;
    parameters[Urho3D::EP_LOG_NAME] = "simulator.log";
    parameters[Urho3D::EP_LOG_LEVEL] = 1;
    parameters[Urho3D::EP_RESOURCE_PACKAGES] = "simulator.pck";
    parameters[Urho3D::EP_MULTI_SAMPLE] = 0;
    parameters[Urho3D::EP_TEXTURE_FILTER_MODE] = "FILTER_TRILINEAR";
    parameters[Urho3D::EP_TEXTURE_QUALITY] = Urho3D::QUALITY_HIGH;

    m_urhoWidget->InitializeUrho3DEngine(parameters);
    OpenFile(m_settingsWidgetDesktop->GetLastScene());

    connect(m_urhoWidget->GetUrho3DCoreWidget(), &Urho3DCoreWidget::FPSUpdated, this, &ApplicationWindowDesktop::updateFPS);
}

void ApplicationWindowDesktop::OnSettingAccepted() {
    if (!m_scene) {
        return;
    }
    auto auv = m_scene->GetAUVOverlay();
    auto hydrophones = m_scene->GetHydrophoneOverlay();
    auv->SetLinearDamping(m_settingsWidgetDesktop->GetLinearDamping());
    auv->SetAngularDamping(m_settingsWidgetDesktop->GetAngularDamping());
    auv->ShowBottomCameraImage(m_settingsWidgetDesktop->ShowBottomCameraImage());
    auv->ShowFrontCameraImage(m_settingsWidgetDesktop->ShowFrontCameraImage());
    auv->SetGravity(m_settingsWidgetDesktop->GetGravity());
    auv->SetRemote(m_remote);
    hydrophones->SetUpdateTime(m_settingsWidgetDesktop->GetHydrophoneUpdateTime());
    hydrophones->SetSignal(m_settingsWidgetDesktop->GetHydrophoneSignal());
    hydrophones->SetSpeed(m_settingsWidgetDesktop->GetHydrophoneSpeed());
    cv::destroyAllWindows();
}

void ApplicationWindowDesktop::OnLuaSettingsAccepted() {
    m_scene->GetLuaOverlay()->DisableNode("particles", m_graphicsWidget->GetCheckBoxValue("particles"));
    m_scene->GetLuaOverlay()->DisableNode("bubbles", m_graphicsWidget->GetCheckBoxValue("particles"));
}

void ApplicationWindowDesktop::CreateMenus() {
    QMenu *menu = nullptr;
    QAction *action = nullptr;

    menu = AddMenu("Scene");
    menuBar()->addMenu(menu);

    action = AddAction("Open", Qt::CTRL | Qt::Key_O);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnSceneOpen);

    menu = AddMenu("Settings");
    menuBar()->addMenu(menu);

    action = AddAction("AUV", Qt::CTRL | Qt::Key_S);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnOpenAUVSettings);

    action = AddAction("Graphics", Qt::CTRL | Qt::Key_G);
    menu->addAction(action);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnOpenGraphicsSettings);
}

void ApplicationWindowDesktop::CreateToolBar() {
    m_toolBar->setMovable(false);

    QAction *action = nullptr;
    addToolBar(m_toolBar.data());

    action = AddAction("Remote mode", Qt::CTRL | Qt::Key_M);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnModeChanged);
    action->setCheckable(true);
    m_toolBar->addAction(action);

    action = AddAction("Robot reset", Qt::CTRL | Qt::Key_R);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnAUVReset);
    m_toolBar->addAction(action);

    action = AddAction("Scene reset", Qt::SHIFT | Qt::Key_R);
    connect(action, &QAction::triggered, this, &ApplicationWindowDesktop::OnSceneReset);
    m_toolBar->addAction(action);

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolBar->addWidget(spacer);

    m_yawLabel->setFont(QFont("monospace"));
    m_pitchLabel->setFont(QFont("monospace"));
    m_rollLabel->setFont(QFont("monospace"));
    m_depthLabel->setFont(QFont("monospace"));
    m_FPSLabel->setFont(QFont("monospace"));

    const int labelWidth = 120;
    m_yawLabel->setFixedWidth(labelWidth);
    m_pitchLabel->setFixedWidth(labelWidth);
    m_rollLabel->setFixedWidth(labelWidth);
    m_depthLabel->setFixedWidth(labelWidth);
    m_FPSLabel->setFixedWidth(labelWidth);

    m_toolBar->addWidget(m_yawLabel.data());
    m_toolBar->addWidget(m_pitchLabel.data());
    m_toolBar->addWidget(m_rollLabel.data());
    m_toolBar->addWidget(m_depthLabel.data());
    m_toolBar->addWidget(m_FPSLabel.data());
}

QAction *ApplicationWindowDesktop::AddAction(const QString &name, const QKeySequence &shortcut) {
    auto action = new QAction{};
    action->setText(name);
    action->setShortcut(shortcut);
    return action;
}

QMenu *ApplicationWindowDesktop::AddMenu(const QString &name) {
    auto menu = new QMenu(name);
    return menu;
}

void ApplicationWindowDesktop::InitializeMainWindow() {
    setCentralWidget(m_mdiWidget.data());
    m_mdiWidget->setViewport(m_urhoWidget.data());
    setContentsMargins(0, 0, 0, 0);
    setMinimumSize(QSize{640, 480});
    CreateMenus();
    CreateToolBar();
}

void ApplicationWindowDesktop::OnSceneOpen() {
    auto fileName = QFileDialog::getOpenFileName(nullptr, "Scene file", nullptr, "*.xml *.mur_scene");
    OpenFile(fileName);
}

ApplicationWindowDesktop::~ApplicationWindowDesktop() = default;

void ApplicationWindowDesktop::closeEvent(QCloseEvent *event) {
    QApplication::closeAllWindows();
    cv::destroyAllWindows();
    event->accept();
    m_urhoWidget->Exit();

    QApplication::quit();
}

void ApplicationWindowDesktop::OnOpenAUVSettings() {
    m_settingsWidgetDesktop->setModal(true);
    m_settingsWidgetDesktop->show();
}

void ApplicationWindowDesktop::OnOpenGraphicsSettings() {
    m_graphicsWidget->setModal(true);
    m_graphicsWidget->show();
}


void ApplicationWindowDesktop::OnAUVReset() {
    if (m_scene) {
        m_scene->GetAUVOverlay()->ResetAUV();
        m_scene->GetNetworkOverlay()->Reset();
    }
}

void ApplicationWindowDesktop::OnModeChanged() {
    auto action = qobject_cast<QAction *>(sender());
    if (action->isChecked()) {
        action->setText("Manual mode");
        if (m_scene) {
            m_scene->GetAUVOverlay()->SetRemote(false);
        }
        m_remote = false;
    } else {
        action->setText("Remote mode");
        if (m_scene) {
            m_scene->GetAUVOverlay()->SetRemote(true);
        }
        m_remote = true;
    }
}

void ApplicationWindowDesktop::OnTelemetryUpdated() {
    static qint64 telemetryTimestamp = 0;
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    if (qAbs(now - telemetryTimestamp) > 50) {
        telemetryTimestamp = now;

        auto rotations = m_scene->GetAUVOverlay()->GetAUVRotations();
        auto depth = m_scene->GetAUVOverlay()->GetAUVDepth();

        static auto formatNumber = [](float n){
            return QString::asprintf("%+0.2f", n);
        };

        auto y = QString("Yaw: ") + formatNumber(rotations.y_);
        auto p = QString("Pitch: ") + formatNumber(rotations.x_);
        auto r = QString("Roll: ") + formatNumber(rotations.z_);
        auto d = QString("Depth: ") + formatNumber(depth);

        m_yawLabel->setText(y);
        m_pitchLabel->setText(p);
        m_rollLabel->setText(r);
        m_depthLabel->setText(d);
    }
}

void ApplicationWindowDesktop::updateFPS(int fps) {
    m_FPSLabel->setText(QString("FPS: %1").arg(fps));
}

void ApplicationWindowDesktop::OnSceneLoaded(const QString &scene, const QString &scene_dir) {
    OnSettingAccepted();
    OnGraphicsSettingAccepted();
    m_settingsWidgetDesktop->SetLastScene(scene_dir == "" ? scene : scene_dir);
    m_scene->GetLuaOverlay()->GetPath();
    m_scene->GetLuaOverlay()->RestartScript();

    QString title_name = m_scene->GetSceneName();

    if (QFile::exists(scene_dir)) {
        title_name = QFileInfo(scene_dir).fileName();
    }

    setWindowTitle("Simulator  ꟷ  " + title_name);

    connect(m_scene->GetAUVOverlay(), &AUVOverlay::TelemetryUpdated, this, &ApplicationWindowDesktop::OnTelemetryUpdated);
    if (!isWindows)
        connect(this, &ApplicationWindowDesktop::windowResized, m_scene->GetViewportOverlay(), &ViewportOverlay::UpdateSizes);

}

void ApplicationWindowDesktop::UpdateNotification() {
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Не удается открыть сцену");

    msgBox.setText("Эта сцена создана для новой версии симулятора MUR IDE.<br>"
                   "Пожалуйста, обновите симулятор, чтобы открыть её.<br>"
                   "<div style='margin-left: 45px'>"
                   "<a href='https://murproject.com/#muride'>Скачать последнюю версию MUR IDE</a>"
                   "<br></div>");

    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxWarning);
    msgBox.setIconPixmap(icon.pixmap(32, 32));

    msgBox.addButton("OK", QMessageBox::AcceptRole);

    msgBox.exec();
}

void ApplicationWindowDesktop::OnSceneReset(){
    OpenFile(m_settingsWidgetDesktop->GetLastScene());
}

void ApplicationWindowDesktop::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    emit windowResized(size().height(), size().width());
}

std::pair<int, int> ApplicationWindowDesktop::getScreenSize() {
    QScreen *screen = QGuiApplication::primaryScreen();

    if (screen) {
        QRect screenGeometry = screen->geometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();
        return {screenWidth, screenHeight};
    }
    return {1080, 768};
}
}

#endif
