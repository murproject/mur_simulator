#include "Application.h"
#include "QUrhoWidget.h"
#include "ApplicationWindow.h"

#include <QApplication>
#include <QStyleFactory>

namespace QUrho {
    int Application::execute(int argc, char **argv) {


        QApplication application(argc, argv);
        application.setApplicationName("Simulator");
        QScopedPointer<ApplicationWindow> mainWindow{new ApplicationWindow};
        mainWindow->show();
        mainWindow->InitializeEngine();
        size_t size = sizeof(Urho3D::VariantMap);
        return QApplication::exec();
    }
}

