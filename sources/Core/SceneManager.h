#pragma once

#include <QString>

struct SceneConfig {
    float scene_version = 0.0f;
};

class SceneManager {

public:
    static SceneManager& Instance();

    void LoadJSONConfig(const QString& sceneDir);
    float GetSceneVersion();
    float GetActualSceneVersion();

private:
    SceneManager();
    ~SceneManager();

    SceneManager(SceneManager const&) = delete;
    SceneManager& operator=(SceneManager const&) = delete;

    void ResetSceneConfig();

    float actual_version = 1.0f;

    SceneConfig config;
};
