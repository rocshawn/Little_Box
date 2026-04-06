#pragma once

#include <QMainWindow>
#include <QVector>
#include <QPoint>

class MinesweeperWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MinesweeperWindow(QWidget* parent = nullptr);
    ~MinesweeperWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct Cell {
        Cell() = default;
        bool isMine{ false };
        bool isRevealed{ false };
        bool isFlagged{ false };
        int adjacentMines{ 0 };
    };

    void setupUi();
    void resetGame();
    void generateMines(int excludeR, int excludeC);
    void revealCell(int r, int c);
    void explodeMine(int r, int c);
    void checkWin();
    int countMines(int r, int c) const;

    static constexpr int kRows = 10;
    static constexpr int kCols = 10;
    static constexpr int kMines = 12;
    static constexpr int kCellSize = 36;

    QVector<QVector<Cell>> board_;
    bool isGameOver_{ false };
    bool isWin_{ false };
    bool isFirstClick_{ true };
    int flagsUsed_{ 0 };
};
