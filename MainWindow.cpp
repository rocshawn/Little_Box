#include "MainWindow.h"

#include "FlappyBirdWindow.h"
#include "MazeGameWindow.h"
#include "ShutdownDialog.h"

#include <QColor>
#include <QDesktopServices>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kMinimumWindowWidth = 760;
constexpr int kMinimumWindowHeight = 520;
constexpr int kInitialWindowWidth = 980;
constexpr int kInitialWindowHeight = 680;
constexpr auto kVersionText = "版本 1.1.0";
// The website URL is read from the environment variable `LB_WEBSITE_URL` at runtime
// to avoid hardcoding sensitive addresses in source control.
constexpr auto kWebsiteEnvVar = "LB_WEBSITE_URL";

void bringToFront(QWidget* window) {
    if (window == nullptr) {
        return;
    }

    if (window->isMinimized()) {
        window->showNormal();
    } else {
        window->show();
    }

    window->raise();
    window->activateWindow();
}
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
    setupConnections();
    setupStatusBar();
    updateWindowModeUi();
}

void MainWindow::setupUi() {
    setWindowTitle("Little Box 工具箱");
    setMinimumSize(kMinimumWindowWidth, kMinimumWindowHeight);
    resize(kInitialWindowWidth, kInitialWindowHeight);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    auto* heroCard = new QFrame(centralWidget);
    auto* heroLayout = new QVBoxLayout(heroCard);
    auto* headerLayout = new QHBoxLayout();
    auto* titleLayout = new QVBoxLayout();
    auto* featureGrid = new QGridLayout();

    auto* logoBadge = new QLabel("LB", heroCard);
    auto* titleLabel = new QLabel("Little Box 工具箱", heroCard);
    auto* subtitleLabel = new QLabel("统一的桌面轻工具与休闲游戏入口", heroCard);
    auto* featureLabel = new QLabel("支持自由调整窗口大小、全屏切换，以及轻量工具与小游戏功能。", heroCard);
    auto* highlightLabel = new QLabel("当前提供四项功能：定时关机、打开网站、迷宫闯关、Flappy Bird。", heroCard);

    fullscreenButton_ = new QPushButton(heroCard);
    shutdownButton_ = new QPushButton("定时关机", heroCard);
    weddingAdminButton_ = new QPushButton("打开网站", heroCard);
    mazeGameButton_ = new QPushButton("迷宫闯关", heroCard);
    flappyBirdButton_ = new QPushButton("Flappy Bird", heroCard);

    centralWidget->setObjectName("centralWidget");
    heroCard->setObjectName("heroCard");
    logoBadge->setObjectName("logoBadge");
    titleLabel->setObjectName("titleLabel");
    subtitleLabel->setObjectName("subtitleLabel");
    featureLabel->setObjectName("featureLabel");
    highlightLabel->setObjectName("highlightLabel");
    fullscreenButton_->setObjectName("ghostButton");
    shutdownButton_->setObjectName("primaryButton");
    weddingAdminButton_->setObjectName("secondaryButton");
    mazeGameButton_->setObjectName("featureButton");
    flappyBirdButton_->setObjectName("featureButton");

    logoBadge->setAlignment(Qt::AlignCenter);
    logoBadge->setFixedSize(78, 78);
    subtitleLabel->setWordWrap(true);
    featureLabel->setWordWrap(true);
    highlightLabel->setWordWrap(true);

    for (auto* button : { fullscreenButton_, shutdownButton_, weddingAdminButton_, mazeGameButton_, flappyBirdButton_ }) {
        button->setCursor(Qt::PointingHandCursor);
    }

    shutdownButton_->setMinimumHeight(58);
    weddingAdminButton_->setMinimumHeight(58);
    mazeGameButton_->setMinimumHeight(58);
    flappyBirdButton_->setMinimumHeight(58);
    fullscreenButton_->setMinimumHeight(42);
    fullscreenButton_->setMinimumWidth(128);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->setSpacing(4);

    headerLayout->addWidget(logoBadge, 0, Qt::AlignTop);
    headerLayout->addSpacing(16);
    headerLayout->addLayout(titleLayout, 1);
    headerLayout->addWidget(fullscreenButton_, 0, Qt::AlignTop);

    featureGrid->addWidget(shutdownButton_, 0, 0);
    featureGrid->addWidget(weddingAdminButton_, 0, 1);
    featureGrid->addWidget(mazeGameButton_, 1, 0);
    featureGrid->addWidget(flappyBirdButton_, 1, 1);
    featureGrid->setHorizontalSpacing(14);
    featureGrid->setVerticalSpacing(14);

    heroLayout->addLayout(headerLayout);
    heroLayout->addSpacing(20);
    heroLayout->addWidget(featureLabel);
    heroLayout->addSpacing(12);
    heroLayout->addWidget(highlightLabel);
    heroLayout->addSpacing(22);
    heroLayout->addLayout(featureGrid);
    heroLayout->setContentsMargins(30, 30, 30, 30);
    heroLayout->setSpacing(0);

    rootLayout->addStretch();
    rootLayout->addWidget(heroCard);
    rootLayout->addStretch();
    rootLayout->setContentsMargins(28, 28, 28, 18);

    auto* shadowEffect = new QGraphicsDropShadowEffect(heroCard);
    shadowEffect->setBlurRadius(42);
    shadowEffect->setOffset(0, 18);
    shadowEffect->setColor(QColor(55, 65, 81, 90));
    heroCard->setGraphicsEffect(shadowEffect);

    centralWidget->setStyleSheet(
        "QWidget#centralWidget { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #eef2ff, stop:1 #f8fafc); }"
        "QFrame#heroCard { background:white; border:1px solid #dbe4ff; border-radius:28px; }"
        "QLabel#logoBadge { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6366f1, stop:1 #7c3aed); color:white; border-radius:39px; font-size:28px; font-weight:800; }"
        "QLabel#titleLabel { color:#1e1b4b; font-size:30px; font-weight:800; }"
        "QLabel#subtitleLabel { color:#6366f1; font-size:15px; font-weight:600; }"
        "QLabel#featureLabel { color:#334155; font-size:15px; line-height:1.6; }"
        "QLabel#highlightLabel { color:#475569; font-size:14px; padding:14px 16px; background:#f8faff; border:1px solid #e0e7ff; border-radius:16px; }"
        "QPushButton { border-radius:16px; padding:12px 18px; font-size:14px; font-weight:700; }"
        "QPushButton#primaryButton { background:#5b5cf0; color:white; border:none; }"
        "QPushButton#primaryButton:hover { background:#4f46e5; }"
        "QPushButton#primaryButton:pressed { background:#4338ca; }"
        "QPushButton#secondaryButton { background:#eef2ff; color:#312e81; border:1px solid #c7d2fe; }"
        "QPushButton#secondaryButton:hover { background:#e0e7ff; }"
        "QPushButton#secondaryButton:pressed { background:#c7d2fe; }"
        "QPushButton#featureButton { background:white; color:#1f2937; border:1px solid #dbe4ff; }"
        "QPushButton#featureButton:hover { background:#f8faff; border-color:#c7d2fe; }"
        "QPushButton#featureButton:pressed { background:#eef2ff; }"
        "QPushButton#ghostButton { background:#ffffff; color:#4338ca; border:1px solid #c7d2fe; padding:10px 16px; }"
        "QPushButton#ghostButton:hover { background:#eef2ff; }"
    );

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    connect(shutdownButton_, &QPushButton::clicked, this, &MainWindow::openShutdownDialog);
    connect(weddingAdminButton_, &QPushButton::clicked, this, &MainWindow::openWeddingAdminPage);
    connect(mazeGameButton_, &QPushButton::clicked, this, &MainWindow::openMazeGame);
    connect(flappyBirdButton_, &QPushButton::clicked, this, &MainWindow::openFlappyBirdGame);
    connect(fullscreenButton_, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);

    auto* fullscreenShortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fullscreenShortcut, &QShortcut::activated, this, &MainWindow::toggleFullscreen);
}

void MainWindow::setupStatusBar() {
    auto* versionLabel = new QLabel(QString::fromUtf8(kVersionText), this);
    versionLabel->setStyleSheet("color:#6366f1; font-weight:600; padding-right:8px;");

    statusBar()->addPermanentWidget(versionLabel);
    statusBar()->setStyleSheet(
        "QStatusBar { background:white; color:#475569; border-top:1px solid #e2e8f0; }"
    );
}

void MainWindow::updateWindowModeUi() {
    fullscreenButton_->setText(isFullScreen() ? "退出全屏" : "进入全屏");
    statusBar()->showMessage(isFullScreen() ? "状态：全屏模式（按 F11 退出）" : "状态：窗口模式（支持自由缩放，按 F11 全屏）");
}

void MainWindow::openShutdownDialog() {
    ShutdownDialog shutdownDialog(this);
    shutdownDialog.exec();
}

void MainWindow::openWeddingAdminPage() {
    // Read URL from environment variable to avoid embedding it in source code.
    const QByteArray envUrl = qgetenv(kWebsiteEnvVar);
    const QString website = QString::fromUtf8(envUrl);

    if (website.isEmpty()) {
        QMessageBox::warning(this, "打开失败", "未配置网站地址（环境变量 LB_WEBSITE_URL）。");
        return;
    }

    const bool opened = QDesktopServices::openUrl(QUrl(website));

    if (!opened) {
        QMessageBox::warning(this, "打开失败", "无法使用默认浏览器打开网页。请检查系统浏览器配置。");
    }
}

void MainWindow::openMazeGame() {
    if (mazeWindow_.isNull()) {
        mazeWindow_ = new MazeGameWindow(this);
        mazeWindow_->setAttribute(Qt::WA_DeleteOnClose);
    }

    bringToFront(mazeWindow_.data());
}

void MainWindow::openFlappyBirdGame() {
    if (flappyBirdWindow_.isNull()) {
        flappyBirdWindow_ = new FlappyBirdWindow(this);
        flappyBirdWindow_->setAttribute(Qt::WA_DeleteOnClose);
    }

    bringToFront(flappyBirdWindow_.data());
}

void MainWindow::toggleFullscreen() {
    if (isFullScreen()) {
        showNormal();
    } else {
        showFullScreen();
    }

    updateWindowModeUi();
}
