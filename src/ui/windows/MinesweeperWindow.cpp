#include "MinesweeperWindow.h"

#include "../../core/ThemeManager.h"
#include "../../logic/MinesweeperModel.h"
#include "../../services/StorageService.h"
#include "../widgets/SessionOverlayWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

MinesweeperWindow::MinesweeperWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new MinesweeperModel(this)) {
    setupUi();
    
    connect(model_, &MinesweeperModel::updated, this, &MinesweeperWindow::updateUi);
    connect(model_, &MinesweeperModel::flagsChanged, this, &MinesweeperWindow::onFlagsChanged);
    connect(model_, &MinesweeperModel::gameOver, this, &MinesweeperWindow::onGameOver);

    if (StorageService::instance().hasSession("minesweeper")) {
        model_->restoreSession(StorageService::instance().loadSession("minesweeper"));
        showOverlay();
    } else {
        updateUi();
    }
}

void MinesweeperWindow::setupUi() {
    setWindowTitle("扫雷 - 甜心幻想 (MVC)");
    setFixedSize(model_->cols() * kCellSize + 80, model_->rows() * kCellSize + 160);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #1a0b1c, stop:1 #2d142c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fffaff, stop:1 #fff1f2); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(40, 20, 40, 20);
    layout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    auto* statusLabel = new QLabel(QString("🚩: 0 / 💣: %1").arg(model_->totalMines()), this);
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
    StorageService::instance().clearSession("minesweeper");
    model_->reset();
}

void MinesweeperWindow::onFlagsChanged(int count) {
    if (auto status = centralWidget()->findChild<QLabel*>("statusLabel")) {
        status->setText(QString("🚩: %1 / 💣: %2").arg(count).arg(model_->totalMines()));
    }
}

void MinesweeperWindow::onGameOver(bool win) {
    StorageService::instance().clearSession("minesweeper");
    update();
}

void MinesweeperWindow::updateUi() {
    update();
}

void MinesweeperWindow::showOverlay() {
    model_->pause();
    if (!overlay_) {
        overlay_ = new SessionOverlayWidget(this);
        connect(overlay_, &SessionOverlayWidget::continueRequested, this, &MinesweeperWindow::hideOverlay);
        connect(overlay_, &SessionOverlayWidget::restartRequested, this, [this]() {
            hideOverlay();
            resetGame();
        });
    }
    overlay_->show();
    overlay_->raise();
}

void MinesweeperWindow::hideOverlay() {
    model_->resume();
    if (overlay_) {
        overlay_->hide();
    }
}

void MinesweeperWindow::closeEvent(QCloseEvent* event) {
    if (!model_->isGameOver()) {
        StorageService::instance().saveSession("minesweeper", model_->saveSession());
    }
    QMainWindow::closeEvent(event);
}

void MinesweeperWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance().isDark();
    const QColor unrevealedBg = dark ? QColor(76, 5, 25) : QColor(255, 228, 230);
    const QColor revealedBg   = dark ? QColor(58, 28, 54) : QColor(255, 250, 255);
    const QColor flagColor     = QColor(244, 63, 94);

    const int offsetX = 40, offsetY = 120;
    const auto& board = model_->board();

    for (int r = 0; r < model_->rows(); ++r) {
        for (int c = 0; c < model_->cols(); ++c) {
            QRect rect(offsetX + c * kCellSize, offsetY + r * kCellSize, kCellSize - 4, kCellSize - 4);
            const auto& cell = board[r][c];

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

    if (model_->isGameOver()) {
        painter.setBrush(QColor(0, 0, 0, 100));
        painter.drawRect(this->rect());
        painter.setPen(Qt::white);
        QFont f = painter.font(); f.setPointSize(24); f.setBold(true); painter.setFont(f);
        painter.drawText(this->rect(), Qt::AlignCenter, model_->isWin() ? "太棒了！你赢了 🎉" : "轰隆！踩到雷了 💥");
    }
}

void MinesweeperWindow::mousePressEvent(QMouseEvent* event) {
    if (overlay_ && !overlay_->isHidden()) {
        return;
    }

    if (model_->isGameOver()) { 
        resetGame();
        return;
    }

    int offsetX = 40, offsetY = 120;
    int c = (event->pos().x() - offsetX) / kCellSize;
    int r = (event->pos().y() - offsetY) / kCellSize;

    if (r >= 0 && r < model_->rows() && c >= 0 && c < model_->cols()) {
        if (event->button() == Qt::LeftButton) {
            model_->revealCell(r, c);
        } else if (event->button() == Qt::RightButton) {
            model_->toggleFlag(r, c);
        }
    }
}
