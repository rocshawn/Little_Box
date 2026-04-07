#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>

class MazeGameModel final : public QObject {
    Q_OBJECT

public:
    explicit MazeGameModel(QObject* parent = nullptr);
    ~MazeGameModel() override = default;

    // Getters
    int currentLevel() const { return level_; }
    int totalLevels() const { return kTotalLevels; }
    const QVector<QVector<bool>>& walls() const { return walls_; }
    QPoint playerCell() const { return playerCell_; }
    QPoint exitCell() const { return exitCell_; }
    int boardSize() const { return boardSizeForLevel(level_); }

    // Actions
    void setLevel(int level);
    void restartLevel();
    void tryMove(int dx, int dy);
    bool isOpenCell(const QPoint& cell) const;

signals:
    void updated();
    void levelCompleted();
    void levelChanged(int level);

private:
    void generateLevel();
    int boardSizeForLevel(int level) const;

    QVector<QVector<bool>> walls_;
    QPoint playerCell_;
    QPoint exitCell_;
    int level_{ 1 };

    static constexpr int kTotalLevels = 10;
};
