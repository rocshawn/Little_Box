#pragma once

#include <QObject>
#include <QList>
#include <QPoint>
#include <QJsonObject>
#include "../core/ModuleInterfaces.h"

class SnakeGameModel final : public QObject, public IStatefulModule, public IPausableModule {
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
    // IStatefulModule
    QJsonObject saveSession() const override;
    void restoreSession(const QJsonObject& data) override;
    QJsonObject saveHistory() const override;
    void restoreHistory(const QJsonObject& data) override;

    // IPausableModule
    void pause() override {}
    void resume() override {}

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
