#pragma once

#include <QMainWindow>
#include <QPoint>

class PomodoroModel;
class QLabel;
class QPushButton;
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
    void updateUi();
    void onTimerFinished();
    void switchMode(bool isWork);
    void enterMiniMode();
    void exitMiniMode();

private:
    void setupUi();
    void setupNormalUi();
    void setupMiniUi();

    PomodoroModel* model_{ nullptr };
    
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

    // UI-only State
    bool isMiniMode_{ false };
    QPoint dragPosition_{};
};
