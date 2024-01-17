#include "ShutdownDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QProcess>

ShutdownDialog::ShutdownDialog(QWidget* parent) : QDialog(parent), timeEdit(new QLineEdit(this)) {
    // ���öԻ�������
    this->setWindowTitle("ShutdownTimerSet");
    this->setFixedSize(300, 200);

    // �������ֺͿؼ�
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout();
    QLabel* promptLabel = new QLabel("Enter seconds", this);
    QPushButton* confirmButton = new QPushButton("confirm", this);
    QPushButton* cancelButton = new QPushButton("cancel", this);
    QPushButton* exitButton = new QPushButton("EXIT", this);

    // ���ò���
    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(timeEdit);
    layout->addLayout(inputLayout);
    layout->addWidget(confirmButton);
    layout->addWidget(cancelButton);
    layout->addWidget(exitButton);

    // ���Ӱ�ť�¼�
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
        reply = QMessageBox::question(this, "ȷ��",
            QString("Shut down in %1 seconds ? ").arg(seconds),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            // ִ�йػ�����
            QString command = "shutdown -s -t " + QString::number(seconds);
            QProcess::execute(command);
        }
    }
}

void ShutdownDialog::reject() {
    QProcess::execute("shutdown -a");
}


