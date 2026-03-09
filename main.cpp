#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Little Box");
    QApplication::setOrganizationName("LittleBox");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
