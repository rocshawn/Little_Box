#include "MainWindow.h"
#include "ShutdownDialog.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // 设置主窗口属性
    this->setWindowTitle("ShutdownTimer");
    this->setFixedSize(400, 300);

    // 创建按钮并添加到主窗口
    QPushButton* shutdownButton = new QPushButton("ShutdownTimer", this);
    shutdownButton->setGeometry(150, 130, 100, 30);

    // 连接按钮的点击事件到槽函数
    connect(shutdownButton, &QPushButton::clicked, this, &MainWindow::openShutdownDialog);
}

MainWindow::~MainWindow() {
}

void MainWindow::openShutdownDialog() {
    // 打开定时关机对话框
    ShutdownDialog* shutdownDialog = new ShutdownDialog(this);
    shutdownDialog->show();
}
