#pragma once

#include <QObject>
#include <QRectF>
#include <QVector>

struct FlappyPipe {
    double x{ 0.0 };
    double gapCenterY{ 0.0 };
    bool scored{ false };
};

class FlappyBirdModel final : public QObject {
    Q_OBJECT

public:
    explicit FlappyBirdModel(QObject* parent = nullptr);
    ~FlappyBirdModel() override = default;

    // Getters
    double birdY() const { return birdY_; }
    double birdVelocity() const { return birdVelocity_; }
    const QVector<FlappyPipe>& pipes() const { return pipes_; }
    int score() const { return score_; }
    int bestScore() const { return bestScore_; }
    bool hasStarted() const { return started_; }
    bool isGameOver() const { return gameOver_; }

    // Constants (exposed for UI rendering if needed)
    static constexpr qreal kBirdX = 120.0;
    static constexpr qreal kBirdSize = 34.0;
    static constexpr qreal kPipeWidth = 82.0;
    static constexpr qreal kGroundHeight = 68.0;

    // Actions
    void startNewGame(double viewHeight);
    void flap();
    void update(double viewWidth, double viewHeight);
    void reset();

signals:
    void updated();
    void gameOver();
    void scoreChanged(int current, int best);
    void stateChanged();

private:
    void spawnPipe(double viewWidth, double viewHeight);
    void loadBestScore();
    void updateBestScoreIfNeeded();
    QRectF birdRect() const;
    bool hitsPipe(const QRectF& bird, const FlappyPipe& pipe, double viewHeight) const;

    QVector<FlappyPipe> pipes_;
    double birdY_{ 240.0 };
    double birdVelocity_{ 0.0 };
    int frameCounter_{ 0 };
    int score_{ 0 };
    int bestScore_{ 0 };
    bool started_{ false };
    bool gameOver_{ false };

    // Move constants to cpp or keep as static constexpr
    static constexpr qreal kGravity = 0.42;
    static constexpr qreal kFlapVelocity = -7.4;
    static constexpr qreal kPipeSpeed = 3.2;
    static constexpr int kSpawnIntervalFrames = 95;
};
