#include "MainWindow.h"

#include "ShutdownDialog.h"

#include <QColor>
#include <QDesktopServices>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

namespace {
constexpr int kMainWindowWidth = 640;
constexpr int kMainWindowHeight = 420;
constexpr auto kVersionText = "版本 1.0.0";
} // namespace

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setupUi();
    setupConnections();
    setupStatusBar();
}

void MainWindow::setupUi() {
    setWindowTitle("Little Box 工具箱");
    setFixedSize(kMainWindowWidth, kMainWindowHeight);

    auto* centralWidget = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(centralWidget);
    auto* heroCard = new QFrame(centralWidget);
    auto* heroLayout = new QVBoxLayout(heroCard);
    auto* headerLayout = new QHBoxLayout();
    auto* titleLayout = new QVBoxLayout();
    auto* actionLayout = new QHBoxLayout();

    auto* logoBadge = new QLabel("LB", heroCard);
    auto* titleLabel = new QLabel("Little Box 工具箱", heroCard);
    auto* subtitleLabel = new QLabel("统一的桌面轻工具入口", heroCard);
    auto* featureLabel = new QLabel("提供轻量、直接、统一的桌面操作体验。", heroCard);
    auto* highlightLabel = new QLabel("当前提供两项操作：创建关机计划、打开婚礼管理页面。", heroCard);

    shutdownButton_ = new QPushButton("定时关机", heroCard);
    weddingAdminButton_ = new QPushButton("打开网站", heroCard);

    centralWidget->setObjectName("centralWidget");
    heroCard->setObjectName("heroCard");
    logoBadge->setObjectName("logoBadge");
    titleLabel->setObjectName("titleLabel");
    subtitleLabel->setObjectName("subtitleLabel");
    featureLabel->setObjectName("featureLabel");
    highlightLabel->setObjectName("highlightLabel");
    shutdownButton_->setObjectName("primaryButton");
    weddingAdminButton_->setObjectName("secondaryButton");

    logoBadge->setAlignment(Qt::AlignCenter);
    logoBadge->setFixedSize(72, 72);
    subtitleLabel->setWordWrap(true);
    featureLabel->setWordWrap(true);
    highlightLabel->setWordWrap(true);

    shutdownButton_->setCursor(Qt::PointingHandCursor);
    weddingAdminButton_->setCursor(Qt::PointingHandCursor);
    shutdownButton_->setMinimumHeight(48);
    weddingAdminButton_->setMinimumHeight(48);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    titleLayout->setSpacing(4);

    headerLayout->addWidget(logoBadge, 0, Qt::AlignTop);
    headerLayout->addSpacing(14);
    headerLayout->addLayout(titleLayout, 1);

    actionLayout->addWidget(shutdownButton_);
    actionLayout->addWidget(weddingAdminButton_);
    actionLayout->setSpacing(14);

    heroLayout->addLayout(headerLayout);
    heroLayout->addSpacing(20);
    heroLayout->addWidget(featureLabel);
    heroLayout->addWidget(highlightLabel);
    heroLayout->addSpacing(18);
    heroLayout->addLayout(actionLayout);
    heroLayout->setSpacing(0);
    heroLayout->setContentsMargins(28, 28, 28, 28);

    rootLayout->addStretch();
    rootLayout->addWidget(heroCard);
    rootLayout->addStretch();
    rootLayout->setContentsMargins(26, 26, 26, 18);

    auto* shadowEffect = new QGraphicsDropShadowEffect(heroCard);
    shadowEffect->setBlurRadius(36);
    shadowEffect->setOffset(0, 18);
    shadowEffect->setColor(QColor(55, 65, 81, 90));
    heroCard->setGraphicsEffect(shadowEffect);

    centralWidget->setStyleSheet(
        "QWidget#centralWidget { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #eef2ff, stop:1 #f8fafc); }"
        "QFrame#heroCard { background:white; border:1px solid #dbe4ff; border-radius:28px; }"
        "QLabel#logoBadge { background:qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #6366f1, stop:1 #7c3aed); color:white; border-radius:36px; font-size:26px; font-weight:800; }"
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
    );

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections() {
    connect(shutdownButton_, &QPushButton::clicked, this, &MainWindow::openShutdownDialog);
    connect(weddingAdminButton_, &QPushButton::clicked, this, &MainWindow::openWeddingAdminPage);
}

void MainWindow::setupStatusBar() {
    statusBar()->showMessage("状态：就绪");

    auto* versionLabel = new QLabel(QString::fromUtf8(kVersionText), this);
    versionLabel->setStyleSheet("color:#6366f1; font-weight:600; padding-right:8px;");
    statusBar()->addPermanentWidget(versionLabel);
    statusBar()->setStyleSheet(
        "QStatusBar { background:white; color:#475569; border-top:1px solid #e2e8f0; }"
    );
}

void MainWindow::openShutdownDialog() {
    ShutdownDialog shutdownDialog(this);
    shutdownDialog.exec();
}

void MainWindow::openWeddingAdminPage() {
    const bool opened = QDesktopServices::openUrl(QUrl(QStringLiteral("https://blog.shawn.chat/")));
    if (!opened) {
        QMessageBox::warning(this, "打开失败", "无法使用默认浏览器打开网页。请检查系统浏览器配置。");
    }
}
