#pragma once
#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QDateTime>
#include <QFile>
#include <QRegularExpression>
#include "../Core/QUrhoScene.h"
#include "../Overlay/ScriptsOverlay.h"
#include "../Ui/ApplicationWindowWeb.h"
#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>

class ProcessingVideo : public QObject
{
    Q_OBJECT
public:
    explicit ProcessingVideo(QUrho::QUrhoScene *scene, QObject *parent = nullptr);

    ~ProcessingVideo() override;

private:
    cv::Mat TextOnImage(QString lua_output);

    cv::Mat ProcessImage(QString lua_output);

    void ProcessVideo(QString lua_output);

    void readCpuInfo();
    void readMemoryInfo();

    QUrho::QUrhoScene *m_scene;
    QUrho::ApplicationWindowWeb *mainWindow;

    cv::VideoWriter video;
    uint frames_count = 0;
    uint step = 16;
    uint fontHeight = 14;
    uint thickness = -1;
    uint linestyle = cv::LINE_4;
    uint left_padding = 15;
    uint top_padding = 5;
    uint video_width = 640;
    uint video_height = 480;

    QString cpu_avg_load = "0";
    QString memory_avg_load = "0";
    QTimer *timer;

    cv::Ptr<cv::freetype::FreeType2> ft2;
};
