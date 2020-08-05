/*
 * Created by Vladislav Bolotov on 10/24/2019. <vladislav.bolotov@gmail.com>
*/

#include "QAUVSettingsWidget.h"

#include <QVBoxLayout>
#include <QGroupBox>

namespace QUrho {

    QAUVSettingsWidget::QAUVSettingsWidget(QWidget *parent) :
            QDialog(parent),
            m_auvSettings(new QSettings("settings.ini", QSettings::IniFormat)),
            m_buoyancyLabel(new QLabel("Buoyancy: ", this)),
            m_buoyancySlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
            m_angularDampingLabel(new QLabel("Angular damping: ", this)),
            m_angularDampingSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
            m_linearDampingLabel(new QLabel("Linear damping: ", this)),
            m_linearDampingSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
            m_pingerUpdateTime(new QLabel("Pinger update: ", this)),
            m_pingerUpdateTimeSlider(new ValueSlider(Qt::Orientation::Horizontal, this)),
            m_frontCameraCheckbox(new QCheckBox("Show front camera", this)),
            m_bottomCameraCheckbox(new QCheckBox("Show bottom camera", this)),
            m_apply(new QPushButton(tr("Apply"), this)),
            m_cancel(new QPushButton(tr("Cancel"), this)),
            m_default(new QPushButton(tr("Default"), this)) {
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
        setModal(true);
        CreateLayout();
        CreateConnections();
        m_buoyancySlider->setRange(-100, 100);
        m_pingerUpdateTimeSlider->setRange(0, 4000);
        LoadSettings();
    }

    float QAUVSettingsWidget::GetGravity() {
        return static_cast<float>(m_buoyancySlider->value()) / -1000.0f;
    }

    float QAUVSettingsWidget::GetAngularDamping() {
        return static_cast<float>(m_angularDampingSlider->value()) / 100.0f;
    }

    float QAUVSettingsWidget::GetLinearDamping() {
        return static_cast<float>(m_linearDampingSlider->value()) / 100.0f;
    }

    bool QAUVSettingsWidget::ShowFrontCameraImage() {
        return m_frontCameraCheckbox->isChecked();
    }

    bool QAUVSettingsWidget::ShowBottomCameraImage() {
        return m_bottomCameraCheckbox->isChecked();
    }

    void QAUVSettingsWidget::SaveSettings() {
        m_auvSettings->setValue("buoyancy", m_buoyancySlider->value());
        m_auvSettings->setValue("angular damping", m_angularDampingSlider->value());
        m_auvSettings->setValue("linear damping", m_linearDampingSlider->value());
        m_auvSettings->setValue("show front camera", m_frontCameraCheckbox->isChecked());
        m_auvSettings->setValue("show bottom camera", m_bottomCameraCheckbox->isChecked());
        m_auvSettings->setValue("pinger update time", m_pingerUpdateTimeSlider->value());
    }

    void QAUVSettingsWidget::LoadSettings() {
        auto buoyancy = m_auvSettings->value("buoyancy", 5).toInt();
        auto angularDamping = m_auvSettings->value("angular damping", 80).toInt();
        auto linearDamping = m_auvSettings->value("linear damping", 50).toInt();
        auto frontCamera = m_auvSettings->value("show front camera", false).toBool();
        auto bottomCamera = m_auvSettings->value("show front camera", false).toBool();
        auto pingerUpdate = m_auvSettings->value("pinger update time", 2000).toInt();
        m_buoyancySlider->setValue(buoyancy);
        m_angularDampingSlider->setValue(angularDamping);
        m_linearDampingSlider->setValue(linearDamping);
        m_frontCameraCheckbox->setChecked(frontCamera);
        m_bottomCameraCheckbox->setChecked(bottomCamera);
        m_pingerUpdateTimeSlider->setValue(pingerUpdate);
    }

    void QAUVSettingsWidget::CreateLayout() {
        auto auvLayout = new QVBoxLayout();
        auto addSlider = [](QVBoxLayout *layout, QLabel *label, ValueSlider *slider) {
            auto sub = new QHBoxLayout();
            sub->addWidget(label);
            sub->addWidget(slider);
            layout->addLayout(sub);
        };

        addSlider(auvLayout, m_buoyancyLabel.data(), m_buoyancySlider.data());
        addSlider(auvLayout, m_angularDampingLabel.data(), m_angularDampingSlider.data());
        addSlider(auvLayout, m_linearDampingLabel.data(), m_linearDampingSlider.data());
        addSlider(auvLayout, m_pingerUpdateTime.data(), m_pingerUpdateTimeSlider.data());
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

    void QAUVSettingsWidget::CreateConnections() {
        connect(m_apply.data(), &QPushButton::clicked, this, &QAUVSettingsWidget::OnApply);
        connect(m_cancel.data(), &QPushButton::clicked, this, &QAUVSettingsWidget::OnCancel);
        connect(m_default.data(), &QPushButton::clicked, this, &QAUVSettingsWidget::OnDefault);
    }

    void QAUVSettingsWidget::OnApply() {
        SaveSettings();
        accept();
    }

    void QAUVSettingsWidget::OnCancel() {
        reject();
    }

    void QAUVSettingsWidget::OnDefault() {
        m_buoyancySlider->setValue(5);
        m_angularDampingSlider->setValue(80);
        m_linearDampingSlider->setValue(50);
        m_pingerUpdateTimeSlider->setValue(2000);
        m_frontCameraCheckbox->setChecked(false);
        m_bottomCameraCheckbox->setChecked(false);
    }

    QString QAUVSettingsWidget::GetLastScene() {
        return m_auvSettings->value("last scene", "").toString();
    }

    void QAUVSettingsWidget::SetLastScene(const QString &scene) {
        m_lastScene = scene;
        m_auvSettings->setValue("last scene", m_lastScene);
    }

    float QAUVSettingsWidget::GetPingerUpdateTime() {
        return static_cast<float>(m_pingerUpdateTimeSlider->value()) / 1000.0f;
    }
}