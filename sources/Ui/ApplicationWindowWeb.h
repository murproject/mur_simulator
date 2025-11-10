#ifdef WEB_SIM

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
class QAUVSettingsWidgetWeb;

class ApplicationWindowWeb : public ApplicationWindowBase {
    Q_OBJECT
public:
    explicit ApplicationWindowWeb(QWidget *parent = nullptr);

    ~ApplicationWindowWeb() override;

    void InitializeEngine() override;

    void OnSceneLoaded(const QString &scene, const QString &scene_dir) override;

    void UpdateNotification() override;

    void closeApplication();

    QString LoadPaths(QString name, QString default_path);

private slots:
    void RunLuaScript();
private:
    void OnSettingAccepted() override;
    void OnLuaSettingsAccepted() override;

    QTimer *timer;
    QScopedPointer<QAUVSettingsWidgetWeb> m_settingsWidgetWeb;
};
}

#endif
