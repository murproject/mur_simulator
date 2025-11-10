#include "Application.h"
#include "Ui/ApplicationWindowWeb.h"
#include "Ui/ApplicationWindowDesktop.h"

#include <QApplication>
#include <QStyleFactory>

namespace QUrho {
    int Application::execute(int argc, char **argv) {
        QApplication application(argc, argv);
        application.setApplicationName("Simulator");

#ifdef WEB_SIM
        QScopedPointer<ApplicationWindowWeb> mainWindow{new ApplicationWindowWeb};
        mainWindow->InitializeEngine();
        QString scene_path = mainWindow->LoadPaths("scene", "/usr/share/simulator/scene.xml");
        mainWindow->OpenFile(scene_path);
#elif DESKTOP_SIM
        QScopedPointer<ApplicationWindowDesktop> mainWindow{new ApplicationWindowDesktop};
        mainWindow->show();
        mainWindow->InitializeEngine();
#endif
        return QApplication::exec();
    }
}

