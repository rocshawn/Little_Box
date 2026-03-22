#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QVector>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;
class QPainter;
class QRect;
class QTimer;

class ReactionTestWidget final : public QWidget {
public:
    explicit ReactionTestWidget(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    enum class State {
        ReadyToStart,
        WaitingGreen,
        Measuring,
        Finished
    };

    void startRoundCountdown();
    void enterMeasuringState();
    void completeCurrentRound();
    void finalizeSession();
    void resetSession();
    QString buildMainText() const;
    QString buildProgressText() const;
    QString buildRoundHistoryText() const;
    void drawChart(QPainter& painter, const QRect& panelRect) const;

    static constexpr int kTotalRounds = 5;

    QTimer* waitTimer_{ nullptr };
    QElapsedTimer reactionTimer_;
    QVector<int> roundTimesMs_;
    State state_{ State::ReadyToStart };
    bool clickedTooEarly_{ false };
    int averageMs_{ -1 };
    int bestAverageMs_{ -1 };
    bool isNewRecord_{ false };
};
