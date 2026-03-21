#pragma once

#include <QDialog>
#include <QDateTime>

class QLineEdit;
class QPushButton;

class ShutdownDialog final : public QDialog {
    Q_OBJECT

public:
    explicit ShutdownDialog(QWidget* parent = nullptr);
    ~ShutdownDialog() override = default;

private slots:
    void handleConfirmClicked();
    void handleCancelShutdownClicked();

private:
    void setupUi();
    void setupConnections();

    QLineEdit* yearEdit_{ nullptr };
    QLineEdit* monthEdit_{ nullptr };
    QLineEdit* dayEdit_{ nullptr };
    QLineEdit* hourEdit_{ nullptr };
    QLineEdit* minuteEdit_{ nullptr };
    QLineEdit* secondEdit_{ nullptr };
    QPushButton* confirmButton_{ nullptr };
    QPushButton* cancelShutdownButton_{ nullptr };
    QPushButton* closeButton_{ nullptr };
    bool hasScheduledPlan_{ false };
};
