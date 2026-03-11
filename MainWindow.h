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
    void openMazeGame();
    void openFlappyBirdGame();
    void toggleFullscreen();

private:
    void setupUi();
    void setupConnections();
    void setupStatusBar();
    void updateWindowModeUi();

    QPushButton* shutdownButton_{ nullptr };
    QPushButton* weddingAdminButton_{ nullptr };
    QPushButton* mazeGameButton_{ nullptr };
    QPushButton* flappyBirdButton_{ nullptr };
    QPushButton* fullscreenButton_{ nullptr };
};
