#pragma once

#include <QObject>
#include <QVector>

struct MineCell {
    MineCell() = default;
    bool isMine{ false };
    bool isRevealed{ false };
    bool isFlagged{ false };
    int adjacentMines{ 0 };
};

class MinesweeperModel final : public QObject {
    Q_OBJECT

public:
    explicit MinesweeperModel(QObject* parent = nullptr);
    ~MinesweeperModel() override = default;

    // Getters
    int rows() const { return kRows; }
    int cols() const { return kCols; }
    int totalMines() const { return kMines; }
    const QVector<QVector<MineCell>>& board() const { return board_; }
    bool isGameOver() const { return isGameOver_; }
    bool isWin() const { return isWin_; }
    int flagsUsed() const { return flagsUsed_; }

    // Actions
    void reset();
    void revealCell(int r, int c);
    void toggleFlag(int r, int c);

signals:
    void updated();
    void gameOver(bool win);
    void flagsChanged(int count);

private:
    void generateMines(int excludeR, int excludeC);
    int countMines(int r, int c) const;
    void checkWin();
    void explodeMine(int r, int c);

    static constexpr int kRows = 10;
    static constexpr int kCols = 10;
    static constexpr int kMines = 12;

    QVector<QVector<MineCell>> board_;
    bool isGameOver_{ false };
    bool isWin_{ false };
    bool isFirstClick_{ true };
    int flagsUsed_{ 0 };
};
