/*
 * Created by Vladislav Bolotov on 10/29/2019. <vladislav.bolotov@gmail.com>
*/

#include "ValueSlider.h"

#include <QStyleOptionSlider>
#include <QToolTip>

namespace QUrho {

    ValueSlider::ValueSlider(QWidget *parent) : QSlider(parent) {

    }

    ValueSlider::ValueSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent) {

    }

    void ValueSlider::setDivider(uint value) {
        if (value != 0) {
            divider = static_cast<double>(value);
            decimals = QString::number(value).length() - 1;
        }
    }

    void ValueSlider::sliderChange(QAbstractSlider::SliderChange change) {
        QSlider::sliderChange(change);

        if (change == QAbstractSlider::SliderValueChange) {
            QStyleOptionSlider opt;
            initStyleOption(&opt);

            QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
            QPoint bottomRightCorner = sr.bottomLeft();

            if (divider == 1.0) {
                QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())),
                                   QString::number(value()), this);
            } else {
                QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())),
                                   QString::number(static_cast<double>(value()) / divider, 'f', decimals), this);
            }
        }
    }

    void ValueSlider::mousePressEvent(QMouseEvent *event) {
        QSlider::mousePressEvent(event);

        QStyleOptionSlider opt;
        initStyleOption(&opt);

        QRect sr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
        QPoint bottomRightCorner = sr.bottomLeft();

        if (divider == 1.0) {
            QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())),
                               QString::number(value()), this);
        } else {
            QToolTip::showText(mapToGlobal(QPoint(bottomRightCorner.x(), bottomRightCorner.y())),
                               QString::number(static_cast<double>(value()) / divider, 'f', decimals), this);
        }
    }
}