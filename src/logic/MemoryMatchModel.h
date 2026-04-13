#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QJsonObject>
#include "../core/ModuleInterfaces.h"

struct MemoryCard {
    MemoryCard() = default;
    MemoryCard(const QString& e, bool f, bool m) : emoji(e), isFlipped(f), isMatched(m) {}

    QString emoji;
    bool isFlipped{ false };
    bool isMatched{ false };
};

class MemoryMatchModel final : public QObject, public IStatefulModule, public IPausableModule {
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

    // IStatefulModule
    QJsonObject saveSession() const override;
    void restoreSession(const QJsonObject& data) override;
    QJsonObject saveHistory() const override;
    void restoreHistory(const QJsonObject& data) override;

    // IPausableModule
    void pause() override { isPaused_ = true; }
    void resume() override { isPaused_ = false; }

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
    bool isPaused_{ false };
};
