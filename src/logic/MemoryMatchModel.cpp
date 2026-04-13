#include "MemoryMatchModel.h"

#include <QRandomGenerator>
#include <QJsonArray>
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
    if (isPaused_) return false;
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

QJsonObject MemoryMatchModel::saveSession() const {
    QJsonObject obj;
    obj["matchesFound"] = matchesFound_;
    
    QJsonArray cardsArr;
    for (const auto& card : cards_) {
        QJsonObject cardObj;
        cardObj["emoji"] = card.emoji;
        cardObj["isFlipped"] = card.isFlipped;
        cardObj["isMatched"] = card.isMatched;
        cardsArr.append(cardObj);
    }
    obj["cards"] = cardsArr;
    return obj;
}

void MemoryMatchModel::restoreSession(const QJsonObject& data) {
    if (data.contains("matchesFound")) {
        matchesFound_ = data["matchesFound"].toInt();
    }
    if (data.contains("cards")) {
        QJsonArray cardsArr = data["cards"].toArray();
        cards_.clear();
        for (int i = 0; i < cardsArr.size(); ++i) {
            QJsonObject cardObj = cardsArr[i].toObject();
            cards_.append({ 
                cardObj["emoji"].toString(), 
                cardObj["isFlipped"].toBool(), 
                cardObj["isMatched"].toBool() 
            });
        }
    }
    emit updated();
}

QJsonObject MemoryMatchModel::saveHistory() const {
    return QJsonObject();
}

void MemoryMatchModel::restoreHistory(const QJsonObject& data) {
}
