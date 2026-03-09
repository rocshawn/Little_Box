#include "ShutdownDialog.h"

#include "SystemCommandService.h"

#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr int kDialogWidth = 360;
constexpr int kDialogHeight = 190;
constexpr int kMaxSeconds = 315360000; // 10 years
} // namespace

ShutdownDialog::ShutdownDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
    setupConnections();
}

void ShutdownDialog::setupUi() {
    setWindowTitle("Shutdown Scheduler");
    setFixedSize(kDialogWidth, kDialogHeight);

    auto* mainLayout = new QVBoxLayout(this);
    auto* inputLayout = new QHBoxLayout();
    auto* buttonLayout = new QHBoxLayout();

    auto* promptLabel = new QLabel("Seconds until shutdown:", this);
    secondsEdit_ = new QLineEdit(this);
    secondsEdit_->setPlaceholderText("e.g. 300");
    secondsEdit_->setValidator(new QIntValidator(1, kMaxSeconds, secondsEdit_));

    confirmButton_ = new QPushButton("Schedule", this);
    cancelShutdownButton_ = new QPushButton("Cancel Schedule", this);
    closeButton_ = new QPushButton("Close", this);

    inputLayout->addWidget(promptLabel);
    inputLayout->addWidget(secondsEdit_);

    buttonLayout->addWidget(confirmButton_);
    buttonLayout->addWidget(cancelShutdownButton_);
    buttonLayout->addWidget(closeButton_);

    mainLayout->addLayout(inputLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
}

void ShutdownDialog::setupConnections() {
    connect(confirmButton_, &QPushButton::clicked, this, &ShutdownDialog::handleConfirmClicked);
    connect(cancelShutdownButton_, &QPushButton::clicked, this, &ShutdownDialog::handleCancelShutdownClicked);
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
}

void ShutdownDialog::handleConfirmClicked() {
    bool ok = false;
    const int seconds = secondsEdit_->text().toInt(&ok);
    if (!ok || seconds <= 0) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a positive integer number of seconds.");
        return;
    }

    const auto result = QMessageBox::question(
        this,
        "Confirm Shutdown",
        QString("The system will shut down in %1 seconds. Continue?").arg(seconds),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    if (!SystemCommandService::scheduleShutdown(seconds)) {
        QMessageBox::critical(this, "Command Failed", "Unable to schedule shutdown. Please check system permissions.");
        return;
    }

    QMessageBox::information(this, "Scheduled", "Shutdown schedule created successfully.");
}

void ShutdownDialog::handleCancelShutdownClicked() {
    if (!SystemCommandService::cancelShutdown()) {
        QMessageBox::warning(this, "Cancel Failed", "No active shutdown schedule found or command execution failed.");
        return;
    }

    QMessageBox::information(this, "Cancelled", "Shutdown schedule has been cancelled.");
}
