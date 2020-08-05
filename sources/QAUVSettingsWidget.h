#pragma once

#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QPushButton>
#include "ValueSlider.h"

namespace QUrho {

    class QAUVSettingsWidget : public QDialog {
    Q_OBJECT
    public:
        explicit QAUVSettingsWidget(QWidget *parent = nullptr);

        float GetGravity();

        float GetPingerUpdateTime();

        float GetAngularDamping();

        float GetLinearDamping();

        bool ShowFrontCameraImage();

        bool ShowBottomCameraImage();

        QString GetLastScene();

        void SetLastScene(const QString &scene);

    private:
        void OnApply();

        void OnCancel();

        void OnDefault();

        void CreateLayout();

        void LoadSettings();

        void SaveSettings();

        void CreateConnections();

        QScopedPointer<QSettings> m_auvSettings;
        QScopedPointer<QLabel> m_buoyancyLabel;
        QScopedPointer<ValueSlider> m_buoyancySlider;

        QScopedPointer<QLabel> m_angularDampingLabel;
        QScopedPointer<ValueSlider> m_angularDampingSlider;

        QScopedPointer<QLabel> m_linearDampingLabel;
        QScopedPointer<ValueSlider> m_linearDampingSlider;

        QScopedPointer<QLabel> m_pingerUpdateTime;
        QScopedPointer<ValueSlider> m_pingerUpdateTimeSlider;

        QScopedPointer<QCheckBox> m_frontCameraCheckbox;

        QScopedPointer<QCheckBox> m_bottomCameraCheckbox;

        QScopedPointer<QPushButton> m_apply;
        QScopedPointer<QPushButton> m_cancel;
        QScopedPointer<QPushButton> m_default;

        QString m_lastScene;
//        QScopedPointer<QLabel> m_rightBackThrusterLabel;
//        QScopedPointer<ValueSlider> m_buoyancySlider;
    };

}

