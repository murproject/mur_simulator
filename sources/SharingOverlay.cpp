/*
 * Created by Vladislav Bolotov on 10/23/2019. <vladislav.bolotov@gmail.com>
*/

#include "SharingOverlay.h"
#include "AUVOverlay.h"
#include "QUrhoScene.h"

#include <zmq.h>

#include <shared_mutex>

namespace QUrho {


    SharingOverlay::SharingOverlay(Urho3D::Context *context, QUrhoScene *scene, QObject *parent) :
            QObject(parent),
            Object(context),
            m_urhoScene(scene),
            m_zmqContext(zmq_ctx_new()),
            m_zmqBottomData(zmq_socket(m_zmqContext, ZMQ_PUB)),
            m_zmqFrontData(zmq_socket(m_zmqContext, ZMQ_PUB)),
            m_zmqTelemetryData(zmq_socket(m_zmqContext, ZMQ_PUB)),
            m_userApiPair(zmq_socket(m_zmqContext, ZMQ_PAIR)) {
        CreateSockets();
    }

    void SharingOverlay::Update(QUrhoInput *, float) {
        UpdateTelemetry();
    }

    const Telemetry &SharingOverlay::GetTelemetry() const {
        return m_telemetry;
    }

    const Control &SharingOverlay::GetControl() const {
        return m_control;
    }

    void SharingOverlay::SetTelemetry(Telemetry &telemetry) {
        m_telemetry = telemetry;
    }

    void SharingOverlay::CreateSockets() {
        int hwmOption = 1;
        int linger = 2;

        zmq_setsockopt(m_zmqBottomData, ZMQ_SNDHWM, &hwmOption, sizeof(int));
        zmq_setsockopt(m_zmqBottomData, ZMQ_LINGER, &linger, sizeof(int));
        zmq_bind(m_zmqBottomData, "tcp://127.0.0.1:1771");

        zmq_setsockopt(m_zmqFrontData, ZMQ_SNDHWM, &hwmOption, sizeof(int));
        zmq_setsockopt(m_zmqFrontData, ZMQ_LINGER, &linger, sizeof(int));
        zmq_bind(m_zmqFrontData, "tcp://127.0.0.1:1772");

        zmq_setsockopt(m_zmqTelemetryData, ZMQ_SNDHWM, &hwmOption, sizeof(int));
        zmq_setsockopt(m_zmqTelemetryData, ZMQ_LINGER, &linger, sizeof(int));
        zmq_bind(m_zmqTelemetryData, "tcp://127.0.0.1:3390");

        zmq_bind(m_userApiPair, "tcp://127.0.0.1:3391");
//
//        m_controlUpdateThread = std::thread([this]() {
//            while (m_update) {

//            }
//        });
    }

    SharingOverlay::~SharingOverlay() {
        m_update = false;
//        m_controlUpdateThread.join();
        zmq_close(m_zmqBottomData);
        zmq_close(m_zmqFrontData);
        zmq_close(m_zmqTelemetryData);
        zmq_close(m_userApiPair);
        zmq_ctx_destroy(m_zmqContext);
    }

    void SharingOverlay::UpdateTelemetry() {
        auto frontImage = m_urhoScene->GetAUVOverlay()->GetFrontCameraImage();
        SentData(m_zmqFrontData, frontImage.data(), frontImage.size());

        auto bottomImage = m_urhoScene->GetAUVOverlay()->GetBottomCameraImage();
        SentData(m_zmqBottomData, bottomImage.data(), bottomImage.size());

        auto rotation = m_urhoScene->GetAUVOverlay()->GetAUVRotations();
        m_telemetry.yaw = rotation.y_;
        m_telemetry.pitch = rotation.x_;
        m_telemetry.roll = rotation.z_;
        m_telemetry.depth = m_urhoScene->GetAUVOverlay()->GetAUVDepth();
        SentData(m_zmqTelemetryData, reinterpret_cast<unsigned char *>(&m_telemetry), sizeof(Telemetry));

        zmq_msg_t controlData{};
        zmq_msg_init(&controlData);
        if (zmq_msg_recv(&controlData, m_userApiPair, ZMQ_DONTWAIT) != -1) {
            std::memcpy(&m_control, zmq_msg_data(&controlData), zmq_msg_size(&controlData));
        }
        zmq_msg_close(&controlData);
    }

    void SharingOverlay::SentData(void *socket, const unsigned char *data, size_t size) {
        zmq_msg_t message;
        zmq_msg_init_size(&message, size);
        std::memcpy(zmq_msg_data(&message), data, size);

        if (-1 == zmq_msg_send(&message, socket, 0)) {
            zmq_msg_close(&message);
            return;
        }
        zmq_msg_close(&message);
    }

    void SharingOverlay::Reset() {
        m_control = {};
    }

    void SharingOverlay::SetPingers(Pingers &pinger) {
        m_telemetry.angle_0 = pinger.angle_0;
        m_telemetry.angle_1 = pinger.angle_1;
        m_telemetry.angle_2 = pinger.angle_2;
        m_telemetry.angle_3 = pinger.angle_3;

        m_telemetry.distance_0 = pinger.distance_0;
        m_telemetry.distance_1 = pinger.distance_1;
        m_telemetry.distance_2= pinger.distance_2;
        m_telemetry.distance_3 = pinger.distance_3;

    }
    void SharingOverlay::SetHydrophones(Hydrophones &hydrophone) {
        m_telemetry.signal_tr = hydrophone.signal_tr;
        m_telemetry.signal_tl = hydrophone.signal_tl;
        m_telemetry.signal_fr = hydrophone.signal_fr;

        m_telemetry.distance_tr = hydrophone.distance_tr;
        m_telemetry.distance_tl = hydrophone.distance_tl;
        m_telemetry.distance_fr = hydrophone.distance_fr;
    }
}
