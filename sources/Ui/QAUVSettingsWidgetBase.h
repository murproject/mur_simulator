#pragma once

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>

namespace QUrho {
    struct AUVSettings {
        int buoyancy = 5;
        int angularDamping = 80;
        int linearDamping = 50;
        int hydrophoneUpdate = 4000;
        int hydrophoneSignal = 200;
        int hydrophoneSpeed = 850;
        bool showFrontCameraWeb = false;
        bool showBottomCameraWeb = false;
        bool showFrontCameraDesktop = true;
        bool showBottomCameraDesktop = true;
    };

    inline constexpr AUVSettings default_settings{};

    class QAUVSettingsWidgetBase : public QDialog {
    Q_OBJECT
    public:
        explicit QAUVSettingsWidgetBase(QWidget *parent = nullptr)
            : QDialog(parent),
            m_auvSettings(new QSettings("settings.ini", QSettings::IniFormat))
        {
            setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
            setModal(true);
        }

        virtual float GetGravity() const = 0;
        virtual float GetAngularDamping() const = 0;
        virtual float GetLinearDamping() const = 0;
        virtual float GetHydrophoneUpdateTime() const = 0;
        virtual float GetHydrophoneSignal() const = 0;
        virtual float GetHydrophoneSpeed() const = 0;

        virtual bool ShowFrontCameraImage() const = 0;
        virtual bool ShowBottomCameraImage() const = 0;

    protected:
        QScopedPointer<QSettings> m_auvSettings;
    };
}

