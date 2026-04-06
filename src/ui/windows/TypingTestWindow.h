#pragma once

#include <QMainWindow>
#include <QStringList>

class QTextEdit;
class QTextBrowser;
class QLabel;
class QTimer;
class QPushButton;
class QComboBox;

class TypingTestWindow final : public QMainWindow {
    Q_OBJECT

public:
    enum class Difficulty { Short, Medium, Long };

    explicit TypingTestWindow(QWidget* parent = nullptr);
    ~TypingTestWindow() override = default;

private:
    void setupUi();
    void resetTest();
    void checkTyping();
    void updateStats();
    void onTick();
    
    QString generateQuote();

    QTextBrowser* textDisplay_{ nullptr };
    QTextEdit* inputEdit_{ nullptr };

    QLabel* wpmLabel_{ nullptr };
    QLabel* accuracyLabel_{ nullptr };
    QLabel* bestWpmLabel_{ nullptr };
    QLabel* timeLabel_{ nullptr };
    QComboBox* difficultyCombo_{ nullptr };
    
    QTimer* timer_{ nullptr };
    
    QString targetText_;
    int elapsedSeconds_{ 0 };
    bool isRunning_{ false };
    double bestWpm_{ 0.0 };
    Difficulty currentDifficulty_{ Difficulty::Medium };
    
    const QStringList kQuotes_{
        "To be, or not to be, that is the question: Whether 'tis nobler in the mind to suffer the slings and arrows of outrageous fortune, or to take arms against a sea of troubles...",
        "It is a truth universally acknowledged, that a single man in possession of a good fortune, must be in want of a wife.",
        "Call me Ishmael. Some years ago - never mind how long precisely - having little or no money in my purse, and nothing particular to interest me on shore, I thought I would sail about a little and see the watery part of the world.",
        "It was the best of times, it was the worst of times, it was the age of wisdom, it was the age of foolishness, it was the epoch of belief, it was the epoch of incredulity.",
        "All happy families are alike; each unhappy family is unhappy in its own way.",
        "You only live once, but if you do it right, once is enough.",
        "In three words I can sum up everything I've learned about life: it goes on.",
        "The quick brown fox jumps over the lazy dog."
    };
};
