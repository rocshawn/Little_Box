#include "MinesweeperModel.h"
#include <QRandomGenerator>
#include <QJsonArray>
#include <cmath>

MinesweeperModel::MinesweeperModel(QObject* parent)
    : QObject(parent),
      board_(kRows, QVector<MineCell>(kCols)) {
    reset();
}

void MinesweeperModel::reset() {
    board_ = QVector<QVector<MineCell>>(kRows, QVector<MineCell>(kCols));
    isGameOver_ = false;
    isWin_ = false;
    isFirstClick_ = true;
    flagsUsed_ = 0;
    emit updated();
    emit flagsChanged(flagsUsed_);
}

void MinesweeperModel::revealCell(int r, int c) {
    if (isGameOver_ || isPaused_) return;
    if (r < 0 || r >= kRows || c < 0 || c >= kCols) return;
    if (board_[r][c].isRevealed || board_[r][c].isFlagged) return;

    if (isFirstClick_) {
        generateMines(r, c);
        isFirstClick_ = false;
    }

    board_[r][c].isRevealed = true;
    if (board_[r][c].isMine) {
        explodeMine(r, c);
        return;
    }

    if (board_[r][c].adjacentMines == 0) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                revealCell(r + dr, c + dc);
            }
        }
    }
    checkWin();
    emit updated();
}

void MinesweeperModel::toggleFlag(int r, int c) {
    if (isGameOver_ || isFirstClick_ || isPaused_) return;
    if (r < 0 || r >= kRows || c < 0 || c >= kCols) return;
    if (board_[r][c].isRevealed) return;

    board_[r][c].isFlagged = !board_[r][c].isFlagged;
    flagsUsed_ += board_[r][c].isFlagged ? 1 : -1;
    emit flagsChanged(flagsUsed_);
    emit updated();
}

void MinesweeperModel::generateMines(int excludeR, int excludeC) {
    int m = 0;
    while (m < kMines) {
        int r = QRandomGenerator::global()->bounded(kRows);
        int c = QRandomGenerator::global()->bounded(kCols);
        
        if (std::abs(r - excludeR) <= 1 && std::abs(c - excludeC) <= 1) continue;

        if (!board_[r][c].isMine) {
            board_[r][c].isMine = true;
            m++;
        }
    }

    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (!board_[r][c].isMine) {
                board_[r][c].adjacentMines = countMines(r, c);
            }
        }
    }
}

int MinesweeperModel::countMines(int r, int c) const {
    int n = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < kRows && nc >= 0 && nc < kCols && board_[nr][nc].isMine) n++;
        }
    }
    return n;
}

void MinesweeperModel::explodeMine(int r, int c) {
    isGameOver_ = true;
    for (int i = 0; i < kRows; ++i) {
        for (int j = 0; j < kCols; ++j) {
            if (board_[i][j].isMine) board_[i][j].isRevealed = true;
        }
    }
    emit updated();
    emit gameOver(false);
}

void MinesweeperModel::checkWin() {
    int revealedCount = 0;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (board_[r][c].isRevealed && !board_[r][c].isMine) revealedCount++;
        }
    }
    if (revealedCount == (kRows * kCols - kMines)) {
        isWin_ = true;
        isGameOver_ = true;
        emit updated();
        emit gameOver(true);
    }
}

QJsonObject MinesweeperModel::saveSession() const {
    QJsonObject obj;
    obj["isGameOver"] = isGameOver_;
    obj["isWin"] = isWin_;
    obj["isFirstClick"] = isFirstClick_;
    obj["flagsUsed"] = flagsUsed_;
    
    QJsonArray boardArr;
    for (const auto& row : board_) {
        QJsonArray rowArr;
        for (const auto& cell : row) {
            QJsonObject cellObj;
            cellObj["isMine"] = cell.isMine;
            cellObj["isRevealed"] = cell.isRevealed;
            cellObj["isFlagged"] = cell.isFlagged;
            cellObj["adjacentMines"] = cell.adjacentMines;
            rowArr.append(cellObj);
        }
        boardArr.append(rowArr);
    }
    obj["board"] = boardArr;
    return obj;
}

void MinesweeperModel::restoreSession(const QJsonObject& data) {
    if (data.contains("isGameOver")) isGameOver_ = data["isGameOver"].toBool();
    if (data.contains("isWin")) isWin_ = data["isWin"].toBool();
    if (data.contains("isFirstClick")) isFirstClick_ = data["isFirstClick"].toBool();
    if (data.contains("flagsUsed")) flagsUsed_ = data["flagsUsed"].toInt();
    
    if (data.contains("board")) {
        QJsonArray boardArr = data["board"].toArray();
        for (int r = 0; r < boardArr.size() && r < kRows; ++r) {
            QJsonArray rowArr = boardArr[r].toArray();
            for (int c = 0; c < rowArr.size() && c < kCols; ++c) {
                QJsonObject cellObj = rowArr[c].toObject();
                board_[r][c].isMine = cellObj["isMine"].toBool();
                board_[r][c].isRevealed = cellObj["isRevealed"].toBool();
                board_[r][c].isFlagged = cellObj["isFlagged"].toBool();
                board_[r][c].adjacentMines = cellObj["adjacentMines"].toInt();
            }
        }
    }
    emit flagsChanged(flagsUsed_);
    emit updated();
}

QJsonObject MinesweeperModel::saveHistory() const {
    // Record fastest time for Minesweeper in the future? Nothing for now.
    return QJsonObject();
}

void MinesweeperModel::restoreHistory(const QJsonObject& data) {
}
