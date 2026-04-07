#include "MazeGameWindow.h"

#include "../../logic/MazeGameModel.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QRectF>
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

MazeBoard::MazeBoard(MazeGameModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(460, 460);
    connect(model_, &MazeGameModel::updated, this, [this](){ update(); });
}

void MazeBoard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), kBackgroundColor);

    const auto& walls = model_->walls();
    if (walls.isEmpty()) {
        return;
    }

    const int rows = walls.size();
    const int columns = walls.front().size();
    const qreal cellSize = std::min((width() - kBoardMargin * 2.0) / columns, (height() - kBoardMargin * 2.0) / rows);
    const qreal offsetX = (width() - columns * cellSize) / 2.0;
    const qreal offsetY = (height() - rows * cellSize) / 2.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRoundedRect(QRectF(offsetX - 10, offsetY - 10, columns * cellSize + 20, rows * cellSize + 20), 18, 18);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            const QRectF cellRect(offsetX + column * cellSize, offsetY + row * cellSize, cellSize, cellSize);
            painter.setBrush(walls[row][column] ? kWallColor : kPathColor);
            painter.drawRect(cellRect);

            if (QPoint(column, row) == model_->exitCell()) {
                painter.setBrush(kExitColor);
                painter.drawRoundedRect(cellRect.adjusted(cellSize * 0.16, cellSize * 0.16, -cellSize * 0.16, -cellSize * 0.16), 8, 8);
            }
        }
    }

    const QRectF playerRect(
        offsetX + model_->playerCell().x() * cellSize + cellSize * 0.18,
        offsetY + model_->playerCell().y() * cellSize + cellSize * 0.18,
        cellSize * 0.64,
        cellSize * 0.64);

    painter.setBrush(kPlayerColor);
    painter.drawEllipse(playerRect);
}

void MazeBoard::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Up:    model_->tryMove(0, -1); break;
    case Qt::Key_Down:  model_->tryMove(0, 1); break;
    case Qt::Key_Left:  model_->tryMove(-1, 0); break;
    case Qt::Key_Right: model_->tryMove(1, 0); break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

MazeGameWindow::MazeGameWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new MazeGameModel(this)) {
    setupUi();
    setupConnections();
    updateLabels();
}

void MazeGameWindow::setupUi() {
    setWindowTitle("迷宫闯关 (MVC)");
    setMinimumSize(760, 820);
    resize(840, 900);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    
    auto* headerCard = new QFrame(centralWidget);
    headerCard->setObjectName("headerCard");
    auto* headerLayout = new QVBoxLayout(headerCard);
    
    auto* titleLabel = new QLabel("迷宫闯关", headerCard);
    titleLabel->setObjectName("titleLabel");
    auto* subtitleLabel = new QLabel("使用方向键控制小圆点，从入口移动到绿色出口。", headerCard);
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setWordWrap(true);

    auto* infoLayout = new QHBoxLayout();
    levelLabel_ = new QLabel(headerCard);
    levelLabel_->setObjectName("infoBadge");
    infoLayout->addWidget(levelLabel_, 0, Qt::AlignLeft);
    infoLayout->addStretch();

    hintLabel_ = new QLabel(headerCard);
    hintLabel_->setObjectName("hintLabel");
    hintLabel_->setWordWrap(true);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(subtitleLabel);
    headerLayout->addSpacing(12);
    headerLayout->addLayout(infoLayout);
    headerLayout->addWidget(hintLabel_);
    headerLayout->setContentsMargins(24, 24, 24, 24);

    board_ = new MazeBoard(model_, centralWidget);
    
    auto* actionLayout = new QHBoxLayout();
    restartButton_ = new QPushButton("重开本关", centralWidget);
    restartButton_->setObjectName("primaryButton");
    restartButton_->setCursor(Qt::PointingHandCursor);
    restartButton_->setMinimumHeight(44);
    
    closeButton_ = new QPushButton("关闭窗口", centralWidget);
    closeButton_->setObjectName("secondaryButton");
    closeButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setMinimumHeight(44);

    actionLayout->addWidget(restartButton_);
    actionLayout->addWidget(closeButton_);
    actionLayout->setSpacing(12);

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

    statusBar()->showMessage("方向键移动，抵达绿色出口即可进入下一关。");
}

void MazeGameWindow::setupConnections() {
    connect(model_, &MazeGameModel::levelCompleted, this, &MazeGameWindow::handleLevelCompleted);
    connect(model_, &MazeGameModel::levelChanged, this, &MazeGameWindow::updateLabels);
    
    connect(restartButton_, &QPushButton::clicked, this, [this]() {
        model_->restartLevel();
        statusBar()->showMessage(QString("已重置第 %1 关。继续前往出口。").arg(model_->currentLevel()));
        board_->setFocus();
    });
    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
}

void MazeGameWindow::handleLevelCompleted() {
    int level = model_->currentLevel();
    if (level < model_->totalLevels()) {
        QMessageBox::information(this, "通关成功", QString("已通过第 %1 关，即将进入第 %2 关。").arg(level).arg(level + 1));
        model_->setLevel(level + 1);
        statusBar()->showMessage(QString("第 %1 关已载入，请使用方向键开始移动。").arg(level + 1));
    } else {
        QMessageBox::information(this, "全部通关", "恭喜，你已完成全部 10 关迷宫挑战！将为你重新回到第 1 关。");
        model_->setLevel(1);
    }
    board_->setFocus();
}

void MazeGameWindow::updateLabels() {
    levelLabel_->setText(QString("当前关卡：%1 / %2").arg(model_->currentLevel()).arg(model_->totalLevels()));
    hintLabel_->setText(QString("地图尺寸：%1 × %1。随着关卡提升，迷宫会更大、更难。").arg(model_->boardSize()));
}
