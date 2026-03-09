#include "MainWindow.h"

#include "ShutdownDialog.h"

#include <QPushButton>

namespace {
constexpr int kMainWindowWidth = 420;
constexpr int kMainWindowHeight = 280;
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
    setupConnections();
}

void MainWindow::setupUi() {
    setWindowTitle("Shutdown Timer");
    setFixedSize(kMainWindowWidth, kMainWindowHeight);

    shutdownButton_ = new QPushButton("Open Shutdown Scheduler", this);
    shutdownButton_->setGeometry(120, 120, 180, 36);
}

void MainWindow::setupConnections() {
    connect(shutdownButton_, &QPushButton::clicked, this, &MainWindow::openShutdownDialog);
}

void MainWindow::openShutdownDialog() {
    ShutdownDialog shutdownDialog(this);
    shutdownDialog.exec();
}
