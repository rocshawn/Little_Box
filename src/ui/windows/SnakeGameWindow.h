#pragma once

#include <QMainWindow>
#include <QList>
#include <QPoint>
#include <QTimer>

class SnakeGameModel;
class QLabel;
class QTimer;
class SessionOverlayWidget;

class SnakeGameWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit SnakeGameWindow(QWidget* parent = nullptr);
    ~SnakeGameWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onModelUpdated();
    void onScoreChanged(int score);
    void onGameOver();
    void showOverlay();
    void hideOverlay();

private:
    void setupUi();
    void startNewGame();

    SnakeGameModel* model_{ nullptr };
    SessionOverlayWidget* overlay_{ nullptr };
    QTimer*         timer_{ nullptr };
    QLabel*         scoreLabel_{ nullptr };
    
    static constexpr int kGridSize = 20;
    static constexpr int kCellSize = 22;
};
