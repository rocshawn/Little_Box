#pragma once

#include <QMainWindow>
#include <QPoint>

class QLabel;
class QPushButton;
class QTimer;
class QStackedWidget;
class QWidget;

class PomodoroWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit PomodoroWindow(QWidget* parent = nullptr);
    ~PomodoroWindow() override = default;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onTick();
    void toggleTimer();
    void resetTimer();
    void switchMode(bool isWorkMode);
    void enterMiniMode();
    void exitMiniMode();

private:
    void setupUi();
    void setupNormalUi();
    void setupMiniUi();
    void updateTimeDisplay();
    void timerFinished();

    // UI Elements
    QStackedWidget* stackedWidget_{ nullptr };
    
    // Normal UI
    QWidget* normalWidget_{ nullptr };
    QLabel* normalTimeLabel_{ nullptr };
    QLabel* normalStatusLabel_{ nullptr };
    QPushButton* normalPlayPauseBtn_{ nullptr };
    QPushButton* workModeBtn_{ nullptr };
    QPushButton* breakModeBtn_{ nullptr };

    // Mini UI
    QWidget* miniWidget_{ nullptr };
    QLabel* miniTimeLabel_{ nullptr };
    QPushButton* miniPlayPauseBtn_{ nullptr };

    // State
    QTimer* timer_{ nullptr };
    int remainingSeconds_{ 25 * 60 };
    bool isWorkMode_{ true };
    bool isMiniMode_{ false };

    // For frameless window dragging
    QPoint dragPosition_{};
};
