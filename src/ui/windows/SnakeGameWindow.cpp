#include "SnakeGameWindow.h"

#include "../../core/ThemeManager.h"
#include "../../logic/SnakeGameModel.h"
#include "../../services/StorageService.h"
#include "../widgets/SessionOverlayWidget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>

SnakeGameWindow::SnakeGameWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new SnakeGameModel(kGridSize, this)),
      timer_(new QTimer(this)) {
      
    setupUi();

    connect(model_, &SnakeGameModel::updated, this, &SnakeGameWindow::onModelUpdated);
    connect(model_, &SnakeGameModel::scoreChanged, this, &SnakeGameWindow::onScoreChanged);
    connect(model_, &SnakeGameModel::gameOver, this, &SnakeGameWindow::onGameOver);

    timer_->setInterval(150);
    connect(timer_, &QTimer::timeout, model_, &SnakeGameModel::update);
    
    if (StorageService::instance().hasSession("snake_game")) {
        model_->restoreSession(StorageService::instance().loadSession("snake_game"));
        showOverlay();
        update(); // Ensure rendering covers initial state
    } else {
        startNewGame();
    }
}

void SnakeGameWindow::setupUi() {
    setWindowTitle("贪吃蛇 - 果冻版 (MVC)");
    setFixedSize(kGridSize * kCellSize + 60, kGridSize * kCellSize + 120);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(30, 20, 30, 20);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // Score Label
    scoreLabel_ = new QLabel(this);
    scoreLabel_->setObjectName("scoreLabel");
    scoreLabel_->setAlignment(Qt::AlignCenter);
    scoreLabel_->setStyleSheet(dark 
        ? "color:#fff0f2; font-size:20px; font-weight:800;" 
        : "color:#4c0519; font-size:20px; font-weight:800;");
    layout->addWidget(scoreLabel_);
    
    layout->addStretch();
}

void SnakeGameWindow::startNewGame() {
    StorageService::instance().clearSession("snake_game");
    model_->reset();
    timer_->start();
    update();
}

void SnakeGameWindow::onModelUpdated() {
    update();
}

void SnakeGameWindow::onScoreChanged(int score) {
    if (scoreLabel_) {
        scoreLabel_->setText(QString("SCORE: %1").arg(score));
    }
    
    // Speed up logic (now handled in UI because it affects the Timer interval)
    if (timer_ && timer_->interval() > 70) {
        timer_->setInterval(150 - (score / 10) * 2);
    }
}

void SnakeGameWindow::onGameOver() {
    StorageService::instance().clearSession("snake_game");
    timer_->stop();
}

void SnakeGameWindow::showOverlay() {
    if (!overlay_) {
        overlay_ = new SessionOverlayWidget(this);
        connect(overlay_, &SessionOverlayWidget::continueRequested, this, &SnakeGameWindow::hideOverlay);
        connect(overlay_, &SessionOverlayWidget::restartRequested, this, [this]() {
            hideOverlay();
            startNewGame();
        });
    }
    overlay_->show();
    overlay_->raise();
}

void SnakeGameWindow::hideOverlay() {
    if (overlay_) {
        overlay_->hide();
    }
    if (!model_->isGameOver()) {
        timer_->start(); // Resume timer
    }
}

void SnakeGameWindow::closeEvent(QCloseEvent* event) {
    if (!model_->isGameOver()) {
        StorageService::instance().saveSession("snake_game", model_->saveSession());
    }
    QMainWindow::closeEvent(event);
}

void SnakeGameWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance().isDark();
    const QColor gridBg  = dark ? QColor(58, 28, 54) : QColor(255, 241, 242);
    const QColor snakeHead = dark ? QColor(251, 113, 133) : QColor(244, 63, 94);
    const QColor snakeBody = dark ? QColor(225, 29, 72, 180) : QColor(251, 113, 133, 180);
    const QColor foodColor = QColor(255, 204, 0);

    const int offsetX = 30;
    const int offsetY = 80;

    // Draw Grid Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(gridBg);
    painter.drawRoundedRect(offsetX - 5, offsetY - 5, kGridSize * kCellSize + 10, kGridSize * kCellSize + 10, 16, 16);

    // Draw Snake (from model)
    const auto& snake = model_->snake();
    for (int i = 0; i < snake.size(); ++i) {
        const auto& part = snake[i];
        if (i == 0) painter.setBrush(snakeHead);
        else painter.setBrush(snakeBody);
        
        painter.drawRoundedRect(offsetX + part.y() * kCellSize, 
                                offsetY + part.x() * kCellSize, 
                                kCellSize - 2, kCellSize - 2, 8, 8);
    }

    // Draw Food (from model)
    const auto& food = model_->food();
    painter.setBrush(foodColor);
    painter.drawRoundedRect(offsetX + food.y() * kCellSize, 
                            offsetY + food.x() * kCellSize, 
                            kCellSize - 2, kCellSize - 2, 10, 10);
    
    // Game Over Overlay
    if (model_->isGameOver()) {
        painter.setBrush(QColor(0, 0, 0, 120));
        painter.drawRect(rect());
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(32);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter, "GAME OVER\n[R] 重新开始");
    }
}

void SnakeGameWindow::keyPressEvent(QKeyEvent* event) {
    if (overlay_ && !overlay_->isHidden()) {
        return; // Block key events if overlay is visible
    }

    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W: model_->setDirection(SnakeGameModel::Direction::Up); break;
        case Qt::Key_Down:
        case Qt::Key_S: model_->setDirection(SnakeGameModel::Direction::Down); break;
        case Qt::Key_Left:
        case Qt::Key_A: model_->setDirection(SnakeGameModel::Direction::Left); break;
        case Qt::Key_Right:
        case Qt::Key_D: model_->setDirection(SnakeGameModel::Direction::Right); break;
        case Qt::Key_R: 
            if (model_->isGameOver()) { 
                startNewGame(); 
            } 
            break;
        default: QMainWindow::keyPressEvent(event); break;
    }
}
