#pragma once

#include <QObject>
#include <QStringList>

class QTimer;

class TypingTestModel final : public QObject {
    Q_OBJECT

public:
    enum class Difficulty { Short, Medium, Long };

    explicit TypingTestModel(QObject* parent = nullptr);
    ~TypingTestModel() override = default;

    // Getters
    QString targetText() const { return targetText_; }
    int elapsedSeconds() const { return elapsedSeconds_; }
    bool isRunning() const { return isRunning_; }
    double wpm() const { return wpm_; }
    int accuracy() const { return accuracy_; }
    double bestWpm() const { return bestWpm_; }
    Difficulty difficulty() const { return difficulty_; }

    // Actions
    void setDifficulty(Difficulty diff);
    void reset();
    void updateInput(const QString& typed);

signals:
    void updated();
    void finished();
    void textChanged(const QString& newText);
    void statsChanged(double wpm, int accuracy);

private slots:
    void onTick();

private:
    void generateQuote();
    void calculateStats(const QString& typed);

    QTimer* timer_{ nullptr };
    QString targetText_;
    int elapsedSeconds_{ 0 };
    bool isRunning_{ false };
    double wpm_{ 0.0 };
    int accuracy_{ 100 };
    double bestWpm_{ 0.0 };
    Difficulty difficulty_{ Difficulty::Medium };

    static const QStringList kQuotes_;
};
