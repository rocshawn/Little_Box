#pragma once

#include <QMainWindow>
#include <QVector>

class QGridLayout;
class QLabel;

class Game2048Window final : public QMainWindow {
    Q_OBJECT

public:
    explicit Game2048Window(QWidget* parent = nullptr);
    ~Game2048Window() override = default;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    enum class Direction { Up, Down, Left, Right };

    void setupUi();
    void resetGame();
    void spawnRandomTile();
    void updateUi();
    QString colorForNumber(int num, bool darkTheme) const;

    bool moveBoard(Direction dir);
    bool slideArrayLeft(QVector<int>& line, int& scoreIncrease);
    void rotateBoardClockwise();
    bool isGameOver() const;

    QVector<QVector<int>> board_;
    int score_{ 0 };

    QLabel* scoreLabel_{ nullptr };
    QLabel* statusLabel_{ nullptr };
    QGridLayout* gridLayout_{ nullptr };
    QVector<QLabel*> tiles_;
};
