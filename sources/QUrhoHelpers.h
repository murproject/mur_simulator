#include <QString>
#include <Urho3D/Container/Str.h>
#include <Urho3D/Input/Input.h>
#include <opencv2/opencv.hpp>
#pragma once

namespace QUrho {
    inline Urho3D::String QtUrhoStringCast(const QString &string) { return string.toStdString().c_str(); }

    inline QString UrhoQtStringCast(const Urho3D::String &string) { return string.CString(); }

    inline Qt::MouseButton UrhoQtMouseCast(int button) {
        switch (button) {
            case Urho3D::MOUSEB_LEFT:
                return Qt::LeftButton;
            case Urho3D::MOUSEB_RIGHT:
                return Qt::RightButton;
            case Urho3D::MOUSEB_MIDDLE:
                return Qt::MiddleButton;
            case Urho3D::MOUSEB_X1:
                return Qt::XButton1;
            case Urho3D::MOUSEB_X2:
                return Qt::XButton2;
            default:
                return Qt::NoButton;
        }
    }

    template <typename T>
    inline cv::Mat CreateMat(T* data, int rows, int cols, int chs = 1) {
        cv::Mat mat(rows, cols, CV_MAKETYPE(cv::DataType<T>::type, chs));
        memcpy(mat.data, data, rows*cols*chs * sizeof(T));
        return std::move(mat);
    }
}