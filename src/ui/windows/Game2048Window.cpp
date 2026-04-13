#include "Game2048Window.h"

#include "../../core/ThemeManager.h"
#include "../../logic/Game2048Model.h"
#include "../../services/StorageService.h"
#include "../widgets/SessionOverlayWidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

Game2048Window::Game2048Window(QWidget* parent)
    : QMainWindow(parent),
      model_(new Game2048Model(this)) {
    setupUi();
    
    connect(model_, &Game2048Model::updated, this, &Game2048Window::updateUi);
    connect(model_, &Game2048Model::scoreChanged, this, [this](int score){
        scoreLabel_->setText(QString::number(score));
    });
    connect(model_, &Game2048Model::gameOver, this, &Game2048Window::onGameOver);
    connect(model_, &Game2048Model::gameWon, this, &Game2048Window::onGameWon);

    updateUi();

    if (StorageService::instance().hasSession("game_2048")) {
        model_->restoreSession(StorageService::instance().loadSession("game_2048"));
        updateUi();
        showOverlay();
    }
}

void Game2048Window::setupUi() {
    setWindowTitle("2048 - 甜心版 (MVC)");
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

    // ── Header
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

    // ── Controls
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

    // ── Grid
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
    StorageService::instance().clearSession("game_2048");
    model_->reset();
    statusLabel_->setText("开始吧！加油！");
}

void Game2048Window::onGameOver() {
    StorageService::instance().clearSession("game_2048");
    statusLabel_->setText("啊哦，游戏结束了！");
}

void Game2048Window::onGameWon() {
    StorageService::instance().clearSession("game_2048");
    statusLabel_->setText("恭喜！你拿到了 2048！");
}

void Game2048Window::showOverlay() {
    if (!overlay_) {
        overlay_ = new SessionOverlayWidget(this);
        connect(overlay_, &SessionOverlayWidget::continueRequested, this, &Game2048Window::hideOverlay);
        connect(overlay_, &SessionOverlayWidget::restartRequested, this, [this]() {
            hideOverlay();
            resetGame();
        });
    }
    overlay_->show();
    overlay_->raise();
}

void Game2048Window::hideOverlay() {
    if (overlay_) {
        overlay_->hide();
    }
}

void Game2048Window::closeEvent(QCloseEvent* event) {
    if (!model_->isGameOver() && !model_->isGameWon()) {
        StorageService::instance().saveSession("game_2048", model_->saveSession());
    }
    QMainWindow::closeEvent(event);
}

void Game2048Window::updateUi() {
    const bool dark = ThemeManager::instance().isDark();
    const auto& board = model_->board();
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            int val = board[r][c];
            QLabel* lbl = tiles_[r * 4 + c];
            lbl->setText(val == 0 ? "" : QString::number(val));
            lbl->setStyleSheet(colorForNumber(val, dark));
        }
    }
}

QString Game2048Window::colorForNumber(int num, bool darkTheme) const {
    if (num == 0) return darkTheme ? "background:#4a1c40;" : "background:#ffe4e6;";

    QString bg, color;
    color = darkTheme ? "#ffffff" : "#4c0519";
    switch (num) {
        case 2:    bg = darkTheme ? "#83386d" : "#fef08a"; break;
        case 4:    bg = darkTheme ? "#9f1239" : "#fde047"; break;
        case 8:    bg = darkTheme ? "#e11d48" : "#fca5a5"; color = "#ffffff"; break;
        case 16:   bg = darkTheme ? "#be123c" : "#f87171"; color = "#ffffff"; break;
        case 32:   bg = darkTheme ? "#9d174d" : "#f472b6"; color = "#ffffff"; break;
        case 64:   bg = darkTheme ? "#831843" : "#ec4899"; color = "#ffffff"; break;
        case 128:  bg = darkTheme ? "#db2777" : "#e879f9"; color = "#ffffff"; break;
        case 256:  bg = darkTheme ? "#c026d3" : "#d946ef"; color = "#ffffff"; break;
        case 512:  bg = darkTheme ? "#a21caf" : "#c084fc"; color = "#ffffff"; break;
        case 1024: bg = darkTheme ? "#86198f" : "#a855f7"; color = "#ffffff"; break;
        case 2048: bg = darkTheme ? "#3b0764" : "#9333ea"; color = "#ffffff"; break;
        default:   bg = darkTheme ? "#000000" : "#4c1d95"; color = "#ffffff"; break;
    }
    return QString("background:%1; color:%2; font-size:%3px; font-weight:900; border-radius:18px;")
           .arg(bg).arg(color).arg(num > 1000 ? 26 : (num > 100 ? 32 : 38));
}

void Game2048Window::keyPressEvent(QKeyEvent* event) {
    if (overlay_ && !overlay_->isHidden()) {
        return; // Block key events if overlay is visible
    }

    if (model_->isGameOver()) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    Game2048Model::Direction dir;
    bool validKey = true;
    switch (event->key()) {
        case Qt::Key_Left:
        case Qt::Key_A:      dir = Game2048Model::Direction::Left; break;
        case Qt::Key_Right:
        case Qt::Key_D:      dir = Game2048Model::Direction::Right; break;
        case Qt::Key_Up:
        case Qt::Key_W:      dir = Game2048Model::Direction::Up; break;
        case Qt::Key_Down:
        case Qt::Key_S:      dir = Game2048Model::Direction::Down; break;
        default:
            validKey = false;
            QMainWindow::keyPressEvent(event);
            break;
    }

    if (validKey) {
        model_->move(dir);
    }
}
