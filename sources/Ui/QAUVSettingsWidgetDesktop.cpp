#include "QAUVSettingsWidgetDesktop.h"

#include <QVBoxLayout>
#include <QGroupBox>

using namespace QUrho;

namespace QUrho
{

QAUVSettingsWidgetDesktop::QAUVSettingsWidgetDesktop(QWidget *parent) :
    QAUVSettingsWidgetBase(parent),
    m_buoyancyLabel(new QLabel("Buoyancy: ", this)),
    m_buoyancySlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_angularDampingLabel(new QLabel("Angular damping: ", this)),
    m_angularDampingSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_linearDampingLabel(new QLabel("Linear damping: ", this)),
    m_linearDampingSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_hydrophoneUpdateTime(new QLabel("Pulse period: ", this)),
    m_hydrophoneUpdateTimeSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_hydrophoneSignal(new QLabel("Pulse width: ", this)),
    m_hydrophoneSignalSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_hydrophoneSpeed(new QLabel("Spreading speed: ", this)),
    m_hydrophoneSpeedSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
    m_frontCameraCheckbox(new QCheckBox("Show front camera", this)),
    m_bottomCameraCheckbox(new QCheckBox("Show bottom camera", this)),
    m_apply(new QPushButton(tr("Apply"), this)),
    m_cancel(new QPushButton(tr("Cancel"), this)),
    m_default(new QPushButton(tr("Default"), this)) {
    CreateLayout();
    CreateConnections();
    m_buoyancySlider->setRange(-100, 100);
    m_hydrophoneUpdateTimeSlider->setRange(3000, 5000);
    m_hydrophoneSignalSlider->setRange(100, 300);
    m_hydrophoneSpeedSlider->setRange(800, 900);
    LoadSettings();
}

float QAUVSettingsWidgetDesktop::GetGravity() const {
    return static_cast<float>(m_buoyancySlider->value()) / -1000.0f;
}

float QAUVSettingsWidgetDesktop::GetAngularDamping() const {
    return static_cast<float>(m_angularDampingSlider->value()) / 100.0f;
}

float QAUVSettingsWidgetDesktop::GetLinearDamping() const {
    return static_cast<float>(m_linearDampingSlider->value()) / 100.0f;
}

bool QAUVSettingsWidgetDesktop::ShowFrontCameraImage() const {
    return m_frontCameraCheckbox->isChecked();
}

bool QAUVSettingsWidgetDesktop::ShowBottomCameraImage() const {
    return m_bottomCameraCheckbox->isChecked();
}

void QAUVSettingsWidgetDesktop::CreateLayout() {
    auto auvLayout = new QVBoxLayout();
    auto addSlider = [](QVBoxLayout *layout, QLabel *label, ValueSlider *slider)
    {
        auto sub = new QHBoxLayout();
        sub->addWidget(label);
        sub->addWidget(slider);
        layout->addLayout(sub);
    };

    addSlider(auvLayout, m_buoyancyLabel.data(), m_buoyancySlider.data());
    addSlider(auvLayout, m_angularDampingLabel.data(), m_angularDampingSlider.data());
    addSlider(auvLayout, m_linearDampingLabel.data(), m_linearDampingSlider.data());
    addSlider(auvLayout, m_hydrophoneUpdateTime.data(), m_hydrophoneUpdateTimeSlider.data());
    addSlider(auvLayout, m_hydrophoneSignal.data(), m_hydrophoneSignalSlider.data());
    addSlider(auvLayout, m_hydrophoneSpeed.data(), m_hydrophoneSpeedSlider.data());
    {
        auto sub = new QHBoxLayout;
        sub->addWidget(m_frontCameraCheckbox.data());
        auvLayout->addLayout(sub);
    }
    {
        auto sub = new QHBoxLayout;
        sub->addWidget(m_bottomCameraCheckbox.data());
        auvLayout->addLayout(sub);
    }

    auto buttons = new QHBoxLayout;
    buttons->addWidget(m_apply.data());
    buttons->addWidget(m_cancel.data());
    buttons->addWidget(m_default.data());
    auto auvGroup = new QGroupBox("Environment settings");

    auvGroup->setLayout(auvLayout);

    auto mainLayout = new QVBoxLayout();
    mainLayout->addWidget(auvGroup);

    mainLayout->addLayout(buttons);
    setLayout(mainLayout);
}

float QAUVSettingsWidgetDesktop::GetHydrophoneUpdateTime() const {
    return static_cast<float>(m_hydrophoneUpdateTimeSlider->value()) / 1000.0f;
}

float QAUVSettingsWidgetDesktop::GetHydrophoneSignal() const {
    return static_cast<float>(m_hydrophoneSignalSlider->value()) / 1000.0f;
}

float QAUVSettingsWidgetDesktop::GetHydrophoneSpeed() const {
    return static_cast<float>(m_hydrophoneSpeedSlider->value()) / 100.0f;
}

void QAUVSettingsWidgetDesktop::SaveSettings() {
    m_auvSettings->setValue("buoyancy", m_buoyancySlider->value());
    m_auvSettings->setValue("angular damping", m_angularDampingSlider->value());
    m_auvSettings->setValue("linear damping", m_linearDampingSlider->value());
    m_auvSettings->setValue("show front camera", m_frontCameraCheckbox->isChecked());
    m_auvSettings->setValue("show bottom camera", m_bottomCameraCheckbox->isChecked());
    m_auvSettings->setValue("pulse period", m_hydrophoneUpdateTimeSlider->value());
    m_auvSettings->setValue("pulse width", m_hydrophoneSignalSlider->value());
    m_auvSettings->setValue("spreading speed", m_hydrophoneSpeedSlider->value());
}

void QAUVSettingsWidgetDesktop::LoadSettings() {
    auto buoyancy = m_auvSettings->value("buoyancy", default_settings.buoyancy).toInt();
    auto angularDamping = m_auvSettings->value("angular damping", default_settings.angularDamping).toInt();
    auto linearDamping = m_auvSettings->value("linear damping", default_settings.linearDamping).toInt();
    auto frontCamera = m_auvSettings->value("show front camera", default_settings.showFrontCameraDesktop).toBool();
    auto bottomCamera = m_auvSettings->value("show bottom camera", default_settings.showBottomCameraDesktop).toBool();
    auto hydrophoneUpdate = m_auvSettings->value("pulse period", default_settings.hydrophoneUpdate).toInt();
    auto hydrophoneSignal = m_auvSettings->value("pulse width", default_settings.hydrophoneSignal).toInt();
    auto hydrophoneSpeed = m_auvSettings->value("spreading speed", default_settings.hydrophoneSpeed).toInt();
    m_buoyancySlider->setValue(buoyancy);
    m_angularDampingSlider->setValue(angularDamping);
    m_linearDampingSlider->setValue(linearDamping);
    m_frontCameraCheckbox->setChecked(frontCamera);
    m_bottomCameraCheckbox->setChecked(bottomCamera);
    m_hydrophoneUpdateTimeSlider->setValue(hydrophoneUpdate);
    m_hydrophoneSignalSlider->setValue(hydrophoneSignal);
    m_hydrophoneSpeedSlider->setValue(hydrophoneSpeed);
}

void QAUVSettingsWidgetDesktop::CreateConnections() {
    connect(m_apply.data(), &QPushButton::clicked, this, &QAUVSettingsWidgetDesktop::OnApply);
    connect(m_cancel.data(), &QPushButton::clicked, this, &QAUVSettingsWidgetDesktop::OnCancel);
    connect(m_default.data(), &QPushButton::clicked, this, &QAUVSettingsWidgetDesktop::OnDefault);
}

void QAUVSettingsWidgetDesktop::OnApply() {
    SaveSettings();
    accept();
}

void QAUVSettingsWidgetDesktop::OnCancel() {
    reject();
}

void QAUVSettingsWidgetDesktop::OnDefault() {
    m_buoyancySlider->setValue(default_settings.buoyancy);
    m_angularDampingSlider->setValue(default_settings.angularDamping);
    m_linearDampingSlider->setValue(default_settings.linearDamping);
    m_hydrophoneUpdateTimeSlider->setValue(default_settings.hydrophoneUpdate);
    m_hydrophoneSignalSlider->setValue(default_settings.hydrophoneSignal);
    m_hydrophoneSpeedSlider->setValue(default_settings.hydrophoneSpeed);
    m_frontCameraCheckbox->setChecked(default_settings.showFrontCameraDesktop);
    m_bottomCameraCheckbox->setChecked(default_settings.showBottomCameraDesktop);
}

QString QAUVSettingsWidgetDesktop::GetLastScene() {
    return m_auvSettings->value("last scene", "").toString();
}

void QAUVSettingsWidgetDesktop::SetLastScene(const QString &scene) {
    m_lastScene = scene;
    m_auvSettings->setValue("last scene", m_lastScene);
}
}
