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
    void openWeddingAdminPage();

private:
    void setupUi();
    void setupConnections();
    void setupStatusBar();

    QPushButton* shutdownButton_{ nullptr };
    QPushButton* weddingAdminButton_{ nullptr };
};
