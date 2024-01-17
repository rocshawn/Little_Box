#include "ShutdownDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>

ShutdownDialog::ShutdownDialog(QWidget* parent) : QDialog(parent), timeEdit(new QLineEdit(this)) {
    // 设置对话框属性
    this->setWindowTitle("ShutdownTimerSet");
    this->setFixedSize(300, 200);

    // 创建布局和控件
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* promptLabel = new QLabel("Enter seconds", this);
    QPushButton* confirmButton = new QPushButton("confirm", this);
    QPushButton* cancelButton = new QPushButton("cancel", this);
    QPushButton* exitButton = new QPushButton("EXIT", this);

    // 设置布局
    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(timeEdit);
    layout->addLayout(inputLayout);
    layout->addWidget(confirmButton);
    layout->addWidget(cancelButton);
    layout->addWidget(exitButton);

    // 连接按钮事件
    connect(confirmButton, &QPushButton::clicked, this, &ShutdownDialog::onConfirmClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ShutdownDialog::reject);
    connect(exitButton, &QPushButton::clicked, this, &ShutdownDialog::accept);
}

ShutdownDialog::~ShutdownDialog() {
}

void ShutdownDialog::onConfirmClicked() {
    bool ok;
    int seconds = timeEdit->text().toInt(&ok);
    if (ok && seconds > 0) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "确认",
            QString("Shut down in %1 seconds ? ").arg(seconds),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // 执行关机命令
            QString command = "shutdown -s -t " + QString::number(seconds);
            QProcess::execute(command);
        }
    }
}

void ShutdownDialog::reject() {
    QProcess::execute("shutdown -a");
}


