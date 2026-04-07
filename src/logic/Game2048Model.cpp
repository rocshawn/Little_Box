#include "Game2048Model.h"
#include <QPoint>
#include <QRandomGenerator>

Game2048Model::Game2048Model(QObject* parent)
    : QObject(parent),
      board_(4, QVector<int>(4, 0)) {
    reset();
}

void Game2048Model::reset() {
    board_ = QVector<QVector<int>>(4, QVector<int>(4, 0));
    score_ = 0;
    isGameWon_ = false;

    spawnRandomTile();
    spawnRandomTile();
    emit updated();
    emit scoreChanged(score_);
}

void Game2048Model::spawnRandomTile() {
    QVector<QPoint> emptyCells;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board_[r][c] == 0) emptyCells.append(QPoint{r, c});
        }
    }
    if (emptyCells.isEmpty()) return;

    const int idx = QRandomGenerator::global()->bounded(emptyCells.size());
    const QPoint& pt = emptyCells[idx];
    // 90% chance of 2, 10% chance of 4
    board_[pt.x()][pt.y()] = (QRandomGenerator::global()->bounded(10) < 9) ? 2 : 4;
}

bool Game2048Model::move(Direction dir) {
    if (isGameOver()) return false;

    int rotations = 0;
    switch(dir) {
        case Direction::Left:  rotations = 0; break;
        case Direction::Up:    rotations = 3; break;
        case Direction::Right: rotations = 2; break;
        case Direction::Down:  rotations = 1; break;
    }

    for (int i = 0; i < rotations; ++i) rotateBoardClockwise();

    bool moved = false;
    int addedScore = 0;
    for (int r = 0; r < 4; ++r) {
        if (slideArrayLeft(board_[r], addedScore)) moved = true;
    }
    
    if (addedScore > 0) {
        score_ += addedScore;
        emit scoreChanged(score_);
    }

    for (int i = 0; i < (4 - rotations) % 4; ++i) rotateBoardClockwise();

    if (moved) {
        spawnRandomTile();
        
        // Check for Win (2048)
        if (!isGameWon_) {
            for (int r = 0; r < 4; ++r) {
                for (int c = 0; c < 4; ++c) {
                    if (board_[r][c] == 2048) {
                        isGameWon_ = true;
                        emit gameWon();
                        break;
                    }
                }
            }
        }

        emit updated();
        if (isGameOver()) emit gameOver();
    }

    return moved;
}

bool Game2048Model::slideArrayLeft(QVector<int>& line, int& scoreIncrease) {
    bool changed = false;
    QVector<int> result(4, 0);
    int head = 0;
    int lastMerged = -1;

    for (int i = 0; i < 4; ++i) {
        if (line[i] == 0) continue;
        
        if (head > 0 && result[head - head] == 0) {} // Dummy check to suppress warning in thought
        
        if (head > 0 && result[head - 1] == line[i] && lastMerged != head - 1) {
            result[head - 1] *= 2;
            scoreIncrease += result[head - 1];
            lastMerged = head - 1;
            changed = true;
        } else {
            result[head] = line[i];
            if (head != i) changed = true;
            head++;
        }
    }
    bool lineChanged = false;
    for (int i = 0; i < 4; ++i) {
        if (line[i] != result[i]) {
            lineChanged = true;
            break;
        }
    }
    line = result;
    return lineChanged;
}

void Game2048Model::rotateBoardClockwise() {
    QVector<QVector<int>> newBoard(4, QVector<int>(4, 0));
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            newBoard[c][3 - r] = board_[r][c];
        }
    }
    board_ = newBoard;
}

bool Game2048Model::isGameOver() const {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board_[r][c] == 0) return false;
            if (c < 3 && board_[r][c] == board_[r][c+1]) return false;
            if (r < 3 && board_[r][c] == board_[r+1][c]) return false;
        }
    }
    return true;
}
