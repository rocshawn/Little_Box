#include "PomodoroWindow.h"

#include "../../core/ThemeManager.h"
#include "../../logic/PomodoroModel.h"
#include "../../services/StorageService.h"

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
#include <QVBoxLayout>

PomodoroWindow::PomodoroWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new PomodoroModel(this)),
      isMiniMode_(false) {
    
    // As a top-level system tool window
    setWindowFlags(Qt::Window);

    setupUi();
    
    connect(model_, &PomodoroModel::timeUpdated, this, &PomodoroWindow::updateUi);
    connect(model_, &PomodoroModel::modeChanged, this, &PomodoroWindow::updateUi);
    connect(model_, &PomodoroModel::stateChanged, this, &PomodoroWindow::updateUi);
    connect(model_, &PomodoroModel::finished, this, &PomodoroWindow::onTimerFinished);

    if (StorageService::instance().hasSession("pomodoro")) {
        model_->restoreSession(StorageService::instance().loadSession("pomodoro"));
    }
    updateUi();
}

void PomodoroWindow::setupUi() {
    setWindowTitle("番茄钟 (MVC)");
    setMinimumSize(400, 400);
    resize(400, 480);

    stackedWidget_ = new QStackedWidget(this);
    setCentralWidget(stackedWidget_);

    setupNormalUi();
    setupMiniUi();

    stackedWidget_->addWidget(normalWidget_);
    stackedWidget_->addWidget(miniWidget_);
    stackedWidget_->setCurrentWidget(normalWidget_);

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

    topControlsRow->addStretch();
    topControlsRow->addWidget(workModeBtn_);
    topControlsRow->addSpacing(10);
    topControlsRow->addWidget(breakModeBtn_);
    topControlsRow->addStretch();
    layout->addLayout(topControlsRow);
    layout->addSpacing(40);

    const bool dark = ThemeManager::instance().isDark();
    auto* timeFrame = new QFrame();
    timeFrame->setFixedSize(260, 260);
    timeFrame->setStyleSheet(dark 
        ? "QFrame { background: #064e3b; border-radius: 130px; border: 8px solid #065f46; }"
        : "QFrame { background: white; border-radius: 130px; border: 8px solid #ccfbf1; }");
    
    auto* timeLayout = new QVBoxLayout(timeFrame);
    timeLayout->setAlignment(Qt::AlignCenter);

    normalStatusLabel_ = new QLabel();
    normalStatusLabel_->setAlignment(Qt::AlignCenter);
    normalStatusLabel_->setStyleSheet(dark ? "color:#5eead4; font-size:16px; font-weight: bold;" : "color:#0d9488; font-size:16px; font-weight: bold;");

    normalTimeLabel_ = new QLabel();
    normalTimeLabel_->setAlignment(Qt::AlignCenter);
    normalTimeLabel_->setStyleSheet(dark ? "color:#fff0f2; font-size:64px; font-weight: 900;" : "color:#0d9488; font-size:64px; font-weight: 900;");
    
    timeLayout->addWidget(normalStatusLabel_);
    timeLayout->addWidget(normalTimeLabel_);
    layout->addWidget(timeFrame, 0, Qt::AlignHCenter);
    layout->addSpacing(40);

    auto* mainControlsRow = new QHBoxLayout();
    normalPlayPauseBtn_ = new QPushButton("\xE2\x96\xB6");
    normalPlayPauseBtn_->setFixedSize(60, 60);
    normalPlayPauseBtn_->setCursor(Qt::PointingHandCursor);
    normalPlayPauseBtn_->setStyleSheet(
        "QPushButton { background: #0d9488; color: white; font-size: 24px; border-radius: 30px; border: none; }"
        "QPushButton:hover { background: #0f766e; }"
    );

    auto* resetBtn = new QPushButton("\xE2\x86\xBB");
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

    auto* bottomRow = new QHBoxLayout();
    auto* miniBtn = new QPushButton("\xE2\x86\x99 小窗模式");
    miniBtn->setCursor(Qt::PointingHandCursor);
    miniBtn->setStyleSheet(dark 
        ? "QPushButton { background: transparent; color: #99f6e4; font-weight: bold; font-size: 13px; border: none; }"
          "QPushButton:hover { color: #ccfbf1; }"
        : "QPushButton { background: transparent; color: #0d9488; font-weight: bold; font-size: 13px; border: none; }"
          "QPushButton:hover { color: #0f766e; }");
    bottomRow->addStretch();
    bottomRow->addWidget(miniBtn);
    layout->addLayout(bottomRow);

    connect(workModeBtn_, &QPushButton::clicked, this, [this]{ switchMode(true); });
    connect(breakModeBtn_, &QPushButton::clicked, this, [this]{ switchMode(false); });
    connect(normalPlayPauseBtn_, &QPushButton::clicked, this, [this]{ model_->toggle(); });
    connect(resetBtn, &QPushButton::clicked, this, [this]{ 
        StorageService::instance().clearSession("pomodoro");
        model_->reset(); 
    });
    connect(miniBtn, &QPushButton::clicked, this, &PomodoroWindow::enterMiniMode);
}

void PomodoroWindow::setupMiniUi() {
    miniWidget_ = new QWidget(this);
    const bool dark = ThemeManager::instance().isDark();
    miniWidget_->setStyleSheet(dark 
        ? "QWidget { background: #064e3b; border: 2px solid #065f46; border-radius: 20px; }"
        : "QWidget { background: white; border: 2px solid #ccfbf1; border-radius: 20px; }");

    auto* layout = new QHBoxLayout(miniWidget_);
    layout->setContentsMargins(14, 8, 14, 8);
    layout->setSpacing(12);

    miniTimeLabel_ = new QLabel(miniWidget_);
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

    auto* exitMiniBtn = new QPushButton("\xE2\x86\x97", miniWidget_);
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

    connect(miniPlayPauseBtn_, &QPushButton::clicked, this, [this]{ model_->toggle(); });
    connect(exitMiniBtn, &QPushButton::clicked, this, &PomodoroWindow::exitMiniMode);
}

void PomodoroWindow::onTimerFinished() {
    StorageService::instance().clearSession("pomodoro");
    QMessageBox::information(this, 
                             model_->isWorkMode() ? "专注结束" : "休息结束", 
                             model_->isWorkMode() ? "太棒了！休息一下吧。" : "休息结束，可以开始下一个番茄钟了。");
}

void PomodoroWindow::switchMode(bool isWork) {
    model_->setMode(isWork);
}

void PomodoroWindow::updateUi() {
    const QString clockTxt = model_->formattedTime();
    normalTimeLabel_->setText(clockTxt);
    miniTimeLabel_->setText(clockTxt);
    normalStatusLabel_->setText(model_->statusText());

    // Play/Pause Icon (▶ ❚❚)
    const QString playIcon = model_->isRunning() ? "\xE2\x9D\x9A" : "\xE2\x96\xB6";
    normalPlayPauseBtn_->setText(playIcon);
    miniPlayPauseBtn_->setText(playIcon);

    // Mode Buttons Property
    workModeBtn_->setProperty("active", model_->isWorkMode());
    breakModeBtn_->setProperty("active", !model_->isWorkMode());
    workModeBtn_->style()->unpolish(workModeBtn_);
    workModeBtn_->style()->polish(workModeBtn_);
    breakModeBtn_->style()->unpolish(breakModeBtn_);
    breakModeBtn_->style()->polish(breakModeBtn_);
}

void PomodoroWindow::enterMiniMode() {
    isMiniMode_ = true;
    if (parentWidget() && parentWidget()->isVisible()) parentWidget()->hide();
    stackedWidget_->setCurrentWidget(miniWidget_);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setFixedSize(180, 56);
    show();
}

void PomodoroWindow::exitMiniMode() {
    isMiniMode_ = false;
    if (parentWidget() && parentWidget()->isHidden()) parentWidget()->showNormal();
    stackedWidget_->setCurrentWidget(normalWidget_);
    setWindowFlags(Qt::Window);
    setMinimumSize(400, 400);
    setMaximumSize(16777215, 16777215);
    resize(400, 480);
    show();
}

void PomodoroWindow::closeEvent(QCloseEvent* event) {
    if (parentWidget() && parentWidget()->isHidden()) parentWidget()->showNormal();
    
    StorageService::instance().saveSession("pomodoro", model_->saveSession());
    
    QMainWindow::closeEvent(event);
}

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
