#pragma once

#include <QObject>
#include <QList>
#include <QPoint>

class SnakeGameModel final : public QObject {
    Q_OBJECT

public:
    enum class Direction { Up, Down, Left, Right };

    explicit SnakeGameModel(int gridSize = 20, QObject* parent = nullptr);
    ~SnakeGameModel() override = default;

    // Getters
    const QList<QPoint>& snake() const { return snake_; }
    QPoint food() const { return food_; }
    int score() const { return score_; }
    bool isGameOver() const { return isGameOver_; }
    int gridSize() const { return gridSize_; }

    // Actions
    void setDirection(Direction dir);
    void update();
    void reset();

signals:
    void updated();
    void gameOver();
    void scoreChanged(int newScore);

private:
    void spawnFood();
    bool checkCollision(const QPoint& head) const;

    int gridSize_;
    QList<QPoint> snake_;
    QPoint food_;
    Direction currentDir_{ Direction::Right };
    Direction nextDir_{ Direction::Right };
    int score_{ 0 };
    bool isGameOver_{ false };
    bool canUpdateDir_{ true };
};
