#include "ProcessingVideo.h"
#include "../Overlay/AUVOverlay.h"
#include <opencv2/opencv.hpp>
#include <opencv2/freetype.hpp>

ProcessingVideo::ProcessingVideo(QUrho::QUrhoScene *scene, QObject *parent) : QObject(parent) {
    m_scene = scene;
    QObject::connect(m_scene->GetScriptsOverlay(), &QUrho::ScriptsOverlay::scriptRunning, this, &ProcessingVideo::ProcessVideo);

    QString font_path = mainWindow->LoadPaths("font", "/usr/share/simulator/font/TerminusTTF-4.49.3.ttf");
    ft2 = cv::freetype::createFreeType2();
    ft2->loadFontData(font_path.toStdString(), 0);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ProcessingVideo::readCpuInfo);
    connect(timer, &QTimer::timeout, this, &ProcessingVideo::readMemoryInfo);
    timer->start(1000);

    qDebug() << "ProcessingVideo - started";
}

ProcessingVideo::~ProcessingVideo() {
    qDebug() << "ProcessingVideo - stopped";
    if (video.isOpened()) {
        video.release();
    }
}

cv::Mat ProcessingVideo::TextOnImage(QString lua_output) {
    cv::Mat img(cv::Size(320, 240), CV_8UC3, cv::Scalar(60, 60, 60));

    ft2->putText(img,
            "Загрузка процессора: " + cpu_avg_load.toStdString() + "%",
            cv::Point(left_padding, top_padding + step),
            fontHeight,
            cv::Scalar::all(255),
            thickness,
            linestyle,
            true);

    ft2->putText(img,
            "Загрузка памяти: " + memory_avg_load.toStdString() + "%",
            cv::Point(left_padding, top_padding + step * 2),
            fontHeight,
            cv::Scalar::all(255),
            thickness,
            linestyle,
            true);

    QStringList text = lua_output.split("\n\n\n");

    ft2->putText(img,
            "Баллы: " + text[0].toStdString() + " Время: "  + text[1].toStdString(),
            cv::Point(left_padding, top_padding + step * 3),
            fontHeight,
            cv::Scalar::all(255),
            thickness,
            linestyle,
            true);

    QStringList log = text[2].split("\n");

    if (log.length() > 0) {
        for (int i = 0; i < log.length(); i++){
            ft2->putText(img, log[i].toStdString(),
            cv::Point(left_padding, top_padding + (i * step) + (step * 4)),
            fontHeight,
            cv::Scalar::all(255),
            thickness,
            linestyle,
            true);
        }
    }
    return img;
}

cv::Mat ProcessingVideo::ProcessImage(QString lua_info) {
    cv::Mat info_img = TextOnImage(lua_info);
    auto[front_img, bottom_img, static_img] = m_scene->GetAUVOverlay()->GetCameraImages();
    cv::Mat result, first_col, second_col;

    std::vector<cv::Mat> col_one = {
        front_img,
        bottom_img
    };

    cv::vconcat(col_one, first_col);

    std::vector<cv::Mat> col_two = {
        static_img,
        info_img
    };

    cv::vconcat(col_two, second_col);

    std::vector<cv::Mat> row = {
        first_col,
        second_col
    };
    cv::hconcat(row, result);

    return result;
}

void ProcessingVideo::ProcessVideo(QString lua_info) {
    static bool initialized = false;

    cv::Mat frame = ProcessImage(lua_info);
    if (frame.empty()) return;

    if (!initialized) {
        QString path = mainWindow->LoadPaths("video", "/var/tmp/simulator/video.mkv");
        QString video_fps = mainWindow->LoadPaths("video_fps", "5");
        QString video_bit = mainWindow->LoadPaths("video_bit", "0");

        QString pipeline = QString(
            "appsrc ! "
            "videoconvert ! "
            "video/x-raw,format=NV12,width=%1,height=%2,framerate=%3/1 ! "
            "nvh264enc bitrate=%4 ! "
            "h264parse ! "
            "matroskamux ! "
            "filesink location=%5 sync=true")
            .arg(video_width)
            .arg(video_height)
            .arg(video_fps)
            .arg(video_bit)
            .arg(path);

        video.open(
            pipeline.toStdString(), 0, video_fps.toDouble(), cv::Size(video_width, video_height), true
        );

        if (!video.isOpened()) {
            qDebug() << "Ошибка: не удалось открыть VideoWriter с GStreamer NVENC";
            return;
        }

        initialized = true;
    }

    if (frame.size() != cv::Size(video_width, video_height))
        cv::resize(frame, frame, cv::Size(video_width, video_height));

    video.write(frame);
}


void ProcessingVideo::readCpuInfo() {
    static const QRegularExpression regex("\\s+");
    static unsigned int prev_user = 0, prev_total = 0;

    QString path = "/proc/stat";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "CPU: Не удалось открыть" << path;
        return;
    }

    QString line = QTextStream(&file).readLine();
    file.close();

    QStringList parts = line.split(regex, Qt::SkipEmptyParts);
    if (parts.size() < 5) {
        qWarning() << "CPU: Неизвестный формат данных" << path;
        return;
    }

    unsigned int total_now = 0, user_now = 0;
    for (int i = 0; i < parts.size(); ++i) {
        bool ok;
        unsigned int value = parts[i].toUInt(&ok);
        if (ok) {
            total_now += value;
            if (i < 4) user_now += value;
        }
    }

    unsigned int total_diff = total_now - prev_total;
    unsigned int user_diff = user_now - prev_user;

    if (total_diff > 0) {
        double cpu_usage = (static_cast<double>(user_diff) / total_diff) * 100.0;
        cpu_avg_load = QString::number(cpu_usage, 'f', 0);
    }

    prev_user = user_now;
    prev_total = total_now;
}

void ProcessingVideo::readMemoryInfo() {
    static const QRegularExpression regex("\\s+");

    QString path = "/proc/meminfo";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Memoty: Не удалось открыть" << path;
        return;
    }

    QTextStream in(&file);
    double totalMemory = 0;
    double availableMemory = 0;

    for (QString line = in.readLine(); !line.isNull(); line = in.readLine())  {

        if (line.startsWith("MemTotal:")) {
            totalMemory = line.split(regex, Qt::SkipEmptyParts)[1].toDouble();
        } else if (line.startsWith("MemAvailable:")) {
            availableMemory = line.split(regex, Qt::SkipEmptyParts)[1].toDouble();
        }

        if (totalMemory > 0 && availableMemory > 0) {
            break;
       }
    }

    file.close();

    if (totalMemory == 0) {
        qWarning() << "Memory: Невозможно определить общую память";
        return;
    }

    double usedMemory = totalMemory - availableMemory;
    double memoryUsage = (usedMemory / totalMemory) * 100.0;

    memory_avg_load = QString::number(memoryUsage, 'f', 0);
}
