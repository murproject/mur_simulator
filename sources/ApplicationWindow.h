#pragma once

#include <QMainWindow>
#include <QMdiArea>
#include <QAction>
#include <QCloseEvent>
#include <QToolBar>
#include <QLabel>

namespace QUrho {

    class QUrhoWidget;

    class QUrhoScene;

    class QAUVSettingsWidget;

    class ApplicationWindow : public QMainWindow {
    Q_OBJECT
    public:
        explicit ApplicationWindow(QWidget *parent = nullptr);

        ~ApplicationWindow() override;

        void InitializeEngine();

    protected:
        void closeEvent(QCloseEvent *event) override;

    private:
        void OnSceneOpen();

        void OpenScene(const QString &scene);

        void OnSettingAccepted();

        void OnOpenAUVSettings();

        void OnAUVReset();

        void OnTelemetryUpdated();

        void OnModeChanged();

        QAction *AddAction(const QString &name, const QKeySequence &shortcut = {});

        QMenu *AddMenu(const QString &name);

        void CreateMenus();

        void CreateToolBar();

        void InitializeMainWindow();

//        QScopedPointer<QMdiArea> m_mdiWidget;
        QScopedPointer<QUrhoWidget> m_urhoWidget;
        QScopedPointer<QUrhoScene> m_scene;
        QScopedPointer<QAUVSettingsWidget> m_settingsWidget;
        QScopedPointer<QToolBar> m_toolBar;
        QScopedPointer<QLabel> m_yawLabel;
        QScopedPointer<QLabel> m_pitchLabel;
        QScopedPointer<QLabel> m_rollLabel;
        QScopedPointer<QLabel> m_depthLabel;
        bool m_remote = true;
    };
}




