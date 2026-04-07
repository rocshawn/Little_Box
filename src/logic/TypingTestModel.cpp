#include "TypingTestModel.h"

#include <QRandomGenerator>
#include <QSettings>
#include <QTimer>

#include <algorithm>

const QStringList TypingTestModel::kQuotes_ = {
    "To be, or not to be, that is the question: Whether 'tis nobler in the mind to suffer the slings and arrows of outrageous fortune, or to take arms against a sea of troubles...",
    "It is a truth universally acknowledged, that a single man in possession of a good fortune, must be in want of a wife.",
    "Call me Ishmael. Some years ago - never mind how long precisely - having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world.",
    "It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity.",
    "All happy families are alike; each unhappy family is unhappy in its own way.",
    "You only live once, but if you do it right, once is enough.",
    "In three words I can sum up everything I've learned about life: it goes on.",
    "The quick brown fox jumps over the lazy dog."
};

TypingTestModel::TypingTestModel(QObject* parent)
    : QObject(parent),
      timer_(new QTimer(this)),
      targetText_(""),
      elapsedSeconds_(0),
      isRunning_(false),
      wpm_(0.0),
      accuracy_(100),
      bestWpm_(0.0),
      difficulty_(Difficulty::Medium) {
    
    QSettings settings;
    bestWpm_ = settings.value("typing_test/best_wpm", 0.0).toDouble();

    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &TypingTestModel::onTick);

    reset();
}

void TypingTestModel::setDifficulty(Difficulty diff) {
    if (difficulty_ == diff) return;
    difficulty_ = diff;
    reset();
}

void TypingTestModel::reset() {
    timer_->stop();
    isRunning_ = false;
    elapsedSeconds_ = 0;
    wpm_ = 0.0;
    accuracy_ = 100;
    generateQuote();
    emit updated();
}

void TypingTestModel::updateInput(const QString& typed) {
    if (typed.isEmpty()) return;

    if (!isRunning_) {
        isRunning_ = true;
        timer_->start();
    }

    calculateStats(typed);

    if (typed.length() >= targetText_.length()) {
        timer_->stop();
        isRunning_ = false;
        emit finished();
    }
}

void TypingTestModel::onTick() {
    elapsedSeconds_++;
    emit updated();
}

void TypingTestModel::generateQuote() {
    QStringList filtered;
    for (const auto& quote : kQuotes_) {
        const int len = static_cast<int>(quote.length());
        if (difficulty_ == Difficulty::Short && len < 60) filtered.append(quote);
        else if (difficulty_ == Difficulty::Medium && len >= 60 && len < 150) filtered.append(quote);
        else if (difficulty_ == Difficulty::Long && len >= 150) filtered.append(quote);
    }
    
    if (filtered.isEmpty()) filtered = kQuotes_;

    int idx = QRandomGenerator::global()->bounded(static_cast<int>(filtered.size()));
    targetText_ = filtered.at(idx);
    emit textChanged(targetText_);
}

void TypingTestModel::calculateStats(const QString& typed) {
    if (elapsedSeconds_ == 0 && !isRunning_) return;

    // Calculate accuracy
    int correctCount = 0;
    int limit = std::min(static_cast<int>(typed.length()), static_cast<int>(targetText_.length()));
    for (int i = 0; i < limit; ++i) {
        if (typed.at(i) == targetText_.at(i)) correctCount++;
    }
    
    if (!typed.isEmpty()) {
        accuracy_ = (correctCount * 100) / static_cast<int>(typed.length());
    }

    // Words per minute standard formula: (chars / 5) / (seconds / 60)
    double minutes = std::max(1.0, static_cast<double>(elapsedSeconds_)) / 60.0;
    wpm_ = (static_cast<double>(typed.length()) / 5.0) / minutes;

    if (wpm_ > bestWpm_) {
        bestWpm_ = wpm_;
        QSettings settings;
        settings.setValue("typing_test/best_wpm", bestWpm_);
    }

    emit statsChanged(wpm_, accuracy_);
}
