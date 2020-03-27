#include "QUrhoInput.h"
#include "QUrhoWidget.h"
#include "QUrhoHelpers.h"

#include <QKeyEvent>
#include <QMouseEvent>

#include <Urho3D/Input/Input.h>

namespace QUrho {

    QUrhoInput::QUrhoInput(Urho3D::Context *context, Urho3DCoreWidget *widget, QObject *parent) :
            Object(context),
            QObject(parent),
            m_coreWidget(widget) {
        CreateConnections();
    }

    bool QUrhoInput::IsKeyPressed(Qt::Key key) const {
        return m_keysPressed.contains(key);
    }

    bool QUrhoInput::IsMouseButtonDown(Qt::MouseButton button) const {
        return m_mouseButtonDown.contains(button);
    }

    Urho3D::IntVector2 QUrhoInput::GetMousePosition() const {
        return GetSubsystem<Urho3D::Input>()->GetMousePosition();
    }

    Urho3D::IntVector2 QUrhoInput::GetMouseMove() const {
        return GetSubsystem<Urho3D::Input>()->GetMouseMove();
    }

    int QUrhoInput::GetMouseWheelMove() const {
        return m_wheelDelta;
    }

    bool QUrhoInput::IsMouseButtonPressed(Qt::MouseButton button) const {
        return m_mouseButtonPressed.contains(button);
    }

    void QUrhoInput::CreateConnections() {
        connect(m_coreWidget, &Urho3DCoreWidget::KeyPressed, this, &QUrhoInput::OnKeyPressed);
        connect(m_coreWidget, &Urho3DCoreWidget::KeyReleased, this, &QUrhoInput::OnKeyReleased);
        connect(m_coreWidget, &Urho3DCoreWidget::WheelMoved, this, &QUrhoInput::OnWheelMoved);
        connect(m_coreWidget, &Urho3DCoreWidget::FocusOut, this, &QUrhoInput::OnFocusOut);

        SubscribeToEvent(Urho3D::E_MOUSEBUTTONDOWN, URHO3D_HANDLER(QUrhoInput, HandleMouseButtons));
        SubscribeToEvent(Urho3D::E_MOUSEBUTTONUP, URHO3D_HANDLER(QUrhoInput, HandleMouseButtons));
    }

    void QUrhoInput::OnKeyPressed(QKeyEvent *event) {
        m_keysPressed.insert((Qt::Key) event->key());
        if (!event->isAutoRepeat()) {
            m_keysDown.remove((Qt::Key) event->key());
        }
    }

    void QUrhoInput::OnKeyReleased(QKeyEvent *event) {
        m_keysPressed.remove((Qt::Key) event->key());
        if (!event->isAutoRepeat()) {
            m_keysDown.remove((Qt::Key) event->key());
        }
    }

    void QUrhoInput::OnWheelMoved(QWheelEvent *event) {
        m_wheelDelta += event->delta() / 120;
    }

    void QUrhoInput::OnFocusOut(QFocusEvent *event) {
        Q_UNUSED(event)
        m_keysPressed.clear();
        m_mouseButtonPressed.clear();
        m_wheelDelta = 0;
        SetMouseMode(Urho3D::MM_ABSOLUTE);
    }

    void QUrhoInput::HandleMouseButtons(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
        const Qt::MouseButton button = UrhoQtMouseCast(eventData[Urho3D::MouseButtonDown::P_BUTTON].GetInt());
        if (eventType == Urho3D::E_MOUSEBUTTONDOWN) {
            m_mouseButtonPressed.insert(button);
        } else {
            m_mouseButtonPressed.remove(button);
        }
    }

    void QUrhoInput::SetMouseMode(Urho3D::MouseMode mode) {
        GetSubsystem<Urho3D::Input>()->SetMouseMode(mode);
    }
}