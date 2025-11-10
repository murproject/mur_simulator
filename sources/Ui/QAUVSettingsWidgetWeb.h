#pragma once

#include "QAUVSettingsWidgetBase.h"
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>

namespace QUrho {
    class QAUVSettingsWidgetWeb : public QAUVSettingsWidgetBase {
    Q_OBJECT
    public:
        explicit QAUVSettingsWidgetWeb(QWidget *parent = nullptr);

        float GetGravity() const override;
        float GetAngularDamping() const override;
        float GetLinearDamping() const override;
        float GetHydrophoneUpdateTime() const override;
        float GetHydrophoneSignal() const override;
        float GetHydrophoneSpeed() const override;

        bool ShowFrontCameraImage() const override;
        bool ShowBottomCameraImage() const override;
    };

}

