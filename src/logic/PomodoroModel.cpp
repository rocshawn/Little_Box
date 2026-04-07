#include "PomodoroModel.h"

#include <QTimer>

PomodoroModel::PomodoroModel(QObject* parent)
    : QObject(parent),
      timer_(new QTimer(this)),
      remainingSeconds_(kWorkSeconds),
      isWorkMode_(true),
      isRunning_(false) {
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &PomodoroModel::onTick);
}

QString PomodoroModel::statusText() const {
    if (!isRunning_) {
        return isWorkMode_ ? "已暂停" : "休息已暂停";
    }
    return isWorkMode_ ? "专注中..." : "休息中...";
}

QString PomodoroModel::formattedTime() const {
    const int m = remainingSeconds_ / 60;
    const int s = remainingSeconds_ % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}

void PomodoroModel::toggle() {
    if (isRunning_) {
        stop();
    } else {
        start();
    }
}

void PomodoroModel::reset() {
    stop();
    remainingSeconds_ = isWorkMode_ ? kWorkSeconds : kBreakSeconds;
    emit timeUpdated(remainingSeconds_);
}

void PomodoroModel::setMode(bool isWork) {
    if (isWorkMode_ == isWork) return;
    isWorkMode_ = isWork;
    reset();
    emit modeChanged(isWorkMode_);
}

void PomodoroModel::start() {
    if (isRunning_) return;
    if (remainingSeconds_ <= 0) {
        reset();
    }
    isRunning_ = true;
    timer_->start();
    emit stateChanged(isRunning_);
}

void PomodoroModel::stop() {
    if (!isRunning_) return;
    isRunning_ = false;
    timer_->stop();
    emit stateChanged(isRunning_);
}

void PomodoroModel::onTick() {
    if (remainingSeconds_ > 0) {
        remainingSeconds_--;
        emit timeUpdated(remainingSeconds_);
    } else {
        stop();
        emit finished();
    }
}
