#include "HydrophonesOverlay.h"

#include "AUVOverlay.h"
#include "QUrhoScene.h"
#include "QUrhoInput.h"
#include "SharingOverlay.h"
#include <QDebug>
#include <math.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Graphics.h>

namespace QUrho{
    HydrophoneOverlay::HydrophoneOverlay(Urho3D::Context *context, QUrho::QUrhoScene *scene, QObject *parent) :
        QObject{parent},
        Urho3D::Object{context},
        m_urhoScene{scene},
        m_scene{scene->GetScene()} {
    }

    void HydrophoneOverlay::Update(QUrhoInput *input, float timeStep) {
        float m_updatePeriod = 0.02f;
        m_signalTime += timeStep;
        m_updateDelta += timeStep;

        if(m_updateDelta >= m_updatePeriod)
        {
            {
                if(m_signalTime <= m_updateTime * 0.2){
                    SignalCalculate(m_pinger_0, m_signalTime, 1, 0.0f);
                }
                if((m_signalTime >= m_updateTime * 0.2) && (m_signalTime <= m_updateTime * 0.4)){
                    SignalCalculate(m_pinger_1, m_signalTime, 2, m_updateTime * 0.2f);
                }
                if((m_signalTime >= m_updateTime * 0.4) && (m_signalTime <= m_updateTime * 0.6)){
                    SignalCalculate(m_pinger_2, m_signalTime, 3, m_updateTime * 0.4f);
                }
                if((m_signalTime >= m_updateTime * 0.6) && (m_signalTime <= m_updateTime * 0.8)){
                    SignalCalculate(m_pinger_3, m_signalTime, 4, m_updateTime * 0.6f);
                }
                if(m_signalTime >= m_updateTime * 0.8){
                    SignalCalculate(m_pinger_4, m_signalTime, 5, m_updateTime * 0.8f);
                }
                if(m_signalTime >= m_updateTime){
                    m_signalTime = 0.0f;
                }
            }
            m_updateDelta = 0.0f;
        }
}

    void HydrophoneOverlay::SignalCalculate(Urho3D::Node *pinger, float signal_time, uint8_t signal_value, float val){
        uint8_t value[3];
        uint16_t distance[3];
        auto speed = m_speed;
        auto signal_duration = m_signalDuration;

        Hydrophones hydrophones;
        {
            auto[auv_length, length_0, length_1, length_2] = CalculateLength(pinger);
            float length[3] = {length_0, length_1, length_2};

            if(auv_length !=0){

                for(int i = 0; i < 3; i++){
                    float start_time = ((length[i] + 0.2f) - auv_length) * (10.0f - speed) + val;

                    if(signal_time >= start_time + signal_duration) {
                        distance[i] = 0;
                        value[i] = 0;
                    }
                    else if(signal_time >= start_time) {
                        distance[i] = length[i] * 100;
                        value[i] = signal_value;
                    }
                    else {
                        distance[i] = 0;
                        value[i] = 0;
                    }
                }

                hydrophones.signal_tr = value[0];
                hydrophones.signal_tl = value[1];
                hydrophones.signal_fr = value[2];
                hydrophones.distance_tr = distance[0];
                hydrophones.distance_tl = distance[1];
                hydrophones.distance_fr = distance[2];
            }
        }
        m_urhoScene->GetNetworkOverlay()->SetHydrophones(hydrophones);
    }

    void HydrophoneOverlay::CreateHydrophones() {
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
                if (str.Contains("Pinger_4")) {
                    m_pinger_4 = item;
                }
                if (str.Contains("AUV")) {
                    m_auv = item;
                }
                if (str.Contains("Thruster_TR")) {
                    m_topRightThruster = item;
                }
                if (str.Contains("Thruster_TL")) {
                    m_topLeftThruster = item;
                }
                if (str.Contains("Thruster_FR")) {
                    m_forwardRightThruster = item;
                }
            }
        }

    std::tuple<float, float, float, float> HydrophoneOverlay::CalculateLength(Urho3D::Node *pinger) {
             if (!pinger) {
                 return {0, 0, 0, 0};
             }
             auto auv_pos = m_auv->GetWorldPosition();
             auto pinger_pos = pinger->GetWorldPosition();
             auto top_right_pos = m_topRightThruster->GetWorldPosition();
             auto top_left_pos = m_topLeftThruster->GetWorldPosition();
             auto forward_right_pos = m_forwardRightThruster->GetWorldPosition();

             auto offset_top_right = pinger_pos - top_right_pos;
             auto offset_top_left = pinger_pos - top_left_pos;
             auto offset_forward_right = pinger_pos - forward_right_pos;
             auto offset_auv = pinger_pos - auv_pos;

             auto length_0 = offset_top_right.Length();
             auto length_1 = offset_top_left.Length();
             auto length_2 = offset_forward_right.Length();
             auto auv_length = offset_auv.Length();

             return {auv_length, length_0, length_1, length_2};
          }

    void HydrophoneOverlay::SetUpdateTime(float value) {
        m_updateTime = value;
        }

    void HydrophoneOverlay::SetSignal(float value){
        m_signalDuration = value;
    }

    void HydrophoneOverlay::SetSpeed(float value){
        m_speed = value;
    }
}
