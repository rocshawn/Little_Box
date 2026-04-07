#pragma once

#include <QMainWindow>
#include <QStringList>

class TypingTestModel;
class QTextEdit;
class QTextBrowser;
class QLabel;
class QPushButton;
class QComboBox;

class TypingTestWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit TypingTestWindow(QWidget* parent = nullptr);
    ~TypingTestWindow() override = default;

private slots:
    void updateUi();
    void onFinished();
    void resetTest();
    void checkTyping();

private:
    void setupUi();

    TypingTestModel* model_{ nullptr };

    QTextBrowser* textDisplay_{ nullptr };
    QTextEdit* inputEdit_{ nullptr };

    QLabel* wpmLabel_{ nullptr };
    QLabel* accuracyLabel_{ nullptr };
    QLabel* bestWpmLabel_{ nullptr };
    QLabel* timeLabel_{ nullptr };
    QComboBox* difficultyCombo_{ nullptr };
};
