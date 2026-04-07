#include "MemoryMatchWindow.h"

#include "../../core/ThemeManager.h"
#include "../../logic/MemoryMatchModel.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

MemoryMatchWindow::MemoryMatchWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new MemoryMatchModel(this)),
      firstFlippedIdx_(-1),
      secondFlippedIdx_(-1),
      isProcessing_(false) {
    setupUi();
    
    connect(model_, &MemoryMatchModel::updated, this, &MemoryMatchWindow::updateUi);
    connect(model_, &MemoryMatchModel::matchResult, this, &MemoryMatchWindow::onMatchResult);
    connect(model_, &MemoryMatchModel::gameOver, this, &MemoryMatchWindow::onGameOver);

    updateUi();
}

void MemoryMatchWindow::setupUi() {
    setWindowTitle("记忆翻牌 - 甜心挑战 (MVC)");
    setFixedSize(kCols * (kCellSize + 10) + 100, kRows * (kCellSize + 10) + 180);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(50, 30, 50, 30);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto* statusLabel = new QLabel("找到所有的配对！", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(dark 
        ? "color:#fbcfe8; font-size:20px; font-weight:800;" 
        : "color:#fb7185; font-size:20px; font-weight:800;");
    layout->addWidget(statusLabel);

    auto* resetBtn = new QPushButton("重新洗牌", this);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(dark 
        ? "QPushButton { background:#881337; color:white; border-radius:12px; padding:6px 16px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#9f1239; }"
        : "QPushButton { background:#fb7185; color:white; border-radius:12px; padding:6px 16px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#f43f5e; }");
    connect(resetBtn, &QPushButton::clicked, this, &MemoryMatchWindow::resetGame);
    layout->addWidget(resetBtn);

    layout->addStretch();
}

void MemoryMatchWindow::resetGame() {
    model_->reset();
    firstFlippedIdx_ = -1;
    secondFlippedIdx_ = -1;
    isProcessing_ = false;
    if (auto status = centralWidget()->findChild<QLabel*>("statusLabel")) {
        status->setText("找到所有的配对！");
    }
}

void MemoryMatchWindow::onMatchResult(bool success, int idx1, int idx2) {
    if (success) {
        firstFlippedIdx_ = -1;
        secondFlippedIdx_ = -1;
        isProcessing_ = false;
    } else {
        QTimer::singleShot(800, this, [this, idx1, idx2]() {
            model_->resetFlippedCards(idx1, idx2);
            firstFlippedIdx_ = -1;
            secondFlippedIdx_ = -1;
            isProcessing_ = false;
        });
    }
}

void MemoryMatchWindow::onGameOver() {
    if (auto status = centralWidget()->findChild<QLabel*>("statusLabel")) {
        status->setText("全部匹配！真厉害 🎉");
    }
}

void MemoryMatchWindow::updateUi() {
    update();
}

void MemoryMatchWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance().isDark();
    const QColor cardBackBg  = dark ? QColor(76, 5, 25) : QColor(255, 228, 230);
    const QColor cardFrontBg = dark ? QColor(58, 28, 54) : QColor(255, 250, 255);
    const QColor borderCol   = dark ? QColor(94, 44, 86) : QColor(254, 205, 211);

    const int offsetX = 50, offsetY = 140;
    const auto& cards = model_->cards();

    for (int i = 0; i < cards.size(); ++i) {
        int r = i / kCols, c = i % kCols;
        QRect rect(offsetX + c * (kCellSize + 10), offsetY + r * (kCellSize + 10), kCellSize, kCellSize);
        const auto& card = cards[i];

        if (card.isMatched || card.isFlipped) {
            painter.setBrush(cardFrontBg);
            painter.setPen(QPen(borderCol, 2));
            painter.drawRoundedRect(rect, 14, 14);
            painter.drawText(rect, Qt::AlignCenter, card.emoji);
        } else {
            painter.setBrush(cardBackBg);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect, 14, 14);
            painter.setPen(dark ? QColor(251, 113, 133, 100) : QColor(244, 63, 94, 100));
            painter.drawText(rect, Qt::AlignCenter, "?");
        }
    }
}

void MemoryMatchWindow::mousePressEvent(QMouseEvent* event) {
    if (isProcessing_) return;

    int offsetX = 50, offsetY = 140;
    int c = (event->pos().x() - offsetX) / (kCellSize + 10);
    int r = (event->pos().y() - offsetY) / (kCellSize + 10);
    int idx = r * kCols + c;

    if (idx >= 0 && idx < model_->cards().size()) {
        if (model_->attemptFlip(idx)) {
            if (firstFlippedIdx_ == -1) {
                firstFlippedIdx_ = idx;
            } else {
                secondFlippedIdx_ = idx;
                isProcessing_ = true;
                model_->checkMatch(firstFlippedIdx_, secondFlippedIdx_);
            }
        }
    }
}
