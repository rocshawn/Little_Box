#include <QApplication>
//#include <QTextCodec>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    // ���ñ���
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
