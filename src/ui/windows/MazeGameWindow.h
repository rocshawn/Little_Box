#pragma once

#include <QMainWindow>
#include <QPoint>
#include <QVector>
#include <QWidget>

#include <functional>
#include <utility>

class QKeyEvent;
class QLabel;
class QPaintEvent;
class QPushButton;


class MazeBoard final : public QWidget {
public:
    explicit MazeBoard(QWidget* parent = nullptr);

    void setLevel(int level);
    int currentLevel() const noexcept;
    void restartLevel();
    void setLevelCompletedCallback(std::function<void()> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void generateLevel();
    void tryMove(int dx, int dy);
    bool isOpenCell(const QPoint& cell) const;
    int boardSizeForLevel(int level) const;

    QVector<QVector<bool>> walls_;
    QPoint playerCell_;
    QPoint exitCell_;
    int level_{ 1 };
    std::function<void()> onLevelCompleted_;
};

class MazeGameWindow final : public QMainWindow {
public:
    explicit MazeGameWindow(QWidget* parent = nullptr);
    ~MazeGameWindow() override = default;

private:
    void setupUi();
    void setupConnections();
    void loadLevel(int level);
    void handleLevelCompleted();
    void updateLabels();

    MazeBoard* board_{ nullptr };
    QLabel* levelLabel_{ nullptr };
    QLabel* hintLabel_{ nullptr };
    QPushButton* restartButton_{ nullptr };
    QPushButton* closeButton_{ nullptr };
    int currentLevel_{ 1 };

    static constexpr int kTotalLevels = 10;
};
