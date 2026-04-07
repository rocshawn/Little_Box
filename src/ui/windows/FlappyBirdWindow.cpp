#include "FlappyBirdWindow.h"

#include "../../logic/FlappyBirdModel.h"

#include <QColor>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QPolygonF>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>

FlappyBirdWidget::FlappyBirdWidget(FlappyBirdModel* model, QWidget* parent)
    : QWidget(parent), model_(model) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(720, 420);

    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &FlappyBirdWidget::advanceFrame);
    timer_.start();
}

void FlappyBirdWidget::advanceFrame() {
    if (!isVisible()) return;
    model_->update(width(), height());
    update();
}

void FlappyBirdWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QLinearGradient sky(0, 0, 0, height());
    sky.setColorAt(0.0, QColor("#bfdbfe"));
    sky.setColorAt(1.0, QColor("#eff6ff"));
    painter.fillRect(rect(), sky);

    // Clouds
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 190));
    painter.drawEllipse(QRectF(width() * 0.12, height() * 0.12, 118, 42));
    painter.drawEllipse(QRectF(width() * 0.58, height() * 0.18, 148, 48));
    painter.drawEllipse(QRectF(width() * 0.36, height() * 0.08, 86, 32));

    const qreal gapSize = qMax(132.0, 178.0 - model_->score() * 2.0);
    const qreal playHeight = height() - FlappyBirdModel::kGroundHeight;

    // Draw Pipes
    painter.setBrush(QColor("#16a34a"));
    for (const auto& pipe : model_->pipes()) {
        const qreal gapTop = pipe.gapCenterY - gapSize / 2.0;
        const qreal gapBottom = pipe.gapCenterY + gapSize / 2.0;
        painter.drawRoundedRect(QRectF(pipe.x, 0, FlappyBirdModel::kPipeWidth, gapTop), 12, 12);
        painter.drawRoundedRect(QRectF(pipe.x, gapBottom, FlappyBirdModel::kPipeWidth, playHeight - gapBottom), 12, 12);
        painter.setBrush(QColor("#22c55e"));
        painter.drawRect(QRectF(pipe.x - 6, gapTop - 18, FlappyBirdModel::kPipeWidth + 12, 18));
        painter.drawRect(QRectF(pipe.x - 6, gapBottom, FlappyBirdModel::kPipeWidth + 12, 18));
        painter.setBrush(QColor("#16a34a"));
    }

    // Draw Bird
    painter.setBrush(QColor("#facc15"));
    const QRectF bird(FlappyBirdModel::kBirdX, model_->birdY(), FlappyBirdModel::kBirdSize, FlappyBirdModel::kBirdSize);
    painter.drawEllipse(bird);
    
    // Bird Details
    painter.setBrush(QColor("#f59e0b"));
    painter.drawEllipse(QRectF(bird.left() + 8, bird.top() + 14, 18, 12));
    painter.setBrush(Qt::white);
    painter.drawEllipse(QRectF(bird.left() + 18, bird.top() + 8, 8, 8));
    painter.setBrush(QColor("#111827"));
    painter.drawEllipse(QRectF(bird.left() + 21, bird.top() + 11, 3, 3));
    painter.setBrush(QColor("#0d9488"));
    painter.drawPolygon(QPolygonF{ 
        QPointF(bird.right() - 2, bird.center().y()), 
        QPointF(bird.right() + 12, bird.center().y() - 5), 
        QPointF(bird.right() + 12, bird.center().y() + 5) 
    });

    // Ground
    painter.setBrush(QColor("#65a30d"));
    painter.drawRect(QRectF(0, playHeight, width(), FlappyBirdModel::kGroundHeight));
    painter.setBrush(QColor("#84cc16"));
    painter.drawRect(QRectF(0, playHeight, width(), 12));

    // Score Text
    painter.setPen(QColor("#1e3a8a"));
    QFont scoreFont = painter.font();
    scoreFont.setPointSize(24);
    scoreFont.setBold(true);
    painter.setFont(scoreFont);
    painter.drawText(QRectF(0, 18, width(), 36), Qt::AlignCenter, QString::number(model_->score()));

    // Overlays
    if (!model_->hasStarted()) {
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

    if (model_->isGameOver()) {
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
        model_->flap();
        event->accept();
        break;
    case Qt::Key_R:
        model_->startNewGame(height());
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void FlappyBirdWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        model_->flap();
        event->accept();
    } else {
        QWidget::mousePressEvent(event);
    }
}

void FlappyBirdWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

FlappyBirdWindow::FlappyBirdWindow(QWidget* parent)
    : QMainWindow(parent),
      model_(new FlappyBirdModel(this)) {
    setupUi();
    
    connect(model_, &FlappyBirdModel::updated, this, [this](){ gameWidget_->update(); });
    connect(model_, &FlappyBirdModel::scoreChanged, this, &FlappyBirdWindow::updateScorePanel);
    connect(model_, &FlappyBirdModel::stateChanged, this, &FlappyBirdWindow::updateScorePanel);
    
    updateScorePanel(); 
}

void FlappyBirdWindow::setupUi() {
    setWindowTitle("Flappy Bird (MVC)");
    setMinimumSize(860, 680);
    resize(940, 760);

    auto* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    
    auto* headerCard = new QFrame(centralWidget);
    headerCard->setObjectName("headerCard");
    auto* headerLayout = new QVBoxLayout(headerCard);
    
    auto* titleLabel = new QLabel("Flappy Bird", headerCard);
    titleLabel->setObjectName("titleLabel");
    auto* descriptionLabel = new QLabel("按空格、方向键上或点击画面让小鸟上升，穿过水管缝隙即可得分。", headerCard);
    descriptionLabel->setObjectName("descriptionLabel");
    descriptionLabel->setWordWrap(true);

    auto* statsLayout = new QHBoxLayout();
    
    auto* scoreCard = new QFrame(headerCard);
    scoreCard->setObjectName("metricCard");
    auto* scoreCardLayout = new QVBoxLayout(scoreCard);
    auto* scoreTitleLabel = new QLabel("当前分数", scoreCard);
    scoreTitleLabel->setObjectName("metricTitle");
    scoreValueLabel_ = new QLabel("0", scoreCard);
    scoreValueLabel_->setObjectName("metricValue");
    scoreCardLayout->addWidget(scoreTitleLabel);
    scoreCardLayout->addWidget(scoreValueLabel_);

    auto* bestScoreCard = new QFrame(headerCard);
    bestScoreCard->setObjectName("metricCard");
    auto* bestScoreCardLayout = new QVBoxLayout(bestScoreCard);
    auto* bestScoreTitleLabel = new QLabel("最高分", bestScoreCard);
    bestScoreTitleLabel->setObjectName("metricTitle");
    bestScoreValueLabel_ = new QLabel("0", bestScoreCard);
    bestScoreValueLabel_->setObjectName("metricValue");
    bestScoreCardLayout->addWidget(bestScoreTitleLabel);
    bestScoreCardLayout->addWidget(bestScoreValueLabel_);

    statsLayout->addWidget(scoreCard);
    statsLayout->addWidget(bestScoreCard);
    statsLayout->setSpacing(12);

    stateLabel_ = new QLabel(headerCard);
    stateLabel_->setObjectName("stateLabel");
    stateLabel_->setWordWrap(true);

    headerLayout->addWidget(titleLabel);
    headerLayout->addWidget(descriptionLabel);
    headerLayout->addSpacing(10);
    headerLayout->addLayout(statsLayout);
    headerLayout->addSpacing(10);
    headerLayout->addWidget(stateLabel_);
    headerLayout->setContentsMargins(24, 24, 24, 24);

    gameWidget_ = new FlappyBirdWidget(model_, centralWidget);
    
    auto* actionsLayout = new QHBoxLayout();
    restartButton_ = new QPushButton("重新开始", centralWidget);
    restartButton_->setObjectName("primaryButton");
    restartButton_->setCursor(Qt::PointingHandCursor);
    restartButton_->setMinimumHeight(44);
    
    closeButton_ = new QPushButton("关闭窗口", centralWidget);
    closeButton_->setObjectName("secondaryButton");
    closeButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setMinimumHeight(44);
    
    actionsLayout->addWidget(restartButton_);
    actionsLayout->addWidget(closeButton_);
    actionsLayout->setSpacing(12);

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

    connect(restartButton_, &QPushButton::clicked, this, &FlappyBirdWindow::startNewGame);
    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);
}

void FlappyBirdWindow::startNewGame() {
    model_->startNewGame(gameWidget_->height());
    gameWidget_->setFocus();
}

void FlappyBirdWindow::updateScorePanel() {
    scoreValueLabel_->setText(QString::number(model_->score()));
    bestScoreValueLabel_->setText(QString::number(model_->bestScore()));

    if (model_->isGameOver()) {
        stateLabel_->setText("状态：游戏结束。按空格、R 或点击“重新开始”继续挑战。");
    } else if (!model_->hasStarted()) {
        stateLabel_->setText("状态：待开始。按空格、方向键上或点击画面开始飞行。");
    } else {
        stateLabel_->setText("状态：进行中。控制飞行节奏，穿过更多水管拿到更高分。");
    }
}
