#include "FlappyBirdWindow.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QSettings>
#include <QVBoxLayout>
#include <QWidget>



#include <algorithm>

namespace {
constexpr qreal kBirdX = 120.0;
constexpr qreal kBirdSize = 34.0;
constexpr qreal kGravity = 0.42;
constexpr qreal kFlapVelocity = -7.4;
constexpr qreal kPipeSpeed = 3.2;
constexpr qreal kPipeWidth = 82.0;
constexpr qreal kGroundHeight = 68.0;
constexpr int kSpawnIntervalFrames = 95;
constexpr auto kBestScoreSettingsKey = "games/flappyBird/bestScore";

} // namespace

FlappyBirdWidget::FlappyBirdWidget(QWidget* parent)
    : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(720, 420);
    loadBestScore();

    timer_.setInterval(16);

    connect(&timer_, &QTimer::timeout, this, [this]() {
        advanceFrame();
    });
    timer_.start();

    startNewGame();
}

void FlappyBirdWidget::startNewGame() {
    pipes_.clear();
    birdY_ = qMax(160.0, height() / 2.0);
    birdVelocity_ = 0.0;
    frameCounter_ = 0;
    score_ = 0;
    started_ = false;
    gameOver_ = false;
    spawnPipe();
    notifyStateChanged();
    update();
    setFocus();
}

int FlappyBirdWidget::currentScore() const noexcept {
    return score_;
}

int FlappyBirdWidget::bestScore() const noexcept {
    return bestScore_;
}

bool FlappyBirdWidget::hasStarted() const noexcept {
    return started_;
}

bool FlappyBirdWidget::isGameOver() const noexcept {
    return gameOver_;
}

void FlappyBirdWidget::setStateChangedCallback(std::function<void()> callback) {
    onStateChanged_ = callback;
}

void FlappyBirdWidget::loadBestScore() {
    QSettings settings;
    const QString settingsKey = QString::fromLatin1(kBestScoreSettingsKey);
    bestScore_ = std::max(0, settings.value(settingsKey, 0).toInt());
}

void FlappyBirdWidget::updateBestScoreIfNeeded() {
    if (score_ <= bestScore_) {
        return;
    }

    bestScore_ = score_;
    QSettings settings;
    const QString settingsKey = QString::fromLatin1(kBestScoreSettingsKey);
    settings.setValue(settingsKey, bestScore_);
}


void FlappyBirdWidget::paintEvent(QPaintEvent* event) {

    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor("#bfdbfe"));
    sky.setColorAt(1.0, QColor("#eff6ff"));
    painter.fillRect(rect(), sky);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 190));
    painter.drawEllipse(QRectF(width() * 0.12, height() * 0.12, 118, 42));
    painter.drawEllipse(QRectF(width() * 0.58, height() * 0.18, 148, 48));
    painter.drawEllipse(QRectF(width() * 0.36, height() * 0.08, 86, 32));

    const qreal gapSize = qMax(132.0, 178.0 - score_ * 2.0);
    const qreal playHeight = height() - kGroundHeight;

    painter.setBrush(QColor("#16a34a"));
    for (const FlappyPipe& pipe : pipes_) {
        const qreal gapTop = pipe.gapCenterY - gapSize / 2.0;
        const qreal gapBottom = pipe.gapCenterY + gapSize / 2.0;
        painter.drawRoundedRect(QRectF(pipe.x, 0, kPipeWidth, gapTop), 12, 12);
        painter.drawRoundedRect(QRectF(pipe.x, gapBottom, kPipeWidth, playHeight - gapBottom), 12, 12);
        painter.setBrush(QColor("#22c55e"));
        painter.drawRect(QRectF(pipe.x - 6, gapTop - 18, kPipeWidth + 12, 18));
        painter.drawRect(QRectF(pipe.x - 6, gapBottom, kPipeWidth + 12, 18));
        painter.setBrush(QColor("#16a34a"));
    }

    painter.setBrush(QColor("#facc15"));
    const QRectF bird = birdRect();
    painter.drawEllipse(bird);
    painter.setBrush(QColor("#f59e0b"));
    painter.drawEllipse(QRectF(bird.left() + 8, bird.top() + 14, 18, 12));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QRectF(bird.left() + 18, bird.top() + 8, 8, 8));
    painter.setBrush(QColor("#111827"));
    painter.drawEllipse(QRectF(bird.left() + 21, bird.top() + 11, 3, 3));
    painter.setBrush(QColor("#fb7185"));
    painter.drawPolygon(QPolygonF{ QPointF(bird.right() - 2, bird.center().y()), QPointF(bird.right() + 12, bird.center().y() - 5), QPointF(bird.right() + 12, bird.center().y() + 5) });

    painter.setBrush(QColor("#65a30d"));
    painter.drawRect(QRectF(0, playHeight, width(), kGroundHeight));
    painter.setBrush(QColor("#84cc16"));
    painter.drawRect(QRectF(0, playHeight, width(), 12));

    painter.setPen(QColor("#1e3a8a"));
    QFont scoreFont = painter.font();
    scoreFont.setPointSize(24);
    scoreFont.setBold(true);
    painter.setFont(scoreFont);
    painter.drawText(QRectF(0, 18, width(), 36), Qt::AlignCenter, QString::number(score_));

    if (!started_) {
        painter.fillRect(rect(), QColor(15, 23, 42, 76));
        painter.setPen(Qt::white);
        QFont titleFont = painter.font();
        titleFont.setPointSize(22);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(QRectF(0, height() * 0.28, width(), 40), Qt::AlignCenter, "Flappy Bird");

        QFont hintFont = painter.font();
        hintFont.setPointSize(12);
        hintFont.setBold(false);
        painter.setFont(hintFont);
        painter.drawText(QRectF(0, height() * 0.40, width(), 56), Qt::AlignCenter, "按空格、方向键上或点击画面开始飞行");
    }

    if (gameOver_) {
        painter.fillRect(rect(), QColor(15, 23, 42, 118));
        painter.setPen(Qt::white);
        QFont titleFont = painter.font();
        titleFont.setPointSize(22);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(QRectF(0, height() * 0.26, width(), 40), Qt::AlignCenter, "游戏结束");

        QFont hintFont = painter.font();
        hintFont.setPointSize(12);
        hintFont.setBold(false);
        painter.setFont(hintFont);
        painter.drawText(QRectF(0, height() * 0.38, width(), 64), Qt::AlignCenter, "按空格、R 或点击重新开始\n躲开水管并尽量拿到更高分");
    }
}

void FlappyBirdWidget::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Up:
    case Qt::Key_W:
        flap();
        event->accept();
        return;
    case Qt::Key_R:
        startNewGame();
        event->accept();
        return;
    default:
        QWidget::keyPressEvent(event);
        return;
    }
}

void FlappyBirdWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        flap();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void FlappyBirdWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (!started_ && !gameOver_) {
        birdY_ = qMax(160.0, height() / 2.0);
        update();
    }
}

void FlappyBirdWidget::advanceFrame() {
    if (!isVisible()) {
        return;
    }

    ++frameCounter_;

    if (!started_ || gameOver_) {
        update();
        return;
    }

    birdVelocity_ += kGravity;
    birdY_ += birdVelocity_;

    if (frameCounter_ % kSpawnIntervalFrames == 0) {
        spawnPipe();
    }

    for (FlappyPipe& pipe : pipes_) {
        pipe.x -= kPipeSpeed;
        if (!pipe.scored && pipe.x + kPipeWidth < kBirdX) {
            pipe.scored = true;
            ++score_;
            updateBestScoreIfNeeded();
            notifyStateChanged();
        }

    }

    while (!pipes_.isEmpty() && pipes_.front().x + kPipeWidth < -2) {
        pipes_.removeFirst();
    }

    const QRectF bird = birdRect();
    const qreal playHeight = height() - kGroundHeight;
    if (bird.top() <= 0 || bird.bottom() >= playHeight) {
        gameOver_ = true;
        updateBestScoreIfNeeded();
        notifyStateChanged();
        update();
        return;
    }


    for (const FlappyPipe& pipe : pipes_) {
        if (hitsPipe(bird, pipe)) {
            gameOver_ = true;
            updateBestScoreIfNeeded();
            notifyStateChanged();
            update();
            return;
        }

    }

    update();
}

void FlappyBirdWidget::flap() {
    if (gameOver_) {
        startNewGame();
    }

    if (!started_) {
        started_ = true;
        notifyStateChanged();
    }

    birdVelocity_ = kFlapVelocity;
    update();
}

void FlappyBirdWidget::spawnPipe() {
    if (width() <= 0 || height() <= 0) {
        return;
    }

    const qreal playHeight = qMax(320.0, height() - kGroundHeight);
    const qreal gapSize = qMax(132.0, 178.0 - score_ * 2.0);
    const int minGapCenter = static_cast<int>(gapSize / 2.0 + 34.0);
    const int maxGapCenter = static_cast<int>(playHeight - gapSize / 2.0 - 34.0);
    const int safeMax = std::max(minGapCenter + 1, maxGapCenter);

    FlappyPipe pipe;
    pipe.x = width() + 80.0;
    pipe.gapCenterY = QRandomGenerator::global()->bounded(minGapCenter, safeMax);
    pipe.scored = false;
    pipes_.append(pipe);
}

void FlappyBirdWidget::notifyStateChanged() {
    if (onStateChanged_) {
        onStateChanged_();
    }
}

QRectF FlappyBirdWidget::birdRect() const {
    return QRectF(kBirdX, birdY_, kBirdSize, kBirdSize);
}

bool FlappyBirdWidget::hitsPipe(const QRectF& bird, const FlappyPipe& pipe) const {
    const qreal gapSize = qMax(132.0, 178.0 - score_ * 2.0);
    const qreal playHeight = height() - kGroundHeight;
    const qreal gapTop = pipe.gapCenterY - gapSize / 2.0;
    const qreal gapBottom = pipe.gapCenterY + gapSize / 2.0;

    const QRectF topPipe(pipe.x, 0, kPipeWidth, gapTop);
    const QRectF bottomPipe(pipe.x, gapBottom, kPipeWidth, playHeight - gapBottom);
    return bird.intersects(topPipe) || bird.intersects(bottomPipe);
}

FlappyBirdWindow::FlappyBirdWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
    updateScorePanel();
}

void FlappyBirdWindow::setupUi() {
    setWindowTitle("Flappy Bird");
    setMinimumSize(860, 680);
    resize(940, 760);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    auto* headerCard = new QFrame(centralWidget);
    auto* headerLayout = new QVBoxLayout(headerCard);
    auto* statsLayout = new QHBoxLayout();
    auto* actionsLayout = new QHBoxLayout();

    auto* titleLabel = new QLabel("Flappy Bird", headerCard);
    auto* descriptionLabel = new QLabel("按空格、方向键上或点击画面让小鸟上升，穿过水管缝隙即可得分。", headerCard);
    auto* scoreTitleLabel = new QLabel("当前分数", headerCard);
    scoreValueLabel_ = new QLabel("0", headerCard);
    auto* bestScoreTitleLabel = new QLabel("最高分", headerCard);
    bestScoreValueLabel_ = new QLabel("0", headerCard);
    stateLabel_ = new QLabel(headerCard);
    gameWidget_ = new FlappyBirdWidget(centralWidget);
    restartButton_ = new QPushButton("重新开始", centralWidget);
    closeButton_ = new QPushButton("关闭窗口", centralWidget);

    headerCard->setObjectName("headerCard");
    titleLabel->setObjectName("titleLabel");
    descriptionLabel->setObjectName("descriptionLabel");
    scoreTitleLabel->setObjectName("metricTitle");
    scoreValueLabel_->setObjectName("metricValue");
    bestScoreTitleLabel->setObjectName("metricTitle");
    bestScoreValueLabel_->setObjectName("metricValue");
    stateLabel_->setObjectName("stateLabel");
    restartButton_->setObjectName("primaryButton");
    closeButton_->setObjectName("secondaryButton");

    descriptionLabel->setWordWrap(true);
    stateLabel_->setWordWrap(true);
    restartButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setCursor(Qt::PointingHandCursor);
    restartButton_->setMinimumHeight(44);
    closeButton_->setMinimumHeight(44);

    auto* scoreCard = new QFrame(headerCard);
    auto* scoreCardLayout = new QVBoxLayout(scoreCard);
    scoreCard->setObjectName("metricCard");
    scoreCardLayout->addWidget(scoreTitleLabel);
    scoreCardLayout->addWidget(scoreValueLabel_);
    scoreCardLayout->setContentsMargins(18, 14, 18, 14);

    auto* bestScoreCard = new QFrame(headerCard);
    auto* bestScoreCardLayout = new QVBoxLayout(bestScoreCard);
    bestScoreCard->setObjectName("metricCard");
    bestScoreCardLayout->addWidget(bestScoreTitleLabel);
    bestScoreCardLayout->addWidget(bestScoreValueLabel_);
    bestScoreCardLayout->setContentsMargins(18, 14, 18, 14);

    statsLayout->addWidget(scoreCard);
    statsLayout->addWidget(bestScoreCard);
    statsLayout->setSpacing(12);

    actionsLayout->addWidget(restartButton_);
    actionsLayout->addWidget(closeButton_);
    actionsLayout->setSpacing(12);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
    headerLayout->addSpacing(10);
    headerLayout->addLayout(statsLayout);
    headerLayout->addSpacing(10);
    headerLayout->addWidget(stateLabel_);
    headerLayout->setContentsMargins(24, 24, 24, 24);

    rootLayout->addWidget(headerCard);
    rootLayout->addWidget(gameWidget_, 1);
    rootLayout->addLayout(actionsLayout);
    rootLayout->setContentsMargins(22, 22, 22, 18);
    rootLayout->setSpacing(14);

    centralWidget->setStyleSheet(
        "QWidget { background:#eef2ff; }"
        "QFrame#headerCard, QFrame#metricCard { background:white; border:1px solid #dbe4ff; border-radius:24px; }"
        "QLabel#titleLabel { color:#1e1b4b; font-size:26px; font-weight:800; }"
        "QLabel#descriptionLabel { color:#475569; font-size:14px; }"
        "QLabel#metricTitle { color:#6366f1; font-size:12px; font-weight:700; }"
        "QLabel#metricValue { color:#1e1b4b; font-size:26px; font-weight:800; }"
        "QLabel#stateLabel { color:#334155; background:#f8faff; border:1px solid #e0e7ff; border-radius:14px; padding:12px 14px; }"
        "QPushButton { border-radius:14px; padding:10px 16px; font-size:13px; font-weight:700; }"
        "QPushButton#primaryButton { background:#5b5cf0; color:white; border:none; }"
        "QPushButton#primaryButton:hover { background:#4f46e5; }"
        "QPushButton#secondaryButton { background:white; color:#312e81; border:1px solid #c7d2fe; }"
        "QPushButton#secondaryButton:hover { background:#eef2ff; }"
    );

    setCentralWidget(centralWidget);

    connect(restartButton_, &QPushButton::clicked, this, [this]() {
        gameWidget_->startNewGame();
    });
    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
    gameWidget_->setStateChangedCallback([this]() {
        updateScorePanel();
    });
}

void FlappyBirdWindow::updateScorePanel() {
    scoreValueLabel_->setText(QString::number(gameWidget_->currentScore()));
    bestScoreValueLabel_->setText(QString::number(gameWidget_->bestScore()));

    if (gameWidget_->isGameOver()) {
        stateLabel_->setText("状态：游戏结束。按空格、R 或点击“重新开始”继续挑战。");
        return;
    }

    if (!gameWidget_->hasStarted()) {
        stateLabel_->setText("状态：待开始。按空格、方向键上或点击画面开始飞行。");
        return;
    }

    stateLabel_->setText("状态：进行中。控制飞行节奏，穿过更多水管拿到更高分。");
}
