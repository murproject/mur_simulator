//
// Created by Vlad on 5/26/2020.
//

#include "PingersOverlay.h"

#include "AUVOverlay.h"
#include "QUrhoScene.h"
#include "QUrhoInput.h"
#include "SharingOverlay.h"
#include <QDebug>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Graphics.h>

namespace QUrho {
    PingerOverlay::PingerOverlay(Urho3D::Context *context, QUrho::QUrhoScene *scene, QObject *parent) :
            QObject{parent},
            Urho3D::Object{context},
            m_urhoScene{scene},
            m_scene{scene->GetScene()} {

    }


    void PingerOverlay::Update(QUrhoInput *input, float timeStep) {
        m_updateDelta += timeStep;
        if (m_updateDelta >= m_updateTime) {
            Pingers pingers;
            {
                auto[length, angle] = CalculatePingerLengthAngle(m_pinger_0);
                pingers.angle_0 = angle;
                pingers.distance_0 = length;
            }

            {
                auto[length, angle] = CalculatePingerLengthAngle(m_pinger_1);
                pingers.angle_1 = angle;
                pingers.distance_1 = length;
            }

            {
                auto[length, angle] = CalculatePingerLengthAngle(m_pinger_2);
                pingers.angle_2 = angle;
                pingers.distance_2 = length;
            }

            {
                auto[length, angle] = CalculatePingerLengthAngle(m_pinger_3);
                pingers.angle_3 = angle;
                pingers.distance_3 = length;
            }
            m_urhoScene->GetNetworkOverlay()->SetPingers(pingers);
            m_updateDelta = 0;
        }
    }

    void PingerOverlay::CreatePingers() {
        for (auto &&item : m_scene->GetChildren(true)) {
            Urho3D::String str = item->GetName();
            if (str.Contains("Pinger_0")) {
                m_pinger_0 = item;
            }
            if (str.Contains("Pinger_1")) {
                m_pinger_1 = item;
            }
            if (str.Contains("Pinger_2")) {
                m_pinger_2 = item;
            }
            if (str.Contains("Pinger_3")) {
                m_pinger_3 = item;
            }
            if (str.Contains("AUV")) {
                m_auv = item;
            }
        }
    }

    std::tuple<float, float> PingerOverlay::CalculatePingerLengthAngle(Urho3D::Node *pinger) {
        if (!pinger) {
            return {0, 0};
        }
        auto auv_pos = m_auv->GetWorldPosition();
        auto pinger_pos = pinger->GetWorldPosition();
        auto offset = pinger_pos - auv_pos;
        auto length = offset.Length();
        auto rotation = Urho3D::Quaternion();
        rotation.FromLookRotation(offset);
        auto angle = (rotation * m_auv->GetRotation().Inverse()).YawAngle();
        return {length, angle};
    }

    void PingerOverlay::SetUpdateTime(float value) {
        m_updateTime = value;
    }

}

