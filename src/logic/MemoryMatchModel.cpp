#include "MemoryMatchModel.h"

#include <QRandomGenerator>
#include <QStringList>
#include <algorithm>
#include <random>

MemoryMatchModel::MemoryMatchModel(QObject* parent)
    : QObject(parent) {
    reset();
}

void MemoryMatchModel::reset() {
    const QStringList emojiPool = {
        "🍓", "🍑", "🍋", "🍒", "🥝", "🍩", "🧁", "🍭",
        "🧸", "🎈", "🎨", "🧩", "🦄", "🌈", "🦋", "🌸"
    };

    int numPairs = (kRows * kCols) / 2;
    QStringList gameEmojis;
    for (int i = 0; i < numPairs; ++i) {
        gameEmojis.append(emojiPool[i % emojiPool.size()]);
        gameEmojis.append(emojiPool[i % emojiPool.size()]);
    }

    // Shuffle manually
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(gameEmojis.begin(), gameEmojis.end(), g);

    cards_.clear();
    for (const auto& emoji : gameEmojis) {
        cards_.append({ emoji, false, false });
    }

    matchesFound_ = 0;
    emit updated();
}

bool MemoryMatchModel::attemptFlip(int index) {
    if (index < 0 || index >= cards_.size()) return false;
    if (cards_[index].isMatched || cards_[index].isFlipped) return false;

    cards_[index].isFlipped = true;
    emit cardFlipped(index);
    emit updated();
    return true;
}

void MemoryMatchModel::checkMatch(int idx1, int idx2) {
    if (idx1 < 0 || idx1 >= cards_.size() || idx2 < 0 || idx2 >= cards_.size()) return;

    if (cards_[idx1].emoji == cards_[idx2].emoji) {
        cards_[idx1].isMatched = true;
        cards_[idx2].isMatched = true;
        matchesFound_++;
        emit matchResult(true, idx1, idx2);
        if (isGameOver()) {
            emit gameOver();
        }
    } else {
        emit matchResult(false, idx1, idx2);
    }
    emit updated();
}

void MemoryMatchModel::resetFlippedCards(int idx1, int idx2) {
    if (idx1 >= 0 && idx1 < cards_.size()) cards_[idx1].isFlipped = false;
    if (idx2 >= 0 && idx2 < cards_.size()) cards_[idx2].isFlipped = false;
    emit updated();
}
