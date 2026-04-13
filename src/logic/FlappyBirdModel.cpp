#include "FlappyBirdModel.h"

#include <QRandomGenerator>
#include <QJsonArray>
#include <algorithm>

FlappyBirdModel::FlappyBirdModel(QObject* parent)
    : QObject(parent) {
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
    if (isPaused_) return;
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
    if (isPaused_) return;
    
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

void FlappyBirdModel::updateBestScoreIfNeeded() {
    if (score_ <= bestScore_) return;

    bestScore_ = score_;
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

QJsonObject FlappyBirdModel::saveSession() const {
    QJsonObject obj;
    obj["birdY"] = birdY_;
    obj["birdVelocity"] = birdVelocity_;
    obj["frameCounter"] = frameCounter_;
    obj["score"] = score_;
    obj["started"] = started_;
    obj["gameOver"] = gameOver_;
    
    QJsonArray pipesArr;
    for (const auto& pipe : pipes_) {
        QJsonObject pipeObj;
        pipeObj["x"] = pipe.x;
        pipeObj["gapCenterY"] = pipe.gapCenterY;
        pipeObj["scored"] = pipe.scored;
        pipesArr.append(pipeObj);
    }
    obj["pipes"] = pipesArr;
    return obj;
}

void FlappyBirdModel::restoreSession(const QJsonObject& data) {
    if (data.contains("birdY")) birdY_ = data["birdY"].toDouble();
    if (data.contains("birdVelocity")) birdVelocity_ = data["birdVelocity"].toDouble();
    if (data.contains("frameCounter")) frameCounter_ = data["frameCounter"].toInt();
    if (data.contains("score")) score_ = data["score"].toInt();
    if (data.contains("started")) started_ = data["started"].toBool();
    if (data.contains("gameOver")) gameOver_ = data["gameOver"].toBool();
    
    if (data.contains("pipes")) {
        QJsonArray pipesArr = data["pipes"].toArray();
        pipes_.clear();
        for (int i = 0; i < pipesArr.size(); ++i) {
            QJsonObject pipeObj = pipesArr[i].toObject();
            FlappyPipe p;
            p.x = pipeObj["x"].toDouble();
            p.gapCenterY = pipeObj["gapCenterY"].toDouble();
            p.scored = pipeObj["scored"].toBool();
            pipes_.append(p);
        }
    }
    emit stateChanged();
    emit updated();
    emit scoreChanged(score_, bestScore_);
}

QJsonObject FlappyBirdModel::saveHistory() const {
    QJsonObject obj;
    obj["bestScore"] = bestScore_;
    return obj;
}

void FlappyBirdModel::restoreHistory(const QJsonObject& data) {
    if (data.contains("bestScore")) {
        bestScore_ = data["bestScore"].toInt();
    }
}
