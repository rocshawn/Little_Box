#include "ReactionTestModel.h"

#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>

#include <numeric>

ReactionTestModel::ReactionTestModel(QObject* parent)
    : QObject(parent),
      waitTimer_(new QTimer(this)),
      reactionTimer_(),
      roundTimesMs_(),
      state_(State::ReadyToStart),
      clickedTooEarly_(false),
      averageMs_(-1),
      bestAverageMs_(-1),
      isNewRecord_(false) {
    waitTimer_->setSingleShot(true);
    connect(waitTimer_, &QTimer::timeout, this, &ReactionTestModel::enterMeasuringState);
}

void ReactionTestModel::handleInteraction() {
    switch (state_) {
    case State::ReadyToStart:
        clickedTooEarly_ = false;
        startRoundCountdown();
        break;
    case State::WaitingGreen:
        clickedTooEarly_ = true;
        startRoundCountdown();
        break;
    case State::Measuring:
        completeCurrentRound();
        break;
    case State::Finished:
        resetSession();
        startRoundCountdown();
        break;
    }
}

void ReactionTestModel::startRoundCountdown() {
    state_ = State::WaitingGreen;
    waitTimer_->stop();

    const int delayMs = QRandomGenerator::global()->bounded(kDelayMinMs, kDelayMaxMs + 1);
    waitTimer_->start(delayMs);

    emit stateChanged(state_);
    emit updated();
}

void ReactionTestModel::enterMeasuringState() {
    state_ = State::Measuring;
    clickedTooEarly_ = false;
    reactionTimer_.restart();
    emit stateChanged(state_);
    emit updated();
}

void ReactionTestModel::completeCurrentRound() {
    if (!reactionTimer_.isValid()) {
        return;
    }

    const int roundTime = static_cast<int>(reactionTimer_.elapsed());
    roundTimesMs_.append(roundTime);
    emit roundFinished(roundTime);

    if (roundTimesMs_.size() >= kTotalRounds) {
        finalizeSession();
    } else {
        state_ = State::ReadyToStart;
        emit stateChanged(state_);
    }

    emit updated();
}

void ReactionTestModel::finalizeSession() {
    waitTimer_->stop();
    state_ = State::Finished;
    isNewRecord_ = false;

    if (roundTimesMs_.isEmpty()) {
        averageMs_ = -1;
    } else {
        const int total = std::accumulate(roundTimesMs_.cbegin(), roundTimesMs_.cend(), 0);
        averageMs_ = qRound(static_cast<double>(total) / static_cast<double>(roundTimesMs_.size()));

        if (averageMs_ > 0 && (bestAverageMs_ < 0 || averageMs_ < bestAverageMs_)) {
            bestAverageMs_ = averageMs_;
            isNewRecord_ = true;
        }
    }
    emit sessionFinished(averageMs_);
    emit stateChanged(state_);
}

void ReactionTestModel::resetSession() {
    waitTimer_->stop();
    reactionTimer_.invalidate();
    roundTimesMs_.clear();
    state_ = State::ReadyToStart;
    clickedTooEarly_ = false;
    averageMs_ = -1;
    isNewRecord_ = false;
    emit stateChanged(state_);
    emit updated();
}

QJsonObject ReactionTestModel::saveHistory() const {
    QJsonObject obj;
    obj["bestAverageMs"] = bestAverageMs_;
    return obj;
}

void ReactionTestModel::restoreHistory(const QJsonObject& data) {
    if (data.contains("bestAverageMs")) {
        const int savedBest = data["bestAverageMs"].toInt(-1);
        if (savedBest > 0) {
            bestAverageMs_ = savedBest;
        }
    }
}
