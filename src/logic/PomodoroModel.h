#pragma once

#include <QObject>
#include <QString>

class QTimer;

class PomodoroModel final : public QObject {
    Q_OBJECT

public:
    explicit PomodoroModel(QObject* parent = nullptr);
    ~PomodoroModel() override = default;

    // Getters
    int remainingSeconds() const { return remainingSeconds_; }
    bool isWorkMode() const { return isWorkMode_; }
    bool isRunning() const { return isRunning_; }
    QString statusText() const;
    QString formattedTime() const;

    // Actions
    void toggle();
    void reset();
    void setMode(bool isWork);
    void start();
    void stop();

signals:
    void timeUpdated(int seconds);
    void modeChanged(bool isWork);
    void stateChanged(bool isRunning);
    void finished();

private slots:
    void onTick();

private:
    static constexpr int kWorkSeconds = 25 * 60;
    static constexpr int kBreakSeconds = 5 * 60;

    QTimer* timer_{ nullptr };
    int remainingSeconds_{ kWorkSeconds };
    bool isWorkMode_{ true };
    bool isRunning_{ false };
};
