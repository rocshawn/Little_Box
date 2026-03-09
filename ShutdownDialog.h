#pragma once

#include <QDialog>

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

    QLineEdit* secondsEdit_{ nullptr };
    QPushButton* confirmButton_{ nullptr };
    QPushButton* cancelShutdownButton_{ nullptr };
    QPushButton* closeButton_{ nullptr };
};
