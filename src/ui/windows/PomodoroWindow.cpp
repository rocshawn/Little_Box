#include "PomodoroWindow.h"

#include "../../core/ThemeManager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kWorkSeconds = 25 * 60;
constexpr int kBreakSeconds = 5 * 60;

QString formatTime(int seconds) {
    const int m = seconds / 60;
    const int s = seconds % 60;
    return QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
}
} // namespace

PomodoroWindow::PomodoroWindow(QWidget* parent)
    : QMainWindow(parent),
      timer_(new QTimer(this)),
      remainingSeconds_(kWorkSeconds),
      isWorkMode_(true),
      isMiniMode_(false) {
    
    // As a top-level system tool window
    setWindowFlags(Qt::Window);

    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &PomodoroWindow::onTick);

    setupUi();
    updateTimeDisplay();
}

void PomodoroWindow::setupUi() {
    setWindowTitle("番茄钟");
    setMinimumSize(400, 400);
    resize(400, 480);

    stackedWidget_ = new QStackedWidget(this);
    setCentralWidget(stackedWidget_);

    // ── Setup both UI modes
    setupNormalUi();
    setupMiniUi();

    stackedWidget_->addWidget(normalWidget_);
    stackedWidget_->addWidget(miniWidget_);
    stackedWidget_->setCurrentWidget(normalWidget_);

    // ── Theme
    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #064e3b, stop:1 #065f46); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #f0fdfa, stop:1 #ccfbf1); }");
}

void PomodoroWindow::setupNormalUi() {
    normalWidget_ = new QWidget(this);
    auto* layout = new QVBoxLayout(normalWidget_);
    layout->setContentsMargins(30, 40, 30, 30);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    // Mode switch buttons
    auto* topControlsRow = new QHBoxLayout();
    workModeBtn_ = new QPushButton("专注模式");
    breakModeBtn_ = new QPushButton("休息模式");

    const QString btnStyle = "QPushButton { padding: 8px 16px; border-radius: 16px; font-weight: bold; font-size: 14px; background: #ccfbf1; color: #0d9488; border: none; }"
                             "QPushButton:hover { background: #99f6e4; color: white; }"
                             "QPushButton[active=\"true\"] { background: #0d9488; color: white; }";
    
    workModeBtn_->setStyleSheet(btnStyle);
    breakModeBtn_->setStyleSheet(btnStyle);
    workModeBtn_->setCursor(Qt::PointingHandCursor);
    breakModeBtn_->setCursor(Qt::PointingHandCursor);
    
    // Mark initial active
    workModeBtn_->setProperty("active", true);

    topControlsRow->addStretch();
    topControlsRow->addWidget(workModeBtn_);
    topControlsRow->addSpacing(10);
    topControlsRow->addWidget(breakModeBtn_);
    topControlsRow->addStretch();
    layout->addLayout(topControlsRow);
    layout->addSpacing(40);

    // Circular/Large Time Display Area
    const bool dark = ThemeManager::instance().isDark();
    
    auto* timeFrame = new QFrame();
    timeFrame->setFixedSize(260, 260);
    timeFrame->setStyleSheet(dark 
        ? "QFrame { background: #064e3b; border-radius: 130px; border: 8px solid #065f46; }"
        : "QFrame { background: white; border-radius: 130px; border: 8px solid #ccfbf1; }");
    
    auto* timeLayout = new QVBoxLayout(timeFrame);
    timeLayout->setAlignment(Qt::AlignCenter);

    normalStatusLabel_ = new QLabel("专注中...");
    normalStatusLabel_->setAlignment(Qt::AlignCenter);
    normalStatusLabel_->setStyleSheet(dark ? "color:#5eead4; font-size:16px; font-weight: bold;" : "color:#0d9488; font-size:16px; font-weight: bold;");

    normalTimeLabel_ = new QLabel(formatTime(remainingSeconds_));
    normalTimeLabel_->setAlignment(Qt::AlignCenter);
    normalTimeLabel_->setStyleSheet(dark ? "color:#fff0f2; font-size:64px; font-weight: 900;" : "color:#0d9488; font-size:64px; font-weight: 900;");
    
    timeLayout->addWidget(normalStatusLabel_);
    timeLayout->addWidget(normalTimeLabel_);
    layout->addWidget(timeFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(40);

    // Play/Pause & Reset controls
    auto* mainControlsRow = new QHBoxLayout();
    
    normalPlayPauseBtn_ = new QPushButton("\xE2\x96\xB6"); // Play ▶
    normalPlayPauseBtn_->setFixedSize(60, 60);
    normalPlayPauseBtn_->setCursor(Qt::PointingHandCursor);
    normalPlayPauseBtn_->setStyleSheet(
        "QPushButton { background: #0d9488; color: white; font-size: 24px; border-radius: 30px; border: none; }"
        "QPushButton:hover { background: #0f766e; }"
    );

    auto* resetBtn = new QPushButton("\xE2\x86\xBB"); // Reset ↻
    resetBtn->setFixedSize(60, 60);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(
        "QPushButton { background: #ccfbf1; color: #0d9488; font-size: 26px; border-radius: 30px; border: none; }"
        "QPushButton:hover { background: #99f6e4; color: white; }"
    );

    mainControlsRow->addStretch();
    mainControlsRow->addWidget(resetBtn);
    mainControlsRow->addSpacing(30);
    mainControlsRow->addWidget(normalPlayPauseBtn_);
    mainControlsRow->addStretch();
    layout->addLayout(mainControlsRow);

    layout->addStretch();

    // Mini Mode entry button at bottom right
    auto* bottomRow = new QHBoxLayout();
    auto* miniBtn = new QPushButton("\xE2\x86\x99 小窗模式"); // ↙ Small window mode
    miniBtn->setCursor(Qt::PointingHandCursor);
    miniBtn->setStyleSheet(dark 
        ? "QPushButton { background: transparent; color: #99f6e4; font-weight: bold; font-size: 13px; border: none; }"
          "QPushButton:hover { color: #ccfbf1; }"
        : "QPushButton { background: transparent; color: #0d9488; font-weight: bold; font-size: 13px; border: none; }"
          "QPushButton:hover { color: #0f766e; }");
    bottomRow->addStretch();
    bottomRow->addWidget(miniBtn);
    layout->addLayout(bottomRow);

    // Connected signals
    connect(workModeBtn_, &QPushButton::clicked, this, [this]{ switchMode(true); });
    connect(breakModeBtn_, &QPushButton::clicked, this, [this]{ switchMode(false); });
    connect(normalPlayPauseBtn_, &QPushButton::clicked, this, &PomodoroWindow::toggleTimer);
    connect(resetBtn, &QPushButton::clicked, this, &PomodoroWindow::resetTimer);
    connect(miniBtn, &QPushButton::clicked, this, &PomodoroWindow::enterMiniMode);
}

void PomodoroWindow::setupMiniUi() {
    miniWidget_ = new QWidget(this);
    
    // Give mini background
    const bool dark = ThemeManager::instance().isDark();
    miniWidget_->setStyleSheet(dark 
        ? "QWidget { background: #064e3b; border: 2px solid #065f46; border-radius: 20px; }"
        : "QWidget { background: white; border: 2px solid #ccfbf1; border-radius: 20px; }");

    auto* layout = new QHBoxLayout(miniWidget_);
    layout->setContentsMargins(14, 8, 14, 8);
    layout->setSpacing(12);

    miniTimeLabel_ = new QLabel(formatTime(remainingSeconds_), miniWidget_);
    miniTimeLabel_->setStyleSheet(dark 
        ? "color:#fff0f2; font-size:24px; font-weight: 900; background: transparent; border: none;" 
        : "color:#0d9488; font-size:24px; font-weight: 900; background: transparent; border: none;");
    
    miniPlayPauseBtn_ = new QPushButton("\xE2\x96\xB6", miniWidget_);
    miniPlayPauseBtn_->setFixedSize(32, 32);
    miniPlayPauseBtn_->setCursor(Qt::PointingHandCursor);
    miniPlayPauseBtn_->setStyleSheet(
        "QPushButton { background: #0d9488; color: white; font-size: 14px; border-radius: 16px; border: none; }"
        "QPushButton:hover { background: #0f766e; }"
    );

    auto* exitMiniBtn = new QPushButton("\xE2\x86\x97", miniWidget_); // ↗ Expand
    exitMiniBtn->setFixedSize(32, 32);
    exitMiniBtn->setCursor(Qt::PointingHandCursor);
    exitMiniBtn->setStyleSheet(dark
        ? "QPushButton { background: #881337; color: white; font-size: 16px; border-radius: 16px; border: none; }"
          "QPushButton:hover { background: #9f1239; }"
        : "QPushButton { background: #ffe4e6; color: #f43f5e; font-size: 16px; border-radius: 16px; border: none; }"
          "QPushButton:hover { background: #fda4af; color: white; }");

    layout->addWidget(miniTimeLabel_);
    layout->addWidget(miniPlayPauseBtn_);
    layout->addWidget(exitMiniBtn);

    connect(miniPlayPauseBtn_, &QPushButton::clicked, this, &PomodoroWindow::toggleTimer);
    connect(exitMiniBtn, &QPushButton::clicked, this, &PomodoroWindow::exitMiniMode);
}

void PomodoroWindow::onTick() {
    if (remainingSeconds_ > 0) {
        remainingSeconds_--;
        updateTimeDisplay();
    } else {
        timerFinished();
    }
}

void PomodoroWindow::toggleTimer() {
    if (timer_->isActive()) {
        timer_->stop();
        normalPlayPauseBtn_->setText("\xE2\x96\xB6");
        miniPlayPauseBtn_->setText("\xE2\x96\xB6");
        normalStatusLabel_->setText(isWorkMode_ ? "已暂停" : "休息已暂停");
    } else {
        if (remainingSeconds_ == 0) {
            resetTimer(); // Restart if finished
        }
        timer_->start();
        normalPlayPauseBtn_->setText("\xE2\x9D\x9A"); // ❚❚
        miniPlayPauseBtn_->setText("\xE2\x9D\x9A");
        normalStatusLabel_->setText(isWorkMode_ ? "专注中..." : "休息中...");
    }
}

void PomodoroWindow::resetTimer() {
    timer_->stop();
    remainingSeconds_ = isWorkMode_ ? kWorkSeconds : kBreakSeconds;
    normalPlayPauseBtn_->setText("\xE2\x96\xB6");
    miniPlayPauseBtn_->setText("\xE2\x96\xB6");
    normalStatusLabel_->setText(isWorkMode_ ? "准备专注" : "准备休息");
    updateTimeDisplay();
}

void PomodoroWindow::switchMode(const bool isWorkMode) {
    if (isWorkMode_ == isWorkMode) return;
    
    isWorkMode_ = isWorkMode;
    workModeBtn_->setProperty("active", isWorkMode);
    breakModeBtn_->setProperty("active", !isWorkMode);

    // Refresh styling dynamically
    workModeBtn_->style()->unpolish(workModeBtn_);
    workModeBtn_->style()->polish(workModeBtn_);
    breakModeBtn_->style()->unpolish(breakModeBtn_);
    breakModeBtn_->style()->polish(breakModeBtn_);

    resetTimer();
}

void PomodoroWindow::updateTimeDisplay() {
    const QString txt = formatTime(remainingSeconds_);
    normalTimeLabel_->setText(txt);
    miniTimeLabel_->setText(txt);
}

void PomodoroWindow::timerFinished() {
    timer_->stop();
    normalPlayPauseBtn_->setText("\xE2\x96\xB6");
    miniPlayPauseBtn_->setText("\xE2\x96\xB6");
    normalStatusLabel_->setText(isWorkMode_ ? "专注完成！" : "休息结束！");

    // Standard QMessageBox makes the system warning sound automatically
    QMessageBox::information(this, 
                             isWorkMode_ ? "专注结束" : "休息结束", 
                             isWorkMode_ ? "太棒了！休息一下吧。" : "休息结束，可以开始下一个番茄钟了。");
}

void PomodoroWindow::enterMiniMode() {
    isMiniMode_ = true;
    
    // Hide MainWindow gracefully
    if (parentWidget() && parentWidget()->isVisible()) {
        parentWidget()->hide();
    }

    stackedWidget_->setCurrentWidget(miniWidget_);

    // Apply frameless window flags + stays on top
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // Resize to fit the small widget properly
    setFixedSize(180, 56);

    // Re-show window (required after flag changes on Windows)
    show();
}

void PomodoroWindow::exitMiniMode() {
    isMiniMode_ = false;

    // Show MainWindow again
    if (parentWidget() && parentWidget()->isHidden()) {
        parentWidget()->showNormal();
    }

    stackedWidget_->setCurrentWidget(normalWidget_);

    // Restore flags (remove frameless and topmost)
    setWindowFlags(Qt::Window);
    
    // Restore resizability and size
    setMinimumSize(400, 400);
    setMaximumSize(16777215, 16777215); // Unset max fixed limits
    resize(400, 480);

    show();
}

void PomodoroWindow::closeEvent(QCloseEvent* event) {
    // Before dying, if main window was hidden from our mini mode, restore it
    if (parentWidget() && parentWidget()->isHidden()) {
        parentWidget()->showNormal();
    }
    QMainWindow::closeEvent(event);
}

// ── Frameless Dragging Logic (Mini Mode only) ──

void PomodoroWindow::mousePressEvent(QMouseEvent* event) {
    if (isMiniMode_ && event->button() == Qt::LeftButton) {
        dragPosition_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    } else {
        QMainWindow::mousePressEvent(event);
    }
}

void PomodoroWindow::mouseMoveEvent(QMouseEvent* event) {
    if (isMiniMode_ && event->buttons().testFlag(Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition_);
        event->accept();
    } else {
        QMainWindow::mouseMoveEvent(event);
    }
}
