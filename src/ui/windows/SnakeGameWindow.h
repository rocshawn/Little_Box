#pragma once

#include <QMainWindow>
#include <QList>
#include <QPoint>
#include <QTimer>

class SnakeGameWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit SnakeGameWindow(QWidget* parent = nullptr);
    ~SnakeGameWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum class Direction { Up, Down, Left, Right };

    void setupUi();
    void resetGame();
    void spawnFood();
    void moveSnake();
    void checkCollision();
    void updateGame();

    Direction currentDir_{ Direction::Right };
    Direction nextDir_{ Direction::Right };
    QList<QPoint> snake_;
    QPoint food_;
    int score_{ 0 };
    bool isGameOver_{ false };
    bool canUpdateDir_{ true };

    QTimer* timer_{ nullptr };
    
    static constexpr int kGridSize = 20;
    static constexpr int kCellSize = 22;
};
