#pragma once

#include <QMainWindow>
#include <QRectF>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <functional>
#include <utility>


class QKeyEvent;
class QLabel;
class QMouseEvent;
class QPushButton;
class QResizeEvent;
class QPaintEvent;

struct FlappyPipe {
    double x{ 0.0 };
    double gapCenterY{ 0.0 };
    bool scored{ false };
};


class FlappyBirdWidget final : public QWidget {
public:
    explicit FlappyBirdWidget(QWidget* parent = nullptr);

    void startNewGame();
    int currentScore() const noexcept;
    int bestScore() const noexcept;
    bool hasStarted() const noexcept;
    bool isGameOver() const noexcept;
    void setStateChangedCallback(std::function<void()> callback);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void advanceFrame();
    void flap();
    void spawnPipe();
    void loadBestScore();
    void updateBestScoreIfNeeded();
    void notifyStateChanged();
    QRectF birdRect() const;
    bool hitsPipe(const QRectF& bird, const FlappyPipe& pipe) const;


    QTimer timer_;
    QVector<FlappyPipe> pipes_;
    std::function<void()> onStateChanged_;
    double birdY_{ 240.0 };
    double birdVelocity_{ 0.0 };

    int frameCounter_{ 0 };
    int score_{ 0 };
    int bestScore_{ 0 };
    bool started_{ false };
    bool gameOver_{ false };
};

class FlappyBirdWindow final : public QMainWindow {
public:
    explicit FlappyBirdWindow(QWidget* parent = nullptr);
    ~FlappyBirdWindow() override = default;

private:
    void setupUi();
    void updateScorePanel();

    FlappyBirdWidget* gameWidget_{ nullptr };
    QLabel* scoreValueLabel_{ nullptr };
    QLabel* bestScoreValueLabel_{ nullptr };
    QLabel* stateLabel_{ nullptr };
    QPushButton* restartButton_{ nullptr };
    QPushButton* closeButton_{ nullptr };
};
