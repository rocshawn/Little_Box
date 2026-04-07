#pragma once

#include <QObject>
#include <QVector>

class Game2048Model final : public QObject {
    Q_OBJECT

public:
    enum class Direction { Up, Down, Left, Right };

    explicit Game2048Model(QObject* parent = nullptr);
    ~Game2048Model() override = default;

    // Getters
    const QVector<QVector<int>>& board() const { return board_; }
    int score() const { return score_; }
    bool isGameOver() const;
    bool isGameWon() const { return isGameWon_; }

    // Actions
    void reset();
    bool move(Direction dir);

signals:
    void updated();
    void scoreChanged(int score);
    void gameOver();
    void gameWon();

private:
    void spawnRandomTile();
    bool slideArrayLeft(QVector<int>& line, int& scoreIncrease);
    void rotateBoardClockwise();

    QVector<QVector<int>> board_;
    int score_{ 0 };
    bool isGameWon_{ false };
};
