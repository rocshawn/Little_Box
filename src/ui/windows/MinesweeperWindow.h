#pragma once

#include <QMainWindow>
#include <QVector>

class MinesweeperModel;
class QLabel;

class MinesweeperWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MinesweeperWindow(QWidget* parent = nullptr);
    ~MinesweeperWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void updateUi();
    void resetGame();
    void onFlagsChanged(int count);
    void onGameOver(bool win);

private:
    void setupUi();

    static constexpr int kCellSize = 36;

    MinesweeperModel* model_{ nullptr };
};
