#pragma once

#include <QDateTime>
#include <QDialog>

class QLineEdit;
class QDateTimeEdit;
class QTimer;
class QPushButton;

class ShutdownDialog final : public QDialog {
    Q_OBJECT

public:
    explicit ShutdownDialog(QWidget* parent = nullptr);
    ~ShutdownDialog() override = default;

private slots:
    void handleConfirmClicked();
    void handleCancelShutdownClicked();
    void handleWatchTimer();

private:
    void setupUi();
    void setupConnections();

    QLineEdit* secondsEdit_{ nullptr };
    QDateTimeEdit* dateTimeEdit_{ nullptr };
    QPushButton* confirmButton_{ nullptr };
    QPushButton* cancelShutdownButton_{ nullptr };
    QPushButton* closeButton_{ nullptr };
    QTimer* watchTimer_{ nullptr };
    QDateTime scheduledTime_;
};
