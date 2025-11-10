#include "QAUVSettingsWidgetWeb.h"

#include <QVBoxLayout>
#include <QGroupBox>

using namespace QUrho;

namespace QUrho {

QAUVSettingsWidgetWeb::QAUVSettingsWidgetWeb(QWidget *parent) : QAUVSettingsWidgetBase(parent) {}

float QAUVSettingsWidgetWeb::GetGravity() const {
    return static_cast<float>(default_settings.buoyancy) / -1000.0f;
}

float QAUVSettingsWidgetWeb::GetAngularDamping() const {
    return static_cast<float>(default_settings.angularDamping) / 100.0f;
}

float QAUVSettingsWidgetWeb::GetLinearDamping() const {
    return static_cast<float>(default_settings.linearDamping) / 100.0f;
}

bool QAUVSettingsWidgetWeb::ShowFrontCameraImage() const {
    return default_settings.showFrontCameraWeb;
}

bool QAUVSettingsWidgetWeb::ShowBottomCameraImage() const {
    return default_settings.showBottomCameraWeb;
}

float QAUVSettingsWidgetWeb::GetHydrophoneUpdateTime() const {
    return static_cast<float>(default_settings.hydrophoneUpdate) / 1000.0f;

}

float QAUVSettingsWidgetWeb::GetHydrophoneSignal() const {
    return static_cast<float>(default_settings.hydrophoneSignal) / 1000.0f;
}

float QAUVSettingsWidgetWeb::GetHydrophoneSpeed() const {
    return static_cast<float>(default_settings.hydrophoneSpeed) / 100.0f;

}
}
