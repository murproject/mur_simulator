#pragma once

#include "QAUVSettingsWidgetBase.h"
#include "ValueSlider.h"
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>

namespace QUrho {
class QAUVSettingsWidgetDesktop : public QAUVSettingsWidgetBase {
    Q_OBJECT
public:
    explicit QAUVSettingsWidgetDesktop(QWidget *parent = nullptr);

    float GetGravity() const override;
    float GetAngularDamping() const override;
    float GetLinearDamping() const override;
    float GetHydrophoneUpdateTime() const override;
    float GetHydrophoneSignal() const override;
    float GetHydrophoneSpeed() const override;

    bool ShowFrontCameraImage() const override;
    bool ShowBottomCameraImage() const override;

    QString GetLastScene();
    void SetLastScene(const QString &scene);

private:
    void OnApply();
    void OnCancel();
    void OnDefault();
    void LoadSettings();
    void SaveSettings();
    void CreateConnections();
    void CreateLayout();

    QScopedPointer<QLabel> m_buoyancyLabel;
    QScopedPointer<ValueSlider> m_buoyancySlider;

    QScopedPointer<QLabel> m_angularDampingLabel;
    QScopedPointer<ValueSlider> m_angularDampingSlider;

    QScopedPointer<QLabel> m_linearDampingLabel;
    QScopedPointer<ValueSlider> m_linearDampingSlider;

    QScopedPointer<QLabel> m_hydrophoneUpdateTime;
    QScopedPointer<ValueSlider> m_hydrophoneUpdateTimeSlider;

    QScopedPointer<QLabel> m_hydrophoneSignal;
    QScopedPointer<ValueSlider> m_hydrophoneSignalSlider;

    QScopedPointer<QLabel> m_hydrophoneSpeed;
    QScopedPointer<ValueSlider> m_hydrophoneSpeedSlider;

    QScopedPointer<QCheckBox> m_frontCameraCheckbox;
    QScopedPointer<QCheckBox> m_bottomCameraCheckbox;

    QScopedPointer<QPushButton> m_apply;
    QScopedPointer<QPushButton> m_cancel;
    QScopedPointer<QPushButton> m_default;

    QString m_lastScene;
};

}

