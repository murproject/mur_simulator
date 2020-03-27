#include "QUrhoWidget.h"
#include "QUrhoInput.h"

#include <QEvent>
#include <QDebug>
#include <QLayout>
#include <QVBoxLayout>

#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Resource/XMLFile.h>


namespace QUrho {

    Urho3DCoreWidget::Urho3DCoreWidget(Urho3D::Context *context, QWidget *parent) :
            QWidget(parent),
            Object(context),
            m_engine{new Urho3D::Engine{context}} {
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_PaintOnScreen);
        startTimer(30);
    }

    bool Urho3DCoreWidget::Initialize(const Urho3D::VariantMap &parameters) {
        if (m_engine->IsInitialized()) {
            return true;
        }

        auto parametersToSet = parameters;
        parametersToSet[Urho3D::EP_FULL_SCREEN] = false;
        parametersToSet[Urho3D::EP_EXTERNAL_WINDOW] = reinterpret_cast<void *>(winId());

        return m_engine->Initialize(parametersToSet);
    }

    void Urho3DCoreWidget::ClearResourceCache() {
        if (!m_engine->IsInitialized() || m_engine->IsExiting()) {
            return;
        }

        if (auto cache = m_engine->GetSubsystem<Urho3D::ResourceCache>()) {
            cache->ReleaseAllResources(true);
        }
    }

    bool Urho3DCoreWidget::SetResourceCache(const Urho3D::VariantMap &parameters) {
        if (!m_engine->IsInitialized() || m_engine->IsExiting()) {
            return false;
        }

        if (m_engine->IsInitialized()) {
            return m_engine->InitializeResourceCache(parameters);
        }
        return false;
    }

    QPaintEngine *Urho3DCoreWidget::paintEngine() const {
        return nullptr;
    }

    void Urho3DCoreWidget::paintEvent(QPaintEvent *event) {
        Q_UNUSED(event);
    }

    void Urho3DCoreWidget::keyPressEvent(QKeyEvent *event) {
        emit KeyPressed(event);
    }

    void Urho3DCoreWidget::keyReleaseEvent(QKeyEvent *event) {
        emit KeyReleased(event);
    }

    void Urho3DCoreWidget::wheelEvent(QWheelEvent *event) {
        emit WheelMoved(event);
    }

    void Urho3DCoreWidget::focusOutEvent(QFocusEvent *event) {
        emit FocusOut(event);
    }

    void Urho3DCoreWidget::timerEvent(QTimerEvent *event) {
        nextFrame();
    }

    void Urho3DCoreWidget::nextFrame() {
        if (m_engine->IsInitialized() && !m_engine->IsExiting()) {
            m_engine->RunFrame();
        }
    }

    QUrhoWidget::QUrhoWidget(QWidget *parent) :
            QWidget(parent),
            m_coreWidget{new Urho3DCoreWidget{new Urho3D::Context, this}},
            m_input{new QUrhoInput{m_coreWidget->GetContext(), m_coreWidget.data(), this}} {

        auto mainLayout = new QVBoxLayout(this);
        layout()->addWidget(m_coreWidget.data());
        layout()->setContentsMargins(0, 0, 0, 0);
    }

    bool QUrhoWidget::InitializeUrho3DEngine(const Urho3D::VariantMap &parameters) {
        return m_coreWidget->Initialize(parameters);
    }

    Urho3DCoreWidget *QUrhoWidget::GetUrho3DCoreWidget() {
        return m_coreWidget.data();
    }

    Urho3D::Context *QUrhoWidget::GetUrho3DContext() {
        return m_coreWidget->GetContext();
    }

    QUrhoInput *QUrhoWidget::GetUrho3DInput() {
        return m_input.data();
    }
}