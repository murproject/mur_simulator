#pragma once

#include "QSceneOverlay.h"

#include <Urho3D/Core/Context.h>

#include <thread>
#include <shared_mutex>
#include <QObject>

namespace QUrho {
#pragma pack(push, 1)
    struct Telemetry {
        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        float depth = 0.0f;
        float temperature = 0.0f;
        float pressure = 0.0f;
        float voltage = 0.0f;

        uint8_t signal_tr = 0;
        uint8_t signal_tl = 0;
        uint8_t signal_fr = 0;
        uint16_t distance_tr = 0;
        uint16_t distance_tl = 0;
        uint16_t distance_fr = 0;

    };

    struct Control {
        int8_t thrust_flag = 0;
        int16_t thrusters[8] = {0};
        uint8_t colorR = 0;
        uint8_t colorG = 0;
        uint8_t colorB = 0;
        float onDelay = 0;
        float offDelay = 0;
    };

    struct Hydrophones {
        uint8_t signal_tr = 0;
        uint8_t signal_tl = 0;
        uint8_t signal_fr = 0;
        uint16_t distance_tr = 0;
        uint16_t distance_tl = 0;
        uint16_t distance_fr = 0;

    };

#pragma pack(pop)


    class QUrhoScene;

    class QUrhoInput;

    class SharingOverlay : public QObject, public Urho3D::Object, public QSceneOverlay {
    Q_OBJECT
    URHO3D_OBJECT(SharingOverlay, Urho3D::Object)

    public:
        explicit SharingOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent = nullptr);

        ~SharingOverlay() override;

        void Update(QUrhoInput *input, float timeStep) override;

        const Telemetry &GetTelemetry() const;

        const Control &GetControl() const;

        void SetTelemetry(Telemetry &telemetry);

        void SetHydrophones(Hydrophones &hydrophone);

        void Reset();

    private:
        static inline void SentData(void *socket, const unsigned char* data, size_t size);

        void CreateSockets();

        void UpdateTelemetry();

        QUrhoScene *m_urhoScene;
        Telemetry m_telemetry;
        Control m_control;

        void *m_zmqContext;
        void *m_zmqBottomData;
        void *m_zmqTelemetryData;
        void *m_zmqFrontData;
        void *m_userApiPair;

        mutable std::mutex m_controlMutex;
        std::atomic_bool m_update = true;
        std::thread m_controlUpdateThread;
    };

}

