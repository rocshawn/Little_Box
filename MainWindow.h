#pragma once

#include <QMainWindow>
#include <QPointer>

class QPushButton;
class MazeGameWindow;
class FlappyBirdWindow;
class ReactionTestWindow;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void openShutdownDialog();
    void openWeddingAdminPage();
    void openWebsiteInputDialog();
    void openMazeGame();
    void openFlappyBirdGame();
    void openReactionTest();
    void toggleFullscreen();

private:
    void setupUi();
    void setupConnections();
    void setupStatusBar();
    void updateWindowModeUi();

    QPushButton* shutdownButton_{ nullptr };
    QPushButton* weddingAdminButton_{ nullptr };
    QPushButton* websiteSwitchButton_{ nullptr };
    QPushButton* mazeGameButton_{ nullptr };
    QPushButton* flappyBirdButton_{ nullptr };
    QPushButton* reactionTestButton_{ nullptr };
    QPushButton* fullscreenButton_{ nullptr };
    QPointer<MazeGameWindow> mazeWindow_{ nullptr };
    QPointer<FlappyBirdWindow> flappyBirdWindow_{ nullptr };
    QPointer<ReactionTestWindow> reactionTestWindow_{ nullptr };
};
