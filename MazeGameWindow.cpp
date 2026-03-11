#include "MazeGameWindow.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

namespace {
constexpr int kBoardMargin = 18;
const QColor kBackgroundColor("#eef2ff");
const QColor kWallColor("#312e81");
const QColor kPathColor("#f8fafc");
const QColor kExitColor("#22c55e");
const QColor kPlayerColor("#f97316");
} // namespace


MazeBoard::MazeBoard(QWidget* parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(460, 460);
    generateLevel();
}

void MazeBoard::setLevel(const int level) {
    level_ = level;
    generateLevel();
    setFocus();
}

int MazeBoard::currentLevel() const noexcept {
    return level_;
}

void MazeBoard::restartLevel() {
    generateLevel();
    setFocus();
}

void MazeBoard::setLevelCompletedCallback(std::function<void()> callback) {
    onLevelCompleted_ = callback;
}


void MazeBoard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), kBackgroundColor);

    if (walls_.isEmpty()) {
        return;
    }

    const int rows = walls_.size();
    const int columns = walls_.front().size();
    const qreal cellSize = std::min((width() - kBoardMargin * 2.0) / columns, (height() - kBoardMargin * 2.0) / rows);
    const qreal offsetX = (width() - columns * cellSize) / 2.0;
    const qreal offsetY = (height() - rows * cellSize) / 2.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(QRectF(offsetX - 10, offsetY - 10, columns * cellSize + 20, rows * cellSize + 20), 18, 18);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const QRectF cellRect(offsetX + column * cellSize, offsetY + row * cellSize, cellSize, cellSize);
            painter.setBrush(walls_[row][column] ? kWallColor : kPathColor);
            painter.drawRect(cellRect);

            if (QPoint(column, row) == exitCell_) {
                painter.setBrush(kExitColor);
                painter.drawRoundedRect(cellRect.adjusted(cellSize * 0.16, cellSize * 0.16, -cellSize * 0.16, -cellSize * 0.16), 8, 8);
            }
        }
    }

    const QRectF playerRect(
        offsetX + playerCell_.x() * cellSize + cellSize * 0.18,
        offsetY + playerCell_.y() * cellSize + cellSize * 0.18,
        cellSize * 0.64,
        cellSize * 0.64);

    painter.setBrush(kPlayerColor);
    painter.drawEllipse(playerRect);
}

void MazeBoard::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Up:
        tryMove(0, -1);
        break;
    case Qt::Key_Down:
        tryMove(0, 1);
        break;
    case Qt::Key_Left:
        tryMove(-1, 0);
        break;
    case Qt::Key_Right:
        tryMove(1, 0);
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();
}

void MazeBoard::generateLevel() {
    const int size = boardSizeForLevel(level_);
    walls_ = QVector<QVector<bool>>(size, QVector<bool>(size, true));

    QVector<QPoint> stack;
    playerCell_ = QPoint(1, 1);
    walls_[playerCell_.y()][playerCell_.x()] = false;
    stack.append(playerCell_);

    while (!stack.isEmpty()) {
        const QPoint current = stack.back();
        QVector<QPoint> directions{ QPoint(0, -2), QPoint(0, 2), QPoint(-2, 0), QPoint(2, 0) };

        for (int index = directions.size() - 1; index > 0; --index) {
            const int swapIndex = QRandomGenerator::global()->bounded(index + 1);
            directions.swapItemsAt(index, swapIndex);
        }

        bool moved = false;
        for (const QPoint& direction : directions) {
            const QPoint next = current + direction;
            if (next.x() <= 0 || next.y() <= 0 || next.x() >= size - 1 || next.y() >= size - 1) {
                continue;
            }

            if (!walls_[next.y()][next.x()]) {
                continue;
            }

            const QPoint midpoint(current.x() + direction.x() / 2, current.y() + direction.y() / 2);
            walls_[midpoint.y()][midpoint.x()] = false;
            walls_[next.y()][next.x()] = false;
            stack.append(next);
            moved = true;
            break;
        }

        if (!moved) {
            stack.removeLast();
        }
    }

    exitCell_ = QPoint(size - 2, size - 2);
    walls_[exitCell_.y()][exitCell_.x()] = false;
    update();
}

void MazeBoard::tryMove(const int dx, const int dy) {
    const QPoint target = playerCell_ + QPoint(dx, dy);
    if (!isOpenCell(target)) {
        return;
    }

    playerCell_ = target;
    update();

    if (playerCell_ == exitCell_ && onLevelCompleted_) {
        onLevelCompleted_();
    }
}

bool MazeBoard::isOpenCell(const QPoint& cell) const {
    if (cell.y() < 0 || cell.y() >= walls_.size()) {
        return false;
    }

    if (cell.x() < 0 || cell.x() >= walls_.front().size()) {
        return false;
    }

    return !walls_[cell.y()][cell.x()];
}

int MazeBoard::boardSizeForLevel(const int level) const {
    return 13 + level * 2;
}

MazeGameWindow::MazeGameWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
    setupConnections();
    loadLevel(1);
}

void MazeGameWindow::setupUi() {
    setWindowTitle("迷宫闯关");
    setMinimumSize(760, 820);
    resize(840, 900);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    auto* headerCard = new QFrame(centralWidget);
    auto* headerLayout = new QVBoxLayout(headerCard);
    auto* infoLayout = new QHBoxLayout();
    auto* actionLayout = new QHBoxLayout();

    auto* titleLabel = new QLabel("迷宫闯关", headerCard);
    auto* subtitleLabel = new QLabel("使用方向键控制小圆点，从入口移动到绿色出口。", headerCard);
    levelLabel_ = new QLabel(headerCard);
    hintLabel_ = new QLabel("共 10 关，地图会逐关变大，通关后自动进入下一关。", headerCard);
    board_ = new MazeBoard(centralWidget);
    restartButton_ = new QPushButton("重开本关", centralWidget);
    closeButton_ = new QPushButton("关闭窗口", centralWidget);

    titleLabel->setObjectName("titleLabel");
    subtitleLabel->setObjectName("subtitleLabel");
    levelLabel_->setObjectName("infoBadge");
    hintLabel_->setObjectName("hintLabel");
    restartButton_->setObjectName("primaryButton");
    closeButton_->setObjectName("secondaryButton");
    headerCard->setObjectName("headerCard");

    subtitleLabel->setWordWrap(true);
    hintLabel_->setWordWrap(true);
    restartButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setCursor(Qt::PointingHandCursor);
    restartButton_->setMinimumHeight(44);
    closeButton_->setMinimumHeight(44);

    infoLayout->addWidget(levelLabel_, 0, Qt::AlignLeft);
    infoLayout->addStretch();

    actionLayout->addWidget(restartButton_);
    actionLayout->addWidget(closeButton_);
    actionLayout->setSpacing(12);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(infoLayout);
    headerLayout->addWidget(hintLabel_);
    headerLayout->setContentsMargins(24, 24, 24, 24);

    rootLayout->addWidget(headerCard);
    rootLayout->addWidget(board_, 1);
    rootLayout->addLayout(actionLayout);
    rootLayout->setContentsMargins(22, 22, 22, 18);
    rootLayout->setSpacing(14);

    centralWidget->setStyleSheet(
        "QWidget { background:#eef2ff; }"
        "QFrame#headerCard { background:white; border:1px solid #dbe4ff; border-radius:24px; }"
        "QLabel#titleLabel { color:#1e1b4b; font-size:26px; font-weight:800; }"
        "QLabel#subtitleLabel { color:#475569; font-size:14px; }"
        "QLabel#infoBadge { background:#eef2ff; color:#4338ca; border:1px solid #c7d2fe; border-radius:12px; padding:8px 12px; font-weight:700; }"
        "QLabel#hintLabel { color:#475569; background:#f8faff; border:1px solid #e0e7ff; border-radius:14px; padding:12px 14px; }"
        "QPushButton { border-radius:14px; padding:10px 16px; font-size:13px; font-weight:700; }"
        "QPushButton#primaryButton { background:#5b5cf0; color:white; border:none; }"
        "QPushButton#primaryButton:hover { background:#4f46e5; }"
        "QPushButton#secondaryButton { background:white; color:#312e81; border:1px solid #c7d2fe; }"
        "QPushButton#secondaryButton:hover { background:#eef2ff; }"
    );

    setCentralWidget(centralWidget);
    statusBar()->showMessage("方向键移动，抵达绿色出口即可进入下一关。");
}

void MazeGameWindow::setupConnections() {
    board_->setLevelCompletedCallback([this]() {
        handleLevelCompleted();
    });

    connect(restartButton_, &QPushButton::clicked, this, [this]() {
        board_->restartLevel();
        statusBar()->showMessage(QString("已重置第 %1 关。继续前往出口。").arg(currentLevel_));
    });
    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
}

void MazeGameWindow::loadLevel(const int level) {
    currentLevel_ = level;
    board_->setLevel(level);
    updateLabels();
    statusBar()->showMessage(QString("第 %1 关已载入，请使用方向键开始移动。").arg(currentLevel_));
}

void MazeGameWindow::handleLevelCompleted() {
    if (currentLevel_ < kTotalLevels) {
        QMessageBox::information(this, "通关成功", QString("已通过第 %1 关，即将进入第 %2 关。").arg(currentLevel_).arg(currentLevel_ + 1));
        loadLevel(currentLevel_ + 1);
        return;
    }

    QMessageBox::information(this, "全部通关", "恭喜，你已完成全部 10 关迷宫挑战！将为你重新回到第 1 关。");
    loadLevel(1);
}

void MazeGameWindow::updateLabels() {
    levelLabel_->setText(QString("当前关卡：%1 / %2").arg(currentLevel_).arg(kTotalLevels));
    hintLabel_->setText(QString("地图尺寸：%1 × %1。随着关卡提升，迷宫会更大、更难。")
        .arg(13 + currentLevel_ * 2));
}
