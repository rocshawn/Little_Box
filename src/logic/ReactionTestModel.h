#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include <QJsonObject>
#include "../core/ModuleInterfaces.h"

class QTimer;

class ReactionTestModel final : public QObject, public IStatefulModule {
    Q_OBJECT

public:
    enum class State {
        ReadyToStart,
        WaitingGreen,
        Measuring,
        Finished
    };

    explicit ReactionTestModel(QObject* parent = nullptr);
    ~ReactionTestModel() override = default;

    // Getters
    State state() const { return state_; }
    const QVector<int>& roundTimesMs() const { return roundTimesMs_; }
    int averageMs() const { return averageMs_; }
    int bestAverageMs() const { return bestAverageMs_; }
    bool isNewRecord() const { return isNewRecord_; }
    bool clickedTooEarly() const { return clickedTooEarly_; }
    int totalRounds() const { return kTotalRounds; }

    // Actions
    void handleInteraction();
    void resetSession();

    // IStatefulModule
    QJsonObject saveSession() const override { return QJsonObject(); }
    void restoreSession(const QJsonObject& data) override {}
    QJsonObject saveHistory() const override;
    void restoreHistory(const QJsonObject& data) override;

signals:
    void updated();
    void stateChanged(State newState);
    void roundFinished(int ms);
    void sessionFinished(int avg);

private slots:
    void enterMeasuringState();

private:
    void startRoundCountdown();
    void completeCurrentRound();
    void finalizeSession();

    static constexpr int kTotalRounds = 5;
    static constexpr int kDelayMinMs = 2000;
    static constexpr int kDelayMaxMs = 5000;

    QTimer* waitTimer_{ nullptr };
    QElapsedTimer reactionTimer_;
    QVector<int> roundTimesMs_;
    State state_{ State::ReadyToStart };
    bool clickedTooEarly_{ false };
    int averageMs_{ -1 };
    int bestAverageMs_{ -1 };
    bool isNewRecord_{ false };
};
