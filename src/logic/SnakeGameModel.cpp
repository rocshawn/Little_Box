#include "SnakeGameModel.h"

#include <QRandomGenerator>

SnakeGameModel::SnakeGameModel(int gridSize, QObject* parent)
    : QObject(parent), gridSize_(gridSize) {
    reset();
}

void SnakeGameModel::reset() {
    snake_.clear();
    // Default starting position
    snake_.append({ gridSize_ / 2, gridSize_ / 2 });
    snake_.append({ gridSize_ / 2 - 1, gridSize_ / 2 });
    snake_.append({ gridSize_ / 2 - 2, gridSize_ / 2 });

    currentDir_ = Direction::Right;
    nextDir_ = Direction::Right;
    score_ = 0;
    isGameOver_ = false;
    canUpdateDir_ = true;

    spawnFood();
    emit scoreChanged(score_);
    emit updated();
}

void SnakeGameModel::spawnFood() {
    while (true) {
        int x = QRandomGenerator::global()->bounded(gridSize_);
        int y = QRandomGenerator::global()->bounded(gridSize_);
        
        bool collision = false;
        for (const auto& part : snake_) {
            if (part == QPoint(x, y)) {
                collision = true;
                break;
            }
        }
        
        if (!collision) {
            food_ = { x, y };
            break;
        }
    }
}

void SnakeGameModel::setDirection(Direction dir) {
    if (!canUpdateDir_) return;

    // Disallow 180 degree turns
    if (dir == Direction::Up && currentDir_ != Direction::Down) nextDir_ = dir;
    else if (dir == Direction::Down && currentDir_ != Direction::Up) nextDir_ = dir;
    else if (dir == Direction::Left && currentDir_ != Direction::Right) nextDir_ = dir;
    else if (dir == Direction::Right && currentDir_ != Direction::Left) nextDir_ = dir;

    if (nextDir_ != currentDir_) {
        canUpdateDir_ = false;
    }
}

void SnakeGameModel::update() {
    if (isGameOver_) return;

    currentDir_ = nextDir_;
    canUpdateDir_ = true;

    QPoint head = snake_.front();
    switch (currentDir_) {
        case Direction::Up:    head.rx()--; break;
        case Direction::Down:  head.rx()++; break;
        case Direction::Left:  head.ry()--; break;
        case Direction::Right: head.ry()++; break;
    }

    // Check collisions
    if (checkCollision(head)) {
        isGameOver_ = true;
        emit gameOver();
        emit updated();
        return;
    }

    snake_.push_front(head);

    if (head == food_) {
        score_ += 10;
        spawnFood();
        emit scoreChanged(score_);
    } else {
        snake_.pop_back();
    }

    emit updated();
}

bool SnakeGameModel::checkCollision(const QPoint& head) const {
    // Wall collision
    if (head.x() < 0 || head.x() >= gridSize_ || head.y() < 0 || head.y() >= gridSize_) {
        return true;
    }

    // Self collision
    for (const auto& part : snake_) {
        if (part == head) return true;
    }

    return false;
}
