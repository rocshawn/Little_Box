#include "MinesweeperWindow.h"
#include "../../core/ThemeManager.h"
#include <cmath>
#include <QPainter>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

MinesweeperWindow::MinesweeperWindow(QWidget* parent)
    : QMainWindow(parent),
      board_(),
      isGameOver_(false),
      isWin_(false),
      isFirstClick_(true),
      flagsUsed_(0) {
    setupUi();
    resetGame();
}

void MinesweeperWindow::setupUi() {
    setWindowTitle("扫雷 - 甜心幻想");
    setFixedSize(kCols * kCellSize + 80, kRows * kCellSize + 160);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(40, 20, 40, 20);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto* statusLabel = new QLabel("🚩: 0 / 💣: 12", this);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet(dark 
        ? "color:#fbcfe8; font-size:18px; font-weight:800;" 
        : "color:#fb7185; font-size:18px; font-weight:800;");
    layout->addWidget(statusLabel);

    auto* resetBtn = new QPushButton("重新生成地雷", this);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(dark 
        ? "QPushButton { background:#881337; color:white; border-radius:12px; padding:6px 12px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#9f1239; }"
        : "QPushButton { background:#fb7185; color:white; border-radius:12px; padding:6px 12px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#f43f5e; }");
    connect(resetBtn, &QPushButton::clicked, this, &MinesweeperWindow::resetGame);
    layout->addWidget(resetBtn);

    layout->addStretch();
}

void MinesweeperWindow::resetGame() {
    board_ = QVector<QVector<Cell>>(kRows, QVector<Cell>(kCols));
    isGameOver_ = false;
    isWin_ = false;
    isFirstClick_ = true;
    flagsUsed_ = 0;
    update();
}

void MinesweeperWindow::generateMines(int excludeR, int excludeC) {
    // Place mines randomly
    int m = 0;
    while (m < kMines) {
        int r = QRandomGenerator::global()->bounded(kRows);
        int c = QRandomGenerator::global()->bounded(kCols);
        
        // Don't place mine on the first clicked cell or its immediate neighbors
        // to give a small safe opening area.
        if (std::abs(r - excludeR) <= 1 && std::abs(c - excludeC) <= 1) continue;

        if (!board_[r][c].isMine) {
            board_[r][c].isMine = true;
            m++;
        }
    }

    // Calculate numbers
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (!board_[r][c].isMine) {
                board_[r][c].adjacentMines = countMines(r, c);
            }
        }
    }
    isFirstClick_ = false;
}

int MinesweeperWindow::countMines(int r, int c) const {
    int n = 0;
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            int nr = r + dr, nc = c + dc;
            if (nr >= 0 && nr < kRows && nc >= 0 && nc < kCols && board_[nr][nc].isMine) n++;
        }
    }
    return n;
}

void MinesweeperWindow::revealCell(int r, int c) {
    if (r < 0 || r >= kRows || c < 0 || c >= kCols) return;
    if (board_[r][c].isRevealed || board_[r][c].isFlagged) return;

    board_[r][c].isRevealed = true;
    if (board_[r][c].isMine) {
        explodeMine(r, c);
        return;
    }

    if (board_[r][c].adjacentMines == 0) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                revealCell(r + dr, c + dc);
            }
        }
    }
    checkWin();
}

void MinesweeperWindow::explodeMine(int r, int c) {
    isGameOver_ = true;
    for (int i = 0; i < kRows; ++i) {
        for (int j = 0; j < kCols; ++j) {
            if (board_[i][j].isMine) board_[i][j].isRevealed = true;
        }
    }
}

void MinesweeperWindow::checkWin() {
    int revealedCount = 0;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            if (board_[r][c].isRevealed && !board_[r][c].isMine) revealedCount++;
        }
    }
    if (revealedCount == (kRows * kCols - kMines)) {
        isWin_ = true;
        isGameOver_ = true;
    }
}

void MinesweeperWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance().isDark();
    const QColor unrevealedBg = dark ? QColor(76, 5, 25) : QColor(255, 228, 230);
    const QColor revealedBg   = dark ? QColor(58, 28, 54) : QColor(255, 250, 255);
    const QColor flagColor     = QColor(244, 63, 94);
    const QColor mineColor     = QColor(31, 41, 55);

    const int offsetX = 40;
    const int offsetY = 120;

    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            QRect rect(offsetX + c * kCellSize, offsetY + r * kCellSize, kCellSize - 4, kCellSize - 4);
            const Cell& cell = board_[r][c];

            if (!cell.isRevealed) {
                painter.setBrush(unrevealedBg);
                painter.setPen(Qt::NoPen);
                painter.drawRoundedRect(rect, 8, 8);
                if (cell.isFlagged) {
                    painter.setPen(flagColor);
                    painter.drawText(rect, Qt::AlignCenter, "🚩");
                }
            } else {
                painter.setBrush(revealedBg);
                painter.setPen(dark ? QColor(94, 44, 86) : QColor(253, 164, 175));
                painter.drawRoundedRect(rect, 8, 8);

                if (cell.isMine) {
                    painter.drawText(rect, Qt::AlignCenter, "💣");
                } else if (cell.adjacentMines > 0) {
                    painter.setPen(dark ? QColor(251, 113, 133) : QColor(225, 29, 72));
                    painter.drawText(rect, Qt::AlignCenter, QString::number(cell.adjacentMines));
                }
            }
        }
    }

    if (isGameOver_) {
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.drawRect(this->rect());
        painter.setPen(Qt::white);
        QFont f = painter.font(); f.setPointSize(24); f.setBold(true); painter.setFont(f);
        painter.drawText(this->rect(), Qt::AlignCenter, isWin_ ? "太棒了！你赢了 🎉" : "轰隆！踩到雷了 💥");
    }
    
    if (auto status = centralWidget()->findChild<QLabel*>("statusLabel")) {
        status->setText(QString("🚩: %1 / 💣: %2").arg(flagsUsed_).arg(kMines));
    }
}

void MinesweeperWindow::mousePressEvent(QMouseEvent* event) {
    if (isGameOver_) { resetGame(); return; }

    int offsetX = 40, offsetY = 120;
    int c = (event->pos().x() - offsetX) / kCellSize;
    int r = (event->pos().y() - offsetY) / kCellSize;

    if (r >= 0 && r < kRows && c >= 0 && c < kCols) {
        if (event->button() == Qt::LeftButton) {
            if (isFirstClick_) {
                generateMines(r, c);
            }
            revealCell(r, c);
        } else if (event->button() == Qt::RightButton) {
            if (!isFirstClick_) { // Can't flag before starting
                if (!board_[r][c].isRevealed) {
                    board_[r][c].isFlagged = !board_[r][c].isFlagged;
                    flagsUsed_ += board_[r][c].isFlagged ? 1 : -1;
                }
            }
        }
        update();
    }
}
