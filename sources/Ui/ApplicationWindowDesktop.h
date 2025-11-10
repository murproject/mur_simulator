#ifdef DESKTOP_SIM

#pragma once
#include "ApplicationWindowBase.h"
#include <QMainWindow>
#include <QMdiArea>
#include <QAction>
#include <QCloseEvent>
#include <QToolBar>
#include <QLabel>
#include <QTimer>
namespace QUrho {

class QUrhoWidget;
class QUrhoScene;
class QAUVSettingsWidgetDesktop;

class ApplicationWindowDesktop : public ApplicationWindowBase {
    Q_OBJECT
public:
    explicit ApplicationWindowDesktop(QWidget *parent = nullptr);

    ~ApplicationWindowDesktop() override;

    void InitializeEngine() override;

    void OnSceneLoaded(const QString &scene, const QString &scene_dir) override;

    void UpdateNotification() override;

public slots:
    void updateFPS(int);

signals:
    void windowResized(int, int);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void OnSceneOpen();

    void OnOpenAUVSettings();

    void OnOpenGraphicsSettings();

    void OnAUVReset();

    void OnTelemetryUpdated();

    void OnModeChanged();

    void OnSceneReset();

    void OnSettingAccepted() override;

    QAction *AddAction(const QString &name, const QKeySequence &shortcut = {});

    QMenu *AddMenu(const QString &name);

    void CreateMenus();

    void CreateToolBar();

    void InitializeMainWindow();

    void OnLuaSettingsAccepted() override;

    std::pair<int, int> getScreenSize();

    QScopedPointer<QToolBar> m_toolBar;
    QScopedPointer<QLabel> m_yawLabel;
    QScopedPointer<QLabel> m_pitchLabel;
    QScopedPointer<QLabel> m_rollLabel;
    QScopedPointer<QLabel> m_depthLabel;
    QScopedPointer<QLabel> m_FPSLabel;
    QScopedPointer<QAUVSettingsWidgetDesktop> m_settingsWidgetDesktop;

    bool isWindows = true;
};
}

#endif
