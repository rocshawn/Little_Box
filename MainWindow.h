#pragma once

#include <QMainWindow>

class QPushButton;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void openShutdownDialog();

private:
    void setupUi();
    void setupConnections();

    QPushButton* shutdownButton_{ nullptr };
};
