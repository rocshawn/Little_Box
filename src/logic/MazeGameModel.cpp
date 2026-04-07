#include "MazeGameModel.h"

#include <QRandomGenerator>

#include <algorithm>

MazeGameModel::MazeGameModel(QObject* parent)
    : QObject(parent), level_{ 1 } {
    generateLevel();
}

void MazeGameModel::setLevel(int level) {
    level_ = std::max(1, std::min(level, kTotalLevels));
    generateLevel();
    emit levelChanged(level_);
}

void MazeGameModel::restartLevel() {
    generateLevel();
    emit updated();
}

void MazeGameModel::tryMove(int dx, int dy) {
    const QPoint target = playerCell_ + QPoint(dx, dy);
    if (!isOpenCell(target)) {
        return;
    }

    playerCell_ = target;
    emit updated();

    if (playerCell_ == exitCell_) {
        emit levelCompleted();
    }
}

bool MazeGameModel::isOpenCell(const QPoint& cell) const {
    if (walls_.isEmpty() || walls_.front().isEmpty()) {
        return false;
    }

    if (cell.y() < 0 || cell.y() >= walls_.size()) {
        return false;
    }

    if (cell.x() < 0 || cell.x() >= walls_.front().size()) {
        return false;
    }

    return !walls_[cell.y()][cell.x()];
}

void MazeGameModel::generateLevel() {
    const int size = boardSizeForLevel(level_);
    walls_ = QVector<QVector<bool>>(size, QVector<bool>(size, true));

    QVector<QPoint> stack;
    playerCell_ = QPoint(1, 1);
    walls_[playerCell_.y()][playerCell_.x()] = false;
    stack.append(playerCell_);

    while (!stack.isEmpty()) {
        const QPoint current = stack.back();
        QVector<QPoint> directions{ QPoint(0, -2), QPoint(0, 2), QPoint(-2, 0), QPoint(2, 0) };

        // Shuffling directions
        for (int index = directions.size() - 1; index > 0; --index) {
            const int swapIndex = QRandomGenerator::global()->bounded(index + 1);
            directions.swapItemsAt(index, swapIndex);
        }

        bool moved = false;
        for (const QPoint& direction : directions) {
            const QPoint next = current + direction;
            if (next.x() <= 0 || next.y() <= 0 || next.x() >= size - 1 || next.y() >= size - 1) {
                continue;
            }

            if (!walls_[next.y()][next.x()]) {
                continue;
            }

            const QPoint midpoint(current.x() + direction.x() / 2, current.y() + direction.y() / 2);
            walls_[midpoint.y()][midpoint.x()] = false;
            walls_[next.y()][next.x()] = false;
            stack.append(next);
            moved = true;
            break;
        }

        if (!moved) {
            stack.removeLast();
        }
    }

    exitCell_ = QPoint(size - 2, size - 2);
    walls_[exitCell_.y()][exitCell_.x()] = false;
    emit updated();
}

int MazeGameModel::boardSizeForLevel(int level) const {
    const int safeLevel = std::max(1, level);
    return 13 + safeLevel * 2;
}
