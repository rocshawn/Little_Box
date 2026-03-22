#include "ReactionTestWindow.h"

#include "ReactionTestWidget.h"

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kMinimumWindowWidth = 860;
constexpr int kMinimumWindowHeight = 620;
constexpr int kInitialWindowWidth = 980;
constexpr int kInitialWindowHeight = 720;
} // namespace

ReactionTestWindow::ReactionTestWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
}

void ReactionTestWindow::setupUi() {
    setWindowTitle("反应速度测试");
    setMinimumSize(kMinimumWindowWidth, kMinimumWindowHeight);
    resize(kInitialWindowWidth, kInitialWindowHeight);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    auto* card = new QFrame(centralWidget);
    auto* cardLayout = new QVBoxLayout(card);
    auto* headerLayout = new QHBoxLayout();
    auto* titleLayout = new QVBoxLayout();

    titleLabel_ = new QLabel("反应速度测试", card);
    subtitleLabel_ = new QLabel("共 5 轮，每轮在屏幕变绿后点击，结果以毫秒（ms）显示并绘制折线图。", card);
    reactionTestWidget_ = new ReactionTestWidget(card);
    closeButton_ = new QPushButton("关闭", card);

    centralWidget->setObjectName("centralWidget");
    card->setObjectName("card");
    titleLabel_->setObjectName("titleLabel");
    subtitleLabel_->setObjectName("subtitleLabel");
    closeButton_->setObjectName("ghostButton");

    subtitleLabel_->setWordWrap(true);
    closeButton_->setCursor(Qt::PointingHandCursor);
    closeButton_->setMinimumHeight(42);
    closeButton_->setMinimumWidth(120);

    titleLayout->addWidget(titleLabel_);
    titleLayout->addWidget(subtitleLabel_);
    titleLayout->setSpacing(4);

    headerLayout->addLayout(titleLayout, 1);
    headerLayout->addWidget(closeButton_, 0, Qt::AlignTop);

    cardLayout->addLayout(headerLayout);
    cardLayout->addSpacing(16);
    cardLayout->addWidget(reactionTestWidget_, 1);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(0);

    rootLayout->addStretch();
    rootLayout->addWidget(card);
    rootLayout->addStretch();
    rootLayout->setContentsMargins(22, 22, 22, 20);

    auto* shadowEffect = new QGraphicsDropShadowEffect(card);
    shadowEffect->setBlurRadius(40);
    shadowEffect->setOffset(0, 16);
    shadowEffect->setColor(QColor(55, 65, 81, 80));
    card->setGraphicsEffect(shadowEffect);

    centralWidget->setStyleSheet(
        "QWidget#centralWidget { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #eef2ff, stop:1 #f8fafc); }"
        "QFrame#card { background:white; border:1px solid #dbe4ff; border-radius:24px; }"
        "QLabel#titleLabel { color:#1e1b4b; font-size:28px; font-weight:800; }"
        "QLabel#subtitleLabel { color:#475569; font-size:14px; }"
        "QPushButton#ghostButton { background:#ffffff; color:#4338ca; border:1px solid #c7d2fe; border-radius:14px; padding:10px 16px; font-size:14px; font-weight:700; }"
        "QPushButton#ghostButton:hover { background:#eef2ff; }"
    );

    connect(closeButton_, &QPushButton::clicked, this, &QWidget::close);

    setCentralWidget(centralWidget);
}
