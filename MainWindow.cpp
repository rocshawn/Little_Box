#include "MainWindow.h"
#include "ShutdownDialog.h"
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // ��������������
    this->setWindowTitle("ShutdownTimer");
    this->setFixedSize(400, 300);

    // ������ť����ӵ�������
    QPushButton* shutdownButton = new QPushButton("ShutdownTimer", this);
    shutdownButton->setGeometry(150, 130, 100, 30);

    // ���Ӱ�ť�ĵ���¼����ۺ���
    connect(shutdownButton, &QPushButton::clicked, this, &MainWindow::openShutdownDialog);
}

MainWindow::~MainWindow() {
}

void MainWindow::openShutdownDialog() {
    // �򿪶�ʱ�ػ��Ի���
    ShutdownDialog* shutdownDialog = new ShutdownDialog(this);
    shutdownDialog->show();
}
