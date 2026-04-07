#pragma once

#include <QObject>
#include <QString>
#include <QVector>

struct MemoryCard {
    MemoryCard() = default;
    MemoryCard(const QString& e, bool f, bool m) : emoji(e), isFlipped(f), isMatched(m) {}

    QString emoji;
    bool isFlipped{ false };
    bool isMatched{ false };
};

class MemoryMatchModel final : public QObject {
    Q_OBJECT

public:
    explicit MemoryMatchModel(QObject* parent = nullptr);
    ~MemoryMatchModel() override = default;

    // Getters
    const QVector<MemoryCard>& cards() const { return cards_; }
    int matchesFound() const { return matchesFound_; }
    int totalPairs() const { return (kRows * kCols) / 2; }
    bool isGameOver() const { return matchesFound_ == totalPairs(); }

    // Actions
    void reset();
    bool attemptFlip(int index);
    void checkMatch(int idx1, int idx2);
    void resetFlippedCards(int idx1, int idx2);

signals:
    void updated();
    void cardFlipped(int index);
    void matchResult(bool success, int idx1, int idx2);
    void gameOver();

private:
    static constexpr int kRows = 4;
    static constexpr int kCols = 4;

    QVector<MemoryCard> cards_;
    int matchesFound_{ 0 };
};
