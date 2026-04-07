#pragma once

#include <QMainWindow>
#include <QVector>

class MemoryMatchModel;
class QLabel;

class MemoryMatchWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MemoryMatchWindow(QWidget* parent = nullptr);
    ~MemoryMatchWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void updateUi();
    void resetGame();
    void onMatchResult(bool success, int idx1, int idx2);
    void onGameOver();

private:
    void setupUi();

    static constexpr int kRows = 4;
    static constexpr int kCols = 4;
    static constexpr int kCellSize = 80;

    MemoryMatchModel* model_{ nullptr };
    int firstFlippedIdx_{ -1 };
    int secondFlippedIdx_{ -1 };
    bool isProcessing_{ false };
};
