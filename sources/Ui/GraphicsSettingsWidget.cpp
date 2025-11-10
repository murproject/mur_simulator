#include "GraphicsSettingsWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>

namespace QUrho {

GraphicsSettingsWidget::GraphicsSettingsWidget(QWidget *parent) :
    QDialog(parent),
    m_graphicsSettings(new QSettings("settings.ini", QSettings::IniFormat)),
    m_textureLabel(new QLabel("Texture Quality:", this)),
    m_textureSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_shadowsLabel(new QLabel("Shadows Quality:", this)),
    m_shadowsSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_fisheyeLabel(new QLabel("Fisheye:", this)),
    m_fisheyeSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_fovLabel(new QLabel("Fov:", this)),
    m_fovSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_noiseLabel(new QLabel("Noise:", this)),
    m_noiseSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_chromAbberLabel(new QLabel("Chromatic Aberration:", this)),
    m_chromAbberSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),

    m_bloomBox(new QCheckBox("Bloom", this)),
    m_FXAA3Box(new QCheckBox("FXAA3", this)),
    m_focusBlurBox(new QCheckBox("Focus blur", this)),
    m_fishEyeBox(new QCheckBox("Fisheye", this)),
    m_noiseBox(new QCheckBox("Noise", this)),
    m_shadowsBox(new QCheckBox("Shadows", this)),
    m_particlesBox(new QCheckBox("Particles", this)),

    m_apply(new QPushButton(tr("Apply"), this)),
    m_cancel(new QPushButton(tr("Cancel"), this)),
    m_default(new QPushButton(tr("Default"), this))
{
    m_sliders = {
        {"texture quality", m_textureSlider.data(), m_textureLabel.data(), default_graphics.texture_value},
        {"shadows quality", m_shadowsSlider.data(), m_shadowsLabel.data(), default_graphics.shadows_value},
        {"fisheye value", m_fisheyeSlider.data(), m_fisheyeLabel.data(), default_graphics.fisheye_value},
        {"fov value", m_fovSlider.data(), m_fovLabel.data(), default_graphics.fov_value},
        {"noise value", m_noiseSlider.data(), m_noiseLabel.data(), default_graphics.noise_value},
        {"chromabber value", m_chromAbberSlider.data(), m_chromAbberLabel.data(), default_graphics.chrom_abber_value}
    };

    m_checkbox = {
        {"render bloom", m_bloomBox.data(), default_graphics.bloom},
        {"render fxaa3", m_FXAA3Box.data(), default_graphics.fxaa3},
        {"render focus blur", m_focusBlurBox.data(), default_graphics.focus_blur},
        {"render fish eye", m_fishEyeBox.data(), default_graphics.fisheye},
        {"render noise", m_noiseBox.data(), default_graphics.noise},
        {"shadows draw", m_shadowsBox.data(), default_graphics.shadows},
        {"particles", m_particlesBox.data(), default_graphics.patricles}
    };

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setModal(true);

    m_shadowsSlider->setRange(0, 3);
    m_textureSlider->setRange(0, 2);
    m_fovSlider->setRange(45, 75);

    m_fisheyeSlider->setRange(-100, 100);
    m_fisheyeSlider->setDivider(100);

    m_noiseSlider->setRange(0, 8);
    m_noiseSlider->setDivider(10);

    m_chromAbberSlider->setRange(-20, 20);
    m_chromAbberSlider->setDivider(10);

    CreateLayout();
    CreateConnections();
    LoadSettings();
}

bool GraphicsSettingsWidget::GetCheckBoxValue(const QString& name)  {
    for (auto& e : m_checkbox) {
        if (e.key == name)
            return e.box->isChecked();
    }
    return false;
}

int GraphicsSettingsWidget::GetSliderValue(const QString& name)  {
    for (auto& e : m_sliders) {
        if (e.key == name)
            return e.slider->value();
    }
    return 0;
}

void GraphicsSettingsWidget::SaveSettings() {
    m_graphicsSettings->beginGroup("Graphics");

    for (auto& e : m_checkbox) {
        m_graphicsSettings->setValue(e.key, e.box->isChecked());
    }

    for (auto& e : m_sliders) {
        m_graphicsSettings->setValue(e.key, e.slider->value());
    }

    m_graphicsSettings->endGroup();
}

void GraphicsSettingsWidget::LoadSettings() {
    m_graphicsSettings->beginGroup("Graphics");

    for (auto& e : m_checkbox) {
        auto value = m_graphicsSettings->value(e.key, e.defaultValue);
        e.box->setChecked(value.toBool());
    }

    for (auto& e : m_sliders) {
        auto value = m_graphicsSettings->value(e.key, e.defaultValue);
        e.slider->setValue(value.toInt());
    }

    m_graphicsSettings->endGroup();
}

void GraphicsSettingsWidget::CreateLayout() {
    auto layout = new QVBoxLayout();

    auto addSlider = [](QVBoxLayout *layout, QLabel *label, ValueSlider *slider) {
        auto sub = new QHBoxLayout();
        sub->addWidget(label);
        sub->addWidget(slider);
        layout->addLayout(sub);
    };

    auto addCheckBox = [](QVBoxLayout *layout, QCheckBox *box) {
        auto sub = new QHBoxLayout;
        sub->addWidget(box);
        layout->addLayout(sub);
    };

    for (auto& e : m_sliders) {
        addSlider(layout, e.label, e.slider);
    }

    for (auto& e : m_checkbox) {
        addCheckBox(layout, e.box);
    }

    auto buttons = new QHBoxLayout();
    buttons->addWidget(m_apply.data());
    buttons->addWidget(m_cancel.data());
    buttons->addWidget(m_default.data());

    auto group = new QGroupBox("Graphics settings");
    group->setLayout(layout);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(group);
    mainLayout->addLayout(buttons);

    setLayout(mainLayout);
}

void GraphicsSettingsWidget::CreateConnections() {
    connect(m_apply.data(), &QPushButton::clicked, this, &GraphicsSettingsWidget::OnApply);
    connect(m_cancel.data(), &QPushButton::clicked, this, &GraphicsSettingsWidget::OnCancel);
    connect(m_default.data(), &QPushButton::clicked, this, &GraphicsSettingsWidget::OnDefault);
}

void GraphicsSettingsWidget::OnApply() {
    SaveSettings();
    accept();
}

void GraphicsSettingsWidget::OnCancel() {
    reject();
}

void GraphicsSettingsWidget::OnDefault() {
    for (auto& e : m_checkbox) {
        e.box->setChecked(e.defaultValue.toBool());
    }

    for (auto& e : m_sliders) {
        e.slider->setValue(e.defaultValue.toInt());
    }
}

}
