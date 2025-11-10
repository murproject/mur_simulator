#pragma once

#include <QDialog>
#include <QSharedPointer>
#include "ValueSlider.h"
#include "qcheckbox.h"
#include <QVariant>

class QLabel;
class QSlider;
class QPushButton;
class QSettings;

namespace QUrho {
    struct GraphicsSettings {
        uint8_t texture_value = 2;
        uint8_t shadows_value = 3;
        int8_t  fisheye_value = 75;
        uint8_t fov_value = 75;
        uint8_t noise_value = 1;
        int8_t  chrom_abber_value = 7;

        bool bloom = true;
        bool fxaa3 = true;
        bool focus_blur = true;
        bool fisheye = true;
        bool noise = true;
        bool shadows = true;
        bool patricles = true;
    };

    inline constexpr GraphicsSettings default_graphics{};

class GraphicsSettingsWidget : public QDialog {
    Q_OBJECT

public:
    explicit GraphicsSettingsWidget(QWidget *parent = nullptr);

    bool GetCheckBoxValue(const QString&);
    int GetSliderValue(const QString&);

private slots:
    void OnApply();
    void OnCancel();
    void OnDefault();

private:
    void CreateLayout();
    void CreateConnections();
    void SaveSettings();
    void LoadSettings();

private:
    QSharedPointer<QSettings> m_graphicsSettings;

    QSharedPointer<QLabel> m_textureLabel;
    QSharedPointer<ValueSlider> m_textureSlider;

    QSharedPointer<QLabel> m_shadowsLabel;
    QSharedPointer<ValueSlider> m_shadowsSlider;

    QSharedPointer<QLabel> m_fisheyeLabel;
    QSharedPointer<ValueSlider> m_fisheyeSlider;

    QSharedPointer<QLabel> m_fovLabel;
    QSharedPointer<ValueSlider> m_fovSlider;

    QSharedPointer<QLabel> m_noiseLabel;
    QSharedPointer<ValueSlider> m_noiseSlider;

    QSharedPointer<QLabel> m_chromAbberLabel;
    QSharedPointer<ValueSlider> m_chromAbberSlider;

    QScopedPointer<QCheckBox> m_bloomBox;
    QScopedPointer<QCheckBox> m_FXAA3Box;
    QScopedPointer<QCheckBox> m_focusBlurBox;
    QScopedPointer<QCheckBox> m_fishEyeBox;
    QScopedPointer<QCheckBox> m_noiseBox;
    QScopedPointer<QCheckBox> m_shadowsBox;
    QScopedPointer<QCheckBox> m_particlesBox;

    QSharedPointer<QPushButton> m_apply;
    QSharedPointer<QPushButton> m_cancel;
    QSharedPointer<QPushButton> m_default;

    struct SliderEntry {
        const char* key;
        ValueSlider* slider;
        QLabel* label;
        QVariant defaultValue;
    };

    struct CheckBoxEntry {
        const char* key;
        QCheckBox* box;
        QVariant defaultValue;
    };

    std::vector<SliderEntry> m_sliders;
    std::vector<CheckBoxEntry> m_checkbox;
};

}
