#include "Game2048Window.h"

#include "../../core/ThemeManager.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPoint>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRandomGenerator>

Game2048Window::Game2048Window(QWidget* parent)
    : QMainWindow(parent),
      board_(),
      score_(0),
      scoreLabel_(nullptr),
      statusLabel_(nullptr),
      gridLayout_(nullptr) {
    setupUi();
    resetGame();
}

void Game2048Window::setupUi() {
    setWindowTitle("2048 - 甜心版");
    setMinimumSize(480, 650);
    resize(480, 650);

    const bool dark = ThemeManager::instance().isDark();
    setStyleSheet(dark 
        ? "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #2d142c, stop:1 #1a0b1c); }"
        : "QMainWindow { background:qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #fff1f2, stop:1 #fffaff); }");

    auto* central = new QWidget(this);
    setCentralWidget(central);
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    // ── Header Layout
    auto* headerLayout = new QHBoxLayout();
    
    auto* titleLabel = new QLabel("2048", this);
    titleLabel->setStyleSheet(dark 
        ? "color:#fff0f2; font-size:42px; font-weight:900;" 
        : "color:#fb7185; font-size:42px; font-weight:900;");
    headerLayout->addWidget(titleLabel);

    headerLayout->addStretch();

    auto* scoreBox = new QFrame(this);
    scoreBox->setStyleSheet(dark
        ? "background:#4c0519; border-radius:12px; padding:8px 16px;"
        : "background:#ffe4e6; border-radius:12px; padding:8px 16px;");
    auto* scoreLayout = new QVBoxLayout(scoreBox);
    scoreLayout->setContentsMargins(0,0,0,0);
    
    auto* scoreTitle = new QLabel("SCORE", scoreBox);
    scoreTitle->setAlignment(Qt::AlignCenter);
    scoreTitle->setStyleSheet(dark ? "color:#fda4af; font-size:12px; font-weight:800;" : "color:#fb7185; font-size:12px; font-weight:800;");
    
    scoreLabel_ = new QLabel("0", scoreBox);
    scoreLabel_->setAlignment(Qt::AlignCenter);
    scoreLabel_->setStyleSheet(dark ? "color:#fff0f2; font-size:24px; font-weight:900;" : "color:#4c0519; font-size:24px; font-weight:900;");
    
    scoreLayout->addWidget(scoreTitle);
    scoreLayout->addWidget(scoreLabel_);
    headerLayout->addWidget(scoreBox);

    mainLayout->addLayout(headerLayout);

    auto* topControls = new QHBoxLayout();
    statusLabel_ = new QLabel("连出更大的数字！", this);
    statusLabel_->setStyleSheet(dark ? "color:#fbcfe8; font-size:16px;" : "color:#fb7185; font-size:16px;");
    
    auto* resetBtn = new QPushButton("重新开始", this);
    resetBtn->setCursor(Qt::PointingHandCursor);
    resetBtn->setStyleSheet(dark 
        ? "QPushButton { background:#881337; color:white; border-radius:14px; padding:8px 16px; font-size:14px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#9f1239; }"
        : "QPushButton { background:#fb7185; color:white; border-radius:14px; padding:8px 16px; font-size:14px; font-weight:bold; border:none; }"
          "QPushButton:hover { background:#f43f5e; }");
    connect(resetBtn, &QPushButton::clicked, this, &Game2048Window::resetGame);
    
    topControls->addWidget(statusLabel_);
    topControls->addStretch();
    topControls->addWidget(resetBtn);
    mainLayout->addLayout(topControls);
    mainLayout->addSpacing(30);

    // ── Grid Setup
    auto* gridFrame = new QFrame(this);
    gridFrame->setFixedSize(400, 400);
    gridFrame->setStyleSheet(dark
        ? "QFrame { background:#3a1c36; border-radius:24px; border:4px solid #5e2c56; }"
        : "QFrame { background:#fff1f2; border-radius:24px; border:4px solid #ffe4e6; }");
    
    gridLayout_ = new QGridLayout(gridFrame);
    gridLayout_->setContentsMargins(12, 12, 12, 12);
    gridLayout_->setSpacing(12);

    for (int i = 0; i < 16; ++i) {
        auto* tile = new QLabel(gridFrame);
        tile->setAlignment(Qt::AlignCenter);
        tiles_.append(tile);
        gridLayout_->addWidget(tile, i / 4, i % 4);
    }
    
    mainLayout->addWidget(gridFrame, 0, Qt::AlignHCenter);
    mainLayout->addStretch();
}

void Game2048Window::resetGame() {
    board_ = QVector<QVector<int>>(4, QVector<int>(4, 0));
    score_ = 0;
    statusLabel_->setText("开始吧！加油！");

    spawnRandomTile();
    spawnRandomTile();
    updateUi();
}

void Game2048Window::spawnRandomTile() {
    QVector<QPoint> emptyCells;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board_[r][c] == 0) emptyCells.append({r, c});
        }
    }
    if (emptyCells.isEmpty()) return;

    const int idx = QRandomGenerator::global()->bounded(emptyCells.size());
    const QPoint& pt = emptyCells[idx];
    // 90% chance of 2, 10% chance of 4
    board_[pt.x()][pt.y()] = (QRandomGenerator::global()->bounded(10) < 9) ? 2 : 4;
}

QString Game2048Window::colorForNumber(int num, bool darkTheme) const {
    if (num == 0) return darkTheme ? "background:#4a1c40;" : "background:#ffe4e6;";

    QString bg, color;
    color = darkTheme ? "#ffffff" : "#4c0519"; // default text color

    switch (num) {
        case 2:    bg = darkTheme ? "#83386d" : "#fef08a"; break; // Candy yellow
        case 4:    bg = darkTheme ? "#9f1239" : "#fde047"; break; // Stronger yellow
        case 8:    bg = darkTheme ? "#e11d48" : "#fca5a5"; color = "#ffffff"; break; // Soft red
        case 16:   bg = darkTheme ? "#be123c" : "#f87171"; color = "#ffffff"; break;
        case 32:   bg = darkTheme ? "#9d174d" : "#f472b6"; color = "#ffffff"; break; // Pink
        case 64:   bg = darkTheme ? "#831843" : "#ec4899"; color = "#ffffff"; break;
        case 128:  bg = darkTheme ? "#db2777" : "#e879f9"; color = "#ffffff"; break; // Purple
        case 256:  bg = darkTheme ? "#c026d3" : "#d946ef"; color = "#ffffff"; break;
        case 512:  bg = darkTheme ? "#a21caf" : "#c084fc"; color = "#ffffff"; break;
        case 1024: bg = darkTheme ? "#86198f" : "#a855f7"; color = "#ffffff"; break;
        case 2048: bg = darkTheme ? "#3b0764" : "#9333ea"; color = "#ffffff"; break; // Deep majestic purple
        default:   bg = darkTheme ? "#000000" : "#4c1d95"; color = "#ffffff"; break; // Super high
    }
    
    // Smooth bubbly gradient simulation via linear gradient and huge borders
    return QString("background:%1; color:%2; font-size:%3px; font-weight:900; border-radius:18px;")
           .arg(bg)
           .arg(color)
           .arg(num > 1000 ? 26 : (num > 100 ? 32 : 38));
}

void Game2048Window::updateUi() {
    const bool dark = ThemeManager::instance().isDark();
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            int val = board_[r][c];
            QLabel* lbl = tiles_[r * 4 + c];
            lbl->setText(val == 0 ? "" : QString::number(val));
            lbl->setStyleSheet(colorForNumber(val, dark));
        }
    }
    scoreLabel_->setText(QString::number(score_));

    if (isGameOver()) {
        statusLabel_->setText("啊哦，游戏结束了！");
    }
}

// ── Logic Algorithms ──

bool Game2048Window::slideArrayLeft(QVector<int>& line, int& scoreIncrease) {
    bool changed = false;
    QVector<int> result(4, 0);
    int head = 0;
    int lastMerged = -1;

    for (int i = 0; i < 4; ++i) {
        if (line[i] == 0) continue;
        
        if (head > 0 && result[head - 1] == line[i] && lastMerged != head - 1) {
            result[head - 1] *= 2;
            scoreIncrease += result[head - 1];
            lastMerged = head - 1;
            changed = true;
        } else {
            result[head] = line[i];
            if (head != i) changed = true;
            head++;
        }
    }
    line = result;
    return changed;
}

void Game2048Window::rotateBoardClockwise() {
    QVector<QVector<int>> newBoard(4, QVector<int>(4, 0));
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            newBoard[c][3 - r] = board_[r][c];
        }
    }
    board_ = newBoard;
}

bool Game2048Window::moveBoard(Direction dir) {
    // We normalize all movements to 'slideLeft' by rotating the board.
    int rotations = 0;
    switch(dir) {
        case Direction::Left:  rotations = 0; break;
        case Direction::Up:    rotations = 3; break;
        case Direction::Right: rotations = 2; break;
        case Direction::Down:  rotations = 1; break;
    }

    for (int i = 0; i < rotations; ++i) rotateBoardClockwise();

    bool moved = false;
    int addedScore = 0;
    for (int r = 0; r < 4; ++r) {
        if (slideArrayLeft(board_[r], addedScore)) moved = true;
    }
    
    score_ += addedScore;

    for (int i = 0; i < (4 - rotations) % 4; ++i) rotateBoardClockwise();

    return moved;
}

bool Game2048Window::isGameOver() const {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            if (board_[r][c] == 0) return false;
            // Check right
            if (c < 3 && board_[r][c] == board_[r][c+1]) return false;
            // Check down
            if (r < 3 && board_[r][c] == board_[r+1][c]) return false;
        }
    }
    return true;
}

void Game2048Window::keyPressEvent(QKeyEvent* event) {
    if (isGameOver()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    bool moved = false;
    switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:      moved = moveBoard(Direction::Left); break;
        case Qt::Key_Right:
        case Qt::Key_D:      moved = moveBoard(Direction::Right); break;
        case Qt::Key_Up:
        case Qt::Key_W:      moved = moveBoard(Direction::Up); break;
        case Qt::Key_Down:
        case Qt::Key_S:      moved = moveBoard(Direction::Down); break;
        default:
            QMainWindow::keyPressEvent(event);
            return;
    }

    if (moved) {
        spawnRandomTile();
        updateUi();
    }
}
