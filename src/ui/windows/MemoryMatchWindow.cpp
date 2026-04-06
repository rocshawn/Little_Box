#include "MemoryMatchWindow.h"

#include "../../core/ThemeManager.h"

#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <algorithm>
#include <random>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

MemoryMatchWindow::MemoryMatchWindow(QWidget* parent)
    : QMainWindow(parent),
      cards_(),
      firstFlippedIdx_(-1),
      secondFlippedIdx_(-1),
      isProcessing_(false),
      matchesFound_(0) {
    setupUi();
    resetGame();
}

void MemoryMatchWindow::setupUi() {
    setWindowTitle("记忆翻牌 - 甜心挑战");
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
    const QStringList emojiPool = {
        "🍓", "🍑", "🍋", "🍒", "🥝", "🍩", "🧁", "🍭",
        "🧸", "🎈", "🎨", "🧩", "🦄", "🌈", "🦋", "🌸"
    };

    int numPairs = (kRows * kCols) / 2;
    QStringList gameEmojis;
    for (int i = 0; i < numPairs; ++i) {
        gameEmojis.append(emojiPool[i % emojiPool.size()]);
        gameEmojis.append(emojiPool[i % emojiPool.size()]);
    }

    // Shuffle manually
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(gameEmojis.begin(), gameEmojis.end(), g);

    cards_.clear();
    for (const auto& emoji : gameEmojis) {
        cards_.append({ emoji, false, false });
    }

    matchesFound_ = 0;
    firstFlippedIdx_ = -1;
    secondFlippedIdx_ = -1;
    isProcessing_ = false;

    update();
}

void MemoryMatchWindow::checkMatch() {
    if (cards_[firstFlippedIdx_].emoji == cards_[secondFlippedIdx_].emoji) {
        cards_[firstFlippedIdx_].isMatched = true;
        cards_[secondFlippedIdx_].isMatched = true;
        matchesFound_++;
        if (matchesFound_ == (kRows * kCols) / 2) {
            if (auto status = centralWidget()->findChild<QLabel*>("statusLabel")) {
                status->setText("全部匹配！真厉害 🎉");
            }
        }
    } else {
        cards_[firstFlippedIdx_].isFlipped = false;
        cards_[secondFlippedIdx_].isFlipped = false;
    }

    firstFlippedIdx_ = -1;
    secondFlippedIdx_ = -1;
    isProcessing_ = false;
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

    const int offsetX = 50;
    const int offsetY = 140;

    for (int i = 0; i < cards_.size(); ++i) {
        int r = i / kCols;
        int c = i % kCols;
        QRect rect(offsetX + c * (kCellSize + 10), offsetY + r * (kCellSize + 10), kCellSize, kCellSize);
        const auto& card = cards_[i];

        if (card.isMatched || card.isFlipped) {
            painter.setBrush(cardFrontBg);
            painter.setPen(QPen(borderCol, 2));
            painter.drawRoundedRect(rect, 14, 14);
            
            QFont f = painter.font(); f.setPointSize(36); painter.setFont(f);
            painter.drawText(rect, Qt::AlignCenter, card.emoji);
        } else {
            painter.setBrush(cardBackBg);
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect, 14, 14);
            
            painter.setPen(dark ? QColor(251, 113, 133, 100) : QColor(244, 63, 94, 100));
            QFont f = painter.font(); f.setPointSize(28); f.setBold(true); painter.setFont(f);
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

    if (idx >= 0 && idx < cards_.size()) {
        auto& card = cards_[idx];
        
        if (card.isMatched || card.isFlipped) return;

        card.isFlipped = true;
        if (firstFlippedIdx_ == -1) {
            firstFlippedIdx_ = idx;
        } else {
            secondFlippedIdx_ = idx;
            isProcessing_ = true;
            QTimer::singleShot(800, this, &MemoryMatchWindow::checkMatch);
        }
        update();
    }
}
