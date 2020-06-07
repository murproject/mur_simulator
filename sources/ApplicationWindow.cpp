#include "ApplicationWindow.h"
#include "QUrhoWidget.h"
#include "QUrhoInput.h"
#include "QUrhoScene.h"
#include "QAUVSettingsWidget.h"
#include "AUVOverlay.h"
#include "SharingOverlay.h"
#include "PingersOverlay.h"

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

    ApplicationWindow::ApplicationWindow(QWidget *parent) :
            QMainWindow{parent},
            m_mdiWidget{new QMdiArea{this}},
            m_urhoWidget{new QUrhoWidget{m_mdiWidget.data()}},
            m_settingsWidget{new QAUVSettingsWidget{this}},
            m_toolBar{new QToolBar{this}},
            m_yawLabel{new QLabel{"Yaw: 0.0 "}},
            m_pitchLabel{new QLabel{"Pitch: 0.0 "}},
            m_rollLabel{new QLabel{"Roll: 0.0 "}},
            m_depthLabel{new QLabel{"Depth: 0.0 "}} {
        InitializeMainWindow();
        resize(1024, 768);
        connect(m_settingsWidget.data(), &QAUVSettingsWidget::accepted, this, &ApplicationWindow::OnSettingAccepted);
    }

    void ApplicationWindow::InitializeEngine() {
        Urho3D::VariantMap parameters;
        parameters[Urho3D::EP_RESOURCE_PREFIX_PATHS] = "";
        parameters[Urho3D::EP_RESOURCE_PATHS] = "";
        parameters[Urho3D::EP_RESOURCE_PACKAGES] = "";
        parameters[Urho3D::EP_AUTOLOAD_PATHS] = "";
        parameters[Urho3D::EP_MULTI_SAMPLE] = 16;
        parameters[Urho3D::EP_WINDOW_RESIZABLE] = true;
        parameters[Urho3D::EP_LOG_NAME] = "simulator.log";
        parameters[Urho3D::EP_LOG_LEVEL] = 1;
        parameters[Urho3D::EP_RESOURCE_PACKAGES] = "simulator.pck";

        m_urhoWidget->InitializeUrho3DEngine(parameters);
        OpenScene(m_settingsWidget->GetLastScene());
    }

    void ApplicationWindow::CreateMenus() {
        QMenu *menu = nullptr;
        QAction *action = nullptr;

        menu = AddMenu("Scene");
        menuBar()->addMenu(menu);

        action = AddAction("Open", Qt::CTRL + Qt::Key_O);
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &ApplicationWindow::OnSceneOpen);

        menu = AddMenu("Settings");
        menuBar()->addMenu(menu);

        action = AddAction("AUV Settings", Qt::CTRL + Qt::Key_S);
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &ApplicationWindow::OnOpenAUVSettings);
    }

    void ApplicationWindow::CreateToolBar() {
        m_toolBar->setMovable(false);

        QAction *action = nullptr;
        addToolBar(m_toolBar.data());

        action = AddAction("Remote mode", Qt::CTRL + Qt::Key_M);
        connect(action, &QAction::triggered, this, &ApplicationWindow::OnModeChanged);
        action->setCheckable(true);
        m_toolBar->addAction(action);

        action = AddAction("Robot reset", Qt::CTRL + Qt::Key_R);
        connect(action, &QAction::triggered, this, &ApplicationWindow::OnAUVReset);
        m_toolBar->addAction(action);

        auto spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_toolBar->addWidget(spacer);

        m_toolBar->addWidget(m_yawLabel.data());
        m_toolBar->addWidget(m_pitchLabel.data());
        m_toolBar->addWidget(m_rollLabel.data());
        m_toolBar->addWidget(m_depthLabel.data());
    }

    QAction *ApplicationWindow::AddAction(const QString &name, const QKeySequence &shortcut) {
        auto action = new QAction{};
        action->setText(name);
        action->setShortcut(shortcut);
        return action;
    }

    QMenu *ApplicationWindow::AddMenu(const QString &name) {
        auto menu = new QMenu(name);
        return menu;
    }

    void ApplicationWindow::InitializeMainWindow() {
        setCentralWidget(m_mdiWidget.data());
        m_mdiWidget->setViewport(m_urhoWidget.data());
        setContentsMargins(0, 0, 0, 0);
        setMinimumSize(QSize{640, 480});
        CreateMenus();
        CreateToolBar();
    }

    void ApplicationWindow::OnSceneOpen() {
        auto fileName = QFileDialog::getOpenFileName(nullptr, "Scene file", nullptr, "*.xml");
        OpenScene(fileName);
    }

    ApplicationWindow::~ApplicationWindow() {
    }

    void ApplicationWindow::closeEvent(QCloseEvent *event) {
        QApplication::closeAllWindows();
        cv::destroyAllWindows();
        event->accept();
        m_urhoWidget->Exit();
        m_scene.reset(nullptr);

        QApplication::quit();
    }

    void ApplicationWindow::OnOpenAUVSettings() {
        m_settingsWidget->setModal(true);
        m_settingsWidget->show();
    }

    void ApplicationWindow::OnAUVReset() {
        if (m_scene) {
            m_scene->GetAUVOverlay()->ResetAUV();
            m_scene->GetNetworkOverlay()->Reset();
        }
    }

    void ApplicationWindow::OnModeChanged() {
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

    void ApplicationWindow::OnTelemetryUpdated() {
        auto rotations = m_scene->GetAUVOverlay()->GetAUVRotations();
        auto depth = m_scene->GetAUVOverlay()->GetAUVDepth();

        auto y = QString("Yaw: ") + QString::number(rotations.y_, 'f', 2) + " ";
        auto p = QString("Pitch: ") + QString::number(rotations.x_, 'f', 2) + " ";
        auto r = QString("Roll: ") + QString::number(rotations.z_, 'f', 2) + " ";
        auto d = QString("Depth: ") + QString::number(depth, 'f', 2) + " ";

        m_yawLabel->setText(y);
        m_pitchLabel->setText(p);
        m_rollLabel->setText(r);
        m_depthLabel->setText(d);
    }

    void ApplicationWindow::OnSettingAccepted() {
        if (!m_scene) {
            return;
        }
        auto auv = m_scene->GetAUVOverlay();
        auto pingers = m_scene->GetPingerOverlay();
        auv->SetLinearDamping(m_settingsWidget->GetLinearDamping());
        auv->SetAngularDamping(m_settingsWidget->GetAngularDamping());
        auv->ShowBottomCameraImage(m_settingsWidget->ShowBottomCameraImage());
        auv->ShowFrontCameraImage(m_settingsWidget->ShowFrontCameraImage());
        auv->SetGravity(m_settingsWidget->GetGravity());
        auv->SetRemote(m_remote);
        pingers->SetUpdateTime(m_settingsWidget->GetPingerUpdateTime());
        cv::destroyAllWindows();
    }

    void ApplicationWindow::OpenScene(const QString &scene) {
        if (scene.isEmpty() || scene.size() < 3) {
            return;
        }

        QFileInfo info(scene);
        if (!info.exists() || !info.isFile()) {
            return;
        }
        if (m_scene) {
            m_scene.reset(nullptr);
        }
        m_scene.reset(new QUrhoScene{m_urhoWidget->GetUrho3DContext(), m_urhoWidget.data(), this});

        if (m_scene->Load(scene)) {
            connect(m_scene->GetAUVOverlay(), &AUVOverlay::TelemetryUpdated, this,
                    &ApplicationWindow::OnTelemetryUpdated);
            OnSettingAccepted();
            m_settingsWidget->SetLastScene(scene);
        } else {
            m_scene.reset();
        }
    }
}


