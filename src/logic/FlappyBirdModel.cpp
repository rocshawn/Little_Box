#include "FlappyBirdModel.h"

#include <QRandomGenerator>
#include <QSettings>
#include <algorithm>

namespace {
constexpr auto kBestScoreSettingsKey = "games/flappyBird/bestScore";
}

FlappyBirdModel::FlappyBirdModel(QObject* parent)
    : QObject(parent) {
    loadBestScore();
    reset();
}

void FlappyBirdModel::reset() {
    pipes_.clear();
    birdY_ = 240.0;
    birdVelocity_ = 0.0;
    frameCounter_ = 0;
    score_ = 0;
    started_ = false;
    gameOver_ = false;
    emit stateChanged();
    emit updated();
}

void FlappyBirdModel::startNewGame(double viewHeight) {
    reset();
    birdY_ = qMax(160.0, viewHeight / 2.0);
    emit stateChanged();
}

void FlappyBirdModel::flap() {
    if (gameOver_) {
        reset(); // Potentially restart here or let UI call reset/start
        return;
    }

    if (!started_) {
        started_ = true;
        emit stateChanged();
    }

    birdVelocity_ = kFlapVelocity;
    emit updated();
}

void FlappyBirdModel::update(double viewWidth, double viewHeight) {
    if (!started_ || gameOver_) {
        emit updated();
        return;
    }

    ++frameCounter_;
    birdVelocity_ += kGravity;
    birdY_ += birdVelocity_;

    if (frameCounter_ % kSpawnIntervalFrames == 0 || pipes_.isEmpty()) {
        spawnPipe(viewWidth, viewHeight);
    }

    for (FlappyPipe& pipe : pipes_) {
        pipe.x -= kPipeSpeed;
        if (!pipe.scored && pipe.x + kPipeWidth < kBirdX) {
            pipe.scored = true;
            ++score_;
            updateBestScoreIfNeeded();
            emit scoreChanged(score_, bestScore_);
        }
    }

    while (!pipes_.isEmpty() && pipes_.front().x + kPipeWidth < -10) {
        pipes_.removeFirst();
    }

    const QRectF bird = birdRect();
    const qreal playHeight = viewHeight - kGroundHeight;
    
    if (bird.top() <= 0 || bird.bottom() >= playHeight) {
        gameOver_ = true;
        updateBestScoreIfNeeded();
        emit stateChanged();
        emit gameOver();
        emit updated();
        return;
    }

    for (const FlappyPipe& pipe : pipes_) {
        if (hitsPipe(bird, pipe, viewHeight)) {
            gameOver_ = true;
            updateBestScoreIfNeeded();
            emit stateChanged();
            emit gameOver();
            emit updated();
            return;
        }
    }

    emit updated();
}

void FlappyBirdModel::spawnPipe(double viewWidth, double viewHeight) {
    if (viewWidth <= 0 || viewHeight <= 0) return;

    const qreal playHeight = qMax(320.0, viewHeight - kGroundHeight);
    const qreal gapSize = qMax(132.0, 178.0 - score_ * 2.0);
    const int minGapCenter = static_cast<int>(gapSize / 2.0 + 34.0);
    const int maxGapCenter = static_cast<int>(playHeight - gapSize / 2.0 - 34.0);
    const int safeMax = std::max(minGapCenter + 1, maxGapCenter);

    FlappyPipe pipe;
    pipe.x = viewWidth + 80.0;
    pipe.gapCenterY = QRandomGenerator::global()->bounded(minGapCenter, safeMax);
    pipe.scored = false;
    pipes_.append(pipe);
}

void FlappyBirdModel::loadBestScore() {
    QSettings settings;
    const QString settingsKey = QString::fromLatin1(kBestScoreSettingsKey);
    bestScore_ = std::max(0, settings.value(settingsKey, 0).toInt());
}

void FlappyBirdModel::updateBestScoreIfNeeded() {
    if (score_ <= bestScore_) return;

    bestScore_ = score_;
    QSettings settings;
    const QString settingsKey = QString::fromLatin1(kBestScoreSettingsKey);
    settings.setValue(settingsKey, bestScore_);
}

QRectF FlappyBirdModel::birdRect() const {
    return QRectF(kBirdX, birdY_, kBirdSize, kBirdSize);
}

bool FlappyBirdModel::hitsPipe(const QRectF& bird, const FlappyPipe& pipe, double viewHeight) const {
    const qreal gapSize = qMax(132.0, 178.0 - score_ * 2.0);
    const qreal playHeight = viewHeight - kGroundHeight;
    const qreal gapTop = pipe.gapCenterY - gapSize / 2.0;
    const qreal gapBottom = pipe.gapCenterY + gapSize / 2.0;

    const QRectF topPipe(pipe.x, 0, kPipeWidth, gapTop);
    const QRectF bottomPipe(pipe.x, gapBottom, kPipeWidth, playHeight - gapBottom);
    return bird.intersects(topPipe) || bird.intersects(bottomPipe);
}
