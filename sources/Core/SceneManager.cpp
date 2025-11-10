#include "SceneManager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SceneManager::SceneManager() = default;
SceneManager::~SceneManager() = default;

SceneManager& SceneManager::Instance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::LoadJSONConfig(const QString& scene_path) {
    QFileInfo fileInfo{scene_path};
    QString scene_dir = fileInfo.absolutePath();

    ResetSceneConfig();

    QFile file(QDir(scene_dir).filePath("Config.json"));
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();
    if (json.contains("version"))
        config.scene_version = json["version"].toDouble();
}

float SceneManager::GetSceneVersion() {
    return config.scene_version;
}

float SceneManager::GetActualSceneVersion() {
    return actual_version;
}

void SceneManager::ResetSceneConfig() {
    config.scene_version = 0.0;
}
