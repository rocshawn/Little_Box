#pragma once

#include <QObject>
#include <QPoint>
#include <QVector>
#include <QJsonObject>
#include "../core/ModuleInterfaces.h"

class MazeGameModel final : public QObject, public IStatefulModule, public IPausableModule {
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
    // IStatefulModule
    QJsonObject saveSession() const override;
    void restoreSession(const QJsonObject& data) override;
    QJsonObject saveHistory() const override;
    void restoreHistory(const QJsonObject& data) override;

    // IPausableModule
    void pause() override { isPaused_ = true; }
    void resume() override { isPaused_ = false; }

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
    bool isPaused_{ false };

    static constexpr int kTotalLevels = 10;
};
