/*
 * Created by Vladislav Bolotov on 10/29/2019. <vladislav.bolotov@gmail.com>
*/

#pragma once

#include <QSlider>
#include <QObject>
#include <QAbstractSlider>

namespace QUrho {
    class ValueSlider : public QSlider {
    Q_OBJECT
    public:
        explicit ValueSlider(QWidget *parent = 0);

        explicit ValueSlider(Qt::Orientation orientation, QWidget *parent = 0);

    protected:
        void sliderChange(SliderChange change) override;

        void mousePressEvent(QMouseEvent *event) override;

    };
}




