/*
 * Created by Vladislav Bolotov on 10/7/2019. <vladislav.bolotov@gmail.com>
*/

#pragma once

namespace QUrho {
    class QUrhoInput;

    class QSceneOverlay {
    public:
        virtual void Update(QUrhoInput *input, float timeStep) = 0;
    };
}

