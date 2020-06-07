#pragma once

#include <QWidget>

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>

namespace QUrho {
    class Urho3DCoreWidget : public QWidget, public Urho3D::Object {
    Q_OBJECT
    URHO3D_OBJECT(Urho3DCoreWidget, Urho3D::Object)

    public:
        explicit Urho3DCoreWidget(Urho3D::Context *context, QWidget *parent = nullptr);

        bool Initialize(const Urho3D::VariantMap &parameters);

        void ClearResourceCache();

        bool SetResourceCache(const Urho3D::VariantMap &parameters);

        void Exit();

        ~Urho3DCoreWidget();
    signals:

        void KeyPressed(QKeyEvent *event);

        void KeyReleased(QKeyEvent *event);

        void WheelMoved(QWheelEvent *event);

        void FocusOut(QFocusEvent *event);

    protected:
        [[nodiscard]] QPaintEngine *paintEngine() const override;

        void paintEvent(QPaintEvent *event) override;

        void keyPressEvent(QKeyEvent *event) override;

        void keyReleaseEvent(QKeyEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

        void focusOutEvent(QFocusEvent *event) override;

        void timerEvent(QTimerEvent *event) override;

    private:
        void nextFrame();

        Urho3D::SharedPtr<Urho3D::Engine> m_engine;
    };

    class QUrhoInput;

    class QUrhoWidget : public QWidget {
    Q_OBJECT
    public:
        explicit QUrhoWidget(QWidget *parent = nullptr);

        bool InitializeUrho3DEngine(const Urho3D::VariantMap &parameters);

        Urho3DCoreWidget *GetUrho3DCoreWidget();

        Urho3D::Context *GetUrho3DContext();

        QUrhoInput *GetUrho3DInput();

        void Exit();

    private:
        QScopedPointer<Urho3DCoreWidget> m_coreWidget;
        QScopedPointer<QUrhoInput> m_input;
    };
}