#pragma once

#include <QMainWindow>
#include <QVector>

class MemoryMatchModel;
class QLabel;
class SessionOverlayWidget;

class MemoryMatchWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MemoryMatchWindow(QWidget* parent = nullptr);
    ~MemoryMatchWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateUi();
    void resetGame();
    void onMatchResult(bool success, int idx1, int idx2);
    void onGameOver();
    void showOverlay();
    void hideOverlay();

private:
    void setupUi();

    static constexpr int kRows = 4;
    static constexpr int kCols = 4;
    static constexpr int kCellSize = 80;

    MemoryMatchModel* model_{ nullptr };
    SessionOverlayWidget* overlay_{ nullptr };
    int firstFlippedIdx_{ -1 };
    int secondFlippedIdx_{ -1 };
    bool isProcessing_{ false };
};
