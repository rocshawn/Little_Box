#pragma once

#include <QMainWindow>
#include <QVector>

class MinesweeperModel;
class QLabel;
class SessionOverlayWidget;

class MinesweeperWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MinesweeperWindow(QWidget* parent = nullptr);
    ~MinesweeperWindow() override = default;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateUi();
    void resetGame();
    void onFlagsChanged(int count);
    void onGameOver(bool win);
    void showOverlay();
    void hideOverlay();

private:
    void setupUi();

    static constexpr int kCellSize = 36;

    MinesweeperModel* model_{ nullptr };
    SessionOverlayWidget* overlay_{ nullptr };
};
