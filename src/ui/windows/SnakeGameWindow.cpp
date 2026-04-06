#include "SnakeGameWindow.h"

#include "../../core/ThemeManager.h"

#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

SnakeGameWindow::SnakeGameWindow(QWidget* parent)
    : QMainWindow(parent),
      currentDir_(Direction::Right),
      nextDir_(Direction::Right),
      snake_(),
      food_(),
      score_(0),
      isGameOver_(false),
      canUpdateDir_(true),
      timer_{ nullptr } {
    setupUi();
    resetGame();

    timer_ = new QTimer(this);
    timer_->setInterval(150); // Standard speed
    connect(timer_, &QTimer::timeout, this, &SnakeGameWindow::updateGame);
    timer_->start();
}

void SnakeGameWindow::setupUi() {
    setWindowTitle("贪吃蛇 - 果冻版");
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
    auto* scoreLabel = new QLabel(this);
    scoreLabel->setObjectName("scoreLabel");
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setStyleSheet(dark 
        ? "color:#fff0f2; font-size:20px; font-weight:800;" 
        : "color:#4c0519; font-size:20px; font-weight:800;");
    layout->addWidget(scoreLabel);
    
    layout->addStretch();
}

void SnakeGameWindow::resetGame() {
    snake_.clear();
    snake_.append({ kGridSize / 2, kGridSize / 2 });
    snake_.append({ kGridSize / 2 - 1, kGridSize / 2 });
    snake_.append({ kGridSize / 2 - 2, kGridSize / 2 });

    currentDir_ = Direction::Right;
    nextDir_ = Direction::Right;
    score_ = 0;
    isGameOver_ = false;

    spawnFood();
}

void SnakeGameWindow::spawnFood() {
    while (true) {
        int x = QRandomGenerator::global()->bounded(kGridSize);
        int y = QRandomGenerator::global()->bounded(kGridSize);
        bool collision = false;
        for (const auto& part : snake_) {
            if (part.x() == x && part.y() == y) {
                collision = true;
                break;
            }
        }
        if (!collision) {
            food_ = { x, y };
            break;
        }
    }
}

void SnakeGameWindow::updateGame() {
    if (isGameOver_) {
        timer_->stop();
        return;
    }

    canUpdateDir_ = true; // Reset lock every frame
    moveSnake();
    checkCollision();
    
    // Update score label
    if (auto* lbl = centralWidget()->findChild<QLabel*>("scoreLabel")) {
        lbl->setText(QString("SCORE: %1").arg(score_));
    }
    
    update();
}

void SnakeGameWindow::moveSnake() {
    currentDir_ = nextDir_;
    QPoint head = snake_.front();
    
    switch (currentDir_) {
        case Direction::Up:    head.rx()--; break;
        case Direction::Down:  head.rx()++; break;
        case Direction::Left:  head.ry()--; break;
        case Direction::Right: head.ry()++; break;
    }

    // Wrap around logic or wall collision? Let's go with wall collision for a challenge.
    if (head.x() < 0 || head.x() >= kGridSize || head.y() < 0 || head.y() >= kGridSize) {
        isGameOver_ = true;
        return;
    }

    // Self-collision
    for (const auto& part : snake_) {
        if (part == head) {
            isGameOver_ = true;
            return;
        }
    }

    snake_.push_front(head);

    if (head == food_) {
        score_ += 10;
        spawnFood();
        // Speed up a bit?
        if (timer_->interval() > 70) {
            timer_->setInterval(timer_->interval() - 2);
        }
    } else {
        snake_.pop_back();
    }
}

void SnakeGameWindow::checkCollision() {
    // Already checked in moveSnake
}

void SnakeGameWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance().isDark();
    const QColor gridBg  = dark ? QColor(58, 28, 54) : QColor(255, 241, 242);
    const QColor snakeHead = dark ? QColor(251, 113, 133) : QColor(244, 63, 94);
    const QColor snakeBody = dark ? QColor(225, 29, 72, 180) : QColor(251, 113, 133, 180);
    const QColor foodColor = QColor(255, 204, 0); // Apple goldish

    const int offsetX = 30;
    const int offsetY = 80;

    // Draw Grid Background
    painter.setPen(Qt::NoPen);
    painter.setBrush(gridBg);
    painter.drawRoundedRect(offsetX - 5, offsetY - 5, kGridSize * kCellSize + 10, kGridSize * kCellSize + 10, 16, 16);

    // Draw Snake
    for (int i = 0; i < snake_.size(); ++i) {
        const auto& part = snake_[i];
        if (i == 0) painter.setBrush(snakeHead);
        else painter.setBrush(snakeBody);
        
        painter.drawRoundedRect(offsetX + part.y() * kCellSize, 
                                offsetY + part.x() * kCellSize, 
                                kCellSize - 2, kCellSize - 2, 8, 8);
    }

    // Draw Food
    painter.setBrush(foodColor);
    painter.drawRoundedRect(offsetX + food_.y() * kCellSize, 
                            offsetY + food_.x() * kCellSize, 
                            kCellSize - 2, kCellSize - 2, 10, 10);
    
    // Game Over Overlay
    if (isGameOver_) {
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
    if (!canUpdateDir_) return;

    Direction potentialDir = currentDir_;
    switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_W: if (currentDir_ != Direction::Down) potentialDir = Direction::Up; break;
        case Qt::Key_Down:
        case Qt::Key_S: if (currentDir_ != Direction::Up) potentialDir = Direction::Down; break;
        case Qt::Key_Left:
        case Qt::Key_A: if (currentDir_ != Direction::Right) potentialDir = Direction::Left; break;
        case Qt::Key_Right:
        case Qt::Key_D: if (currentDir_ != Direction::Left) potentialDir = Direction::Right; break;
        case Qt::Key_R: if (isGameOver_) { resetGame(); timer_->start(); update(); } return;
        default: QMainWindow::keyPressEvent(event); return;
    }

    if (potentialDir != currentDir_) {
        nextDir_ = potentialDir;
        canUpdateDir_ = false; // Lock until next frame
    }
}
